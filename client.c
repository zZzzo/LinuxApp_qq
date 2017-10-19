#include "client.h"
#include "print.h"

int fd = -1;
int qq,type = -1;
char name[16] = {"\0"};
char buf[BUFSIZ] = {"\0"};
char password[16] = {'\0'};
char my_name[16] = {"\0"};
pthread_mutex_t mutex;

void usage(char *s)
{
	printf("Usage:\n\t%s serv_ip\n",s);
	printf("\tsverv_ip: the server internet ip address.\n");
}


int main(int argc, char *argv[])
{
	if( argc != 2 ) {
		usage(argv[0]);
		exit(1);
	}

	int choise,ret;
	char recv_str[1024];
	pthread_t tid_cli,tid_board;

	pthread_mutex_init(&mutex,NULL);//线程锁
	/*绑定套接字*/
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(1);
	}
	/*绑定服务器*/
	struct sockaddr_in sin;
	bzero(&sin,sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(SERVER_PORT);
	sin.sin_addr.s_addr = inet_addr(argv[1]);

	if(connect(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		perror("connect failed");
		exit(1);
	}
	while(1)
	{
		choise = menu_main();
		switch (choise){
			case 1:
				ret = login();
				if(ret){
					system("gnome-terminal -e ./udp_server");//udp服务器进程
					sec_choise(&tid_cli,&tid_board);

				}
				else
				{	
					printf("按回车继续：");
					while(getchar() != '\n');
				}
				break;
			case 2:
				sec_regit();	break;
			default:	break;
		}
	}
	return 0;
}


int menu_main(void)
{
	int ret,choise;

	system("clear");
	printf(RED"*************************\n"NONE);
	printf(RED"*\t\t\t*\n");
	printf("*"NONE"    1.登录   2.注册    "RED"*\n");
	printf("*\t\t\t*\n");
	printf("*************************\n"NONE);

	while(1)
	{
		ret = 0;
		printf("请选择：");
		ret = scanf("%d",&choise);
		while(getchar() != '\n');
		if(ret == 0 || (choise!=1 && choise!=2))
		{
			printf("输入有误！\n\n");
			continue;
		}
		else
			break;
	}
	return choise;
}

bool login()
{
	int ret = -1;
	char *send_buf;

	char recv_string[1024];

	while(1)
	{
		ret = 0;
		printf("请输入QQ号码：");
		ret = scanf("%d",&qq);
		while(getchar() != '\n');
		if(ret == 0)
		{
			printf("输入有误！请重输\n\n");
			continue;
		}
		else
			break;
	}

	printf("请输入QQ密码：");
	scanf("%s",password);
	while(getchar() != '\n');
	//占用 封包
	type = 1;
	send_buf = packing();

	do{
		ret = send(fd,send_buf,strlen(send_buf),0);
	}while(ret < 0 && EINTR == errno);

	do
	{
		ret = recv(fd,recv_string,1024,0);
	}while(ret < 0 && EINTR == errno);

	ret = unpacking(recv_string);
	if(ret)
	{	
		printf("登陆成功!\n");
		sleep(1);
		return true;
	}
	else
	{	
		printf("登陆失败!\n");
		printf("%s\n",buf);
		sleep(1);
		return false;
	}
}
/*注册选项*/
int menu_regit()
{
	int ret,choise;

	system("clear");
	printf(RED"*************************\n"NONE);
	printf(RED"*\t\t\t*\n");
	printf("*"NONE" 1.普通注册 2.靓号注册 "RED"*\n");
	printf("*\t\t\t*\n");
	printf("*************************\n"NONE);

	while(1)
	{
		ret = 0;
		printf("请选择：");
		ret = scanf("%d",&choise);
		while(getchar() != '\n');
		if(ret == 0 || (choise!=1 && choise!=2))
		{
			printf("输入有误！\n\n");
			continue;
		}
		else
			break;
	}
	return choise;
}
/*靓号注册*/
void bet_regit()
{
	int ret = -1;
	char en;
	bzero(buf,BUFSIZ);
	bzero(name,16);
	bzero(password,16);

	char *send_buf;
	char recv_string[1024];

	while(1){
	printf("请输入QQ昵称：");
	scanf("%s",name);
	while(getchar() != '\n');

	printf("请输入QQ：");
	scanf("%d",&qq);
	while(getchar() != '\n');

	printf("请输入QQ密码：");
	scanf("%s",password);
	while(getchar() != '\n');
	type = 5;
	send_buf = packing();

	do{
		ret = send(fd,send_buf,strlen(send_buf),0);
	}while(ret < 0 && EINTR == errno);

	do
	{
		ret = recv(fd,recv_string,1024,0);
	}while(ret < 0 && EINTR == errno);
	ret = unpacking(recv_string);
	if(ret)
	{	
		printf("付款二维码暂无，所以恭喜你免费获得QQ号\n");
		printf("您的QQ靓号为: %d\n",qq);
		printf("请牢记！按回车返回登陆/注册界面\n");
		while(getchar() != '\n');
		break;
	}
	else
	{	
		printf("注册失败!\n");
		printf("该QQ已被注册\n");
		printf("按回车返回输入界面");
		while(getchar() != '\n');
		continue;
	}
	}
}
/*注册选择函数*/
void sec_regit()
{
	int ret,choise = -1;
	choise = menu_regit();
	switch (choise){
		case 1:
			regit();
			break;
			case 2:
				bet_regit();	break;
			default:	break;
		}
}

/*普通注册*/
void regit()
{
	int ret = -1;
	bzero(buf,BUFSIZ);
	bzero(name,16);
	bzero(password,16);

	char *send_buf;
	char recv_string[1024];
	printf("请输入QQ昵称：");
	scanf("%s",name);
	while(getchar() != '\n');

	printf("请输入QQ密码：");
	scanf("%s",password);
	while(getchar() != '\n');
	type = 0;
	send_buf = packing();


	do{
		ret = send(fd,send_buf,strlen(send_buf),0);
	}while(ret < 0 && EINTR == errno);
	do
	{
		ret = recv(fd,recv_string,1024,0);
	}while(ret < 0 && EINTR == errno);
	ret = unpacking(recv_string);
	if(ret)
	{	
		printf("注册成功!\n");
		printf("您的QQ号为: %d\n",qq);
		printf("请牢记！按回车返回登陆/注册界面");
		while(getchar() != '\n');
	}
}


char* packing()
{
	int i,state = -1;
	char *send_buf;
	char send_pbuf[1024];
	struct json_object *fat,*json_type,*json_name,*json_qq,*json_password,*json_buf;

	fat = json_object_new_object();

	json_type = json_object_new_int(type);
	json_object_object_add(fat,"type",json_type);

	state = type;
	for(i=0;i<2;i++)
	{
		switch(state)
		{
			case 0:	//注册
				json_name = json_object_new_string(name);
				json_object_object_add(fat,"name",json_name);

				json_password = json_object_new_string(password);
				json_object_object_add(fat,"password",json_password);
				state = 9;
				break;
			case 1://登录
				json_qq = json_object_new_int(qq);
				json_object_object_add(fat,"qq",json_qq);

				json_password = json_object_new_string(password);
				json_object_object_add(fat,"password",json_password);
				state = 9;
				break;
			case 2://私聊封包
				json_qq = json_object_new_int(qq);
				json_object_object_add(fat,"qq",json_qq);
				state = 9;
				break;
			case 3: //udp发送信息
				json_name = json_object_new_string(my_name);
				json_object_object_add(fat,"name",json_name);

				json_buf = json_object_new_string(buf);
				json_object_object_add(fat,"buf",json_buf);
				state = 9;
				break;
			case 4://查看在线用户
				state = 9;
				break;
			case 5://靓号封包
				json_name = json_object_new_string(name);
				json_object_object_add(fat,"name",json_name);

				json_qq = json_object_new_int(qq);
				json_object_object_add(fat,"qq",json_qq);

				json_password = json_object_new_string(password);
				json_object_object_add(fat,"password",json_password);
				state = 9;
				break;
			case 6://退出封包
				state = 9;
				break;

			case 7: //udp广播信息
				json_name = json_object_new_string(my_name);
				json_object_object_add(fat,"name",json_name);

				json_buf = json_object_new_string(buf);
				json_object_object_add(fat,"buf",json_buf);
				state = 9;
				break;
			case 9:
				{
				const char* send_string = json_object_to_json_string(fat);
				strcpy(send_pbuf,send_string);
				send_buf = send_pbuf;}
				break;

			default:
				break;
		}
	}
	return send_buf;
}

bool unpacking(char *recv_buf)
{
	int i,state = INIT;
	char *tmp = (char*)malloc(1024);
	bzero(buf,BUFSIZ);
	bzero(name,16);
	bzero(password,16);

	struct json_object *fat,*json_type,*json_qq,*json_name,*json_password,*json_buf;
	fat = json_tokener_parse(recv_buf);
	for(i=0;i<2;i++)
	{
		switch(state)
		{
			case INIT:
				json_type = json_object_object_get(fat,"type");
				type = json_object_get_int(json_type);
				if(type == 0)	//登陆成功
					state = RGST;
				else if(type == 1)	//无此用户/登陆失败
					state = LGIN;
				else if(type == 2)	//注册失败
					state = GCHAT;
				else if(type == 3)// 注册成功
					state = HCHAT;
				else if(type == 5) //私聊服务器回复
					state = JCHAT;
			/*	else if(type == 7)	//空
					state = PCHAT;*/
				else if(type == 8) //查看在线用户
					state = KCHAT;
				break;	
			case RGST:	//登陆成功
				json_name = json_object_object_get(fat,"name");
				tmp = (char*)json_object_get_string(json_name);
				strcpy(my_name,tmp);
				break;

			case LGIN:	//登陆失败
				json_buf = json_object_object_get(fat,"buf");
				tmp=(char*)json_object_get_string(json_buf);
				strcpy(buf,tmp);
				return false;

		/*	case PCHAT:	//空
				json_name = json_object_object_get(fat,"name");
				tmp = (char*)json_object_get_string(json_name);
				strcpy(name,tmp);
				json_buf = json_object_object_get(fat,"buf");
				tmp = (char*)json_object_get_string(json_buf);
				strcpy(buf,tmp);
				break;*/
			case GCHAT:	//注册失败
				return false;
			case HCHAT: //注册成功
				json_qq = json_object_object_get(fat,"qq");
				qq = json_object_get_int(json_qq);
				break;
			case JCHAT: //私聊服务器回复
				json_buf = json_object_object_get(fat,"buf");
				tmp = (char*)json_object_get_string(json_buf);
				strcpy(buf,tmp);
				break;
			case KCHAT: //查看在线用户
				json_buf = json_object_object_get(fat,"buf");
				tmp = (char*)json_object_get_string(json_buf);
				strcpy(buf,tmp);
				break;

			default:
				break;
		}
	}
	return true;
}

/*线程udp客户端*/
void* udp_client(void* arg)
{
	printf(RED"*****输入quit返回菜单****\n"NONE);
	int udpc_fd = -1;
	/*1.建立套接字 */
	if( (udpc_fd = socket(AF_INET, SOCK_DGRAM ,0))< 0) { //基于互联网的UDP
		perror("socket");
		exit(1);
	}

	/*2 指定服务器的IP地址和端口  */
	struct sockaddr_in sin;
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(SERVER_UDP_PORT);  //16位网络字节序的端口号
	sin.sin_addr.s_addr = inet_addr((char*)arg);


	/*3.和服务器进行数据的交互:从标准键盘获取用户输入,发送给服务器 */
	int n; 
	int ret = -1;
	char send_buf[1024];
	char in_buf[1024];
	while(1) {
		printf("you say:\n");
		bzero(in_buf, 1024);
		read(0, in_buf, 1024);
		if(!strncasecmp(in_buf,"quit",4 ))   //用户输入了quit
			break;
		strcpy(buf,in_buf);
		type = 3;
		strcpy(send_buf,packing());
		n = strlen(send_buf);

		ret = sendto(udpc_fd, send_buf, n, 0, (struct sockaddr *)&sin, sizeof(sin));  //往套接字上写数据
		if(ret < 0) {
			perror("sento");
			continue;
		}
	}
	close(udpc_fd);
}

/*群聊私聊菜单*/
int menu_sec(void)
{
	int ret,choise;

	system("clear");
	printf(RED"*************************\n"NONE);
	printf(RED"*\t\t\t*\n");
	printf("*"NONE"   1.私聊   2.群聊     "RED"*\n");
	printf("*"NONE"3.查看在线用户 4.退出  "RED"*\n");
	printf("*\t\t\t*\n");
	printf("*************************\n"NONE);

	while(1)
	{
		ret = 0;
		printf("请选择：");
		ret = scanf("%d",&choise);
		while(getchar() != '\n');
		if(ret == 0 || (choise!=1 && choise!=2 && choise!=3 && choise!=4))
		{
			printf("输入有误！\n\n");
			continue;
		}
		else
			break;
	}
	return choise;
}

/*sec_menu:群聊私聊*/
void sec_choise(pthread_t* tid_cli,pthread_t* tid_board)
{
	printf(RED"*****输入quit返回菜单****\n"NONE);
	char *send_buf;
	int choise = -1,ret = -1,i;
	char recv_string[1024] = {'\0'};
	while(1){
	choise = menu_sec();
	switch(choise){
		case 1:
			printf("请输入对方QQ号：");
			scanf("%d",&qq);
			type = 2;
			send_buf = packing();
			do{
				ret = send(fd,send_buf,strlen(send_buf),0);
			}while(ret < 0 && EINTR == errno);
			bzero(recv_string,1024);
			do
			{
				ret = recv(fd,recv_string,1024,0);
			}while(ret < 0 && EINTR == errno);
			unpacking(recv_string);
			if(1 == type)
			{
				printf("%s\n",buf);
				sleep(2);
			}
			else
				pthread_create(tid_cli,NULL,udp_client,(void*)buf);// 线程udp客户端
			if(pthread_join(*tid_cli,NULL) < 0)
			{
				perror("pthread_join");
				exit(1);
			}
			break;
		case 2:/*群聊*/
			pthread_create(tid_board,NULL,udp_board,NULL);// 线程广播	
			if(pthread_join(*tid_board,NULL) < 0)
			{
				perror("pthread_join2");
				exit(1);
			}
			break;
		case 3:/*查看在线用户*/
			type = 4;
			send_buf = packing();
			do{
				ret = send(fd,send_buf,strlen(send_buf),0);
			}while(ret < 0 && EINTR == errno);

			bzero(recv_string,1024);
			do{
				ret = recv(fd,recv_string,1024,0);
			}while(ret < 0 && EINTR == errno);
			printf("在线用户如下：\n");
			pthread_mutex_lock(&mutex);
			unpacking(recv_string);
			printf("%s\n",buf);
			pthread_mutex_unlock(&mutex);
			printf("按回车退出查看");
			while(getchar() != '\n');
			break;
		case 4:
			type = 6;
			send_buf = packing();
			do{
				ret = send(fd,send_buf,strlen(send_buf),0);
			}while(ret < 0 && EINTR == errno);
			close(fd);
			exit(1);
		}
	}

}

/*广播*/
void* udp_board(void *arg)
{
	printf(RED"*****输入quit返回菜单****\n"NONE);
	int board_fd = -1;

	/*1.建立套接字 */
	if((board_fd = socket(AF_INET, SOCK_DGRAM ,0))< 0) { //基于互联网的UDP
		perror("socket");
		exit(1);
	}

	/*设置广播属性 */
	int bc = 1;
	setsockopt(board_fd, SOL_SOCKET, SO_BROADCAST, &bc, sizeof(int));

	/*2 指定服务器的IP地址和端口  */
	struct sockaddr_in sin;
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(SERVER_UDP_PORT);  //16位网络字节序的端口号
	sin.sin_addr.s_addr = inet_addr(SERVER_BROAD_IP);
	printf("Boardcast sender starting....OK\n");
	/*3.和服务器进行数据的交互:从标准键盘获取用户输入,发送给服务器 */
	int n; 
	int ret = -1;
	char send_buf[1024];
	char in_buf[1024];
	while(1) {
		printf("you say:\n");
		bzero(in_buf, 1024);
		read(0, in_buf, 1024);
		if(!strncasecmp(in_buf,"quit",4 ))   //用户输入了quit
			break;
		strcpy(buf,in_buf);
		type = 7;
		strcpy(send_buf,packing());
		n = strlen(send_buf);

		ret = sendto(board_fd, send_buf, n, 0, (struct sockaddr *)&sin, sizeof(sin));  //往套接字上写数据
		if(ret < 0) {
			perror("sento");
			continue;
		}
	}
	close(board_fd);
}

