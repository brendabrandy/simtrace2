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

extern uint8_t rcvdChar;
extern uint32_t char_stat;

//#define BUFLEN  14
// FIXME: Remove:
#define PR TRACE_DEBUG
//#define PR printf 

/*typedef struct ring_buffer
{
    uint8_t     buf[BUFLEN*2];   // data buffer
    uint8_t     idx;                // number of items in the buffer
} ring_buffer;
*/
ring_buffer buf = { {0}, 0 };

void buf_push(uint8_t item)
{
    buf.buf[buf.idx % (BUFLEN*2)] = item;
    PR("----- Push: %x %x\n\r", buf.idx, buf.buf[buf.idx]);
    buf.idx = (buf.idx+1) % (BUFLEN*2);
}

uint8_t get_buf_start(uint8_t *buf_start)
{
    *buf_start = &(buf.buf[buf.idx]);
    return 2*BUFLEN-buf.idx;
}

/** Initializes a ISO driver
 */ 
// FIXME: This function is implemented in iso7816_4.c !! Only MCK instead of SCK is always taken. Change that!                                                                               
void _ISO7816_Init( void )
{   
    printf("ISO_Init\n\r");
    TRACE_DEBUG("ISO_Init\n\r");
                        
    USART_Configure( USART_PHONE,                                                          
                     US_MR_USART_MODE_IS07816_T_0
// Nope, we aren't master:   
 //                    | US_MR_USCLKS_MCK                                                  
                     | US_MR_USCLKS_SCK                                                    
                     | US_MR_NBSTOP_1_BIT                                                  
                     | US_MR_PAR_EVEN                                                      
                     | US_MR_CHRL_8_BIT                                                    
                     | US_MR_CLKO   /** TODO: This field was set in the original simtrace firmware..why? */      
                     | (3<<24), /* MAX_ITERATION */                                        
                     1,
                     0);
    /*                                                                                     
    SYNC = 0 (async mode)
    OVER = 0 (oversampling by 8?)                                                          
    FIDI = 372 (default val on startup before other value is negotiated)                   
    USCLKS = 3 (Select SCK as input clock) --> US_MR_USCLKS_SCK                            
    CD = 1 ?    --> US_BRGR_CD(1)
    */                                                                                     
    USART_PHONE->US_FIDI = 372; 
//    USART_PHONE->US_IDR = (uint32_t) -1;
    USART_PHONE->US_BRGR = US_BRGR_CD(1);
//    USART_PHONE->US_BRGR = BOARD_MCK / (372*9600);
    USART_PHONE->US_TTGR = 5;

    /* Configure USART */
    PMC_EnablePeripheral(ID_USART_PHONE);

    USART1->US_IDR = 0xffffffff;
    USART_EnableIt( USART1, US_IER_RXRDY) ;
    /* enable USART1 interrupt */
    NVIC_EnableIRQ( USART1_IRQn ) ;
    
//    USART_PHONE->US_IER = US_IER_RXRDY | US_IER_OVRE | US_IER_FRAME | US_IER_PARE | US_IER_NACK | US_IER_ITER;
}

/* 
 *  Initializes rcvdChar with the char received on USART interface                            
 *  char_stat is zero if no error occured. 
 *  Otherwise it is filled with the content of the status register.                           
 */
void USART1_IrqHandler( void )                                                                
{   
    uint32_t stat;                                                                            
    char_stat = 0;
    // Rcv buf full
/*    if((stat & US_CSR_RXBUFF) == US_CSR_RXBUFF) {                                           
        TRACE_DEBUG("Rcv buf full");
        USART_DisableIt(USART1, US_IDR_RXBUFF);                                               
    }                                                                                         
*/  
    uint32_t csr = USART_PHONE->US_CSR;                                                       
    
    PR("---- stat: %x\n\r", csr);

    if (csr & US_CSR_TXRDY) {
        /* transmit buffer empty, nothing to transmit */                                      
    }  
    if (csr & US_CSR_RXRDY) {
        stat = (csr&(US_CSR_OVRE|US_CSR_FRAME|
                        US_CSR_PARE|US_CSR_TIMEOUT|US_CSR_NACK|
                        (1<<10)));

        if (stat == 0 ) {
            /* Fill char into buffer */
            PR("---- BUFLEN %x\n\r", buf.idx);
            buf_push((USART_PHONE->US_RHR) & 0xFF);
        } else {
       //     buf_push((USART_PHONE->US_RHR) & 0xFF);
            PR("%x\n\r", (USART_PHONE->US_RHR) & 0xFF);
        } /* else: error occured */

        if ((buf.idx % BUFLEN) == 0) {
            rcvdChar = 1;
        }

        char_stat = stat;
    }
}