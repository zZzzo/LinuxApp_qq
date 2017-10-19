#include "server.h"

int type = -1,qq;					//JSON->类型/QQ号
char name[MAX_NAME_LEN];			//JSON->昵称
char password[MAX_PSW_LEN];			//JSON->密码
char buf[BUFSIZ];					//JSON->消息内容
knlist *head;						//内核链表头

sqlite3 *db;						//数据库描述符
sql_pdata sd;						//数据库结构体
char sql[SQL_SIZE] = {'\0'};		//描述
char *errmsg;						//数据库错误信息

int note_fd;						//登陆信息
//int fifo_fd;
char fd_buf[FD_SIZE];
//pthread_mutex_t mutex;

int main(void)
{
	int listenfd,newfd;		
	socklen_t addrlen;
	struct sockaddr_in sin,cin;
	cli_info cinfo = {0};
	pthread_t tid,tid_udp;
	int ret_sql = -1;
	char main_pid[8];
//	pthread_mutex_init(&mutex,NULL);


	//创建链表
	head = list_init();
	//初始化监听套接字
	listenfd = socket_init();
	printf("Waiting for the user to connect...\n");
	//打开数据库文件
	ret_sql = sqlite3_open("./test.db", &db);
	if(ret_sql!= SQLITE_OK)
	{
		sqlite3_close(db);
		printf("open failure:%s\n", errmsg);
		exit(0);
	}
	//建表(数据库)
	creat_table(db);
	//
	if((note_fd=open("./note.txt",O_APPEND|O_CREAT|O_WRONLY,0664)) < 0)
	{
		perror("open");
		exit(1);
	}
//	if(mkfifo("./fifo",0777)<0)
//	{
//		perror("mkfifo");
//		exit(1);
//	}
	
//	if((fifo_fd=open("./fifo",O_RDONLY)) < 0)
//	{
//		perror("open");
//		exit(1);
//	}
//	sprintf(main_pid,"%d",getpid());
//	write(fifo_fd,main_pid,strlen(buf));

	system("gnome-terminal -e ./menu");
	
	/* 等待客户端连接 */
	while(1)
	{
		addrlen = sizeof(cin);
		if((newfd = accept(listenfd,(struct sockaddr*)&cin,&addrlen))<0)
		{
			perror("accept");
			exit(1);
		}
		/* 线程参数 */
		/*
		   fd
		   sockaddr_in cin
		 */
		cinfo.fd = newfd;
		memcpy(&cinfo.cin,&cin,sizeof(cin));
		/* 创建线程 */
		if(pthread_create(&tid,NULL,cli_deal,&cinfo)<0)
		{
			perror("pthread_create");
			exit(1);
		}
	}
	return 0;
}

