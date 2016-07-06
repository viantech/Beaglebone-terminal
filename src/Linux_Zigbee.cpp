#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>

#include <sstream>
#include <string.h>
#include <iostream>
#include <cctype>

//#include "BlackLib.h"
//#include "BlackUART.h"

#define FALSE 0
#define TRUE 1
#define DEVICE "/dev/ttyUSB0"
#define BAUDRATE B115200
//using namespace BlackLib;
using namespace std;

void signal_handler_IO (int status); 	// signal hanler
int _wait_flag_ACK = 0;					// TRUE while no signal received
int fd;
void Write_Uart (const void *mess, int port)
{
	int len = 0;
	char *ptr_i;
	char *c_mess = (char*)mess;
	for (ptr_i = c_mess; *ptr_i; ptr_i++)
	     len++;
	write(port, c_mess, len + 1);
	//tcflush(port, TCOFLUSH);
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

int main(){

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
    saio.sa_flags = 0;
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
    /*File handler*/
    FILE *file_ptr;
    size_t flen = 0;
    char* line = NULL;
    file_ptr = fopen ("/home/quangdo/Downloads/Intelhex/helllo.hex", "r");
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
    	strcat(line, "\r\n");
    	memcpy(&send_com[9], line, strlen(line)+1);
//    	strcat(send_com, line);
//    	cout  << send_com;
    	Write_Uart(send_com, fd);
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
    //if (line)  free(line);

    close(fd);
    return 0;
}
  /***************************************************************************
  * signal handler. sets wait_flag to FALSE, to indicate above loop that     *
  * characters have been received.                                           *
  ***************************************************************************/

  void signal_handler_IO (int status)
  {
      int i, nbytes;
      char buffer[100] = {0}, *bufptr = NULL;
      if(status != SIGIO) return;

      //read characters into our string buffer until we get a CR or NL
      bufptr = buffer;

      while ((nbytes = read(fd, bufptr, buffer + sizeof(buffer) - bufptr - 1)) > 0)
      {
          bufptr += nbytes;
          if(nbytes > 4)
          {
              if (*(bufptr - 1) == '\n' || *(bufptr - 1) == '\r')
                      break;
          }

      }
      if (nbytes < 4) return;
      /*nul terminate the string and see if we got an OK response */
      //printf("signal_handler_IO: signum = %d\r\n", signum);
      *bufptr = '\0';
      bufptr = buffer;
      cout << "Received: " << bufptr << endl;

//    if(strstr(buffer, "ACK") != NULL)
      if(Contains(buffer, "ACK") == true)
      	  _wait_flag_ACK = 1;
  }

