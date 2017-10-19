#include "udpser.h"
#include "print.h"
/*udp服务器进程*/

int type = -1;
char name[16] = {"\0"};
char buf[BUFSIZ] =  {"\0"};
void unpacking(char *recv_buf);

int main(void)
{
  int udps_fd = -1;

/*1.建立套接字 */
if( (udps_fd = socket(AF_INET, SOCK_DGRAM, 0))< 0) { //基于互联网的UDP
	perror("udp socket");
	exit(1);
}

/* 允许地址快速重用 */
int b_reuse = 1;
setsockopt(udps_fd, SOL_SOCKET, SO_REUSEADDR, &b_reuse, sizeof(int));

/*2.绑定:IP地址和端口号 */
struct sockaddr_in sin;

bzero(&sin, sizeof(sin));
sin.sin_family = AF_INET;
sin.sin_port = htons(SERVER_UDP_PORT);  //16位网络字节序的端口号
sin.sin_addr.s_addr = htonl(INADDR_ANY); /* 绑定在本机任意的IP地址上*/

if( bind(udps_fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
	perror("bind udp");
	exit(1);
}
/*3.阻塞等待客户端数据 */	
char udp_recvfrom_buf[BUFSIZ];
int ret = -1;
while(1) {
	bzero(udp_recvfrom_buf, BUFSIZ);
	do {
		ret = recvfrom(udps_fd,udp_recvfrom_buf, BUFSIZ-1, 0,NULL,NULL);
	}while(ret < 0 && EINTR ==errno);
	/*json解包*/
	unpacking(udp_recvfrom_buf);
	if(3 ==type)
		printf(RED"**%s say**:%s\n"NONE,name,buf);
	if(7 == type)
		printf(CYAN"**%s say us**:%s\n"NONE,name,buf);
}
close(udps_fd);
return 0;
}
/*json解包*/
void unpacking(char *recv_buf)
{
	int i,state = OCHAT;
	char *tmp = (char*)malloc(1024);
	bzero(buf,BUFSIZ);
	bzero(name,16);

	struct json_object *fat,*json_type,*json_name,*json_buf;
	fat = json_tokener_parse(recv_buf);
	for(i=0;i<2;i++)
	{
		switch(state)
		{
			case OCHAT :
				json_type = json_object_object_get(fat,"type");
				type = json_object_get_int(json_type);

				if(type == 3)	//接受私聊信息
					state = RCHAT;
				else if(type == 7)	//接受群聊信息
					state = LCHAT;
				break;	
			case RCHAT:	//私聊
				json_name = json_object_object_get(fat,"name");
				tmp = (char*)json_object_get_string(json_name);
				strcpy(name,tmp);
				json_buf = json_object_object_get(fat,"buf");
				tmp = (char*)json_object_get_string(json_buf);
				strcpy(buf,tmp);
				break;
			case LCHAT:	//群聊
				json_name = json_object_object_get(fat,"name");
				tmp = (char*)json_object_get_string(json_name);
				strcpy(name,tmp);
				json_buf = json_object_object_get(fat,"buf");
				tmp = (char*)json_object_get_string(json_buf);
				strcpy(buf,tmp);
				break;

			default:
				break;
		}
	}
}
