/*
 * uart.h
 *
 *  Created on: Jul 21, 2016
 *      Author: quangdo
 */

#ifndef UART_H_
#define UART_H_

#define FALSE 0
#define TRUE 1
#define DEVICE "/dev/ttyUSB0"
#define CHMOD_C "sudo chmod a+rw "
#define COM_UART CHMOD_C DEVICE
#define BAUDRATE B115200
//#define FILE_MODE 1
//#define COMM_MODE 0
//#define MODE_APP 1
void Init_UART();
void Write_Uart (const void *mess, int port);
void sendFile();
void close_log();
void Command_Handler(char *command);
void logg_File(char *buf);
extern int fd;
//extern volatile bool _flag_file;
extern char file_buf[44];

#endif /* UART_H_ */
