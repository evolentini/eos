/* Copyright 2016-2021, Laboratorio de Microprocesadores
 * Facultad de Ciencias Exactas y Tecnología
 * Universidad Nacional de Tucuman
 * http://www.microprocesadores.unt.edu.ar/
 * Copyright 2016-2021, Esteban Volentini <evolentini@herrera.unt.edu.ar>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/** @file tareas.c
 ** @brief Implementación de las funciones para la gestion de tareas
 **
 **| REV | YYYY.MM.DD | Autor           | Descripción de los cambios                              |
 **|-----|------------|-----------------|---------------------------------------------------------|
 **|   5 | 2021.07.02 | evolentini      | Se integra todo el estado en la estructura kernel       |
 **|   4 | 2021.07.02 | evolentini      | Se separa el codigo dependiente del procesador          |
 **|   3 | 2021.07.25 | evolentini      | Se agrega un puntero que permite parametrizar la tarea  |
 **|   2 | 2021.07.25 | evolentini      | Se mueve el cambio de contexto a la rutina PendSV       |
 **|   1 | 2021.07.25 | evolentini      | Version inicial del archivo                             |
 **
 ** @addtogroup eos
 ** @brief Sistema operativo
 ** @{ */

/* === Inclusiones de cabeceras ================================================================ */

#include "tareas.h"
#include "sapi.h"
#include <stddef.h>
#include <stdint.h>

/* === Definiciones y Macros =================================================================== */

/**
 * @brief Cantidad de máxima de tareas que se podrán crear
 */
#define TASKS_MAX_COUNT 8

/**
 * @brief Cantidad de bytes asignado como pila para cada tarea
 */
#define TASK_STACK_SIZE 256

/* === Declaraciones de tipos de datos internos ================================================ */

/**
 * @brief Tipo de datos enumerado con los estados de las tareas
 */
typedef enum task_state_e {
    CREATING = 0,
    READY,
    RUNNING,
} task_state_t;

/**
 * @brief Estructura que almacena el descriptor de una tarea
 */
typedef struct task_s {
    //! Estado actual de la tarea
    task_state_t state;
    //! Copia del puntero de pila de la tarea
    void* stack_pointer;
} * task_t;

/**
 * @brief Estructura de datos que almacena el estado del nuclo
 */
typedef struct kernel_s {
    //! Vector que almacena los descriptores de tareas
    struct task_s tasks[TASKS_MAX_COUNT];
    //! Vector que proporciona espacio para la pila de las tareas
    uint8_t task_stacks[TASKS_MAX_COUNT][TASK_STACK_SIZE];
    //! Variable con el indice de la ultima tarea creada
    uint8_t last_created;
    //! Puntero al descriptor de la tarea en ejecución
    uint8_t active_task;
} * kernel_t;

/**
 * @brief Estructura con los registros del contexto de la tarea almacenados en la pila
 */
typedef struct task_context_s {
    //! Contexto almacenado manualmente en el cambio de contexto
    struct task_context_manual_s {
        uint32_t r4;
        uint32_t r5;
        uint32_t r6;
        uint32_t r7;
        uint32_t r8;
        uint32_t r9;
        uint32_t r10;
        uint32_t r11;
        uint32_t lr;
    } context_manual;
    //! Contexto almacenado automaticamente por la excepción
    struct task_context_auto_s {
        uint32_t r0;
        uint32_t r1;
        uint32_t r2;
        uint32_t r3;
        uint32_t ip;
        uint32_t lr;
        uint32_t pc;
        uint32_t xPSR;
    } context_auto;
} * task_context_t;

/* === Declaraciones de funciones internas ===================================================== */

/**
 * @brief Función que indica un error en el cambio de contexto
 *
 * @remark Esta funcion no debería ejecutarse nunca, solo se accede a la misma si las funciones
 * que implementan las tareas terminan
 */
void TaskError(void);

/**
 * @brief Busca y asigna un desciptor para una nueva tarea
 *
 */
task_t AllocateDescriptor(void);

/**
 * @brief Función para preparar el contexto inicial de una tarea nueva
 *
 * @param[in]  task         Puntero al desciptor de la nueva tarea
 * @param[in]  entry_point  Punto de entrada de la función que imeplementa la tarea
 * @param[in]  data         Puntero al bloque de datos para parametrizar la tarea
 */
void PrepareContext(task_t task, task_entry_point_t entry_point, void* data);

/**
 * @brief Función para recuperar el contexto de una tarea y cederle el procesador
 *
 * @param[in]  stack_pinter Contenido del puntero de pila de la tarea a restaurar
 */
__attribute__((naked())) void RetoreContext(void* stack_pointer);

