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
 **|  16 | 2021.08.16 | evolentini      | Se mueven las notificaciones al usuario a la API        |
 **|  15 | 2021.08.16 | evolentini      | Se incluye una funcion para ceder el procesador         |
 **|  14 | 2021.08.15 | evolentini      | Compatibilidad con los handlers de interrupciones       |
 **|  13 | 2021.08.09 | evolentini      | Se publican funciones necesarias implementar semaforos  |
 **|  12 | 2021.08.10 | evolentini      | Soporte para encolar las tareas con una lista enlazada  |
 **|  11 | 2021.08.09 | evolentini      | Se separan las funciones publicas y privadas del SO     |
 **|  10 | 2021.08.08 | evolentini      | Se agregan notificaciones del sistema al usuario        |
 **|   9 | 2021.08.08 | evolentini      | Se agrega soporte para una tarea inactiva del sistema   |
 **|   8 | 2021.08.08 | evolentini      | Se agrega soporte para prioridades en las tareas        |
 **|   7 | 2021.08.08 | evolentini      | Se separa el planificador en un archivo independiente   |
 **|   6 | 2021.08.07 | evolentini      | Se agrega un servicio de espera pasiva                  |
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
#include "planificador.h"
#include "semaforos.h"
#include "sapi.h"
#include <stddef.h>
#include <stdint.h>

/* === Definiciones y Macros =================================================================== */

/* === Declaraciones de tipos de datos internos ================================================ */

/**
 * @brief Estructura que almacena el descriptor de una tarea
 */
typedef struct eos_task_s {
    //! Estado actual de la tarea
    eos_task_state_t state;
    //! Copia del puntero de pila de la tarea
    void* stack_pointer;
    //! Cantidad de ticks para terminar la espera
    uint32_t wait_ticks;
    //! Prioridad actual de la tarea
    uint8_t priority;
    //! Puntero a la siguiente tarea en la cola
    eos_task_t next_task;
} * eos_task_t;

/**
 * @brief Estructura de datos que almacena el estado del nuclo
 */
typedef struct kernel_s {
    //! Vector que almacena el descriptor de la tarea inactiva
    struct eos_task_s background[1];
    //! Vector que almacena los descriptores de tareas
    struct eos_task_s tasks[EOS_MAX_TASK_COUNT][1];
    //! Vector que proporciona espacio para la pila de las tareas
    uint8_t eos_task_stacks[EOS_MAX_TASK_COUNT + 1][EOS_TASK_STACK_SIZE];
    //! Puntero al descriptor de la tarea en ejecución
    eos_task_t active_task;
    //! Variable con el indice de la ultima tarea creada
    uint8_t last_created;
    //! Puntero a la instancia del planificador
    scheduler_t scheduler;
    // Variable con la ultima dirección de pila asignada
    void* asigned_stack;
} * kernel_t;

/**
 * @brief Estructura con los registros del contexto de la tarea almacenados en la pila
 */
