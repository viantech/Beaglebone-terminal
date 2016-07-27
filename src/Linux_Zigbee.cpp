#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/signal.h>
#include <sys/types.h>
#include <sys/ioctl.h>
//#include <sstream>
#include <string.h>
//#include <iostream>
#include <cctype>
#include <errno.h>
#include "uart.h"
//#include "BlackLib.h"
//#include "BlackUART.h"
int fd;
char file_buf[44] = {0};

//using namespace BlackLib;
//using namespace std;

int readKeyboard(char *buffer)
{
	//char buffer[128];
	int result, nread = 0;
	fd_set inputs, testfds;
	struct timeval timeout;
	FD_ZERO (&inputs);
	FD_SET(0, &inputs);
	testfds = inputs;
	timeout.tv_sec = 5;
	timeout.tv_usec = 300000;
	result = select(FD_SETSIZE, &testfds, (fd_set*)0, (fd_set*)0, &timeout);


	switch(result)
	{
		case 0:
			//printf("timeout \n");
			break;
		case -1:
			if (errno != EINTR)
			{
				perror ("select");
				return 1;
			}
			break;
		default:
			if(FD_ISSET(0, &testfds))
			{
				ioctl(0, FIONREAD, &nread);
				if(nread == 0)
				{
					printf("keyboard done \n");
					return 1;
				}
				nread = read(0, buffer, nread);
				if (nread < 0) {
					if (errno != EINTR) {
						perror ("read");
						return 1;
					}
				}

				buffer[nread] = 0;
				//printf("read %d from keyboard: %s \n", nread, buffer);
			}
			break;
	}
	return nread;
}

int main(){
	int nread = 0, nwrite = 0;
	char key_strok[100];

	/* File log */
	FILE *log_f;
	log_f = fopen("uart_log.txt", "w+");
	if (log_f == NULL) {
		perror ("log file");
 		exit(EXIT_FAILURE);
	}

	Init_UART();
    while(1)
    {
    	nread = readKeyboard(key_strok);
    	if (nread > 0) {
//    		printf("Send:");
//    		fgets(mess, 100, stdin);
    		if (strstr(key_strok, "file") != NULL)
    			sendFile();
    		if (strstr(key_strok, "at") != NULL)
    		{
//    			mess[strlen(mess) - 1] = '\0';
    			key_strok[strlen(key_strok) - 1] = 0x0D;
    			write(fd, key_strok, strlen(key_strok) + 1);
				if (nwrite < 0) {
					if (errno != EINTR) {
						perror ("write");
						return 1;
					}
				}
    		}
    		nread = 0;
    	}
    	if (file_buf[0] != 0)
    	{
//    		printf("quang: %s \n", file_buf);
//    		file_buf[strlen(file_buf)] = '\n';
    		fputs(file_buf, log_f);
    		fflush(log_f);
    		bzero(file_buf, sizeof(file_buf));
    	}
    }
    fclose(log_f);
    close(fd);
    return 0;
}
