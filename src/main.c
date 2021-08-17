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

/** @file main.c
 ** @brief Excamen final de la asignatura Implementación de Sistemas Operativos
 **
 **| REV | YYYY.MM.DD | Autor           | Descripción de los cambios                              |
 **|-----|------------|-----------------|---------------------------------------------------------|
 **|  16 | 2021.08.16 | evolentini      | Se cambia el programa para resolver el examen propuesto |
 **|  15 | 2021.08.15 | evolentini      | Se agrega un ejemplo de uso los handler de interrupcion |
 **|  14 | 2021.08.14 | evolentini      | Se agrega un ejemplo de uso de las colas de datos       |
 **|  14 | 2021.08.09 | evolentini      | Se agrega un ejemplo de uso de un semaforo              |
 **|  13 | 2021.08.09 | evolentini      | Se utilizan solo las funciones publicas del SO          |
 **|  12 | 2021.08.08 | evolentini      | Se cambia para utilizar las notificaciones del sistema  |
 **|  11 | 2021.08.08 | evolentini      | Se cambia para crear tareas con diferentes prioridades  |
 **|  10 | 2021.08.08 | evolentini      | Se cambia para crear una tarea desde una tarea          |
 **|   9 | 2021.08.08 | evolentini      | Se cambia para usar espera pasiva del sistema operativo |
 **|   8 | 2021.07.25 | evolentini      | Se cambia para usar las tareas parametrizadas           |
 **|   7 | 2021.07.25 | evolentini      | Las funciones de tareas se mueven a un archivo separado |
 **|   6 | 2021.07.25 | evolentini      | Se agregan descriptores y estados para las tareas       |
 **|   5 | 2021.07.25 | evolentini      | Punteros de pila separados y modo no privilegiado       |
 **|   4 | 2021.07.09 | evolentini      | Migracion del ejemplo a firmware CIAA V2                |
 **|   3 | 2017.10.16 | evolentini      | Correción en el formato del archivo                     |
 **|   2 | 2017.09.21 | evolentini      | Cambio para utilizar drivers_bm de UNER                 |
 **|   1 | 2016.06.25 | evolentini      | Version inicial del archivo                             |
 **
 ** @addtogroup eos
 ** @brief Sistema operativo
 ** @{ */

/* === Inclusiones de cabeceras ================================================================ */

#include "eos.h"
#include "sapi.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* === Definiciones y Macros =================================================================== */

//! Define la cantidad máxima de elementos en la cola de eventos de teclas
#define TECLAS_CANTIDAD 4

//! Define el tamaño de las entradas en la cola de eventos de teclas
#define TECLAS_TAMANO sizeof(struct tecla_evento_s)

//! Define la cantidad máxima de elementos en la cola de eventos de teclas
#define COLORES_CANTIDAD 4

//! Define el tamaño de las entradas en la cola de eventos de teclas
#define COLORES_TAMANO sizeof(struct color_evento_s)

//! Define la cantidad máxima de elementos en la cola de eventos de mensajes por consola
#define MENSAJES_CANTIDAD 4

/* === Declaraciones de tipos de datos internos ================================================ */

//! Tipo de datos enumerado con las acciones sobre la teclas
typedef enum {
    TECLA_B1_PRESIONADA = 0,
    TECLA_B1_LIBERADA = 1,
    TECLA_B2_PRESIONADA = 2,
    TECLA_B2_LIBERADA = 3,
} tecla_accion_t;

//! Tipo de datos enumerado con los estados de la MEF que procesa las acciones en las teclas
typedef enum {
    ESTADO_REPOSO = 0,
    ESTADO_UNA_TECLA = 1,
    ESTADO_DOS_TECLAS = 2,
    ESTADO_TERMINANDO = 3,
} estados_t;

//! Tipo de datos enumerado con los estados de la MEF que procesa las acciones en las teclas
typedef enum {
    COLOR_VERDE = 0,
    COLOR_ROJO = 1,
    COLOR_AMARILLO = 2,
    COLOR_AZUL = 3,
} colores_t;

//! Estructura de datos con evento de una tecla y el valor de reloj correspondiente
typedef struct tecla_evento_s {
    //! Tipo de accion sobre la tecla
    tecla_accion_t accion;
    //! Valor del contador de reloj al relizarse la accion sobre la tecla
    uint32_t reloj;
} * tecla_evento_t;

//! Estructura de datos con un color y los tiempos medidos
typedef struct color_evento_s {
    //! Diferencia de tiempo entre flancos descendentes
    uint32_t t1;
    //! Diferencia de tiempo entre flancos ascendentes
    uint32_t t2;
    //! Color correspondiente segun la secuencia de flancos
    colores_t color;
} * color_evento_t;

