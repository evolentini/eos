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

/** @file interrupciones.c
 ** @brief Implementación de las funciones privadas para la gestion de interrupciones
 **
 **| REV | YYYY.MM.DD | Autor           | Descripción de los cambios                              |
 **|-----|------------|-----------------|---------------------------------------------------------|
 **|   1 | 2021.08.15 | evolentini      | Version inicial del archivo                             |
 **
 ** @addtogroup eos
 ** @brief Sistema operativo
 ** @{ */

/* === Inclusiones de cabeceras ================================================================ */

#include "interrupciones.h"
#include "sapi.h"
#include <stddef.h>
#include <stdint.h>

/* === Definiciones y Macros =================================================================== */

//! Cantidad de handlers de interrupciones requeridos por el procesador
#define HANDLERS_COUNT 52

/* === Declaraciones de tipos de datos internos ================================================ */

//! Estructura para almacenar el handler de una interrupción
typedef struct handler_s {
    //! Punto de entrada a la función que implementa el handler
    eos_entry_point_t entry_point;
    //! Puntero a un bloque de datos que se envia como parametro al handler
    void* data;
} * handler_t;

/* === Declaraciones de funciones internas ===================================================== */
/**
 * @brief Función para obtener una referencia al descriptor del handler de una interrupción
 *
 * @param[in] service       Numero de interupcion para la que se desea obtener el descriptor
 * @return                  Puntero al descriptor del handler de la interrupción solicitada
 */
static handler_t GetHandler(uint8_t service);

/**
 * @brief Función interna que llama al handler correspondiente a una interrupción
 *
 * @param[in] service       Numero de interupcion para la que se debe notificar al handler
 */
void InterruptHandler(uint8_t service);

/* === Definiciones de variables internas ====================================================== */

//! Variable local con los handles registrados para las inteerupciones
static struct handler_s handlers[HANDLERS_COUNT] = { 0 };

//! Cantidad de handler activos atendiendo interrupciones anidadas
static int handler_actives = 0;

/* === Definiciones de variables externas ====================================================== */

/* === Definiciones de funciones internas ====================================================== */

static handler_t GetHandler(uint8_t service)
{
    handler_t result = NULL;
    if (service < HANDLERS_COUNT) {
        result = &handlers[service];
    }
    return result;
}

void InterruptHandler(uint8_t service)
{
    handler_t handler = GetHandler(service);
    if (handler && handler->entry_point) {
        __asm__ volatile("cpsid i");
        handler_actives++;
        __asm__ volatile("cpsie i");

        handler->entry_point(handler->data);

        __asm__ volatile("cpsid i");
        handler_actives--;
        __asm__ volatile("cpsie i");
    }
}

