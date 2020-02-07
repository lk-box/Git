#ifndef __MY_SERIAL_PORT_H__
#define __MY_SERIAL_PORT_H__

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <string.h>
#include <pthread.h>

int SerialPort_Send(unsigned char *buf, int len);
int SerialPort_Recv(unsigned char *buf, int len);
int SerialPort_Setup(const char *device,int baudrate);

#endif

