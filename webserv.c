#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<pthread.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#define LISTEN_NUM 20

void *clnt_handler(void* arg);
void send_data(FILE *fp,char *filename,char *c_type);

int main(int argc,char **argv)
{
	int n;
	pthread_t thr_id;
	int serv_sock,clnt_sock;
	struct sockaddr_in serv_addr,clnt_addr;
	socklen_t clnt_len;

	if(argc!=2)
	{
		printf("error!\n");
		exit(1);
	}

	serv_sock=socket(AF_INET,SOCK_STREAM,0);
	if(serv_sock==-1)
	{
		printf("socket() error!\n");
		exit(1);
	}

	memset((void*)&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));

	n=bind(serv_sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
	if(n==-1)
	{
		printf("bind() error!\n");
		exit(1);
	}
	printf("before listen\n");
	n=listen(serv_sock,LISTEN_NUM);
	printf("n:%d\n",n);
	if(n==-1)
	{
		printf("listen() error!\n");
		exit(1);
	}
	printf("after listen\n");

	while(1)
	{
		clnt_len=sizeof(clnt_addr);
		printf("befort accept\n");
		clnt_sock=accept(serv_sock,(struct sockaddr*)&clnt_addr,&clnt_len);
		if(clnt_sock==-1)
		{
			printf("accept error\n");
			exit(1);
		}
		printf("Connection Request : %s:%d\n", 
			inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
		printf("after accept\n");

		pthread_create(&thr_id,NULL,clnt_handler,(void*)&clnt_sock);
		printf("after pthread_create,before pthread_detach()\n");
		pthread_detach(thr_id);
		printf("after pthread_detach()!\n");
	}
	close(serv_sock);
	return 0;
}

void * clnt_handler(void *arg)
{
	int fd;
	fd=*((int*)arg);
	FILE* clnt_write;
	FILE* clnt_read;
	char request[35];
	char filename[13];
	char tmp[13];
	//char con_type[12];
	char *p;

	clnt_write=fdopen(fd,"w");
	clnt_read=fdopen(dup(fd),"r");

	memset((void*)request,0,sizeof(request));
	fgets(request,35,clnt_read);
	if(strstr(request,"HTTP/")==NULL)
	{
		printf("request error!\n");
		exit(0);
	}
	
	p=strtok(request," /");
	if(strcmp(p,"GET")!=0)
	{
		printf("strcmp error!\n");
		exit(1);
	}
	p=strtok(NULL," /");
	memset((void*)filename,0,sizeof(filename));
	strcpy(filename,p);

	memset((void*)tmp,0,sizeof(tmp));
	strcpy(tmp,p);
	strtok(tmp,".");
	p=strtok(NULL,".");    //p point to "html"

	//memset((void*)con_type,0,sizeof(con_type));
	//strcpy(con_type,p);
	
	if(strcmp(p,"html")==0||strcmp(p,"htm")==0)
		p="text/html";
	else
		p="text/plain";

	printf("in clnt_handler(),before send_data(),p:%s\n",p);
	printf("in clnt_handler() before send_data(),filename:%s\n",filename);
	send_data(clnt_write,filename,p);
	printf("after send_data,end clnt_handler!\n");
	//fclose(clnt_write);
	fclose(clnt_read);
	return ((void*)0);
}

void send_data(FILE *fp,char *filename,char *c_type)
{
	FILE *fp1;
	char type[35];
	char content[100];
	char status[]="HTTP/1.1 200 OK\r\n";	
	char server[]="Server:Linux Host\r\n";
	char length[]="Content_Length:2048\r\n";
	
	memset((void*)type,0,sizeof(type));
	sprintf(type,"Content-Type:%s\r\n\r\n",c_type);
	
	fputs(status,fp);
	fputs(server,fp);
	fputs(length,fp);
	fputs(type,fp);
	fflush(fp);
	printf("in send_data,after four fputs\n");

	fp1=fopen(filename,"r");
	while(fgets(content,100,fp1)!=NULL)
	{
		fputs(content,fp);
		fflush(fp);
	}
	
	printf("in send_data,after while\n");
	fflush(fp);
	fclose(fp);
	fclose(fp1);
	printf("leave send_data\n");
}
