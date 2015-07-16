#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <iostream>
#include <bitset>

#define SERIAL_PORT        "/dev/ttyUSB0"
#define BAUDRATE           B115200

struct termios tty, ttyOld;
int tty_fd;

time_t t = time(NULL);
struct tm tm = *localtime(&t);
int fd;
bool spew = 0;//if 1 spew buffer of serial
bool debug = 0;//if 1 print line by line buf rs232
char cmd[500];
char dataToSend[100];
char text[1000];
char buf[1000];
char textReceive[1000];
char ip[18];

void 
inicSerial(void)
{
    /* Open the file descriptor in non-blocking mode */
   tty_fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY);
 //  if (tty_fd <0) {perror(SERIAL_PORT); exit(-1); }
   
   tcgetattr(tty_fd,&ttyOld); /* save current port settings */
   
   memset(&tty,0,sizeof(tty));
    /* Setting other Port Stuff */
   tty.c_cflag     &=  ~PARENB;            // Make 8n1
   tty.c_cflag     &=  ~CSTOPB;
   tty.c_cflag     &=  ~CSIZE;
   tty.c_cflag     |=  CS8;

   tty.c_cflag     &=  ~CRTSCTS;           // no flow control
   tty.c_cc[VMIN]   =  0;                  // read doesn't block
   tty.c_cc[VTIME]  =  0;                  // 0.5 seconds read timeout
   tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

   /* Make raw */
   cfmakeraw(&tty);

   tty_fd=open(SERIAL_PORT, O_RDWR | O_NOCTTY);
   cfsetospeed(&tty,B115200);            // 115200 baud
   cfsetispeed(&tty,B115200);            // 115200 baud

   tcflush( tty_fd, TCIFLUSH );
   if ( tcsetattr ( tty_fd, TCSANOW, &tty ) != 0) {
   std::cout << "Error " << std::endl;
   }
}

//!Reset ESP
void resetEsp(void)
{
   memset(cmd, '\0', sizeof cmd);
   strcpy(cmd, "AT+RST\r\n");
   memset(text, '\0', sizeof text);
   write (tty_fd, cmd, strlen(cmd));           // send
   usleep ((10 + 1550) * 100);
   while(strstr(text, "ready") == NULL)
   {
      if(read (tty_fd, buf, sizeof buf) > 0)
      {
         strcat(text, buf);
         if (debug == 1)
            printf(">>> %s <<<\n", buf);
         memset(buf, '\0', sizeof buf);
      }      
   }
   if (spew == 1)
      printf("%s\r\n", text);
}

//!Quest IP in string
void
questIp(char data[1000])
{
   char *word;
   bool state = 0;
   word = strtok(data, "\n");
   while(word != NULL)
   {
      word = strtok(NULL, ",");
      if(state == 0)
      {
         word[strlen(word)] = '\0';
         if (spew == 1)
            printf("Receive connection from: %s\n", word);
         sprintf(ip, "%s", word);
         state = 1;
      }    
   }
}

//! Check connections
void
checkCom(void)
{
   memset(cmd, '\0', sizeof cmd);
   strcpy(cmd, "AT+CWLIF\r\n");
   cmd[strlen(cmd)]='\0';
   memset(text, '\0', sizeof text);
   write (tty_fd, cmd, strlen(cmd));           // send
   usleep ((10 + 1550) * 100);
   bool checkCom = 0;
   while(!checkCom)
   {
      while(strstr(text, "OK\r\n") == NULL)
      {
         if(read (tty_fd, buf, sizeof buf) > 0)
         {
            strcat(text, buf);
            if (debug == 1)
               printf(">>> %s <<<\n", buf);
            memset(buf, '\0', sizeof buf);
         }
      }
      if (spew == 1)
         printf("%s\n", text);
      if(strstr(text, "192.168.4.") != NULL)
      {
         checkCom = 1;
         questIp(text);
      }
      else
      {
         sleep(1);
         write (tty_fd, cmd, strlen(cmd));           // send
      }
      memset(text, '\0', sizeof text);
   }
}

//!Set multi connection mode
void
setMultiCom(void)
{
   memset(cmd, '\0', sizeof cmd);
   strcpy(cmd, "AT+CIPMUX=1\r\n");
   memset(text, '\0', sizeof text);
   write (tty_fd, cmd, strlen(cmd));           // send
   usleep ((10 + 1550) * 100);
   while(strstr(text, "OK\r\n") == NULL)
   {
      if(read (tty_fd, buf, sizeof buf) > 0)
      {
         strcat(text, buf);
         if (debug == 1)
            printf(">>> %s <<<\n", buf);
         memset(buf, '\0', sizeof buf);
      }      
   }
   if(spew == 1)
      printf("%s\n", text);
}

