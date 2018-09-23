#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>	  
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int32_t init_tcp_server(uint16_t port)
{
    struct sockaddr_in serv_addr;
    int32_t sock = 0;
    int32_t opt = true;

    bzero(&serv_addr, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    sock = socket(AF_INET, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
    if (sock < 0)
        return -1;

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));

    if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0)
        return -1;

    if (listen(sock, 1) < 0)
        return -1;


    return sock;
}

int init_tcp_client(const int8_t *ip, uint16_t port)
{
    struct sockaddr_in peer_addr;
    int sock = 0;


    if (!ip)
        return -1;

    bzero(&peer_addr, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port   = htons(port);
    peer_addr.sin_addr.s_addr = inet_addr(ip);
    sock = socket(AF_INET, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
    if (sock < 0)
        return -1;

    if (connect(sock, (const struct sockaddr *)&peer_addr, sizeof(struct sockaddr)) < 0 
            && errno != EINPROGRESS)
        return -1;
    
    return sock;
}

/*
 *int main()
 *{
 *    int32_t ret = 0;
 *    fd_set readfds, writefds;
 *
 *    FD_ZERO(&writefds);
 *    FD_ZERO(&readfds);
 *
 *    FD_SET(0, &readfds);
 *    FD_SET(1, &writefds);
 *
 *    while (true)
 *    {
 *        ret = select(3, NULL, &writefds, NULL, NULL);
 *        if (ret > 0)
 *        {
 *            if (FD_ISSET(1, &writefds))
 *            {
 *                printf("1 is writable\n");
 *            }
 *        }
 *    }
 *
 *    return 0;
 *}
 */

int main(int argc, char *argv[])
{
   int epoll_fd;
   int monitor_fd = 0;
   int8_t buf[1024] = {0};
   struct epoll_event events[1024], event;

   bzero(&event, sizeof(event));
   epoll_fd = epoll_create(1);
   if (epoll_fd < 0)
       return -1;

   if (strcmp(argv[1], "client") == 0)
   {
       monitor_fd = init_tcp_client("127.0.0.1", 9999);
       if (monitor_fd < 0)
           return -1;

       event.events = EPOLLIN  | EPOLLOUT| EPOLLPRI | EPOLLRDHUP;
       event.data.fd = monitor_fd;

       epoll_ctl(epoll_fd, EPOLL_CTL_ADD, monitor_fd, &event); 

       while (true)
       {
           if (epoll_wait(epoll_fd, &events[0], 1024, 100) > 0)
           {
               if (events[0].events & EPOLLIN)
               {
                   recv(monitor_fd, buf, sizeof(buf), 0);
                   //printf("%s\n", buf);
                   //memset(buf, 0, sizeof(buf));
               }
               
               if (events[0].events & EPOLLRDHUP)
               {
                   printf("Recv EPOLLRDHUP\n");
                   //epoll_ctl(epoll_fd, EPOLL_CTL_DEL, monitor_fd, NULL);
                   //close(monitor_fd);
                   //send(monitor_fd, "123", 3, 0);
                   //recv(monitor_fd, buf, sizeof(buf), 0);
               }
               if (events[0].events & EPOLLHUP)
               {
                   printf("Recv EPOLLHUP\n");
               }
               if (events[0].events & EPOLLERR)
               {
                   printf("Recv EPOLLERR\n");
               }
           }
       }

   }
   else if (strcmp(argv[1], "server") == 0)
   {
       int32_t accept_fd = -1; 
       int32_t addrlen = sizeof(struct sockaddr);
       struct sockaddr_in remote_addr;

       bzero(&remote_addr, sizeof(struct sockaddr_in));

       monitor_fd = init_tcp_server(9999);
       if (monitor_fd < 0)
           return -1;

       event.events = EPOLLIN|EPOLLPRI|EPOLLRDHUP;
       event.data.fd = monitor_fd;
       epoll_ctl(epoll_fd, EPOLL_CTL_ADD, monitor_fd, &event);

       while (true)
       {
           if (epoll_wait(epoll_fd, &events[0], 1024, 100) > 0)
           {
               if (events[0].events & EPOLLIN)
               {
                   accept_fd = accept4(monitor_fd, (struct sockaddr *)&remote_addr, &addrlen,
                           SOCK_NONBLOCK|SOCK_CLOEXEC);
                   printf("connect %d\n", accept_fd);
                   sleep(5);
                   shutdown(accept_fd, SHUT_RDWR);
                   //close(accept_fd);
               }
               if (events[0].events & EPOLLRDHUP)
               {
                   printf("Recv EPOLLRDHUP\n");
               }
               if (events[0].events & EPOLLHUP)
               {
                   printf("Recv EPOLLHUP\n");
               }
               if (events[0].events & EPOLLERR)
               {
                   printf("Recv EPOLLERR\n");
               }
           }
       }

   }

  

   return 0;
}
