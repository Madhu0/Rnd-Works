#include <winsock2.h>
#include <StdAfx.h>
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <conio.h>
#include<cstdlib>


int hsock;
struct sockaddr_in my_addr;
int send_data(char *);
char *receive_data();
void initialize_client();
void login();
void delete_file();
void register_user_client();
void add_file();
void display_files();
void download_file();
void display_users();
void initialize_msg();

int getsocket()
{
	int hsock;
	int * p_int ;
	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if(hsock == -1){
		printf("Error initializing socket %d\n",WSAGetLastError());
		return -1;
	}
	
	p_int = (int*)malloc(sizeof(int));
	*p_int = 1;
	if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
		(setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
		printf("Error setting options %d\n", WSAGetLastError());
		free(p_int);
		return -1;
	}
	free(p_int);

	return hsock;
}

void socket_client()
{

	//The port and address you want to connect to
	int host_port= 1101;
	char* host_name="127.0.0.1";

	//Initialize socket support WINDOWS ONLY!
	unsigned short wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );
 	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 || ( LOBYTE( wsaData.wVersion ) != 2 ||
		    HIBYTE( wsaData.wVersion ) != 2 )) {
	    fprintf(stderr, "Could not find sock dll %d\n",WSAGetLastError());
		goto FINISH;
	}

	//Initialize sockets and set any options

	//Connect to the server
	

	my_addr.sin_family = AF_INET ;
	my_addr.sin_port = htons(host_port);
	
	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = inet_addr(host_name);

	//if( connect( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR ){
	//	fprintf(stderr, "Error connecting socket %d\n", WSAGetLastError());
	//	goto FINISH;
	//}

	//Now lets do the client related stuff
	char buffer[4064];
	int buffer_len = 4064;
	int bytecount;
	int c;
	int flag = 0;
	char *result;

	while(true) {

/*		hsock = getsocket();
		//add error checking on hsock...
		if( connect(hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR ){
			fprintf(stderr, "Error connecting socket %d\n", WSAGetLastError());
			goto FINISH;
		}

		memset(buffer, '\0', buffer_len);
		if (flag == 0){
			printf("Enter a key to continue\n");
			flag = 1;
		}
		fflush(stdin);
		for (char* p = buffer; (c = getch()) != 13; p++){
			printf("%c", c);
			*p = c;
		}
		fflush(stdin);*/

		initialize_client();

/*		result = receive_data(hsock);
		printf("%s\n", result);*/
		closesocket(hsock);
	}

	//closesocket(hsock);
FINISH:
;
}


