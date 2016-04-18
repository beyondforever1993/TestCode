#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/types.h>
#include <regex.h>

#define LEN                   128
#define IP_LEN                16
#define ETH0_INTERFACE_NAME   "eth0"
#define WLAN0_INTERFACE_NAME  "wlan0"
#define PPP0_INTERFACE_NAME   "ppp0"
#define GATEWAY_FILE          "/etc/network/gateway-domain"
#define dprintf(fmt,args...)  do {printf("[%s:%d] "fmt, __FUNCTION__, __LINE__, ##args);} while(0)

static int s_init_eth0_default_route = 0;

static int get_eth0_gateway(char *gateway, char *domain)
{
    FILE *fp       = NULL;
    char buff[LEN] = {0}; int  size      = 0;

    if (!gateway)
    {
        dprintf("gateway is null.\n");
        return -1;
    }

    if ((fp = fopen(GATEWAY_FILE, "r")))
    {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        if(size == 0)
        {
            fclose(fp);
            return -1;
        }
        fread(buff, size, 1, fp);
        sscanf(buff, "GATEWAY=%s\nDOMAIN=%s\n", gateway, domain);
        /*
         *dprintf("GATEWAY:%s, DOMAIN:%s\n", gateway, domain);
         */
        fclose(fp);
        if (strlen(gateway) == 0)
            return -1;
        else
            return 0;
    }

    return -1;
}

static int is_valid_ip(char *ip_str)
{
    unsigned char ip_bytes[4] = {0};
    int      cnt              = 0;
    unsigned i;
    char     *ptr             = NULL;

    if (!ip_str || strlen(ip_str) == 0)
    {
        dprintf("ip_str is null.");
        return 0;
    }

    do
    {
        ip_bytes[cnt++] = strtol(ip_str, &ptr, 10);
    } while ((ptr != NULL) && (*ptr != 0) && (ip_str = ++ptr));

    if (cnt == 4)
    {
        if (ip_bytes[0] == 169 && ip_bytes[1] == 254)
        {
            return 0;
        }

        if (ip_bytes[0] == 0
                && ip_bytes[1] == 0
                && ip_bytes[2] == 0
                && ip_bytes[3] == 0)
        {
            return 0;
        }

        
        for (i = 0; i < sizeof(ip_bytes)/sizeof(ip_bytes[0]); i++)
        {
            if (ip_bytes[i] > 255 || ip_bytes[i] < 0)
                break;
        }

        if (i == 4)
        {
            /*
             *for (i = 0; i < sizeof(ip_bytes)/sizeof(ip_bytes[0]); i++)
             *{
             *    printf("%03d ", ip_bytes[i]);
             *}
             *printf("\n");
             */
            return 1;
        }
        else
        {
            return 0;
        }
    }

    return 0;
}

static int is_in_same_subnet(char *ip_str1, char *ip_str2)
{
    char ip_addr1[LEN] = {0};
    char ip_addr2[LEN] = {0};
    char *ptr          = NULL;

    if (!ip_str1 || !ip_str2)
    {
        dprintf("ip_str1 or ip_str2 is null.\n");
        return 0;
    }

    snprintf(ip_addr1, sizeof(ip_addr1), "%s", ip_str1);
    snprintf(ip_addr2, sizeof(ip_addr1), "%s", ip_str2);

    if ((ptr = strrchr(ip_addr1, '.')))
    {
        *ptr = 0;
    }
    if ((ptr = strrchr(ip_addr2, '.')))
    {
        *ptr = 0;
    }

    if (strncmp(ip_addr1, ip_addr2, strlen(ip_addr1)) == 0)
        return 1;

    return 0;
}

static int get_netdevice_interface_info(struct ifreq *ifr, const char *interface_name, int request)
{
    int    ret = 0;
    int    fd;
    struct ifreq if_info;

    if (!ifr || !interface_name)
    {
        dprintf("ifro or interface_name is null\n");
        return -1;
    }

    memset(&if_info, 0, sizeof(struct ifreq));
    fd = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);   
    snprintf(if_info.ifr_name, sizeof(if_info.ifr_name), "%s", interface_name);  
    
    if (ioctl(fd, request, &if_info) < 0) {  
        perror("ioctl");  
        ret = -1;
        return ret;
    }     
    memcpy(ifr, &if_info, sizeof(struct ifreq));
    ret = 0;

    return ret;
}