//!Close connetion if exist
void
closeCom(void)
{
   memset(cmd, '\0', sizeof cmd);
   strcpy(cmd, "AT+CIPCLOSE=4\r\n");
   memset(text, '\0', sizeof text);
   write (tty_fd, cmd, strlen(cmd));           // send
   usleep ((10 + 1550) * 100);
   while(strstr(text, "OK\r\n") == NULL && strstr(text, "link is not\r\n") == NULL && strstr(text, "MUX=0") == NULL)
   {
      if(read (tty_fd, buf, sizeof buf) > 0)
      {
         strcat(text, buf);
         if (debug == 1)
            printf(">>> %s <<<\n", buf);
         memset(buf, '\0', sizeof buf);
      }      
   }
   if(spew == 1)
      printf("%s\n", text);
}

//!Make connection for ip:192.168.4.2 port:32000
void
makeCom(void)
{
   memset(cmd, '\0', sizeof cmd);
   sprintf(cmd, "AT+CIPSTART=4,\"UDP\",\"%s\",32000\r\n", ip);
   memset(text, '\0', sizeof text);
   write (tty_fd, cmd, strlen(cmd));           // send
   usleep ((10 + 1550) * 100);
   while(strstr(text, "CONNECT\r\n\r\nOK\r\n") == NULL)
   {
      if(read (tty_fd, buf, sizeof buf) > 0)
      {
         strcat(text, buf);
         if (debug == 1)
            printf(">>> %s <<<\n", buf);
         if(strstr(text, "DNS") != NULL || strstr(text, "FAIL") != NULL || strstr(text, "IP ERROR") != NULL)
         {
            sleep(1);
            write (tty_fd, cmd, strlen(cmd));           // send
            usleep ((10 + 1550) * 100);
            memset(buf, '\0', sizeof buf);
            memset(text, '\0', sizeof buf);
         }
         memset(buf, '\0', sizeof buf);
      }
   }
   if(spew == 1)
      printf("%s\n", text);
}

//!Send info data
void
sendData(void)
{
   if(spew == 1)
      printf("Sending: [ %s ] to IP: [ %s ] ...\n\r",dataToSend, ip);
   memset(cmd, '\0', sizeof cmd);
   sprintf(cmd, "AT+CIPSEND=4,%d\r\n",strlen(dataToSend));
   memset(text, '\0', sizeof text);
   write (tty_fd, cmd, strlen(cmd));           // send
   usleep ((10 + 1550) * 100);
   while(strstr(text, ">") == NULL)
   {
      if(read (tty_fd, buf, sizeof buf) > 0)
      {
         strcat(text, buf);
         if (debug == 1)
            printf(">>> %s <<<\n", buf);
         memset(buf, '\0', sizeof buf);
      }      
   }
   if(spew == 1)
      printf("%s\n", text);

   memset(cmd, '\0', sizeof cmd);
   sprintf(cmd, "%s\r\n", dataToSend);
   memset(text, '\0', sizeof text);
   write (tty_fd, cmd, strlen(cmd));           // send
   usleep ((10 + 1550) * 100);
   while(strstr(text, "SEND OK") == NULL)
   {
      if(read (tty_fd, buf, sizeof buf) > 0)
      {
         strcat(text, buf);
         if (debug == 1)
            printf(">>> %s <<<\n", buf);
         memset(buf, '\0', sizeof buf);
      }      
   }
   if(spew == 1)
      printf("%s\n", text);
}

//!Receive data
void
receiveData(char endString[20])
{
   memset(text, '\0', sizeof text);
   while(strstr(text, endString) == NULL)
   {
      if(read (tty_fd, buf, sizeof buf) > 0)
      {
         strcat(text, buf);
         if (debug == 1)
            printf(">>> %s <<<\n", buf);
         memset(buf, '\0', sizeof buf);
      }      
   }
   if (spew == 1)
      printf("%s\n", text);

   //TODO filter data receive
   memset(textReceive, '\0', sizeof textReceive);
   int stepText = 0, stepTextReceive = 0;
   while(text[stepText] != ':')
      stepText++;
   stepText++;
   while(text[stepText] != '@')
   {
      textReceive[stepTextReceive] = text[stepText];
      stepTextReceive++;
      stepText++;
   }
}

int main(int argc, char**argv)
{
   inicSerial();  // set speed to 115,200 bps, 8n1 (no parity)

   sprintf(dataToSend, "Date: %d-%d-%d  Hour: %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

   char endString[20] = "@\r\n\r\nOK";

   if(argc > 1)
   {
      int numArgc = 1;
      while(numArgc < argc)
      {
         if(strcmp(argv[numArgc], "-s") == 0)
            spew = 1;
         if(strcmp(argv[numArgc], "-r") == 0)
            resetEsp();
         if(strcmp(argv[numArgc], "-d") == 0)
            debug = 1;
         numArgc++;
      }
   }
   
   closeCom();
   checkCom();
   setMultiCom();
   makeCom();
   sendData();

   while(strstr(textReceive, "#") == NULL)
   {
      receiveData(endString);
      
      if(strstr(textReceive, "#") == NULL)
         printf("%s\n", textReceive);
   }

   closeCom();

   return 0;
}