int send_data(char *buffer){
	int bytecount;
	int c;
	int flag = 0;
	hsock = getsocket();
	if (connect(hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR){
		fprintf(stderr, "Error connecting socket %d\n", WSAGetLastError());
		goto FINISH;
	}
	if ((bytecount = send(hsock, buffer, 4064, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		goto FINISH;
	}
	//printf("Sent bytes %d\n", bytecount);
	return 1;
FINISH:
	return -1;
	;
}


char *receive_data(){
	char *buffer=(char *)malloc(4064);
	int buffer_len = 4064;
	int bytecount;
	int c;
	int flag = 0;
/*	hsock = getsocket();
	if (connect(hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR){
		fprintf(stderr, "Error connecting socket %d\n", WSAGetLastError());
		goto FINISH;
	}*/
	if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		goto FINISH;
	}
	buffer[bytecount] = '\0';
	//printf("Sent bytes %d\n", bytecount);
	return buffer;
FINISH:
	;
}

void initialize_client(){
	char ch[4064];
	int res;
	printf("\nConnecting to server....\n");
	res=send_data("hello");
	if (res == -1){
		getchar();
		exit(0);
	}
	while (1){
		fflush(stdin);
		printf("\n\n");
		printf("\nwelcome tp store\n\n1.Login\n2.register\n3.exit\n");
		scanf("%c", &ch[0]);
		ch[1] = '\0';
		send_data( ch);
		system("cls");
		switch (ch[0]){
		case '1':
			login();
			break;
		case '2':
			register_user_client();
			break;
		case '3':
			exit(0);
		default:
			printf("\nOoops!!!! Wrong option\nTry again:-)\n");
		}
		system("cls");
	}
}

void login(){
	int choice;
	char ch[4064];
	char *reply;
	char user[30], password[9];
	printf("\n\n");
	printf("\nenter username\n");
	scanf("%s", user);
	printf("\n\n");
	printf("\nEnter password\n");
	scanf("%s", password);
	ch[0] = '\0';
	strcat(ch, user);
	strcat(ch, "\n");
	strcat(ch, password);
	send_data(ch);
	reply = receive_data();
	send_data("ok");
	if (reply[0] == '1'){
		printf("\nAuthentication Success...\n");
		printf("\n\nEnter 1 for blobstore\n2 for Message store\n\n");
		scanf("%d", &choice);
		ch[0] = choice;
		ch[1] = '\0';
		send_data(ch);
		receive_data();
		system("cls");
		if (choice == 1)
			display_files();
		else
			initialize_msg();
		//add_file();
	}
	else
		printf("\nAuthentcation failure...if you are not registerd register now\n");
}

void add_reply(){
	char buff[4064];
	printf("\nEnetr your reply\n");
	scanf(" %[^\n]s", buff);
	send_data(buff);
	receive_data();
	system("cls");
	return;
}

void delete_reply(){
	receive_data();
	send_data("ok");
	printf("\nDeleting reply is successful....\n");
	system("cls");
	return;
}

void display_replies(){
	char buff[4064], *reply;
	int i,j, ch;
	while (1){
		reply = receive_data();
		j = 1;
		if (reply[0] == '\0'){
			printf("\nOops!! no replies\nenter 1 to add reply\nenter -1 to return\n");
			scanf("%d", &ch);
			buff[0] = ch; buff[1] = '\0';
			send_data(buff);
			system("cls");
			if (ch == 1)
				add_reply();
			else
				return;
		}
		else{
			i = 0;
			while (reply[i] != '\0'){
				printf("\n");
				printf("%d.", j++);
				i++;
				while (reply[i] != '\n')
					printf("%c", reply[i++]);
				printf(" by=> ");
				i++;
				while (reply[i] != '\n'&&reply[i] != '\0')
					printf("%c", reply[i++]);
				printf("\n");
			}
			printf("\nchoose reply to delete\nenter -2 add reply \nenter -1 to go back");
			scanf("%d", &ch);
			buff[0] = ch; buff[1] = '\0';
			send_data(buff);
			system("cls");
			if (ch == -2)
				add_reply();
			else if (ch == -1)
				return;
			else
				delete_reply();
		}
	}
}

void delete_message(){
	char *reply;
	receive_data();
	send_data("ok");
	printf("\nMessage deleted succesfully.....\n");
}

void add_message(){
	char buff[4064];
	printf("\nenter the message:\n");
	scanf(" %[^\n]s", buff);
	send_data(buff);
	receive_data();
	system("cls");
	return;
}


void display_messages(){
	char *reply, buff[4064];
	int ch,i,j;
	while (1){
		j = 1;
		reply = receive_data();
		if (reply[0] == '\0'){
			printf("\nOops!!! No messages\nEnter 1 to add message\n-1 to go back");
			scanf("%d", &ch);
			buff[0] = ch;
			buff[1] = '\0';
			send_data(buff);
			system("cls");
			if (ch == 1)
				add_message();
			else
				return;
		}
		else{
			i = 0;
			while (reply[i] != '\0'){
				printf("\n");
				printf("%d.", j++);
				i++;
				while (reply[i] != '\n')
					printf("%c", reply[i++]);
				printf(" by=> ");
				i++;
				while (reply[i] != '\n'&&reply[i] != '\0')
					printf("%c", reply[i++]);
				printf("\n");
			}
			printf("\nchoose message to proceed\n -1 to exit\n\n-2 to add message");
			scanf("%d", &ch);
			buff[0] = ch; buff[1] = '\0';
			send_data(buff);
			system("cls");
			if (ch == -1)
				return;
			else if (ch == -2)
				add_message();
			else{
				printf("\nEnter 1 to see replies\n2 delete message\n");
				scanf("%d", &ch);
				buff[0] = ch; buff[1] = '\0';
				receive_data();
				send_data(buff);
				system("cls");
				switch (ch){
				case 1:
					display_replies();
					break;
				case 2:
					delete_message();
					break;
				default:
					printf("\nOoops!!! Watch your choice\n");
				}
			}
		}
	}
}


void display_categories(){
	char buff[4064],*reply;
	int i,j,ch;
	while (1){
		j = 1;
		reply = receive_data();
		if (reply[0] == '\0'){
			printf("\nSorry No Categories\n\n");
			return;
		}
		else{
			for (i = 0; reply[i] != '\0'; i++){
				if (reply[i] == '\n'){
					printf("\n");
					printf("%d.", j++);
				}
				else
					printf("%c", reply[i]);
			}
			printf("\nChoose category to view\nenter -1 to go back\n");
			scanf("%d", &ch);
			buff[0] = ch;
			buff[1] = '\0';
			send_data(buff);
			system("cls");
			if (ch == -1)
				return;
			else if (ch < j)
				display_messages();
			else
				printf("\nInvalid Option\n");
		}
	}
}


void add_category(){
	char *buff,reply[4064];
	buff = receive_data();
	send_data("k");
	if (buff[0] == '\0'){
		printf("\nOoops!!No of categories exceeded 5\n");
		return;
	}
	printf("\nEnter the category name\n");
	scanf("%s", reply);
	send_data(reply);
	receive_data();
	system("cls");
}


void register_user_client(){
	char ch[4064];
	char *reply;
	int choice,i;
	char user[30], password[9];
	printf("\n\n");
	printf("\nenter username\n");
	scanf("%s", user);
	printf("\n\n");
	printf("\nEnter password\n");
	scanf("%s", password);
	system("cls");
	ch[0] = '\0';
	strcat(ch, user);
	strcat(ch, "\n");
	strcat(ch, password);
	send_data( ch);
	receive_data();
	for (i = 0; i < 5; i++){
		printf("\nEnter 1 to add category to the user in message store\n-1 to skip\n");
		scanf("%d", &choice);
		ch[0] = choice;
		ch[1] = '\0';
		system("cls");
		send_data(ch);
		receive_data();
		if (choice == 1)
			add_category();
		else
			break;
	}
}

void add_file(){
	FILE *fp;
	char path[100],buff[4064],temp[40];
	int size,i,length;
	char name[20];
	buff[0] = '\0';
	printf("\n\n");
	printf("\nEnter filename with path\n");
	scanf("%s",&path );
	fp = fopen(path, "rb");
	printf("\n\n");
	printf("\nEnter size in bytes\n");
	scanf("%d", &size);
	printf("\n\n");
	printf("\nEnter name to be stored in store\n");
	scanf("%s", &name);
	system("cls");
	itoa(size, temp,10);
	strcat(buff, name);
	strcat(buff, "\n");
	strcat(buff, temp);
	send_data(buff);
	receive_data();
	while (size > 0){
		memset(buff, 0, 4064);
		size = size - 4064;
		length=fread(buff, 1, 4064, fp);
		if (length < 4064){
			itoa(length, temp, 10);
			send_data(temp);
			receive_data();
		}
		send_data(buff);
		receive_data();
	}
}

void display_files(){
	char *buff, data[10];
	int i, j ;
	while (1){
		j = 1;
		buff = receive_data();
		send_data("ok");
		if (buff[0] == '0')
			printf("\nSorry No files\n\n");
		else{
			// = receive_data();
			for (i = 0; buff[i] != '\0'; i++){
				if (buff[i] == '\n'){
					printf("\n");
					printf("%d.", j++);
				}
				else
					printf("%c", buff[i]);
			}
		}
		printf("\nchoose the file to download\n");
		printf("\nEnter 99 to add file\nenter 100 to exit\n");
		scanf("%d", &i);
		data[0] = i;
		system("cls");
		send_data(data);
		receive_data();
		if (i == 99)
			add_file();
		else if (i == 100)
			return;
		else{
			printf("\nEnter \n1 to download file\n2 to delete file");
			scanf("%d", &j);
			data[0] = j;
			system("cls");
			send_data(data);
			receive_data();
			if (j == 1)
				download_file();
			else if (j == 2)
				delete_file();
		}
	}
}


void download_file(){
	char *buff,*end;
	char name[20],temp[10];
	int size=0,i,block,flag;
	FILE *fp;
	buff = receive_data();
	send_data("ok");
	for (i = 0; buff[i] != '\n'; i++)
		name[i] = buff[i];
	name[i] = '\0';
	i++;
	for (; buff[i] != '\0'; i++)
		size = size * 10 + buff[i] - '0';
	block = size;
	printf("\nEnter full path of the file\n");
	scanf("%s", &name);
	fp = fopen(name, "wb");
	while (block > 0){
		printf("downloading.");
		system("cls");
		printf("downloading....");
		system("cls");
		printf("downloading...........");
		system("cls");
		buff = receive_data();
		send_data("ok");
		if (block < 4064){
			end = receive_data();
			send_data("ok");
			flag=atoi(end);
			for (i = 0;i<flag; i++){
				fwrite(buff + i, 1, 1, fp);
				//printf("\n%5d%5d%5d", buff[i], flag,i);
			}
		}
		else
			fwrite(buff, 4064, 1, fp);
		block = block - 4064;
	}
	fclose(fp);
}

void delete_file(){
	receive_data();
	return;
}


void display_users(){
	char *buff,msg[4064];
	int ch,i,j;
	while (1){
		buff = receive_data();
		j = 1;
		if (buff[0] == '\0')
			printf("\n\nNo users\n\n");
		else{
			for (i = 0; buff[i] != '\0'; i++){
				if (buff[i] == '\n'){
					printf("\n");
					printf("%d.", j++);
				}
				else
					printf("%c", buff[i]);
			}
		}
		printf("\nEnter the option\n");
		scanf("%d", &ch);
		msg[0] = ch;
		msg[1] = '\0';
		send_data(msg);
		system("cls");
		printf("\nEnter \n1 display categories\n2 to add category\n go back\n");
		scanf("%d", &ch);
		msg[0] = ch;
		msg[1] = '\0';
		system("cls");
		send_data(msg);
		switch (ch){
		case 1:
			display_categories();
			break;
		case 2:
			add_category();
			break;
		case 3:
			return;
		default:
			printf("\n\nWrong Option\n\n");
			printf("\nEnter \n1 display categories\n2 to add category\n");
			scanf("%d", &ch);
			system("cls");
		}
	}
}

void initialize_msg(){
	int ch;
	char buff[4064], *reply;
	while (1){
		printf("\nWelcome to message store\n");
		printf("\nChoose the one of the options below\n");
		printf("\n1 to view all categories\n2 display your categories\n3 to add a category to your deck\n4 to go back\n");
		scanf("%d", &ch);
		buff[0] = ch; buff[1] = '\0';
		system("cls");
		send_data(buff);
		receive_data();
		switch (ch){
		case 1:
			display_categories();
			break;
		case 2:
			display_categories();
			break;
		case 3:
			add_category();
			break;
		case 4:
			return;
		}
	}
}