static int is_interface_up(char *interface_name)
{
    struct ifreq ifr;

    if (!interface_name)
    {
        dprintf("interface_name is null.\n");
        return 0;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    if (get_netdevice_interface_info(&ifr, interface_name, SIOCGIFFLAGS))
    {
        dprintf("get_netdevice_interface_info fail.\n");
        return 0;
    }

    if (ifr.ifr_flags & IFF_UP)
        return 1;

    return 0;
}

static int is_interface_ip_addr_valid(char *interface_name, char *ip_addr)
{
    struct ifreq ifr;
    struct sockaddr_in dev_ip;
    char   ip_str[IP_LEN] = {0};
    int    ret            = 0;

    if (!interface_name)
    {
        dprintf("interface_name is null.\n");
        return 0;
    }

    memset(&dev_ip, 0, sizeof(struct sockaddr_in));
    memset(&ifr, 0, sizeof(struct ifreq));
    if (get_netdevice_interface_info(&ifr, interface_name, SIOCGIFADDR))
    {
        dprintf("get_netdevice_interface_info fail.\n");
        return 0;
    }

    memcpy(&dev_ip, &ifr.ifr_addr, sizeof(struct sockaddr));
    dprintf("%s\n", inet_ntoa(dev_ip.sin_addr));
    snprintf(ip_str, sizeof(ip_str), "%s", inet_ntoa(dev_ip.sin_addr));
    if ((ret = is_valid_ip(ip_str)))
    {
        if (ip_addr)
        {
            snprintf(ip_addr, IP_LEN, "%s", ip_str);
        }
        return ret;
    }

    return 0;
}

static int is_internet_work_on_interface(char *interface_name, char *ip_addr)
{
    FILE *fp        = NULL;
    char cmd[64]    = {0};
    char *ptr       = NULL;
    char *content_p = NULL;

    if (!interface_name || !ip_addr)
    {
        dprintf("interface_name is null.\n");
        return 0;
    }

    snprintf(cmd, sizeof(cmd), "ping -w 4 -I %s %s", interface_name, ip_addr);
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
                && *(content_p - 2) == '0'
                && *(content_p - 3) == '1')
        {
            dprintf("ptr:%s", ptr);
            free(ptr);
            fclose(fp);
            return 0;
        }
        else
        {
            dprintf("ptr:%s", ptr);
            free(ptr);
            fclose(fp);
            return 1;
        }
    
    }

    return 0;
}

/*
 *int bind_sock_to_interface(int fd, char *interface_name)
 *{
 *    struct ifreq ifr;
 *
 *    if (!interface_name || fd < 0)
 *    {
 *        dprintf("interface_name is null or fd is invalid\n");
 *        return -1;
 *    }
 *
 *    memset(&ifr, 0, sizeof(struct ifreq));
 *    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", interface_name);
 *    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
 *        perror("setsockopt");
 *        return -1;
 *    }
 *
 *    return 0;
 *}
 *
 */

static int is_exist_eth0_default_route_entry(void)
{
    char route_table[BUFSIZ] = {0};
    FILE *fp = NULL;
    regmatch_t pmatch[2];
    regex_t preg;

    if ((fp = popen("netstat -rn", "r")))
    {
        fread(route_table, sizeof(route_table), 1, fp);
        dprintf("%s", route_table);
        if (regcomp(&preg, "0.0.0.0\\s192.168.254.1\\s0.0.0.0\\sUG\\s0\\s0\\s0\\seth0", REG_ICASE) == 0)
        {
            regexec(&preg, route_table, 1, pmatch, 0);
            regfree(&preg);
        }
        else 
        {
            regfree(&preg);
        }

        fclose(fp);
    }

        
    return 0;
}