//! Estructura de datos con las dos colas de datos para la tarea de procesamiento
typedef struct colas_s {
    //! Cola por la que se reciben los eventos de teclas de los handlers
    eos_queue_t cola_teclas;
    //! Cola por la se se envian los eventos de colores a la tarea de presentación
    eos_queue_t cola_colores;
    //! Cola por la se se envian los eventos de colores a la tarea de consola
    eos_queue_t cola_mensajes;
} * colas_t;

/* === Declaraciones de funciones internas ===================================================== */

/**
 * @brief Funcion para configurar una interupción de puerto GPIO
 *
 * @param[in] canal     Numero de canal de interupción GPIO (0 a 7)
 * @param[in] puerto    Numero de puerto GPIO al que pertenece el terimnal (0 a 7)
 * @param[in] terminal  Numero de terminal GPIO que produce interupción (0 a 31)
 */
void ConfigurarInterrupcion(uint8_t canal, uint8_t puerto, uint8_t terminal);

/**
 * @brief Función que incrementa el contador global en un hook del sistick
 */
void EosSysTickCallback(void);

/**
 * @brief Handler para atender la interupcion de teclado
 *
 * @param data Refecencia a la cola por la se que deben enviar los eventos
 */
void EventoTecla(void* data);

/**
 * @brief Tarea para procesar los eventos de teclas y generar eventos de colores
 *
 * @param data Puntero a una estructura con las colas de teclas y de colores
 */
void Procesamiento(void* data);

/**
 * @brief Tarea que prende los leds del color correspondiente por el tiempo medido
 *
 * @param data Puntero la cola por la que se reciben los eventos
 */
void Visualizacion(void* data);

/**
 * @brief Tarea informa por consola el evento de color generado
 *
 * @param data Puntero la cola por la que se reciben los eventos
 */
void Consola(void* data);

/* === Definiciones de variables internas ====================================================== */

//! Variable global con la cuanta de reloj
static uint32_t tick_count = 0;

//! Variable global para el almacenamiento de la cola de teclas
static struct tecla_evento_s vector_teclas[TECLAS_CANTIDAD] = { 0 };

//! Variable global para el almacenamiento de la cola de colores
static struct color_evento_s vector_colores[COLORES_CANTIDAD] = { 0 };

//! Variable global para el almacenamiento de la cola de mensajes
static struct color_evento_s vector_mensajes[MENSAJES_CANTIDAD] = { 0 };

/* === Definiciones de variables externas ====================================================== */

/* === Definiciones de funciones internas ====================================================== */

void ConfigurarInterrupcion(uint8_t canal, uint8_t puerto, uint8_t terminal)
{
    uint32_t mascara = 1 << canal;
    Chip_SCU_GPIOIntPinSel(canal, puerto, terminal);
    Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, mascara);
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, mascara);
    Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, mascara);
    Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, mascara);
}

void EosSysTickCallback(void) { tick_count++; }

void EventoTecla(void* data)
{
    eos_queue_t cola = data;
    struct tecla_evento_s evento[1];
    static uint32_t ultimo_evento[2] = { 0 };

    for (int tecla = 0; tecla < 2; tecla++) {
        bool presionada = (Chip_PININT_GetFallStates(LPC_GPIO_PIN_INT) & (1 << tecla));
        bool liberada = (Chip_PININT_GetRiseStates(LPC_GPIO_PIN_INT) & (1 << tecla));
        Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, 1 << tecla);

        if (presionada && (tick_count - ultimo_evento[tecla] > 100)) {
            ultimo_evento[tecla] = tick_count;
            evento->accion = (tecla & 0x01) << 1;
            evento->reloj = tick_count;
            EosQueueGive(cola, evento);
        }
        if (liberada && (tick_count - ultimo_evento[tecla] > 100)) {
            ultimo_evento[tecla] = tick_count;
            evento->accion = ((tecla & 0x01) << 1) + 1;
            evento->reloj = tick_count;
            EosQueueGive(cola, evento);
        }
    }
}

