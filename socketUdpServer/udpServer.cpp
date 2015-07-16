
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

time_t t = time(NULL);
struct tm tm = *localtime(&t);

int main(int argc, char**argv)
{
   int sockfd,n;
   struct sockaddr_in servaddr,cliaddr;
   socklen_t len;
   char mesg[1000];

   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
   servaddr.sin_port=htons(32000);
   bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

   for (;;)
   {
      len = sizeof(cliaddr);
      n = recvfrom(sockfd,mesg,1000,0,(struct sockaddr *)&cliaddr,&len);
      mesg[n] = 0;
      printf("Received the following: ");
      printf("%s\n",mesg);
      char text[100];
      memset(text, '\0', sizeof text);
      int a = 0;
      while(a < 211)
      {
	  memset(text, '\0', sizeof text);
          t = time(NULL);
          tm = *localtime(&t);
          sprintf(text, "Date: %d-%d-%d  Hour: %d:%d:%d@\r\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
          sendto(sockfd,text,strlen(text),0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
          usleep(300 * 100);
          a++;
      }
      memset(text, '\0', sizeof text);
      strcpy(text, "#@\r\n");
      sendto(sockfd,text,strlen(text),0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));      
   }
}
