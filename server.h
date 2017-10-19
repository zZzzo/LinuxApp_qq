#ifndef _SERVER_H_
#define _SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/reboot.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdbool.h>
#include "list.h"
#include <sqlite3.h>
#include <sys/stat.h>
#include <fcntl.h>
#include </usr/local/include/json/json.h>


#define SERVER_PORT 5001		//tcp端口号
#define SERVER_UDP_PORT 5003	//udp端口号
#define SERVER_IP "192.168.7.128"	//server ip
#define LISTEND_NUM 5			//监听队列最大值
#define MAX_NAME_LEN 16			//昵称
#define MAX_PSW_LEN 16			//密码
#define CLI_IP_LEN 16			//ip长度
#define SQL_SIZE 128
#define FD_SIZE	1024

#define INIT 0
#define RGST 1
#define LOGIN 2
#define PCHAT 3
#define ONLINE 4
#define VRGST 5
#define OFF	6
typedef struct knlist{			//链表信息
	int qq;
	char cli_ip[CLI_IP_LEN];
	char name[MAX_NAME_LEN];
	struct list_head list;
}knlist;

typedef struct cli_info{		//客户端信息
	int fd;
	struct sockaddr_in cin;
}cli_info;

typedef struct sqlite_data{		//数据库内容
	char name[16];
	int qq;
	char password[16];
}sql_data,*sql_pdata;


extern int socket_init(void);				//套接字初始化
extern void *cli_deal(void *);				//线程函数
extern char *packing();						//封包
extern void unpacking(char *);				//拆包
extern knlist *list_init(void);				//初始化链表
extern void list_addqq(char [],int ,char [],knlist *);//添加到链表(上线)
extern void list_delqq(char []);			//从链表删除(下线)
extern void show(knlist *);					//遍历链表
extern bool search_list_qq(knlist *);		//查找在线用户

/******************************************/
/****************数据库********************/
/******************************************/

extern bool creat_table(sqlite3 *db);							//建表
extern bool insert(sqlite3 *db,sql_pdata sd,char *sql_name);
extern void show_table(sqlite3 *db,char *sql_name);
extern bool input(sqlite3 *db,sql_pdata sd,char *sql_name);
extern int search_table(sqlite3 *db,char *sql_name,int qq,char* password);
extern bool search_sql_qq(sqlite3 *db,char *sql_name,int qq);
extern int show_menu(void);
extern bool update_data(sqlite3 *db,char *sql_name);
extern void delete_table(sqlite3 *db,char *sql_name);

#endif
