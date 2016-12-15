/*
 * SerialLib.c
 *
 * Created: 12/7/2016 12:30:34 PM
 * Author : samsung
 */ 

/* Includes */
#include <avr/io.h>
#include <avr/interrupt.h>
#include "SerialLib.h"
#define F_CPU 16000000UL


/* Initialize UART */
void serialBegin( unsigned int baudrate )
{
	unsigned char x;
	uint16_t UBBRVAL= ((F_CPU/16/baudrate)-1);
	UBRR0H = (unsigned char)(UBBRVAL>>8);
	UBRR0L = (unsigned char)UBBRVAL;
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
	UCSR0C = (3<<UCSZ00);
	sei();
	
	x = 0;              /* Flush receive buffer */
	
	UART_RxTail = x;
	UART_RxHead = x;
	UART_TxTail = x;
	UART_TxHead = x;
}

/* Interrupt handlers */

ISR(USART0_RX_vect)
{
	unsigned char data;
	unsigned char tmphead;
	
	data = UDR0;                 /* Read the received data */
	/* Calculate buffer index */
	tmphead = ( UART_RxHead + 1 ) & UART_RX_BUFFER_MASK;
	UART_RxHead = tmphead;      /* Store new index */
	
	if ( tmphead == UART_RxTail )
	{
		/* ERROR! Receive buffer overflow */
	}
	
	UART_RxBuf[tmphead] = data; /* Store received data in buffer */
}

ISR(USART0_UDRE_vect)

{
	unsigned char tmptail;
	
	/* Check if all data is transmitted */
	if ( UART_TxHead != UART_TxTail )
	{
		/* Calculate buffer index */
		tmptail = ( UART_TxTail + 1 ) & UART_TX_BUFFER_MASK;
		UART_TxTail = tmptail;      /* Store new index */
		
		UDR0 = UART_TxBuf[tmptail];  /* Start transmition */
	}
	else
	{
		UCSR0B &= ~(1<<UDRIE0);         /* Disable UDRE interrupt */
	}
}

/* Read and write functions */
unsigned char serialRead( void )
{
	unsigned char tmptail;
	
	while ( UART_RxHead == UART_RxTail )  /* Wait for incomming data */
	;
	tmptail = ( UART_RxTail + 1 ) & UART_RX_BUFFER_MASK;/* Calculate buffer index */
	
	UART_RxTail = tmptail;                /* Store new index */
	
	return UART_RxBuf[tmptail];           /* Return data */
}

void serialWrite( unsigned char data )
{
	unsigned char tmphead;
	/* Calculate buffer index */
	tmphead = ( UART_TxHead + 1 ) & UART_TX_BUFFER_MASK; /* Wait for free space in buffer */
	while ( tmphead == UART_TxTail );
	
	UART_TxBuf[tmphead] = data;           /* Store data in buffer */
	UART_TxHead = tmphead;                /* Store new index */
	
	UCSR0B |= (1<<UDRIE0);                    /* Enable UDRE interrupt */
}

unsigned char DataInReceiveBuffer( void )
{
	return ( UART_RxHead != UART_RxTail ); /* Return 0 (FALSE) if the receive buffer is empty */
}

