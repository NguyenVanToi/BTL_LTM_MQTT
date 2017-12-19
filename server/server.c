//Nguyen Van Hung
//MSSV: 14020592
//Viet chuong trinh chat don gian giua nhieu nguoi dung
//CHUONG TRINH SEVER


#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_CLIENT 10

int chuyenint(char c)
{
	return (int) c - 48;
}
int chuyenchu(int a)
{
	return (char)a + 48;
}

typedef struct  
{
	char name[20];
	int acceptfd;
	int IDclient;
	char IDgroup;
}Users;

int count = 0;
Users user[MAX_CLIENT];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// them client moi
void addClient(int conn, char IDgroup,int IDclient, int i, char *name) //them 1 client
{
	strcpy(user[i].name, name);
	user[i].acceptfd = conn;
	user[i].IDgroup = IDgroup;
	user[i].IDclient = IDclient;
}
//thay doi IDgroup
void changeIDgroup(int connfd, char *buf)
{
	int i;
	char c = buf[strlen(buf) - 1];
	// printf("ID moi:  %c\n",c );
	for(i = 0;  i < MAX_CLIENT; i++)
	{
		if(user[i].acceptfd == connfd)
		{
			user[i].IDgroup = c;
			break;
		}
	}
}
void sendFile(int connfd, int n,char IDgroup, int IDclient, char *mess1, int len)
{
	int i;
	for(i = 0; i < MAX_CLIENT; i++)
	{
		if(n == 1)
		{
			if(user[i].acceptfd != connfd && (strlen(user[i].name)) > 0)
			{
				write(user[i].acceptfd, mess1, len);
			}
		}
		if(n == 2)
		{
			if(user[i].IDgroup == IDgroup && user[i].acceptfd != connfd)
			{
				write(user[i].acceptfd, mess1, len);
			}
		}
		if(n == 3)
		{
			if(user[i].IDclient == IDclient)
			{
				write(user[i].acceptfd, mess1, len);
			}
		}
	}
}
// gui tin nhan theo ID
void sendMessage(int connfd, int n,char IDgroup, int IDclient, char *mess)
{
	int i;
	for(i = 0; i < MAX_CLIENT; i++)
	{
		if(n == 1)
		{
			if(user[i].acceptfd != connfd && (strlen(user[i].name)) > 0)
			{
				write(user[i].acceptfd, mess, strlen(mess));
			}
		}
		if(n == 2)
		{
			if(user[i].IDgroup == IDgroup && user[i].acceptfd != connfd)
			{
				write(user[i].acceptfd, mess, strlen(mess));
			}
		}
		if(n == 3)
		{
			if(user[i].IDclient == IDclient)
			{
				write(user[i].acceptfd, mess, strlen(mess));
			}
		}
	}
}
// ham gui tin nhan la so nguyen theo ID
void sendInt(int connfd, int n,char IDgroup, int IDclient, int mess)
{
	int i;
	for(i = 0; i < MAX_CLIENT; i++)
	{
		if(n == 1)
		{
			if(user[i].acceptfd != connfd && (strlen(user[i].name)) > 0)
			{
				write(user[i].acceptfd, &mess, sizeof(mess));
			}
		}
		if(n == 2)
		{
			if(user[i].IDgroup == IDgroup && user[i].acceptfd != connfd)
			{
				write(user[i].acceptfd, &mess, sizeof(mess));
			}
		}
		if(n == 3)
		{
			if(user[i].IDclient == IDclient)
			{
				write(user[i].acceptfd, &mess, sizeof(mess));
			}
		}
	}
}
//ham gui co client moi login hay logout
void newclient(int connfd,char IDgroup, int IDclient, char *name, int t)
{
	char status[512];
	memset(status, '\0', sizeof(status));
	if (t == 0)
	{
		strcat(status, "\nNew user login: " );
	}
	if (t == 1)
	{
		strcat(status, "User logout: ");
	}
	strcat(status, name);
	sendInt(connfd,1, '0', 0, strlen(status));
	sendMessage(connfd,1, '0', 0, status);

}
// gui nguoi dung dang online
void sendID(int sockfd)
{
	char sendID[512] ;
	int i, strlenID;
	memset(sendID, '\0', sizeof(sendID));
	strcat(sendID, "\nClient online: \n");
	for(i = 0; i < MAX_CLIENT; i++)
	{

		if(strlen(user[i].name) > 0)
		{
			char c = chuyenchu(user[i].IDclient);
			strcat(sendID, "\"Ten: ");
			strcat(sendID, user[i].name);
			strcat(sendID, "\" | \"IDClient : ");
			sendID[strlen(sendID)] = c;
			strcat(sendID, "\" | \"IDGroup: ");
			sendID[strlen(sendID)] = user[i].IDgroup;
			strcat(sendID, "\"");
			sendID[strlen(sendID) ] = '\n';
			strlenID = strlen(sendID);
			write(sockfd, &strlenID, sizeof(int));
			write(sockfd, sendID, strlen(sendID));
			memset(sendID, '\0', sizeof(sendID));
		}
	}

}
// xoa user khi logout
void rmClient( int connfd)
{
	int i, j;
	for(i = 0; i< MAX_CLIENT; i++)
	{
		if(user[i].acceptfd == connfd)
		{
			for(j = i; j < MAX_CLIENT - 1; j++)
			{
				user[j] = user[j+1];
			}
			break;
		}
	}
}

 // xu ly
