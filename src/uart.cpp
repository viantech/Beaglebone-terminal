/*
 * uart.cpp
 *
 *  Created on: Jul 21, 2016
 *      Author: quangdo
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <termios.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cctype>
#include <string.h>
#include <iostream>
#include <fstream>
//#include <sstream>
#include "uart.h"

using namespace std;
static bool Contains(const char* MyChar, const char* Search);
static void signal_handler_IO (int status); 	// signal hanler
volatile bool _wait_flag_ACK = 0;					// TRUE while no signal received
//extern volatile char *file_buf;
volatile bool _flag_com_c = 0;
const char *pre_sink = "AT+SCAST:";
char ACK_CommandC[20] = {0};

void close_log()
{
//	fclose(log_f);
}
void logg_File(char *buf)
{
//	fputs(buf, log_f);
//	fflush(log_f);
}

void Init_UART()
{
	memcpy(ACK_CommandC, pre_sink, strlen(pre_sink));
	ACK_CommandC[strlen(pre_sink)] = 'A';
//	strcat(ACK_CommandC, "A");
	struct termios newtio;
	__sigset_t mask;
	mask.__val[_SIGSET_NWORDS] = {0};
	struct sigaction saio; // definition of signal action

	system(COM_UART);
    fd = open(DEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
    {
    	perror("open_port: Unable to open /dev/ttyUSB0 \n");
    	exit(1);
    }
    /* install the signal handler before making the device asynchronous*/

    saio.sa_handler = signal_handler_IO;
    saio.sa_mask = mask;
    saio.sa_flags = SA_RESTART;
    saio.sa_restorer = NULL;
    sigaction(SIGIO, &saio, NULL);
    /* allow the process to receive SIGIO */
    fcntl(fd, F_SETOWN, getpid());
    /* Make the file descriptor asynchronous */
    fcntl(fd, F_SETFL, FNDELAY | FASYNC);

    /* set new port settings for canonical input processing */
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD ;
    newtio.c_iflag = IGNPAR | ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = ICANON;
    newtio.c_cc[VMIN]=1;
    newtio.c_cc[VTIME]= 0;

    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);
}
void Write_Uart (const void *mess, int port)
{
	int len = 0;
	char *ptr_i;
	char *c_mess = (char*)mess;
	for (ptr_i = c_mess; *ptr_i; ptr_i++)
	     len++;
	write(port, c_mess, len + 1);
//	tcflush(port, TCOFLUSH);
	//sleep((len + 1 + 25) * 100);
}

void sendFile()
{
    /*File handler*/
    FILE *file_ptr;
    size_t flen = 0;
    char* line = NULL;
    file_ptr = fopen ("/home/quangdo/Downloads/Intelhex/demo.hex", "r");
    if (file_ptr == NULL) exit(EXIT_FAILURE);

    char send_com[100];
//    send_com[0] = 0x41; //'A';
//    send_com[1] = 0x54; //'T';
//    send_com[2] = 0x2B; //'+';
//    send_com[3] = 0x53; //'S';
//    send_com[4] = 0x43; //'C';
//    send_com[5] = 0x41; //'A';
//    send_com[6] = 0x53; //'S';
//    send_com[7] = 0x54; //'T';
//    send_com[8] = 0x3A; //':';
    //strcpy(send_com,pre_com);
    memcpy(send_com, pre_sink, strlen(pre_sink));
    while(getline(&line, &flen, file_ptr) != -1 ) /* read a line */
    {
    	strcat(line, "\r"); //watch out
    	memcpy(&send_com[strlen(pre_sink)], line, strlen(line));
    	//Write_Uart(send_com, fd);
    	write(fd, send_com, strlen(send_com) + 1);
    	while (_wait_flag_ACK == 0);
    		_wait_flag_ACK = 0;
    	/*char commandChar[20] = "";
    	readstr = "";
    	string mystr = "";
    	printf (".\n");
    	usleep(100000);
    	cout << "Send: ";
    	getline(cin, mystr);
    	for (int i=0;;i++)
    	{
    		commandChar[i] = getchar();
    		if (commandChar[i] == 8)
    		{
    			commandChar[i] = 0;
    			i -= 2;
    		}
    		if (commandChar[i] == '\n')
    		{
    			commandChar[i] = 0;
    			break;
    		}
    	}
    	string mystr(commandChar);
    	cout << "Transmit to UART: " << mystr << "\n" ;
    	if (mystr.compare("") != 0)
    	{
    		readstr = Uart2.transfer(mystr, 50000);
    		if(readstr.find("OK") != std::string::npos) //OK
    			cout << readstr << endl;
    	}*/
    }
    fclose(file_ptr);
    if (line)  free(line);
}

