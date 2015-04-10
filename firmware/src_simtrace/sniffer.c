/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2009, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/*------------------------------------------------------------------------------
 *          Headers
 *------------------------------------------------------------------------------*/

#include "board.h"

#include <string.h>

/*------------------------------------------------------------------------------
 *         Internal definitions
 *------------------------------------------------------------------------------*/
// FIXME: Remove:
#define PR TRACE_DEBUG

/** Maximum ucSize in bytes of the smartcard answer to a command.*/
#define MAX_ANSWER_SIZE         10

/** Maximum ATR ucSize in bytes.*/
#define MAX_ATR_SIZE            55
/*------------------------------------------------------------------------------
 *         Internal variables
 *------------------------------------------------------------------------------*/
/** ISO7816 pins */
static const Pin pinsISO7816_sniff[]    = {PINS_SIM_SNIFF_SIM};
static const Pin pins_bus[]    = {PINS_BUS_SNIFF};
static const Pin pPwr[] = {
    /* Enable power converter 4.5-6V to 3.3V; low: off */
    {SIM_PWEN, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT},
    
    /* Enable second power converter: VCC_PHONE to VCC_SIM; high: on */
    {VCC_FWD, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
};

static struct Usart_info usart_info = {.base = USART_SIM, .id = ID_USART_SIM};

/*-----------------------------------------------------------------------------
 *          Initialization routine
 *-----------------------------------------------------------------------------*/

void Sniffer_configure( void ){
}

void Sniffer_exit( void ){
    USART_SetReceiverEnabled(USART_PHONE, 0);
}

void Sniffer_init( void )
{
    /*  Configure ISO7816 driver */
    PIO_Configure( pinsISO7816_sniff, PIO_LISTSIZE( pinsISO7816_sniff ) ) ;
    PIO_Configure( pins_bus, PIO_LISTSIZE( pins_bus) ) ;

    PIO_Configure(pPwr, PIO_LISTSIZE( pPwr ));

    ISO7816_Init(&usart_info, CLK_MASTER);

    USART_SetReceiverEnabled(USART_PHONE, 1);
}

void Sniffer_run( void )
{
    if (rcvdChar != 0) {
        /*  DATA_IN for host side is data_out for simtrace side   */
        /* FIXME: Performancewise sending a USB packet for every byte is a disaster */
        PR("----- %x %x %x ..\n\r", buf.buf[0], buf.buf[1],buf.buf[2] );
        USBD_Write( DATAIN, (void *) buf.buf, BUFLEN, 0, 0 );
        PR("----- Rcvd char\n\r");
        rcvdChar = 0;
    }
}