static void *doit(void *arg)
{
	int connfd, nread=0;
	char buf[512],username[512], data[512];
	char buffer[512];
	int len_file, len_data = 0;
	char IDgroup;
	connfd  = *(int *)arg;
	free(arg);
	pthread_detach(pthread_self());
	memset(username,0, sizeof(username));

	//gui danh sach nguoi dang online
	sendID(connfd);

	if((nread = read(connfd, username, sizeof(username)-1))>0){		
		IDgroup = username[strlen(username) - 1];
		username[nread - 1] = '\0';
		printf("User:  %s connect!\n", username);
	}
	int i, j, IDclient ;
	for(i = 0; i< MAX_CLIENT; i++)
	{
		int t = 1;
		for(j = 0; j < MAX_CLIENT; j++)
		{
			if(i == user[j].IDclient)
			{		
				t = 0 ;
				break;
			}	
		}
		if(t == 1)
		{
			IDclient = i;
			break;
		}

	}

	newclient(connfd, IDgroup, IDclient, username, 0);
	addClient(connfd, IDgroup,IDclient, count, username);
	pthread_mutex_lock(&mutex);
	count ++;
	pthread_mutex_unlock(&mutex);
	while(( nread = read(connfd, buf, sizeof(buf))) > 0)
	{
		buf[nread] = '\0';
		if(strcmp(buf, "--S") == 0)
		{
			sendID(connfd);
			continue;
		}
		if(strstr(buf, "--G") != NULL)
		{
			changeIDgroup(connfd, buf);
			IDgroup = buf[strlen(buf) - 1];
			continue;
		}
		if(buf[strlen(buf) - 1] == '0' && buf[strlen(buf) - 2] == '0' && buf[strlen(buf) - 3] == '1' )
		{
			buf[strlen(buf) - 3 ] = '\0';
			if(strstr(buf, "--F") != NULL) {
				sendInt(connfd, 1, '0', 0, strlen(buf));
				sendMessage(connfd,1, '0', 0, buf);
				read(connfd, &len_file, sizeof(int));
				printf("aa: %d\n", len_file);
				sendInt(connfd, 1, '0', 0, len_file);
				if(len_file >= 0) {
					while(len_file >= 0) {
						memset(data, 0, sizeof(data));
						len_data = read(connfd, data, sizeof(data));
						sendFile(connfd,1, '0', 0, data, len_data);
						len_file -= len_data;
						if(len_file == 0) {
							break;
						}
					}
				}
			} 
			else
			{
				strcpy(buffer, username);
				strcat(buffer, ": ");
				strcat(buffer, buf);
				sendInt(connfd, 1, '0', 0, strlen(buffer));
				sendMessage(connfd,1, '0', 0, buffer );
			}
		
		}
		if(buf[strlen(buf) - 1] == '0' && buf[strlen(buf) - 2] != '0' && buf[strlen(buf) - 3] == '1' )
		{
			buf[strlen(buf) - 3 ] = '\0';
			if(strstr(buf, "--F") != NULL) {
				sendInt(connfd, 2, IDgroup, 0, strlen(buf));
				sendMessage(connfd,2, IDgroup, 0, buf);
				read(connfd, &len_file, sizeof(int));
				sendInt(connfd, 2, IDgroup, 0, len_file);
				if(len_file >= 0) {
					while(len_file >= 0) {
						memset(data, 0, sizeof(data));
						len_data = read(connfd, data, sizeof(data));
						sendFile(connfd,2, IDgroup, 0, data, len_data);
						len_file -= len_data;
						if(len_file == 0) {
							break;
						}
					}
				}
			} else {
				strcpy(buffer, username);
				strcat(buffer, ": ");
				strcat(buffer, buf);
				sendInt(connfd,2 ,IDgroup , 0, strlen(buffer));
				sendMessage(connfd,2 ,IDgroup , 0, buffer );
			}
		}
		if(buf[strlen(buf) - 1] != '0' && buf[strlen(buf) - 2] == '0' && buf[strlen(buf) - 3] == '1' )
		{
			int x = chuyenint(buf[strlen(buf) - 1]);
			buf[strlen(buf) - 3 ] = '\0';
			if(strstr(buf, "--F") != NULL) {
				sendInt(connfd, 3,'0', x, strlen(buf));
				sendMessage(connfd,3,'0', x, buf);
				read(connfd, &len_file, sizeof(int));
				sendInt(connfd, 3, '0', x, len_file);
				if(len_file >= 0) {
					while(len_file > 0) {
						memset(data, 0, sizeof(data));
						len_data = read(connfd, data, sizeof(data));
						sendFile(connfd,3,'0', x, data, len_data);
						len_file -= len_data;
						if(len_file == 0) {
							break;
						}
					}
				}
			} else {
				strcpy(buffer, username);
				strcat(buffer, ": ");
				strcat(buffer, buf);
				sendInt(connfd,3,'0', x , strlen(buffer));
				sendMessage(connfd,3,'0', x , buffer);
			}
		}
		if(!strcmp(buf, "@"))
		{
			break;
		}
	
		memset(buf, 0, sizeof(buf));
		memset(buffer, 0, sizeof(buffer));
	}

	newclient(connfd, IDgroup, IDclient, username, 1);
	rmClient(connfd);
	pthread_mutex_lock(&mutex);
	count --;
	pthread_mutex_unlock(&mutex);
	close(connfd);
	printf("User : %s disconnect\n", username);
	return (NULL);
}


int main( int argc, char **argv)
{
	pthread_t tid;
	int *iptr;
	int sockfd;
	struct sockaddr_in sockaddr, cliaddr ;
	sockfd = socket (AF_INET, SOCK_STREAM, 0); //tao 1 socket
	bzero(&sockaddr, sizeof(sockaddr));
	
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sockaddr.sin_port = htons(9876);

	bind(sockfd, (struct sockaddr *) &sockaddr, sizeof(sockaddr));

	listen(sockfd, 5);
	
	for(;;)
	{
		socklen_t  cliaddr_len = sizeof(cliaddr);;
		iptr = malloc(sizeof(int));
		*iptr = accept(sockfd, (struct sockaddr*) &cliaddr, &cliaddr_len);
		pthread_create(&tid, NULL, &doit, (void *)iptr);
		
	}
	
	return 0;
}