static bool Contains(const char* MyChar, const char* Search)
{
	char *cp = (char *)MyChar;
	char *s1;
	char *s2 = (char *)Search;
	while (*cp)
	{
		s1 = cp;
        while ( *s1 && *s2 && !(*s1-*s2) )
                s1++, s2++;

        if (!*s2)
                return true;

        cp++;
	}
	return false;
}
//#if (MODE_APP == COMM_MODE)
void Command_Handler(char *command)
{
	int id = *(command + 2) - '0';
	switch(id){
	case 0:
		ACK_CommandC[10] = '0';
		break;
	case 1:
		if (*(command + 3) - '0' == 0) {
			string status_str;
			status_str = "10 + Battery Level: 79%, Maintenance: Ready\n";
			char *status_c = new char[status_str.size() + 1];
			std::copy(status_str.begin(), status_str.end(), status_c);
			status_c[status_str.size()] = '\0';
			strcat(ACK_CommandC, status_c);
		}
		else if (*(command + 3) - '1' == 0)  strcat(ACK_CommandC, "11");
		else  ACK_CommandC[10] = '1';
		break;
	case 2:
		ACK_CommandC[10] = '2';
		break;
	case 3:
		ACK_CommandC[10] = '3';
		break;
	case 4:
		ACK_CommandC[10] = '4';
		break;
	case 5:
		ACK_CommandC[10] = '5';
		break;
	case 6:
		ACK_CommandC[10] = '6';
		break;
	case 7:
		ACK_CommandC[10] = '7';
		break;
	case 8:
		char posit[14];
		sprintf(posit, "8 + %03d,%03d", rand() % 1000, rand() % 1000);
		strcat(ACK_CommandC, posit);
		break;
	case 9:
		char batt_lv[7];
		batt_lv[7] = {0};
		sprintf(batt_lv, "9 + %d%c", rand() % 100 + 1, 37);
		strcat(ACK_CommandC, batt_lv);
		break;
	default:
		printf("[ERROR]: Please check your typing command \n");
		break;
	}

}
//#endif
  /***************************************************************************
  * signal handler. sets wait_flag to FALSE, to indicate above loop that     *
  * characters have been received.                                           *
  ***************************************************************************/

static void signal_handler_IO (int status)
  {
      int nbytes = 0;
      char buffer[512] = {0}, *bufptr = NULL;
      char *com_ptr;
      if(status != SIGIO) return;

      //read characters into our string buffer until we get a OK<CR><NL>
      bufptr = buffer;
      while ((nbytes = read(fd, bufptr, buffer + sizeof(buffer) - bufptr - 1 )) > 0 )
      {
          bufptr += nbytes;
/*        printf("It Okay %hhX:%d\n", buffer[nbytes],nbytes);
          if(nbytes > 4)
          {
              if (*(bufptr - 2) == 0x0D || *(bufptr - 1) == 0x0A)
          }*/
      }
      /*nul terminate the string and see if we got an OK response */

      *bufptr = '\0';
      bufptr = buffer;
      cout << "Received: " << bufptr << endl;

//#if (MODE_APP == COMM_MODE)
      if ((com_ptr = strstr(buffer, "@C")) != NULL)
      {
    	  Command_Handler(com_ptr);
    	  strcat(ACK_CommandC, "\r");
    	  write(fd, ACK_CommandC, strlen(ACK_CommandC) + 1);
    	  bzero(&ACK_CommandC[strlen(pre_sink) + 1], 9);
      }
//#endif

//#if (MODE_APP == FILE_MODE)
      else if (Contains(buffer, "=:"))
      {
//    	  file_buf = buffer;
//    	  strncpy(file_buf, buffer, sizeof(buffer));
    	  memcpy(file_buf, &buffer[26], sizeof(file_buf));
    	 // if (fputs(buffer, log_f) == EOF) perror("log file");
//    		  while(fputs(buffer, log_f) != 0);
//    			  fflush(log_f);
      }
      else if(Contains(buffer, "ACK"))
      		  _wait_flag_ACK = 1;
//#endif

      bzero(buffer, sizeof(buffer));
}

