//Nguyen Van Hung
//MSSV: 14020592
//Viet chuong trinh chat don gian giua nhieu nguoi dung
//CHUONG TRINH SEVER


#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>

void sendFile(char *namefile, int listenfd) {
	char data[512];
	int len_data, len_file;
	FILE *file;
	file = fopen(namefile, "r");
	if(file == NULL) {
		printf("File not found!\n");
		len_file = -1;
		write(listenfd, &len_file, sizeof(int));
		return;
	} else {
		fseek(file, 0L, SEEK_END);
		len_file = ftell(file);
		write(listenfd, &len_file, sizeof(int));
		rewind(file);
		while (len_file >= 0) {
			len_data = fread(data, 1, 512, file);
			write(listenfd, data, len_data);
			len_file -= len_data;
			if(len_file <= 0) {
				printf("Finshed send file!\n");
				break;
			}
		}
	}
	fclose(file);
}	
void recvFile(char *namefile, int listenfd, int len_file) {
	char data[512];
	memset(data, 0, sizeof(data));
	FILE * file;	
	file = fopen(namefile, "w");
	while(len_file >= 0) {
		int n = read(listenfd, data, sizeof(data));
		fwrite(data, 1, n, file);
		len_file -= n;
		memset(data, 0, sizeof(data));
		if(len_file <= 0) {
			printf("You receive a file %s!\n", namefile);
			break;
		}
	}
	fclose(file);
}

void *sendMess(void *arg)
{
	char x[1];
	char IDGroup[1], IDClient[1] ;
	char buf[512];
	char namefile[30];
	memset(namefile, 0, sizeof(namefile));
	int sockfd = *(int *)arg;
	pthread_detach(pthread_self());
	usleep(100);
	char username[20];
	printf("Username: ");
	fgets(username, sizeof(username), stdin);
	username[strlen(username) - 1] = '\0';
	while(1)
	{
		printf("Group ID(only a character): ");
		scanf("%s", IDGroup);
		if(IDGroup[0] != '0')
			break;
		printf("Group name and 0 are different!\n");
	}
	
	username[strlen(username)] = IDGroup[0];
	write(sockfd, username, strlen(username));

	char t = IDGroup[0];
	NHAP: ;
	for(;;)
	{
		printf("1: Chat All\n");
		printf("2: Chat Group\n");
		printf("3: Chat 1 vs 1\n");
		printf("Choose feature: ");
		scanf("%s", x);
		if(x[0] == '1' || x[0] == '2' || x[0] == '3')
		{
			break;
		}
	}
	if (x[0] == '3')
	{
		write(sockfd, "--S" , 3);
		
		for(;;){
			usleep(200);
			printf("Import ID user:   ");
			scanf("%s", IDClient);
			if(IDClient[0] != '0')
				break;
		}
	}

	while(1)
	{
		usleep(200);
		printf(">>>");
		memset(buf, 0, sizeof(buf));
		fgets(buf, sizeof(buf), stdin);
		buf[(strlen(buf) - 1)] = '\0';
		if(strcmp(buf, "--M") == 0)
		{
			goto NHAP;
		}
		if(strcmp(buf, "--N") == 0) {
			if(x[0] == '1') {
				printf("You are chatting all!\n");
			} 
			if(x[0] == '2') {
				printf("You are chatting group!\n");
			} 
			if(x[0] == '3') {
				printf("You are chatting 1vs1!\n");
			} 
			continue;
		}
		if(strstr(buf, "--G") != NULL)
		{
			if((strcmp(buf, "--G 0") == 0) || strlen(buf) != 5){
				printf("Modify ID group failed!\n");
				continue;
			} else {
				write(sockfd, buf, strlen(buf));
				t = buf[strlen(buf) - 1];
				printf("You just modify ID group: %c\n", t);
				continue;
			}
		}
		if(strcmp(buf, "--S") == 0) {
			write(sockfd, buf, strlen(buf));
			continue;
		}
		if(strcmp(buf, "--H") == 0) {
			printf("--H\tShow menu help.\n");
			printf("--S\tShow list user online.\n");
			printf("--M\tShow menu feature.\n");
			printf("--N\tShow mode now.\n");
			printf("--G\tModify Id group.\n");
			printf("--F\tSend file.\n");
			continue;
		}
		if(strcmp(buf, "@") == 0)
		{
			exit(1);
		}
		if(x[0] == '1')
		{
			strcat(buf, "100");
			if(strlen(buf) > 3)
				write(sockfd, buf, strlen(buf));
			if(strstr(buf, "--F") != NULL) {
				strncpy(namefile, buf + 4, strlen(buf) - 7);
				// printf("buffile: %s\n", namefile);
				sendFile(namefile, sockfd);
			}
		}
		if(x[0] == '2')
		{
			strcat(buf, "1"); 
			buf[strlen(buf)] = t;
			strcat(buf, "0");
			if(strlen(buf) > 3)
				write(sockfd, buf, strlen(buf));
			if(strstr(buf, "--F") != NULL) {
				strncpy(namefile, buf + 4, strlen(buf) - 7);
				sendFile(namefile, sockfd);
			}
		}
		if( x[0] == '3')
		{
			strcat(buf, "10");
			buf[strlen(buf)] = IDClient[0];
			if(strlen(buf) > 3)
				write(sockfd, buf, strlen(buf));
			if(strstr(buf, "--F") != NULL) {
				strncpy(namefile, buf + 4, strlen(buf) - 7);
				sendFile(namefile, sockfd);
			}
		}
	}
	return NULL;
}

void *recvMess(void *arg)
{
	int sockfd = *(int *)arg;
	pthread_detach(pthread_self());
	int nread, len_buf, len_file;
	char buf2[512], recvnamefile[30];
	while((nread = read(sockfd, &len_buf, sizeof(len_buf))) > 0)
	{
		read(sockfd, buf2, len_buf);
		buf2[strlen(buf2)] = '\0';
		if(strstr(buf2, "--F") != NULL) {
			strncpy(recvnamefile, buf2 + 4, strlen(buf2) - 4);
			memset(buf2,0,sizeof(buf2));	
			read(sockfd, &len_file, sizeof(int));
			if(len_file >= 0) {
				recvFile(recvnamefile, sockfd, len_file);
			}
		} 
		else {
			printf("%s\n", buf2);
			memset(buf2,0,sizeof(buf2));
		}
	}
	return NULL;
}

int main(int argc, char  *argv[])
{
	if( argc < 2)
	{
		printf("You need import address ip!\n");
	}
	else
	{
		struct sockaddr_in serv_addr; 
		int sockfd;
		
		pthread_t tid1, tid2;

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		serv_addr.sin_family =AF_INET;
		serv_addr.sin_port = htons(9876);
		inet_pton(AF_INET, argv[1] , &serv_addr.sin_addr);

		if(connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr))<0)
		{
			printf("Connect fail\n " );
			exit(1);
		}	
		int *ptr1 = (int *)malloc(sizeof(int));
		int *ptr2 = (int *)malloc(sizeof(int));

		*ptr1 = sockfd;
		*ptr2 = sockfd;

		pthread_create(&tid1, NULL, &recvMess,(void *) ptr1);
		pthread_create(&tid2, NULL, &sendMess,(void *) ptr2);
		pthread_join(tid1, NULL);
   		pthread_join(tid2, NULL);
		close(sockfd);
	}
	return 0;
}

		