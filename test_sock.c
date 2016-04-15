#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define ETH_INTERFACE_NAME    "eth0"
#define WLAN_INTERFACE_NAME   "wlan0"
#define MOBILE_INTERFACE_NAME "ppp0"

static int get_netdevice_interface_info(struct ifreq *ifr, const char *interface_name, int request)
{
    int ret = 0; 
    int fd;
    struct ifreq if_info;

    if (!ifr || !interface_name)
    {
        printf("ifro or interface_name is null\n");
        return -1;
    }

    memset(&if_info, 0, sizeof(struct ifreq));
    fd = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);   
    snprintf(if_info.ifr_name, sizeof(if_info.ifr_name), "%s", interface_name);  
    
    if (ioctl(fd, request, &if_info) < 0) {  
        perror("ioctl");  
        ret = -1;
    }     
    memcpy(ifr, &if_info, sizeof(struct ifreq));
    ret = 0;

    return ret;
}

int is_interface_up(char *interface_name)
{
    struct ifreq ifr;

    if (!interface_name)
    {
        printf("interface_name is null.\n");
        return 0;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    if (get_netdevice_interface_info(&ifr, interface_name, SIOCGIFFLAGS))
    {
        printf("get_netdevice_interface_info fail.\n");
        return 0;
    }

    if (ifr.ifr_flags & IFF_UP)
        return 1;

    return 0;
}

int is_interface_ip_addr_valid(char *interface_name)
{
    struct ifreq ifr;
    struct sockaddr_in dev_ip;

    if (!interface_name)
    {
        printf("interface_name is null.\n");
        return 0;
    }

    memset(&dev_ip, 0, sizeof(struct sockaddr_in));
    memset(&ifr, 0, sizeof(struct ifreq));
    if (get_netdevice_interface_info(&ifr, interface_name, SIOCGIFADDR))
    {
        printf("get_netdevice_interface_info fail.\n");
        return 0;
    }

    memcpy(&dev_ip, &ifr.ifr_addr, sizeof(struct sockaddr));
    printf("ipaddr:%s\n", inet_ntoa(dev_ip.sin_addr));

    return 1;
}

int is_internet_work_on_interface(char *interface_name, char *ip_addr)
{
    FILE *fp        = NULL;
    char cmd[64]    = {0};
    char *ptr       = NULL;
    char *content_p = NULL;

    if (!interface_name || !ip_addr)
    {
        printf("interface_name is null.\n");
        return 0;
    }

    snprintf(cmd, sizeof(cmd), "ping -w 2 -I %s %s", interface_name, ip_addr);
    fp = popen(cmd, "r");
    if (!fp)
    {
        perror("popen");
        return 0;
    }

    ptr = calloc(BUFSIZ, 1);
    if (!ptr)
    {
        perror("calloc");
        return 0;
    }

    fread(ptr, BUFSIZ, 1, fp);
    if ((content_p = strchr(ptr, '%')))
    {
        if (*(content_p - 1) == '0'
                || *(content_p - 2) == '0'
                || *(content_p - 3) == '1')
        {
            printf("ptr:%s", ptr);
            free(ptr);
            return 0;
        }
        else
        {
            printf("ptr:%s", ptr);
            free(ptr);
            return 1;
        }
    
    }

    return 0;
}

int bind_sock_to_interface(int fd, char *interface_name)
{
    struct ifreq ifr;

    if (!interface_name || fd < 0)
    {
        printf("interface_name is null or fd is invalid\n");
        return -1;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", interface_name);
    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
        perror("setsockopt");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    /*
     *if (argc != 2)
     *{
     *    printf("Usage: %s ip", argv[0]);
     *    return 0;
     *}
     */

    //is_internet_work_on_interface("vpn0", argv[1]);
    printf("interface eth0 is %s\n", is_interface_up("eth0")?"up":"down");
    printf("interface ppp0 is %s\n", is_interface_up("ppp0")?"up":"down");
    printf("interface wlan is %s\n", is_interface_up("wlan0")?"up":"down");

    is_interface_ip_addr_valid("eth0");
    is_interface_ip_addr_valid("ppp0");
    is_interface_ip_addr_valid("wlan0");
    return 0;

}
/*
 *int main(int argc, char *argv[])
 *{
 *    struct in_addr dst;
 *    int fd, i;
 *    struct sockaddr_in client;
 *    struct ifreq ifr;
 *
 *    if (argc != 2)
 *    {
 *        printf("usage: %s ip\n", argv[0]);
 *        exit(-1);
 *    }
 *
 *
 *    memset(&client, 0, sizeof(client));
 *    memset(&ifr, 0, sizeof(struct ifreq));
 *    if (inet_aton(argv[1], &dst) == 0) {
 *
 *        perror("inet_aton");
 *        printf("%s isn't a valid IP address\n", argv[1]);
 *        return 1;
 *    }
 *
 *    client.sin_family = AF_INET;
 *    client.sin_port  = htons(2102);
 *    client.sin_addr  = dst;
 *    
 *
 *    fd = socket(AF_INET, SOCK_STREAM, 0);
 *    if (fd < 0)
 *    {
 *        perror("socket");
 *        return -1;
 *    }
 *
 *    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "eth0");
 *    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
 *        perror("setsockopt");
 *        return -1;
 *    }
 *
 *    for (i = 0; i < 100; i++)
 *    {
 *        if (connect(fd, (struct sockaddr *)&client, sizeof(client)) < 0)
 *        {
 *            perror("connect");
 *            sleep(1);
 *        }
 *        else
 *            sleep(100);
 *    }
 *
 *    close(fd);
 *    return 0;
 *}
 *
 *
 */
