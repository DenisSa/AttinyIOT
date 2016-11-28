/*
 * main.c
 *
 *  Created on: Jun 20, 2016
 *      Author: d
 */
#define F_CPU 8000000UL
#include <util/delay.h>
#include <avr/io.h>
#include "gpio.h"
#include "am2302Sensor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "softuart.h"

#define AM2302DATA PB3

void initPins();
void ftoa(float n, char *res, int afterpoint);
int intToStr(int x, char str[], int d);
void reverse(char *str, int len);

int main(void) {
	uint8_t data[5];
	int i;
	float ftemperature = 0;
	float fhumidity = 0;
	char hdata[6];
	char tdata[6];
	char msgString[256];
	char msgSize[8];
	_delay_ms(5000);
	initPins();
	while (1) {
		if (readAM2302Data(AM2302DATA, data) == 0) {
			ftemperature = getTemperature(data);
			fhumidity = getHumidity(data);
			ftoa(ftemperature, tdata, 1);
			ftoa(fhumidity, hdata, 1);
			snprintf(msgString, sizeof msgString, "T:%s;H:%s;", tdata, hdata);
			snprintf(msgSize, sizeof msgSize, "%d", strlen(msgString));
			softuart_init();
			sei();
			//softuart_puts("test\r\n\n");
			softuart_puts("AT+CIPSTART=\"TCP\",\"192.168.0.101\",5555\r\n\n");
			_delay_ms(5000);
			softuart_puts("AT+CIPSEND=");
			softuart_puts(msgSize);
			softuart_puts("\r\n\n");
			_delay_ms(1000);
			softuart_puts("T:");
			softuart_puts(tdata);
			softuart_puts(";H:");
			softuart_puts(hdata);
			softuart_puts(";");
			softuart_puts("");
			_delay_ms(1000);
			softuart_puts("AT+CIPCLOSE\r\n\n");
			cli();
			for (i = 0; i < 60; i++) {
				_delay_ms(1000);
			}
		}
	}
}

void initPins() {
	/*
	 *
	 *  pb2 - IN Analog
	 *  pb1 - OUT Power control
	 *  pb3 - OUT/IN AM2302 comms
	 *  pb4 - OUT Clock
	 *  pb0 - comms out
	 */
	setdirection(PB2, IN);
	setdirection(PB1, OUT);
	setdirection(AM2302DATA, OUT);

	setpin(PB1, 0);
	setpin(AM2302DATA, 1);
}

// Converts a given integer x to string str[].  d is the number
// of digits required in output. If d is more than the number
// of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d) {
	int i = 0;
	while (x) {
		str[i++] = (x % 10) + '0';
		x = x / 10;
	}

	// If number of digits required is more, then
	// add 0s at the beginning
	while (i < d)
		str[i++] = '0';

	reverse(str, i);
	str[i] = '\0';
	return i;
}

// reverses a string 'str' of length 'len'
void reverse(char *str, int len) {
	int i = 0, j = len - 1, temp;
	while (i < j) {
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++;
		j--;
	}
}

// Converts a floating point number to string.
void ftoa(float n, char *res, int afterpoint) {
	// Extract integer part
	int ipart = (int) n;

	// Extract floating part
	float fpart = n - (float) ipart;

	// convert integer part to string
	int i = intToStr(ipart, res, 0);

	// check for display option after point
	if (afterpoint != 0) {
		res[i] = '.';  // add dot

		// Get the value of fraction part upto given no.
		// of points after dot. The third parameter is needed
		// to handle cases like 233.007
		fpart = fpart * pow(10, afterpoint);

		intToStr((int) fpart, res + i + 1, afterpoint);
	}
}