//初始化监听套接字
int socket_init(void)
{
	int listenfd,reuse;
	struct sockaddr_in sin;
	
	
	if((listenfd=socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		perror("socket");
		exit(1);
	}
	bzero(&sin,sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(SERVER_PORT);
	sin.sin_addr.s_addr = INADDR_ANY;
	
	//地址复用
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(void *)&reuse,sizeof(int));

	if(bind(listenfd,(struct sockaddr*)&sin,sizeof(sin)) < 0)
	{
		perror("bind");
		exit(1);
	}

	if(listen(listenfd,LISTEND_NUM) < 0)
	{
		perror("bind");
		exit(1);
	}

	return listenfd;
}

/* 线程函数 */
void *cli_deal(void *arg)
{
	int ret = -1,list_ret = -1;
	int sockfd = -1;					
	char recv_buf[BUFSIZ] = {0};			//收到的内容
	char *send_buf=(char *)malloc(BUFSIZ);	//发送的内容
	char cli_ip[CLI_IP_LEN];				//client ip
	struct cli_info cinfo = {0};
	struct sockaddr_in cin = {0};
	char *sql_name ="usage";		//表名
	char myname[MAX_NAME_LEN];
	time_t t;

	time(&t);
	sd = (sql_pdata)malloc(sizeof(sql_data));
	pthread_detach(pthread_self());			//分离态

	memcpy(&cinfo,arg,sizeof(cinfo));	
	memcpy(&cin,&cinfo.cin,sizeof(cin));
	sockfd = cinfo.fd;
	bzero(cli_ip,CLI_IP_LEN);

	/* 客服端ip地址 */
	if(inet_ntop(AF_INET,&cin.sin_addr.s_addr,cli_ip,sizeof(cin)) != NULL)
	{
		printf("-->%s",ctime(&t));
		printf("client ipaddr:%s port:%d connect!\n",cli_ip,ntohs(cin.sin_port));
		printf("*************************************************\n");
	}
	else{
		perror("inet_ntop");
		exit(1);
	}

	//	pthread_mutex_lock(&mutex);

	while(1)
	{
		bzero(buf,BUFSIZ);
		bzero(recv_buf,BUFSIZ);
		do{
			ret = recv(sockfd,recv_buf,BUFSIZ-1,0);
//			printf("recv_buf:%s\n",recv_buf);
		}while(ret < 0 && EINTR == errno);
		//1.1读失败
		if(ret < 0){
			perror("recv");
			continue;
		}
		//1.2客户端断开
		if(ret == 0){
			close(sockfd);
			printf("-->%s",ctime(&t));
			printf("client ipaddr:%s disconnect\n",cli_ip);
			printf("---------------------------------------------\n");
			//链表删掉(下线)
			list_delqq(cli_ip);
			break;
		}
		//2.拆包(json)
		unpacking(recv_buf);
		//debug:
		//printf("type:%d\n",type);
		//3.操作
		//printf("name:%s\n",name);

		switch(type)
		{
			case 0:		//注册
				//1.生成随机QQ号
				while(1)
				{
					srand((int)time(0));
					qq = rand()%(9999-1000+1)+1000;
					if(search_table(db,sql_name,qq," ") != 1)
					{
						break;
					}
					//	printf("已有此qq\n");
				}
				//2.添加到数据库(insert)
				//debug:
				//printf("name:%s\n",name);
				strcpy(sd->name,name);
				sd->qq = qq;
				strcpy(sd->password,password);
				insert(db,sd,sql_name);
				//3.封包.发送
				type = 3;		//注册返回
				send_buf = packing();
				//debug:
				printf("%s%s注册账号:name:%s,qq:%d",ctime(&t),cli_ip,name,qq);
				do{
					ret = send(sockfd,send_buf,strlen(send_buf),0);
				}while(ret < 0 && EINTR == errno);
				break;
			case 1:		//登陆
				//验证账号密码
				ret = search_table(db,sql_name,qq,password);
				if(!ret)
				{
					//查当前在线用户
					list_ret = search_list_qq(head);
					if(list_ret)
					{
						type = 1;
						strcpy(buf,"此账号已登陆!\n");
					}
					else
					{
						printf("-->%s%s上线\n",ctime(&t),name);
						//1.添加到链表(ip/qq/name)
						list_addqq(cli_ip,qq,name,head);
						strcpy(myname,name);
						bzero(fd_buf,FD_SIZE);
						sprintf(fd_buf,"%s-->[%s](%s) login!\n",ctime(&t),myname,cli_ip);
						write(note_fd,fd_buf,strlen(fd_buf));
						
						type = 0;
						show(head);
					}
				}
				else
				{
					if(ret == 2)
						strcpy(buf,"账号不存在!\n");
					if(ret == 1)
						strcpy(buf,"密码错误,再想想!\n");
					//3.封包.发送
					type = 1;
				}
				send_buf = packing();
				//printf("----->send to client:%s\n",send_buf);
				do{
					ret = send(sockfd,send_buf,strlen(send_buf),0);
				}while(ret < 0 && EINTR == errno);
				break;
			case 2:			//私聊
				//1.查找在线用户
				if(search_list_qq(head))
				{
					bzero(name,MAX_NAME_LEN);
					type = 5;
					send_buf = packing();
				}
				else
				{
					//2.查找数据库
					if(search_sql_qq(db,sql_name,qq))
					{
						strcpy(buf,"对方已下线\n");	
					}
					else
						strcpy(buf,"不存在此用户\n");
					type = 1;
					send_buf = packing();
				}
				//printf("----->send to client:%s\n",send_buf);
				do{
					ret = send(sockfd,send_buf,strlen(send_buf),0);
				}while(ret < 0 && EINTR ==errno);

				break;

			case 4:
				//在线用户
				type = 8;
				show(head);
				send_buf = packing();
				//printf("----->send to client:%s\n",send_buf);
				do{
					ret = send(sockfd,send_buf,strlen(send_buf),0);
				}while(ret < 0 && EINTR ==errno);
				break;
			case 5:
				if(search_sql_qq(db,sql_name,qq))
				{
					type = 2;
					send_buf = packing();
				}
				else
				{
					strcpy(sd->name,name);
					sd->qq = qq;
					strcpy(sd->password,password);
					insert(db,sd,sql_name);
					type = 3;
					send_buf = packing();
				}
				do{
					ret = send(sockfd,send_buf,strlen(send_buf),0);
				}while(ret < 0 && EINTR == errno);
				break;
			case 6:	//下线
				list_delqq(cli_ip);
				printf("-->%s%s下线\n",ctime(&t),myname);
				bzero(fd_buf,FD_SIZE);
				sprintf(fd_buf,"%s-->[%s](%s) offline!\n",ctime(&t),myname,cli_ip);
				write(note_fd,fd_buf,strlen(fd_buf));
				close(sockfd);
				pthread_exit(NULL);
				break;

		}
	}
	close(sockfd);
}
/* 封包 */
char *packing()
{
	int state = -1,i;
	char *send_buf = malloc(BUFSIZ);
	struct json_object *fat,*json_type,*json_qq,*json_name,*json_password,*json_buf;

	fat = json_object_new_object();

	json_type = json_object_new_int(type);
	json_object_object_add(fat,"type",json_type);

	state = type;
	for(i=0;i<2;i++){
		switch(state)
		{
			case 0:		//(登陆)验证成功
					json_name = json_object_new_string(name);
					json_object_object_add(fat,"name",json_name);
				
					state = 7;
					break;
			case 1:		//(登陆)验证失败
					json_buf = json_object_new_string(buf);
					json_object_object_add(fat,"buf",json_buf);
				
					state = 7;
					break;
			case 2:		//注册失败
					state = 7;
					break;
			case 3:		//注册成功
					json_qq = json_object_new_int(qq);
					json_object_object_add(fat,"qq",json_qq);
				
					state = 7;
					break;
			case 5:		//私聊(发ip)
					json_buf = json_object_new_string(buf);
					json_object_object_add(fat,"buf",json_buf);

					state = 7;
					break;
			case 7:{	//打包
					const char *send_string = json_object_to_json_string(fat);
					strcpy(send_buf,send_string);
					break;
				   }
			case 8:		//查看在线用户
				   json_buf = json_object_new_string(buf);
				   json_object_object_add(fat,"buf",json_buf);

				   state = 7;
				   break;
			default:
				   break;
		}
	}
	return send_buf;
}

/* 拆包 */
void unpacking(char *recv_buf)
{
	int i,state = INIT;
	char *tmp = (char *)malloc(1024);
	char *ntmp = (char *)malloc(1024);
	struct json_object *fat,*json_type,*json_qq,*json_name,*json_password,*json_buf;

	bzero(name,MAX_NAME_LEN);
	bzero(password,MAX_PSW_LEN);

	fat = json_tokener_parse(recv_buf);
	for(i=0;i<2;i++)
	{
		switch(state)
		{
			case INIT:
				json_type = json_object_object_get(fat,"type");
				type = json_object_get_int(json_type);
				if(type == 0)		//注册
					state = RGST;
				else if(type == 1)	//登陆
					state = LOGIN;
				else if(type == 2)	//私聊
					state = PCHAT;
				else if(type == 4)
					state = ONLINE;
				else if(type == 5)
					state = VRGST;
				break;
			case RGST:	//注册
				json_name = json_object_object_get(fat,"name");
				tmp=(char*)json_object_get_string(json_name);
				strcpy(name,tmp);

				json_password = json_object_object_get(fat,"password");
				tmp=(char*)json_object_get_string(json_password);
				strcpy(password,tmp);
				break;
			case LOGIN:	//登陆
				json_qq = json_object_object_get(fat,"qq");
				qq = json_object_get_int(json_qq);

				json_password = json_object_object_get(fat,"password");
				tmp=(char*)json_object_get_string(json_password);
				strcpy(password,tmp);

				break;
			case PCHAT:	//私聊：昵称 改qq
				json_qq = json_object_object_get(fat,"qq");
				qq = json_object_get_int(json_qq);

				break;
			case ONLINE: //查看在线好友
				break;
			case VRGST:
				json_password = json_object_object_get(fat,"password");
				tmp=(char*)json_object_get_string(json_password);;
				
				strcpy(password,tmp);
				
				json_name = json_object_object_get(fat,"name");
				tmp=(char*)json_object_get_string(json_name);
				strcpy(name,tmp);

				json_qq = json_object_object_get(fat,"qq");
				qq = json_object_get_int(json_qq);
				
				break;
			case OFF:
				break;
			default:
				break;
		}
	}
}

/*************************************************/
/******************内核链表***********************/
/*************************************************/

/* 链表初始化 */
knlist *list_init(void)
{
	knlist *head = malloc(sizeof(knlist));
	if(head == NULL)
	{
		perror("malloc");
		exit(1);
	}
	INIT_LIST_HEAD(&head->list);
	return head;
}

/*登陆成功，添加到链表*/
void list_addqq(char cli_ip[],int qq,char name[],knlist *head)
{
	knlist *new;

	new = (knlist*)malloc(sizeof(knlist));
	if(NULL == new){
		perror("malloc");
		exit(1);
	}
	strcpy(new->cli_ip,cli_ip);	//ip放进去
	new->qq = qq;				//QQ号放进去
	strcpy(new->name,name);		//昵称放进去
	list_add_tail(&new->list,&head->list);	//进去
}
//删除(客户端下线)
void list_delqq(char cli_ip[])
{
	knlist *p;

	list_for_each_entry(p,&head->list,list)
	{
		if(!strcmp(cli_ip,(char *)&p->cli_ip))
		{
			list_del(&p->list);
			break;
		}
	}

}

//遍历链表
void show(knlist *head)
{
	knlist *p;
	char qq_buf[16];

	list_for_each_entry(p,&head->list,list)
	{
		sprintf(qq_buf,"QQ号:%d\t",p->qq);
		strcat(buf,qq_buf);
		strcat(buf,"昵称:");
		strcat(buf,p->name);
		strcat(buf,"\n");
	}
}
/*查找用户(qq号)*/
bool search_list_qq(knlist *head)
{
	knlist *p;
	bzero(buf,CLI_IP_LEN);

	list_for_each_entry(p,&head->list,list)
	{
		if(qq == p->qq)
		{
			strcpy(buf,p->cli_ip);
			return true;
		}
	}
	return false;
}


/*****************************************/
/***************数据库********************/
/*****************************************/

/*创建*/
bool creat_table(sqlite3 *db)
{
	int ret = -1;
	sprintf(sql, "create  table if not exists 'usage'(name varchar(16),qq int,password varchar(16))");
	ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
	if(SQLITE_OK != ret){
		printf("SQLITE_OK:%d\n",SQLITE_OK);
		return false;
	}
	return true;
}

/*写入数据*/
bool insert(sqlite3 *db,sql_pdata sd,char *sql_name)
{
	int ret = -1;
	sprintf(sql, "insert into %s(name, qq, password) values ('%s', %d, '%s')",sql_name,sd->name,sd->qq,sd->password);
	ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
	if(SQLITE_OK != ret){
		//sqlite3_close(db);
		printf("insert:%s\n", errmsg);
		return false;
	}
	return true;
}

/*查数据库(登陆验证)*/
int search_table(sqlite3 *db,char *sql_name,int qq,char* password)
{

	char **result;
	int nrow = -1;
	int ncolumn = -1;
	int i,ret = -1;
	int n = -1;
	int flag = 2;
	//	bzero(name,MAX_NAME_LEN);

	//查询usage表所有信息
	sprintf(sql, "select * from %s", sql_name);
	ret = sqlite3_get_table(db, sql, &result, &nrow, &ncolumn, &errmsg);

	for(i=1; i<(nrow+1)*ncolumn; i+=3){
		if(qq == atoi(result[i]))
		{
			flag--;
			n = strlen(result[i+1]);
			if(!strncmp(password,result[i+1],n))
			{
				flag--;
				strcpy(name,result[i-1]);
				break;
			}
			break;
		}
	}
	return flag;
}
/*查找用户(qq)*/
bool search_sql_qq(sqlite3 *db,char *sql_name,int qq)
{

	char **result;
	int nrow = -1;
	int ncolumn = -1;
	int i,ret = -1;
	int n = -1;
	n = strlen(password);

	//查询usage表所有信息
	sprintf(sql, "select * from %s", sql_name);
	ret = sqlite3_get_table(db, sql, &result, &nrow, &ncolumn, &errmsg);

	for(i=1; i<(nrow+1)*ncolumn; i+=3)
	{
		if(qq == atoi(result[i]))
			return true;
	}
	return false;
}


/*
//修改信息
bool update_data(sqlite3 *db,char *sql_name)
{

int qq=0,count=0;
int ret=-1;



printf("修改usage qq:\n");
scanf("%d",&qq);
int tt=0;
char menu_name[16];
char menu_password[16];

while(1){
printf("\t1:name \t2:password \t3:退出\n");
while(getchar()!='\n');
scanf("%d",&tt);
if(tt == 1){
printf("namae修改:\n");
while(getchar()!='\n');
scanf("%s",menu_name);

memset(sql,0,128);
sprintf(sql,"update %s set name='%s' where qq=%d",sql_name,menu_name,qq);
ret = sqlite3_exec(db,sql,NULL,NULL,&errmsg);
if(SQLITE_OK != ret){
sqlite3_close(db);
printf("update:%s\n", errmsg);
return false;
}

}
if(tt == 2){
printf("密码修改 password:\n");
while(getchar()!='\n');
scanf("%s",menu_password);
memset(sql,0,128);
sprintf(sql,"update %s set password='%s' where qq=%d",sql_name,menu_password,qq);
ret = sqlite3_exec(db,sql,NULL,NULL,&errmsg);
if(SQLITE_OK != ret){
sqlite3_close(db);
printf("update:%s\n", errmsg);
return false;
}
}

show_table(db,sql_name);
if(tt == 3)
break;
}
return true;
}
 */