typedef struct eos_task_context_s {
    //! Contexto almacenado manualmente en el cambio de contexto
    struct eos_task_context_manual_s {
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
    struct eos_task_context_auto_s {
        uint32_t r0;
        uint32_t r1;
        uint32_t r2;
        uint32_t r3;
        uint32_t ip;
        uint32_t lr;
        uint32_t pc;
        uint32_t xPSR;
    } context_auto;
} * eos_task_context_t;

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
static eos_task_t AllocateDescriptor(void);

/**
 * @brief Función para preparar el contexto inicial de una tarea nueva
 *
 * @param[in]  task         Puntero al desciptor de la nueva tarea
 * @param[in]  entry_point  Punto de entrada de la función que imeplementa la tarea
 * @param[in]  data         Puntero al bloque de datos para parametrizar la tarea
 */
void PrepareContext(eos_task_t task, eos_entry_point_t entry_point, void* data);

/**
 * @brief Función para recuperar el contexto de una tarea y cederle el procesador
 *
 * @param[in]  stack_pinter Contenido del puntero de pila de la tarea a restaurar
 */
__attribute__((naked())) void RetoreContext(void* stack_pointer);

/**
 * @brief Función para implementar el control del tiempos del sistema operativo
 */
void TickEvent(void);

/**
 * @brief Función para asignar la pila a una tarea
 *
 * @param   task    Puntero al descriptor de la tarea a la que se asigna la pila
 * @param   size    Canitdad de bytes que se desean asignar como pila a la tarea
 */
void TaskAsignStack(eos_task_t task, uint16_t size);

/**
 * @brief  Función para implementar la tarea inactiva del sistema
 *
 * @param   data    Puntero con los parametros, siempre es NULL para esta tarea
 */
void TaskBackground(void* data);

/* === Definiciones de variables internas ====================================================== */

/**
 * @brief Variable con el estado del nucleo
 */
static struct kernel_s kernel[1] = { 0 };

/* === Definiciones de variables externas ====================================================== */

/* === Definiciones de funciones internas ====================================================== */

void TaskError(void)
{
    EosEndTaskCallback(kernel->active_task);
    TaskSetState(kernel->active_task, CREATING);
}

static eos_task_t AllocateDescriptor(void)
{
    // Variable con el resultado del descriptor de tarea asignado
    eos_task_t task = NULL;

    if (kernel->last_created < EOS_MAX_TASK_COUNT) {
        task = kernel->tasks[kernel->last_created];
        (kernel->last_created)++;
    }
    return task;
}

void PrepareContext(eos_task_t task, eos_entry_point_t entry_point, void* data)
{
    task->stack_pointer -= sizeof(struct eos_task_context_s);

    eos_task_context_t context = task->stack_pointer;
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
    __asm__ volatile("tst lr,0x10");
    __asm__ volatile("it eq");
    __asm__ volatile("vldmeq r0, {s16-s31}");
    __asm__ volatile("msr psp, r0");
    __asm__ volatile("isb");

    /* Se configura el modo ejecución de la tarea como no privilegiado */
    __asm__ volatile("mrs r0, control");
    __asm__ volatile("orr r0, #1");
    __asm__ volatile("msr control, r0");
    __asm__ volatile("isb");

    __asm__ volatile("cpsie i");
    __asm__ volatile("bx lr");
}

void TickEvent(void)
{
    for (int index = 0; index < EOS_MAX_TASK_COUNT; index++) {
        eos_task_t task = kernel->tasks[index];
        if (task->state == WAITING) {
            task->wait_ticks--;
            if (task->wait_ticks == 0) {
                TaskSetState(task, READY);
                SchedulingRequired();
            }
        }
    }
    EosSysTickCallback();
}

void TaskAsignStack(eos_task_t task, uint16_t size)
{
    // Se inicializa el puntero la primera vez que se asigna una pila
    if (kernel->asigned_stack == NULL) {
        kernel->asigned_stack = kernel->eos_task_stacks;
    }

    // Se asigna la cantidad de memoria solicitada a la pila de la tarea
    if (task) {
        kernel->asigned_stack += size;
        task->stack_pointer = kernel->asigned_stack;
    }
}

void TaskBackground(void* data)
{
    /* El puntero a datos no se utiliza en esta tarea */
    (void)data;

    while (1) {
        EosInactiveCallback();
    }
}

/* === Definiciones de funciones externas ====================================================== */

eos_task_t TaskCreate(eos_entry_point_t entry_point, void* data, uint8_t priority)
{

    // Variable con el descriptor signado a la nueva tarea
    eos_task_t task = AllocateDescriptor();
    if (task) {
        TaskAsignStack(task, EOS_TASK_STACK_SIZE);
        task->priority = priority;
        PrepareContext(task, entry_point, data);
        TaskSetState(task, READY);
    }
    return task;
}

void TaskSetState(eos_task_t task, eos_task_state_t state)
{
    if (task->state != state) {
        if (task == kernel->background) {
            if (state == READY || state == RUNNING) {
                task->state = state;
            }
        } else {
            task->state = state;
            if (task->state == READY && kernel->scheduler) {
                SchedulerEnqueue(kernel->scheduler, task, task->priority);
            }
        }
    }
}

eos_task_t TaskGetDescriptor(void)
{
    // Devuelve el puntero a la tarea actual
    return kernel->active_task;
}

void TaskEnqueue(eos_task_t first_task, eos_task_t last_task)
{
    eos_task_t task = first_task;
    while (task->next_task != NULL) {
        task = task->next_task;
    }
    task->next_task = last_task;
}

eos_task_t TaskDequeue(eos_task_t first_task)
{
    eos_task_t task = first_task->next_task;
    first_task->next_task = NULL;
    return task;
}

void StartScheduler(void)
{
    __asm__ volatile("cpsid i");

    /* Activate SysTick */
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 5000);

    /* Update priority set by SysTick_Config */
    NVIC_SetPriority(SVCall_IRQn, NVIC_EncodePriority(7, 0, 0));
    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(7, 5, 0));
    NVIC_SetPriority(PendSV_IRQn, NVIC_EncodePriority(7, 6, 0));

    /* Creación de la tarea inactiva del sistema */
    TaskAsignStack(kernel->background, EOS_TASK_STACK_SIZE);
    PrepareContext(kernel->background, TaskBackground, NULL);
    TaskSetState(kernel->background, READY);

    /* Creación del planificador y encolado de las tareas creadas */
    kernel->scheduler = SchedulerCreate(kernel->background);
    for (int index = 0; index < EOS_MAX_TASK_COUNT; index++) {
        eos_task_t task = kernel->tasks[index];
        if (task->state == READY) {
            SchedulerEnqueue(kernel->scheduler, task, task->priority);
        }
    }

    SchedulingRequired();
    __asm__ volatile("cpsie i");

    /* Espera de la primera interupción para arrancar el sistema */
    while (1) {
        __asm__ volatile("wfi");
    }
}

