#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>

#include <sstream>
#include <string>
#include <iostream>
#include <cctype>

//#include "BlackLib.h"
//#include "BlackUART.h"

#define FALSE 0
#define TRUE 1
#define DEVICE "/dev/ttyO2"
#define BAUDRATE B9600
//using namespace BlackLib;
using namespace std;

volatile int STOP = FALSE;
void signal_handler_IO (int status); 	// signal hanler
int wait_flag = TRUE;					// TRUE while no signal received

void Write_Uart (const void *mess, int port)
{
	int len = 0;
	char *ptr_i;
	char *c_mess = (char*)mess;
	for (ptr_i = c_mess; *ptr_i; ptr_i++)
	     len++;
	write(port, c_mess, len + 1);
	sleep((len + 1 + 25) * 100);
}

int main(){

//    std::string writeToUart2;
//    std::string readFromUart2;
//    std::ostringstream os2;

	int  fd, c, res;
	struct termios newtio;
	__sigset_t mask;
	mask.__val[_SIGSET_NWORDS] = {0};
	struct sigaction saio; // definition of signal action
	char buf[255];

    fd = open(DEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
    {
    	perror("open_port: Unable to open /dev/ttyO2 \n");
    	exit(1);
    }
    /* isntall the signal handler before making the device asynchronous*/

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
    newtio.c_cc[VTIME]= 0;

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);

    Write_Uart("Hello Quang Do", fd);

    while(STOP == FALSE)
    {
    	//char commandChar[20] = "";
    	//readstr = "";
    	//string mystr = "";
    	//printf (".\n");
    	//usleep(100000);

        if (wait_flag == FALSE)
        {
        	res = read(fd, buf, 255);
        	buf[res]=0;
        	printf("%s - %d\n", buf, res);
        	if (res==1) STOP=TRUE; /* stop loop if only a CR was input */
            wait_flag = TRUE;      /* wait for new input */
        }
        //write (fd, "AT", 2);
    	/*if (Uart2.read().compare("UartReadError") != 0)
    	{
    		read = Uart2.read();
    		cout << "\nRead from UART: " << read  << endl;
    	}
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

    return 0;
}
  /***************************************************************************
  * signal handler. sets wait_flag to FALSE, to indicate above loop that     *
  * characters have been received.                                           *
  ***************************************************************************/

  void signal_handler_IO (int status)
  {
    cout << "Received SIGO: ";
    wait_flag = FALSE;
  }

