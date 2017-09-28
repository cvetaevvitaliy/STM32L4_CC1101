/*
 * utils.c
 *
 *  Created on: Mar 17, 2017
 *      Author: gs-ms
 */

#include "utils.h"
#include "freertos_util.h"

int __errno;


float convert_temp_u16_f(uint16_t temp)
{
    int8_t msb;
    uint8_t lsb;
    msb = (temp>>8)&0xFF;
    lsb = temp&0xFF;
    if (msb < 0) {
        return (float) (1.0*msb - 1.0/256.0*lsb);
    }else {
        return (float) (1.0*msb + 1.0/256.0*lsb);
    }
}

uint16_t convert_temp_f_u16(float temp)
{
    int msb = (int) temp;
    printf("MSB: %d\n", msb);
    float decpart = temp - msb;
    if (decpart < 0.0) {
        decpart = decpart * -1.0;
    }
    printf("DECpart: %f\n", decpart);
    int lsb = (int) (decpart * 256);
    printf("LSB: %u\n", lsb);
    return ((msb&0xFF) << 8) | (lsb&0xff);
}

void check_for_printf_buffer(void)
{
	char print_buffer[256];
	memset(print_buffer, 0, sizeof(print_buffer));
}

void _safe_send(void *p, uint16_t size)
{
	taskENTER_CRITICAL();
	//HAL_UART_Transmit_IT(&huart1, p, size);
	HAL_UART_Transmit_DMA(&huart1, p, size);
	taskEXIT_CRITICAL();
}

int _write(int fd, void *p, size_t len)
{
	/* must enqueue that and another1 process it */
	/* put this shit into a queue! */
	//uart_send(p, len);
	uint8_t *data = p;
	int i = 0;
	while(i < len) {
		osMessagePut(UartQueueTxHandle, data[i], osWaitForever);
		i++;
	}
	/* wait for that to end bro */
	return len;
}

int uart_send(void *p, uint16_t size)
{
	uint8_t *data = p;
	int i = 0;
	while(i < size) {
		osMessagePut(UartQueueTxHandle, data[i], osWaitForever);
		i++;
	}
	/* wait for that to end bro */
	return size;
}

void print_char(char character)
{
	volatile static char last_char = 0;
	uint8_t send_buff[3];
	if (last_char != '\r' && character == '\n'){
		character = '\r';
		send_buff[0] = '\r';
		send_buff[1] = '\n';
		send_buff[2] = '\0';
		uart_send(send_buff, 3);
	}else{
		if (character == '\r'){
			send_buff[0] = '\r';
			send_buff[1] = '\n';
			send_buff[2] = '\0';
			uart_send(send_buff, 3);
		}else{
			send_buff[0] = character;
			uart_send(send_buff, 1);
		}
	}
}

void print_uart(char * fmt, ...)
{
	/* use of vsprintf */
	char print_buffer[256];
	memset(print_buffer, 0 , sizeof(print_buffer));
	va_list args;
	va_start (args, fmt);
	vsprintf (print_buffer, fmt, args);
	va_end (args);
	uart_send((uint8_t *) print_buffer, strlen((const char *) print_buffer));
}

void print_uart_ln(char * fmt, ...)
{
	char print_buffer[256];
	memset(print_buffer, 0 , sizeof(print_buffer));
	va_list args;
	va_start (args, fmt);
	vsprintf (print_buffer, fmt, args);
	va_end (args);
	strcat(print_buffer, "\r\n");
	uart_send((uint8_t *) print_buffer, strlen((const char *) print_buffer));
}
