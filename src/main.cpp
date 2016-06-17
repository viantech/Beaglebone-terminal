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

#include "BlackLib.h"
#include "BlackUART.h"

#define FALSE 0
#define TRUE 1
#define DEVICE "/dev/ttyO2"
#define BAUDRATE B9600
using namespace BlackLib;
using namespace std;

volatile int STOP = FALSE;
void signal_handler_IO (int status); 	// signal hanler
int wait_flag = TRUE;					// TRUE while no signal received
/*
int set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
        	printf ("error %d from tcgetattr", errno);
            return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
        	printf ("error %d from tcsetattr", errno);
            return -1;
        }
        return 0;
}

void set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf ("error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                printf ("error %d setting term attributes", errno);
}
*/
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

/*  BlackLib::BlackUART  Uart2(BlackLib::UART2,
                               BlackLib::Baud9600,
                               BlackLib::ParityNo,//ParityNo
                               BlackLib::StopOne,
                               BlackLib::Char8 );
    // Pins on BeagleBone Black REV C
    // UART2_TX -> GPIO_2 (P9.22)
    // UART2_RX -> GPIO_3 (P9.21)
    Uart2.flush( BlackLib::bothDirection );
    bool isOpened = Uart2.open( BlackLib::ReadWrite | BlackLib::NonBlock );
    if( !isOpened )
    {
    	std::cout << "UART DEVICE CAN\'T OPEN" << std::endl;
    	exit(1);
    }
    else
    {
    	cout << "Start BLE Application " << std::endl;
    }
    string readstr = "";
    readstr = Uart2.transfer("AT",50000);
    if( readstr.compare("OK") == 0)
    	cout << "Test AT Command: " << readstr << endl;
    else
    {
    	cout << "ERROR: Please check HM-10 connection." << endl;
    	exit(1);
    }
    readstr = "";
    readstr = Uart2.transfer("AT+RENEW", 100000);
    if (readstr.compare("OK+RENEW") == 0){
    	cout << "RESET HM-10 to factory setting.\n" << readstr << endl;
    	Uart2.transfer("AT+AFTC000", 80000);
    	Uart2.transfer("AT+MODE2", 50000);
    	Uart2.transfer("AT+RESET", 50000);

    	cout << "Start connect HM-10 Module." << endl;
    }
    else
    {
    	cout << "Fail to renew module.\nRetry. Please wait.. ";
    	readstr = Uart2.transfer("AT+RENEW", 500000);
    	sleep(1);
    	cout << readstr << endl;
    }*/
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
    /*tcgetattr (fd, &newtio);
    cfsetospeed (&newtio, (speed_t)B9600);
    cfsetispeed (&newtio, (speed_t)B9600);

    // Setting Port Stuff
    newtio.c_cflag     &=  ~PARENB;            // Make 8n1
    newtio.c_cflag     &=  ~CSTOPB;
    newtio.c_cflag     &=  ~CSIZE;
    newtio.c_cflag     |=  CS8;
    newtio.c_cflag     |=  (CLOCAL | CREAD);     // turn on READ & ignore ctrl lines
    newtio.c_cflag     &=  ~CRTSCTS;           // no flow control

    newtio.c_cc[VMIN]   =  5;                  // read number of character doesn't block
    newtio.c_cc[VTIME]  =  0;                  // 0.5 seconds read timeout

    newtio.c_iflag &= ~IGNBRK;
    newtio.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
    newtio.c_oflag &= ~OPOST;
    newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    //Make raw
    cfmakeraw(&newtio);*/

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);

//    write (fd, "AT+RENEW\n", 9);
//    usleep ((8 + 25) * 100);

    write (fd, "Hello Quang Do!", 15);
    usleep ((15 + 25) * 100);

//    write (fd, "AT+AFTC000\n", 11);
//    usleep ((11 + 25) * 100);

//    write (fd, "AT+MODE0\n", 9);
//    usleep ((9 + 25) * 100);

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
        	//if (res==1) STOP=TRUE; /* stop loop if only a CR was input */
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

//    Uart2.write("AT+RESET");
//    write (fd, "AT+RESET\n", 9);
//    usleep ((9 + 25) * 100);
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