void DAC_IRQHandler(void) { InterruptHandler(DAC_IRQn); }
void M0APP_IRQHandler(void) { InterruptHandler(M0APP_IRQn); }
void DMA_IRQHandler(void) { InterruptHandler(DMA_IRQn); }
void FLASH_EEPROM_IRQHandler(void) { InterruptHandler(RESERVED1_IRQn); }
void ETH_IRQHandler(void) { InterruptHandler(ETHERNET_IRQn); }
void SDIO_IRQHandler(void) { InterruptHandler(SDIO_IRQn); }
void LCD_IRQHandler(void) { InterruptHandler(LCD_IRQn); }
void USB0_IRQHandler(void) { InterruptHandler(USB0_IRQn); }
void USB1_IRQHandler(void) { InterruptHandler(USB1_IRQn); }
void SCT_IRQHandler(void) { InterruptHandler(SCT_IRQn); }
void RIT_IRQHandler(void) { InterruptHandler(RITIMER_IRQn); }
void TIMER0_IRQHandler(void) { InterruptHandler(TIMER0_IRQn); }
void TIMER1_IRQHandler(void) { InterruptHandler(TIMER1_IRQn); }
void TIMER2_IRQHandler(void) { InterruptHandler(TIMER2_IRQn); }
void TIMER3_IRQHandler(void) { InterruptHandler(TIMER3_IRQn); }
void MCPWM_IRQHandler(void) { InterruptHandler(MCPWM_IRQn); }
void ADC0_IRQHandler(void) { InterruptHandler(ADC0_IRQn); }
void I2C0_IRQHandler(void) { InterruptHandler(I2C0_IRQn); }
void SPI_IRQHandler(void) { InterruptHandler(I2C1_IRQn); }
void I2C1_IRQHandler(void) { InterruptHandler(SPI_INT_IRQn); }
void ADC1_IRQHandler(void) { InterruptHandler(ADC1_IRQn); }
void SSP0_IRQHandler(void) { InterruptHandler(SSP0_IRQn); }
void SSP1_IRQHandler(void) { InterruptHandler(SSP1_IRQn); }
void UART0_IRQHandler(void) { InterruptHandler(USART0_IRQn); }
void UART1_IRQHandler(void) { InterruptHandler(UART1_IRQn); }
void UART2_IRQHandler(void) { InterruptHandler(USART2_IRQn); }
void UART3_IRQHandler(void) { InterruptHandler(USART3_IRQn); }
void I2S0_IRQHandler(void) { InterruptHandler(I2S0_IRQn); }
void I2S1_IRQHandler(void) { InterruptHandler(I2S1_IRQn); }
void SPIFI_IRQHandler(void) { InterruptHandler(RESERVED4_IRQn); }
void SGPIO_IRQHandler(void) { InterruptHandler(SGPIO_INT_IRQn); }
void GPIO0_IRQHandler(void) { InterruptHandler(PIN_INT0_IRQn); }
void GPIO1_IRQHandler(void) { InterruptHandler(PIN_INT1_IRQn); }
void GPIO2_IRQHandler(void) { InterruptHandler(PIN_INT2_IRQn); }
void GPIO3_IRQHandler(void) { InterruptHandler(PIN_INT3_IRQn); }
void GPIO4_IRQHandler(void) { InterruptHandler(PIN_INT4_IRQn); }
void GPIO5_IRQHandler(void) { InterruptHandler(PIN_INT5_IRQn); }
void GPIO6_IRQHandler(void) { InterruptHandler(PIN_INT6_IRQn); }
void GPIO7_IRQHandler(void) { InterruptHandler(PIN_INT7_IRQn); }
void GINT0_IRQHandler(void) { InterruptHandler(GINT0_IRQn); }
void GINT1_IRQHandler(void) { InterruptHandler(GINT1_IRQn); }
void EVRT_IRQHandler(void) { InterruptHandler(EVENTROUTER_IRQn); }
void CAN1_IRQHandler(void) { InterruptHandler(C_CAN1_IRQn); }
void ADCHS_IRQHandler(void) { InterruptHandler(ADCHS_IRQn); }
void ATIMER_IRQHandler(void) { InterruptHandler(ATIMER_IRQn); }
void RTC_IRQHandler(void) { InterruptHandler(RTC_IRQn); }
void WDT_IRQHandler(void) { InterruptHandler(WWDT_IRQn); }
void M0SUB_IRQHandler(void) { InterruptHandler(M0SUB_IRQn); }
void CAN0_IRQHandler(void) { InterruptHandler(C_CAN0_IRQn); }
void QEI_IRQHandler(void) { InterruptHandler(QEI_IRQn); }

/* === Definiciones de funciones externas ====================================================== */

bool HandlerActive(void)
{
    // Retorna verdadero si hay por una o mas interrupciones en curso
    return handler_actives > 0;
}

void HandlerInstall(uint8_t service, uint8_t prioridad, eos_entry_point_t entry_point, void* data)
{
    handler_t handler = GetHandler(service);

    prioridad = 1 + (prioridad & 0x03);
    if (handler) {
        handler->entry_point = entry_point;
        handler->data = data;
        NVIC_SetPriority(service, NVIC_EncodePriority(7, prioridad, 0));
        NVIC_ClearPendingIRQ(service);
        NVIC_EnableIRQ(service);
    }
}

void HandlerRemove(uint8_t service)
{
    handler_t handler = GetHandler(service);
    if (handler) {
        NVIC_DisableIRQ(service);
        handler->entry_point = NULL;
        handler->data = NULL;
    }
}

/* === Ciere de documentacion ================================================================== */

/** @} Final de la definición del modulo para doxygen */
