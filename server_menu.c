#include "server.h"

sqlite3 *db;
sql_pdata sd;
char sql[SQL_SIZE] = {'\0'};
char *errmsg;
char *sql_name ="usage";

int main(void)
{
	int choise;
	int fifo_fd,pid;
	char main_pid[8] = {'\0'};
	sqlite3_open("test.db",&db);
#if 0	
	if(fifo_fd = open("./fifo",O_WRONLY) < 0)
	{
		perror("open");
		exit(1);
	}
	
	read(fifo_fd,main_pid,8);
	pid = atoi(main_pid);
	printf("pif:%d\n",pid);
#endif
r1:
	printf("1.查看所有用户\t2.删除用户\t3.设置管理员(暂)\n");
	scanf("%d",&choise);
	while(getchar()!='\n');
	if(choise == 1)
	{
		system("clear");
		show_table(db,sql_name);
		printf("\n输入回车返回\n");
		while(getchar()!='\n');
		goto r1;
	}
	if(choise == 2)
	{
		delete_table(db,sql_name);
		printf("\n输入回车返回\n");
		while(getchar()!='\n');
		system("clear");
		goto r1;
	}
	if(choise == 3)
	{
		goto r1;
	}
	return 0;
}

/*遍历*/
void show_table(sqlite3 *db,char *sql_name)
{

	char **result;
	int nrow = -1; 
	int ncolumn = -1; 
	int ret=-1;
	int i=0;

	//查询usage表所有信息
	sprintf(sql, "select * from %s", sql_name);
	ret = sqlite3_get_table(db, sql, &result, &nrow, &ncolumn, &errmsg);

	//打印查询结果
	printf("所有用户信息: \n");
	for(i=0; i<(nrow+1)*ncolumn; i++)
	{
		if(i%3 == 0)
			printf("\n");
		printf("%s\t", result[i]);
	}   
}

//删除用户(qq)
void delete_table(sqlite3 *db,char *sql_name)
{
	int ret =-1;
	int qq;

	printf("要删除的qq号:");
	scanf("%d",&qq);
	if(search_sql_qq(db,sql_name,qq))
	{
		sprintf(sql,"delete from %s where qq=%d",sql_name,qq);
		ret = sqlite3_exec(db,sql,NULL,NULL,&errmsg);
		if(SQLITE_OK != ret)
		{
			printf("删除失败:%s\n", errmsg);
			return ;
		}
		else
		{
			printf("删除成功!\n");
			show_table(db,sql_name);
			return ;
		}
	}
	else
		printf("没有此用户,请验证后重试!\n");
	return;
}

bool search_sql_qq(sqlite3 *db,char *sql_name,int qq)
{
	char **result;
	int nrow = -1;
	int ncolumn = -1;
	int i,ret = -1;


	sprintf(sql, "select * from %s", sql_name);
	ret = sqlite3_get_table(db, sql, &result, &nrow, &ncolumn, &errmsg);
	for(i=1; i<(nrow+1)*ncolumn; i+=3)
	{
		if(qq == atoi(result[i]))
			return true;
	}
	return false;
}

