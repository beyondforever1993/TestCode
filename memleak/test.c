#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <time.h> 
#include "cmn.h"
#include "convert_fun.h"


static  fd_set s_readfd;
static unsigned int loop = 0;
int main(int argc, char *argv[])
{
    int           listenfd    = 0, connfd = 0;
    int           maxfd       = 0;
    struct        sockaddr_in serv_addr;
	hrc_dat_buf_t hrc_dat_buf = { 0, 0 };
    unsigned      char buff[4096*2];
    int           count_bytes = 0;
    fd_set        rfds;
    struct        timeval tv;
    int           retval;
	struct in_addr addr = { 0 };

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    memset(buff, 0, sizeof(buff)); 

    if ( inet_pton( AF_INET, "192.168.1.3", &addr ) == 1 )
    {
        serv_addr.sin_addr.s_addr = addr.s_addr; 
    }
    serv_addr.sin_family = AF_INET;
    //serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(50000); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10); 

    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
    tv.tv_sec = 0;
    tv.tv_usec = 40000;
    FD_ZERO(&rfds);
    FD_SET(connfd, &rfds);

    maxfd = connfd;
    s_readfd = rfds;

    while(1)
    {
        retval = select(maxfd+1, &rfds, NULL, NULL, &tv);
        if (retval > 0)
        {
            if (FD_ISSET(connfd, &rfds))
            {
                count_bytes = recv(connfd, buff, sizeof(buff), 0);
                if (count_bytes > 0)
                {
                    construct_hrc_buf( (char *)buff, count_bytes, &hrc_dat_buf );
                }

            }

        }
        rfds = s_readfd;
        printf("1111111111111\n");
        if (loop++ >= 0xfffffffe)
        {
            goto ret;
        }
    
     }

ret:
    close(connfd);

    return 0;
}
