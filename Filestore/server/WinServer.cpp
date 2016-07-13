#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include<limits.h>



struct node
{
	char msg[128];
	int msg_id;
	node *next;
}*flist,*alist,*printid;

struct bufserv{
	
		int userId;
		int forumId;
		int msgId;
		int commentId;
		int choice;
		char *forumname;
		char msg[128];
}buf1;

struct user{
	char name[16];
	char password[8];
	int fileoffset;
	int msgoffset;
};

struct user current_logged_user;
int offset_clu;

bool flag=true;
int mid = 0;
int count1 =0;
char *Data[100];
int count=1;
int values[100];


char * SocketHandler(void*);
void replyto_client(char *buf, int *csock);
int user_count;
int files_count;
char * receive_data(int *);
void add_file(FILE *, int *);
void initialize(FILE *fp,FILE *);
int hsock;

#include"blobstoreheader.h"
#include"messagestore.h"

void socket_server() {

	//The port you want the server to listen on
	int host_port= 1101;

	unsigned short wVersionRequested;
	WSADATA wsaData;
	int err;

	FILE *fp,*msg_fp;
	fp = fopen("blobstore.bin", "rb+");
	msg_fp = fopen("msg_data.bin", "rb+");
	fseek(fp, USERCOUNTS, SEEK_SET);
	fread(&user_count, 4, 1, fp);
	fread(&files_count, 4, 1, fp);
	fread(&categories, 4, 1, msg_fp);
	wVersionRequested = MAKEWORD( 2, 2 );
 	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 || ( LOBYTE( wsaData.wVersion ) != 2 ||
		    HIBYTE( wsaData.wVersion ) != 2 )) {
	    fprintf(stderr, "No sock dll %d\n",WSAGetLastError());
		goto FINISH;
	}

	//Initialize sockets and set options
	int * p_int ;
	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if(hsock == -1){
		printf("Error initializing socket %d\n",WSAGetLastError());
		goto FINISH;
	}
	
	p_int = (int*)malloc(sizeof(int));
	*p_int = 1;
	if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
		(setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
		printf("Error setting options %d\n", WSAGetLastError());
		free(p_int);
		goto FINISH;
	}
	free(p_int);

	//Bind and listen
	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET ;
	my_addr.sin_port = htons(host_port);
	
	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = INADDR_ANY ;
	
	/* if you get error in bind 
	make sure nothing else is listening on that port */
	if( bind( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
		fprintf(stderr,"Error binding to socket %d\n",WSAGetLastError());
		goto FINISH;
	}
	if(listen( hsock, 10) == -1 ){
		fprintf(stderr, "Error listening %d\n",WSAGetLastError());
		goto FINISH;
	}
	
	//Now lets do the actual server stuff

	while (1){
		initialize(fp,msg_fp);
		//receive_data();
	}


FINISH:
;
}


char * receive_data(int *csock){

	sockaddr_in sadr;
	char *recvbuf;
	int	addr_size = sizeof(SOCKADDR);
	printf("waiting for a connection\n");

	if ((*csock = accept(hsock, (SOCKADDR*)&sadr, &addr_size)) != INVALID_SOCKET){
		//printf("Received connection from %s",inet_ntoa(sadr.sin_addr));
		//CreateThread(0, 0, &SocketHandler, (void*)csock, 0, 0);
		recvbuf = SocketHandler((void*)csock);
		printf("Received");
		//printf("msg from client:%s\n", recvbuf);
	}
	else{
		fprintf(stderr, "Error accepting %d\n", WSAGetLastError());
	}
	//replyto_client(recvbuf, csock);
	return recvbuf;
}


void process_input(char *recvbuf, int recv_buf_cnt, int* csock) 
{

	char replybuf[1024]={'\0'};
	printf("%s",recvbuf);
	replyto_client(recvbuf, csock);
	replybuf[0] = '\0';
}

void replyto_client(char *buf, int *csock) {
	int bytecount;
	
	if((bytecount = send(*csock, buf, 4064, 0))==SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		free (csock);
	}
	printf("replied to client: %s\n",buf);
}

char * SocketHandler(void* lp){
    int *csock = (int*)lp;
	char *recvbuf = (char *)malloc(4064);
	memset(recvbuf, 0, 4064);
	int recvbuf_len = 4064;
	int recv_byte_cnt;

	memset(recvbuf, 0, recvbuf_len);
	if((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0))==SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		free (csock);
		return 0;
	}
	//printf("Received bytes %d\nReceived string \"%s\"\n", recv_byte_cnt, recvbuf);

    return recvbuf;
}

void validate_user(FILE *fp,FILE *msg_fp, int *csock){
	char *reply;
	char user[20];
	struct user temp;
	int i, j = 0, flag = 0;
	char password[9];
	//	char msg[] =  "Enter Username:";
	//	replyto_client(msg, csock);
	//	reply = receive_data(csock);
	//	for (i = 0; reply[i] != '\0'; i++)
	//	user[i] = reply[i];
	//user[i] = '\0';
	//char msg1[] = "Enter Password";
	//replyto_client(msg1, csock);
	reply = receive_data(csock);
	for (i = 0; reply[i] != '\n'; i++)
		user[i] = reply[i];
	user[i] = '\0';
	i++;
	for (; reply[i] != '\0'; i++){
		password[j++] = reply[i];
	}
	fseek(fp, USERS, SEEK_SET);
	for (i = 0; i < user_count; i++){
		offset_clu = ftell(fp);
		fread(&temp, 32, 1, fp);
		if (!strcmp(temp.name, user)){
			for (j = 0; j < 8; j++){
				if (temp.password[j] != password[j])
					break;
			}
			if (j == 8){
				flag = 1;
				break;
			}
		}
	}
	if (flag == 1){
		user[0] = '1';
		user[1] = '\0';
		replyto_client(user, csock);
		receive_data(csock);
		current_logged_user = temp;
		//add_file(fp, csock);
		reply = receive_data(csock);
		replyto_client("ok", csock);
		if (reply[0]==1)
			display_files(fp, csock);
		else
			msg_initialize(msg_fp,csock);
	}
	else{
		replyto_client("0", csock);
		receive_data(csock);
	}
}


void register_user(FILE *fp,FILE *msg_fp, int *csock){
	struct user temp;
	memset(&temp, 0, 32);
	char *reply;
	int i, j = 0;
	reply = receive_data(csock);
	replyto_client("ok", csock);
	for (i = 0; reply[i] != '\n'; i++)
		temp.name[i] = reply[i];
	temp.name[i] = '\0';
	i++;
	for (; reply[i] != '\0'; i++){
		temp.password[j++] = reply[i];
	}
	temp.msgoffset = add_user(msg_fp,temp.name,csock);
	fseek(fp, USERS + (user_count * 32), SEEK_SET);
	fwrite(&temp, 32, 1, fp);
	user_count++;
	fseek(fp, USERCOUNTS, SEEK_SET);
	fwrite(&user_count, 4, 1, fp);
	fflush(fp);
}


void initialize(FILE *fp,FILE *msg_fp){
	int* csock;
	char *option;
	csock = (int*)malloc(sizeof(int));
	receive_data(csock);
	while (1)
	{
		//		char msg[] =  "Welcome\nlogin\nregister\tEnter your reply:" ;
		//		replyto_client(msg, csock);
		option = receive_data(csock);
		char ch = option[0];
		switch (ch){
		case '1':
			validate_user(fp,msg_fp, csock);
			break;
		case '2':
			register_user(fp,msg_fp, csock);
			break;
		case '3':
			for (int i = 0; i < 1000; i++){}
			exit(0);
		}
	}
}