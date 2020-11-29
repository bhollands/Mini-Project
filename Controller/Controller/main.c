/*
 * Controller.c
 *
 * Created: 25/11/2020 12:47:18
 * Author : Bernie
 */ 
#define F_CPU 16000000
#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>
#include <math.h>
#include <string.h>
#define BAUDRATE 9600 //setting the baurd rate
#define BAUD_PRESCALLER (((F_CPU/(BAUDRATE* 16UL))) - 1) //defining the pre-scalar
#include <avr/interrupt.h>

/*
sudo code: 

if button is pressed, read from the Hall effect (ADC) and send over 
serial to other controller


*/

unsigned char USART_Char_Recive(){
	while(!(UCSR0A&(1<<RXC0)));  // Wait to receive data
	return UDR0;
}

int analogReadA0(){
	ADCSRA = ADCSRA | (1 << ADSC);
	while(ADCSRA & (1 << ADSC));
	return ADCH;
}

void USART_INIT(void){
	UBRR0H = (uint8_t)(BAUD_PRESCALLER>>8); //setting the pre-scalar
	UBRR0L = (uint8_t)(BAUD_PRESCALLER);
	UCSR0B = (1 << RXEN0)|(1<<TXEN0); //enabling transmit and recive
	UCSR0C = (3<<UCSZ00); //setting max pacet size to 8 bits
}

void USART_putstring(char* StringPtr) {
	while(*StringPtr != 0x00){    //Here we check if there is still more chars to send, this is done checking the actual char and see if it is different from the null char
		USART_Send(*StringPtr);    //Using the simple send function we send one char at a time
	StringPtr++;}        //We increment the pointer so we can read the next char
}

void USART_String_Recive(){
	char string[16]; //inital 16 byte array
	for(int i = 0; i < 16; i++){ //loop throught the array
		char charIn = USART_Char_Recive(); // get the most recent char
		string[i] = charIn; //write that char to the array
		if(charIn == '\n'){ //if the char just read is the stop
			while(i <= 17){ // loop till the end of the array
				string[i] = ' '; //fill the rest of the array with empty chars
				i++;
			}
			//do thing with the string
			break; //break the for loop
		}
	}
}

void PBpinWrite(int ledNum, int state){
	//each array cell relates to each pb pins
	int portID[] = {0B000001,0B000010,0B000100,0B001000,0B010000,0B100000}; //all the port addresses
	
	switch(state){ //state describes on or off and writes it to the selected pin
		case 0: PORTB &= ~portID[ledNum]; //if 0 writes LOW
		break;
		case 1: PORTB ^= portID[ledNum]; //if 1 writes HIGH to
		break;
	}
}

void USART_Send(unsigned char data){
	while(!(UCSR0A & (1<<UDRE0))); //check if data is sent
	UDR0 = data; //load new data to transmit
}

ISR(INT0_vect){ //While interupt is pressed
	//int analogRead = 1111;
	int analogRead = analogReadA0();//read from the hall effect
	char hallEffect[3];
	sprintf(hallEffect, "%d", analogRead); //int to char array
	USART_putstring(hallEffect);
	USART_Send('|');
	_delay_ms(50);
}


void setup(){
	EIMSK = (1 << INT0);
	EICRA =  0b00000000; //trigger while low
	TCCR0B = 0b00000101;//0x05; //clock pre-scalar of 1024
	ADMUX =  0b01100000;// Configure ADC to be left justified, use AVCC as reference, and select ADC0 as ADC input
	ADCSRA = 0b10000111;// Enable the ADC and set the prescaler to max value (128)
	USART_INIT(); //init the serial bus
	DDRB = 0xFF; //set all PB pins to output
	sei();
}

int main()
{
	setup();
	while(1)
	{
		USART_Send('n');
		_delay_ms(50);
	}
	return 0;
}

