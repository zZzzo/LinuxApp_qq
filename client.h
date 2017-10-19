#ifndef _CLIENT_H_
#define _CLIENT_H_
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include </usr/local/include/json/json.h>

#define SERVER_BROAD_IP "192.168.7.255"
#define SERVER_PORT 5001
#define SERVER_UDP_PORT 5002

#define INIT 0
#define RGST 1
#define LGIN 2
#define PCHAT 3
#define GCHAT 4
#define HCHAT 5
#define JCHAT 6
#define KCHAT 7

extern int menu_main(void);
extern int menu_regit();
extern bool login();
extern void regit();
extern void sec_regit();
extern void bet_regit();
extern char* packing();
extern bool unpacking(char *recv_buf);
extern void* udp_server(void* arg);
extern void* udp_client(void* arg);
extern int menu_sec(void);
extern void sec_choise(pthread_t* tid_cli,pthread_t* tid_board);
extern void* udp_board(void *arg);


#endif