static int is_exist_ppp0_default_route_entry(void)
{

    return 0;
}

static int add_eth0_default_route_entry(const char *eth0_gw)
{
    char cmd[LEN] = {0};
  
    if (!eth0_gw)
    {
        dprintf("eth0_gw is null.\n");
        return -1;
    }

    snprintf(cmd, sizeof(cmd), "route add -net 0.0.0.0 netmask 0.0.0.0 gw %s dev eth0", eth0_gw);
    system("cmd");
    s_init_eth0_default_route = 1;
    return 0;
}

static int del_eth0_default_route_entry(const char *eth0_gw)
{
    char cmd[LEN] = {0};
  
    if (!eth0_gw)
    {
        dprintf("eth0_gw is null.\n");
        return -1;
    }

    snprintf(cmd, sizeof(cmd), "route del -net 0.0.0.0 netmask 0.0.0.0 gw %s dev eth0", eth0_gw);
    system("cmd");
    s_init_eth0_default_route = 0;
    return 0;
}

static int add_ppp0_default_route_entry()
{
    system("route add -net 0.0.0.0 netmask 0.0.0.0 dev ppp0");
    return 0;
}

static int del_ppp0_default_route_entry()
{
    system("route del -net 0.0.0.0 netmask 0.0.0.0 dev ppp0");
    return 0;
}

int main(int argc, char *argv[])
{
    
    char gateway[IP_LEN] = {0};
    char eth0_ip[IP_LEN] = {0};

    dprintf("interface eth0 is %s\n", is_interface_up("eth0")?"up":"down");
    dprintf("interface ppp0 is %s\n", is_interface_up("ppp0")?"up":"down");
    dprintf("interface wlan is %s\n", is_interface_up("wlan0")?"up":"down");

    dprintf("eth0 has %s ip\n", is_interface_ip_addr_valid("eth0", NULL)?"valid":"invalid");
    dprintf("ppp0 has %s ip\n", is_interface_ip_addr_valid("ppp0", NULL)?"valid":"invalid");
    dprintf("wlan0 has %s ip\n", is_interface_ip_addr_valid("wlan0", NULL)?"valid":"invalid");

    dprintf("get eth0 %s\n", get_eth0_gateway(gateway, NULL)?"fail":"success");
    if (argc < 2)
    {
        dprintf("Usage: %s ip or domain name\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (get_eth0_gateway(gateway, NULL))
    {
        dprintf("Cann't find eth0's gateway\n");
        exit(EXIT_FAILURE);
    }
    /* 1. 3g is connected to network and ethernet is connected to network
     * 2. 3g is connected to network and ethernet isn't connected to network
     * 3. 3g isn't connected to network and ethernet is connected to network
     * 4. 3g isn't connected to network and ethernet isn't connected to network*/
    while (1)
    {
        sleep(30);
    
        if (is_interface_up(ETH0_INTERFACE_NAME) == 0)
        {
            //Ethernet is down, default network is 3G
            continue;
        }
        else
        {
            if (is_interface_ip_addr_valid(ETH0_INTERFACE_NAME, eth0_ip)
                    && is_valid_ip(gateway) 
                    && is_in_same_subnet(gateway, eth0_ip))
            {
                is_exist_eth0_default_route_entry();

                if (s_init_eth0_default_route  == 0)
                {
                    add_eth0_default_route_entry(gateway);
                }
            
                if (is_internet_work_on_interface(ETH0_INTERFACE_NAME, argv[1]))
                {
                    if (is_interface_up(PPP0_INTERFACE_NAME)
                            && is_interface_ip_addr_valid(PPP0_INTERFACE_NAME, NULL))
                    {
                        del_ppp0_default_route_entry();
                    }
                }
                else 
                {
                    if (is_interface_up(PPP0_INTERFACE_NAME)
                            && is_interface_ip_addr_valid(PPP0_INTERFACE_NAME, NULL))
                    {
                        add_ppp0_default_route_entry();
                    }
                
                }
            }
        
        }
    }

    return 0;

}