void SchedulingRequired(void)
{
    if (kernel->scheduler) {
        // Fija la bandera de pedido de la excepcion PendSV
        SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    }
}

void SysTick_Handler(void)
{
    // Se llama a la funcion del sistema operativo para gestionar los tiempos
    TickEvent();
}

void SVC_Handler(void)
{
    struct eos_task_context_auto_s* contexto;
    __asm__ volatile("mrs r0, psp");
    __asm__ volatile("str r0, %0" : "=m"(contexto));

    uint32_t service = contexto->r0;
    uint32_t data = contexto->r1;
    uint32_t resultado = 0;

    switch (service) {
    case EOS_SERVICE_DELAY:
        kernel->active_task->state = WAITING;
        kernel->active_task->wait_ticks = data;
        break;
    case EOS_SERVICE_YIELD:
        // No es necesario agregar ninguna acción adicional porque
        // al terminar el switch se pide la ejecución del planificador
        break;
    case EOS_SERVICE_GIVE:
        SemaphoreGive((eos_semaphore_t)data);
        break;
    case EOS_SERVICE_TAKE:
        resultado = SemaphoreTake((eos_semaphore_t)data);
        break;
    default:
        break;
    }
    contexto->r0 = resultado;

    SchedulingRequired();
}

__attribute__((naked())) void PendSV_Handler(void)
{
    /* Si hay una tarea activa se salva el contexto en su correspondiente pila */
    if ((kernel->active_task) && (kernel->active_task->state != CREATING)) {
        __asm__ volatile("cpsid i");
        __asm__ volatile("mrs r0, psp");
        __asm__ volatile("tst lr,0x10");
        __asm__ volatile("it eq");
        __asm__ volatile("vstmeq r0, {s16-s31}");
        __asm__ volatile("stmdb r0!, {r4-r11,lr}");
        __asm__ volatile("str r0, %0" : "=m"(kernel->active_task->stack_pointer));

        if (kernel->active_task->state == RUNNING) {
            TaskSetState(kernel->active_task, READY);
        }
    }

    /* Se determina seleciona la proxima tarea que utilizará el procesador */
    kernel->active_task = Schedule(kernel->scheduler);
    TaskSetState(kernel->active_task, RUNNING);

    /*  Se devuelve el uso del procesador a la tarea designada */
    RetoreContext(kernel->active_task->stack_pointer);
}

/* === Ciere de documentacion ================================================================== */

/** @} Final de la definición del modulo para doxygen */