/**
 * @brief Función para programar una llamada al planificado al terminar la interrupcion en curso
 */
void SchedulingRequired(void);

/**
 * @brief Función para determinar la tarea a la que se otorga el procesador
 */
void Schedule(void);

/**
 * @brief Función para implementar el control del tiempos del sistema operativo
 */
void TickEvent(void);

/* === Definiciones de variables internas ====================================================== */

/**
 * @brief Variable con el estado del nucleo
 */
static struct kernel_s kernel[1] = { 0 };

/* === Definiciones de variables externas ====================================================== */

/* === Definiciones de funciones internas ====================================================== */

void TaskError(void)
{
    gpioWrite(LEDR, true);
    while (1) { }
}

task_t AllocateDescriptor(void)
{
    // Variable con el resultado del descriptor de tarea asignado
    task_t task = NULL;

    if (kernel->last_created < TASKS_MAX_COUNT) {
        task = &(kernel->tasks[kernel->last_created]);
        (kernel->last_created)++;
    }
    return task;
}

void PrepareContext(task_t task, task_entry_point_t entry_point, void* data)
{
    task->stack_pointer -= sizeof(struct task_context_s);

    task_context_t context = task->stack_pointer;
    context->context_auto.lr = (uint32_t)TaskError;
    context->context_auto.xPSR = 0x21000000;
    context->context_auto.pc = (uint32_t)entry_point;
    context->context_auto.r0 = (uint32_t)data;
    context->context_manual.lr = 0xFFFFFFFD;
}

__attribute__((naked())) void RetoreContext(void* stack_pointer)
{
    /* Se recupera el contexto de la tarea a ejecutar desde su correspondiente pila */
    __asm__ volatile("ldmia r0!, {r4-r11,lr}");
    __asm__ volatile("msr psp, r0");
    __asm__ volatile("isb");

    /* Se configura el modo ejecución de la tarea como no privilegiado */
    __asm__ volatile("mrs r0, control");
    __asm__ volatile("orr r0, #1");
    __asm__ volatile("msr control, r0");
    __asm__ volatile("isb");

    __asm__ volatile("bx lr");
}

void SchedulingRequired(void)
{
    // Fija la bandera de pedido de la excepcion PendSV
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}

void Schedule(void)
{
    do {
        kernel->active_task = (kernel->active_task + 1) % TASKS_MAX_COUNT;
    } while (kernel->tasks[kernel->active_task].state != READY);
}

void TickEvent(void)
{
    static int divisor = 0;
    divisor = (divisor + 1) % 1000;
    if (divisor == 0)
        gpioToggle(LEDB);
    SchedulingRequired();
}

/* === Definiciones de funciones externas ====================================================== */

void TaskCreate(task_entry_point_t entry_point, void* data)
{
    // Variable con la ultima dirección de pila asignada
    static void* asigned_stack = kernel->task_stacks;

    // Variable con el descriptor signado a la nueva tarea
    task_t task = AllocateDescriptor();
    if (task) {
        asigned_stack += TASK_STACK_SIZE;
        task->stack_pointer = asigned_stack;
        PrepareContext(task, entry_point, data);
        task->state = READY;
    }
}

void StartScheduler(void)
{
    __asm__ volatile("cpsid i");

    /* Activate SysTick */
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 5000);

    /* Update priority set by SysTick_Config */
    NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 2);
    NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS) - 1);

    __asm__ volatile("cpsie i");

    /* Espera de la primera interupción para arrancar el sistema */
    while (1) {
        __asm__ volatile("wfi");
    }
}

void SysTick_Handler(void)
{
    // Se llama a la funcion del sistema operativo para gestionar los tiempos
    TickEvent();
}

__attribute__((naked())) void PendSV_Handler(void)
{
    /* Si hay una tarea activa se salva el contexto en su correspondiente pila */
    if (kernel->tasks[kernel->active_task].state == RUNNING) {
        kernel->tasks[kernel->active_task].state = READY;
        __asm__ volatile("mrs r0, psp");
        __asm__ volatile("stmdb r0!, {r4-r11,lr}");
        __asm__ volatile("str r0, %0" : "=m"(kernel->tasks[kernel->active_task].stack_pointer));
    }

    /* Se determina seleciona la proxima tarea que utilizará el procesador */
    Schedule();
    kernel->tasks[kernel->active_task].state = RUNNING;

    /*  Se devuelve el uso del procesador a la tarea designada */
    RetoreContext(kernel->tasks[kernel->active_task].stack_pointer);
}

/* === Ciere de documentacion ================================================================== */

/** @} Final de la definición del modulo para doxygen */