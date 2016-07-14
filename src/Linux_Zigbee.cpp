#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sstream>
#include <string.h>
#include <iostream>
#include <cctype>
#include <errno.h>

//#include "BlackLib.h"
//#include "BlackUART.h"

#define FALSE 0
#define TRUE 1
#define DEVICE "/dev/ttyUSB0"
#define BAUDRATE B115200
//using namespace BlackLib;
using namespace std;
void sendFile();
int readKeyboard(char *buffer);
void signal_handler_IO (int status); 	// signal hanler
volatile int _wait_flag_ACK = 0;					// TRUE while no signal received
volatile int _flag_OK = 0;
int fd;
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

bool Contains(const char* MyChar, const char* Search)
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

void sendFile()
{
    /*File handler*/
    FILE *file_ptr;
    size_t flen = 0;
    char* line = NULL;
    file_ptr = fopen ("/home/quangdo/Downloads/Intelhex/demo.hex", "r");
    if (file_ptr == NULL) exit(EXIT_FAILURE);

//    char* pre_com[10] = "AT+SCAST:";
    char send_com[100];
    send_com[0] = 0x41; //'A';
    send_com[1] = 0x54; //'T';
    send_com[2] = 0x2B; //'+';
    send_com[3] = 0x53; //'S';
    send_com[4] = 0x43; //'C';
    send_com[5] = 0x41; //'A';
    send_com[6] = 0x53; //'S';
    send_com[7] = 0x54; //'T';
    send_com[8] = 0x3A; //':';
    //strcpy(send_com,pre_com);
    while(getline(&line, &flen, file_ptr) != -1 ) /* read a line */
    {
    	strcat(line, "\r"); //watch out
    	memcpy(&send_com[9], line, strlen(line)+1);
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

int main(){
	int nread = 0, nwrite = 0;
	char mess[100];
	struct termios newtio;
	__sigset_t mask;
	mask.__val[_SIGSET_NWORDS] = {0};
	struct sigaction saio; // definition of signal action

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
    newtio.c_cc[VTIME]= 5;

    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);
    while(1)
    {
    	nread = readKeyboard(mess);
    	if (nread > 0) {
//    		printf("Send:");
//    		fgets(mess, 100, stdin);
    		if (strstr(mess, "file") != NULL)
    			sendFile();
    		if (strstr(mess, "at") != NULL)
    		{
//    			mess[strlen(mess) - 1] = '\0';
    			mess[strlen(mess) - 1] = 0x0D;
    			write(fd, mess, strlen(mess) + 1);
				if (nwrite < 0) {
					if (errno != EINTR) {
						perror ("write");
						return 1;
					}
				}
//    			while(_flag_OK == 0);
//    			_flag_OK = 0;
    		}
    		nread = 0;
    	}
    }

    close(fd);
    return 0;
}

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
	timeout.tv_usec = 500000;
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

  /***************************************************************************
  * signal handler. sets wait_flag to FALSE, to indicate above loop that     *
  * characters have been received.                                           *
  ***************************************************************************/

  void signal_handler_IO (int status)
  {
      int nbytes = 0;
      char buffer[512] = {0}, *bufptr = NULL;
      if(status != SIGIO) return;

      //read characters into our string buffer until we get a OK<CR><NL>
      bufptr = buffer;
      while ((nbytes = read(fd, bufptr, buffer + sizeof(buffer) - bufptr - 1)) > 0)
      {
          bufptr += nbytes;
//          printf("It Okay %hhX:%d\n", buffer[nbytes],nbytes);
//          if(nbytes > 4)
//          {
//              if (*(bufptr - 2) == 0x0D && *(bufptr - 1) == 0x0A)
//          }

      }
      /*nul terminate the string and see if we got an OK response */

		if (nbytes < 0) {
//			if (errno != EINTR) {
//				perror ("read");
//				return;
//			}
//			      printf("It done1 %hhX:%d\n", buffer[nbytes],nbytes);
		}
      *bufptr = '\0';
      bufptr = buffer;
      cout << "Received: " << bufptr << endl;

      if (Contains(buffer, "OK") == true)
    	  _flag_OK = 1;
      if(Contains(buffer, "ACK") == true)
      	  _wait_flag_ACK = 1;
}
