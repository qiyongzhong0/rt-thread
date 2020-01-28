/*
 * File      : guyu_test.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-12-30     qiyongzhong       first version
 */

#include <rtthread.h>
#include <sys/socket.h> 
#include <netdb.h>
#include <string.h>
#include <finsh.h>
#include <sys/time.h>

#define BUFSZ               2048

#ifndef GUYU_URL
#define GUYU_URL            "115.29.240.46"//"cloud.iotxx.com"
#endif

#ifndef GUYU_TCP_PORT
#define GUYU_TCP_PORT       9000
#endif

#ifndef GUYU_UDP_PORT
#define GUYU_UDP_PORT       6000
#endif

#ifndef GUYU_REG_MSG
#define GUYU_REG_MSG        "ep=qiyongzhong0002&pw=952011"
#endif

void guyu_test(int argc, char **argv)
{
    char *recv_data;
    struct hostent *host;
    struct sockaddr_in server_addr;
    char *url;
    int sock, bytes_received;
    int port;
    int type;

    if (argc < 2)
    {
        rt_kprintf("Usage: guyu_test <tcp/udp> [host] [port]\n");
        rt_kprintf("Like: guyu_test tcp 122.114.122.174 45678\n");
        return ;
    }

    if (strcmp(argv[1], "tcp") == 0)
    {
        type = SOCK_STREAM;
        port = GUYU_TCP_PORT;
    }
    else if (strcmp(argv[1], "udp") == 0)
    {
        type = SOCK_DGRAM;
        port = GUYU_UDP_PORT;
    }
    else
    {
        rt_kprintf("Socket type error!\n");
        return;
    }

    if (argc > 2)
    {
        url = argv[2];
    }
    else
    {
        url = GUYU_URL;
    }
    
    if (argc > 3)
    {
        port = atoi(argv[3]);
    }

    recv_data = rt_malloc(BUFSZ+1);
    if (recv_data == RT_NULL)
    {
        rt_kprintf("No memory\n");
        return;
    }

    if ((sock = socket(AF_INET, type, 0)) == -1)
    {
        rt_kprintf("Socket error\n");
        rt_free(recv_data);
        return;
    }
    
    host = gethostbyname(url);
    if (host == RT_NULL)
    {
        rt_kprintf("Host get error!\n");
        rt_free(recv_data);
        closesocket(sock);
        return;
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) != RT_EOK)
    {
        rt_kprintf("Connect fail!\n");
        rt_free(recv_data);
        closesocket(sock);
        return;
    }

    rt_kprintf("Connect successful\n");
    
    {
        const struct timeval tv = {5, 0};
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    }
    
    while(1)
    {
        const char reg_msg[] = GUYU_REG_MSG;
        rt_kprintf("Send reg data to guyu cloud!\n");
        send(sock, reg_msg, strlen(reg_msg), 0);
        bytes_received = recv(sock, recv_data, BUFSZ, 0);
        if (bytes_received > 0)//recv ack
        {
            rt_kprintf("Connect guyu cloud success!\n");
            break;
        }
    }

    while (1)
    {
        bytes_received = recv(sock, recv_data, BUFSZ, 0);
        if (bytes_received < 0)
        {
            rt_kprintf("Receive timeout!\n");
            continue;
        }
        if (bytes_received == 0)
        {
            rt_kprintf("Receive error!\n");
            rt_free(recv_data);
            closesocket(sock);
            return;
        }
        
        recv_data[bytes_received] = '\0';

        if (bytes_received == 1)
        {
            if (strncmp(recv_data, "q", 1) == 0 || strncmp(recv_data, "Q", 1) == 0)
            {
                rt_kprintf("\nGot a 'q' or 'Q',close the socket.\r\n");
                rt_free(recv_data);
                closesocket(sock);
                return;
            }
            if (strncmp(recv_data, "f", 1) == 0 || strncmp(recv_data, "F", 1) == 0)
            {
                int i,j;
                int pkgs_total = 20;
                int pkgs_blk_total = 40;//5 bytes per block
                
                rt_kprintf("\nFast send %d datas %d times", pkgs_blk_total*5, pkgs_total);
                for (i=0; i<pkgs_total; i++)
                {
                    for (j=0; j<pkgs_blk_total; j++)
                    {
                        rt_sprintf(recv_data+j*5, "%02d%02d-", i+1, j+1);
                    }
                    send(sock, recv_data, pkgs_blk_total * 5, 0);
                }
                continue;
            }
            if (strncmp(recv_data, "l", 1) == 0 || strncmp(recv_data, "L", 1) == 0)
            {
                int i;
                rt_kprintf("\nSend a big block datas");
                for (i=0; i<BUFSZ/4; i++)
                { 
                    rt_sprintf(recv_data+i*4, "%03d-", i+1);
                }
                send(sock, recv_data, BUFSZ, 0);
                continue;
            }
        }

        rt_kprintf("\nReceived %d datas = %s ", bytes_received, recv_data);
        send(sock, recv_data, bytes_received, 0);
    }
}

MSH_CMD_EXPORT(guyu_test, GuYu Cloud tcp/udp test);