void Procesamiento(void* data)
{
    colas_t colas = data;
    estados_t estado = ESTADO_REPOSO;
    struct tecla_evento_s eventos[4];
    struct color_evento_s resultado[1];

    while (true) {
        EosQueueTake(colas->cola_teclas, &eventos[estado]);

        switch (estado) {
        case ESTADO_REPOSO:
            // Si el evento es una tecla presionada
            if ((eventos[estado].accion & 0x01) == 0) {
                estado = ESTADO_UNA_TECLA;
            }
            break;
        case ESTADO_UNA_TECLA:
            // Si el evento es una tecla presionada
            if ((eventos[estado].accion & 0x01) == 0) {
                estado = ESTADO_DOS_TECLAS;
            } else {
                estado = ESTADO_REPOSO;
            }
            break;
        case ESTADO_DOS_TECLAS:
            // Si el evento es una tecla liberada
            if ((eventos[estado].accion & 0x01) == 1) {
                estado = ESTADO_TERMINANDO;
            } else {
                estado = ESTADO_REPOSO;
            }
            break;
        default:
            // Si el evento es una tecla liberada
            if ((eventos[estado].accion & 0x01) == 1) {
                resultado->t1 = eventos[1].reloj - eventos[0].reloj;
                resultado->t2 = eventos[3].reloj - eventos[2].reloj;

                if (eventos[0].accion == TECLA_B1_PRESIONADA) {
                    if (eventos[2].accion == TECLA_B1_LIBERADA) {
                        resultado->color = COLOR_VERDE;
                    } else {
                        resultado->color = COLOR_ROJO;
                    }
                } else {
                    if (eventos[2].accion == TECLA_B1_LIBERADA) {
                        resultado->color = COLOR_AMARILLO;
                    } else {
                        resultado->color = COLOR_AZUL;
                    }
                }
                EosQueueGive(colas->cola_colores, resultado);
                EosQueueGive(colas->cola_mensajes, resultado);
            }
            estado = ESTADO_REPOSO;
            break;
        }
    }
}

void Visualizacion(void* data)
{
    eos_queue_t cola = data;
    struct color_evento_s evento[1];

    while (1) {
        EosQueueTake(cola, evento);
        switch (evento->color) {
        case COLOR_VERDE:
            gpioWrite(LED3, true);
            break;
        case COLOR_ROJO:
            gpioWrite(LED1, true);
            break;
        case COLOR_AMARILLO:
            gpioWrite(LED2, true);
            break;
        default:
            gpioWrite(LEDB, true);
            break;
        }
        EosWaitDelay(evento->t1 + evento->t2);
        switch (evento->color) {
        case COLOR_VERDE:
            gpioWrite(LED3, false);
            break;
        case COLOR_ROJO:
            gpioWrite(LED1, false);
            break;
        case COLOR_AMARILLO:
            gpioWrite(LED2, false);
            break;
        default:
            gpioWrite(LEDB, false);
            break;
        }
    }
}

void Consola(void* data)
{
    static const char* COLOR[] = { "Verde", "Rojo", "Amarillo", "Azul" };

    eos_queue_t cola = data;
    struct color_evento_s evento[1];
    static char mensaje[8];

    uartConfig(UART_USB, 115200);
    while (1) {
        EosQueueTake(cola, evento);
        uartWriteString(UART_USB, "Led ");
        uartWriteString(UART_USB, COLOR[evento->color]);
        uartWriteString(UART_USB, " encendido:\r\n");

        uartWriteString(UART_USB, "\t Tiempo encendido: ");
        itoa(evento->t1 + evento->t2, mensaje, 10);
        uartWriteString(UART_USB, mensaje);
        uartWriteString(UART_USB, " ms \r\n");

        uartWriteString(UART_USB, "\t Tiempo entre flancos descendentes: ");
        itoa(evento->t1, mensaje, 10);
        uartWriteString(UART_USB, mensaje);
        uartWriteString(UART_USB, " ms \r\n");

        uartWriteString(UART_USB, "\t Tiempo entre flancos ascendentes: ");
        itoa(evento->t2, mensaje, 10);
        uartWriteString(UART_USB, mensaje);
        uartWriteString(UART_USB, " ms \r\n\r\n");
    }
}

/* === Definiciones de funciones externas ====================================================== */

int main(void)
{
    // Variable con las colas que se envian a la tarea de procesamiento
    static struct colas_s colas[1];

    // Configuración de los dispositivos de la placa
    boardConfig();

    // Variable para almacenar la referencia al descriptor de la cola de datos
    colas->cola_teclas = EosQueueCreate(vector_teclas, TECLAS_CANTIDAD, TECLAS_TAMANO);
    colas->cola_colores = EosQueueCreate(vector_colores, COLORES_CANTIDAD, COLORES_TAMANO);
    colas->cola_mensajes = EosQueueCreate(vector_mensajes, MENSAJES_CANTIDAD, COLORES_TAMANO);

    // Configuraicón de las interrupciones de teclado
    ConfigurarInterrupcion(0, 0, 4);
    EosHandlerInstall(PIN_INT0_IRQn, 0, EventoTecla, colas->cola_teclas);

    ConfigurarInterrupcion(1, 0, 8);
    EosHandlerInstall(PIN_INT1_IRQn, 0, EventoTecla, colas->cola_teclas);

    // Creación de la tarea que procesa los eventos de las teclas
    EosTaskCreate(Procesamiento, colas, 1);
    EosTaskCreate(Visualizacion, colas->cola_colores, 2);
    EosTaskCreate(Consola, colas->cola_mensajes, 3);

    /* Arranque del sistemaoperativo */
    EosStartScheduler();

    return 0;
}

/* === Ciere de documentacion ================================================================== */

/** @} Final de la definición del modulo para doxygen */
