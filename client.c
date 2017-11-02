#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

void *get_ip(struct sockaddr *s)
{
	return &(((struct sockaddr_in *)s)->sin_addr);
}

int main(int argc, char **argv)
{
	int sockfd, i;
	struct addrinfo base, *res, *temp;
	char ch,ip[INET6_ADDRSTRLEN],buf[128],*data,temp_buf[1024],udp_buf[1024];

	memset(&base, 0, sizeof(base));
	base.ai_family = AF_INET;
	base.ai_socktype = SOCK_STREAM;

	if(i = getaddrinfo(argv[1], argv[2], &base, &res) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(i));
		return 0;
	}

	for(temp = res; temp != NULL; temp = temp->ai_next)
	{
		if((sockfd = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol)) == -1)
		{
			perror("client : socket");
			continue;
		}

		if(connect(sockfd, temp->ai_addr, temp->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("client : connect");
			continue;
		}
		break;
	}

	if(temp == NULL)
	{
		fprintf(stderr, "Server failed to find. Exiting.\n");
		exit(1);
	}

	inet_ntop(temp->ai_family, get_ip((struct sockaddr *)temp->ai_addr), ip, INET6_ADDRSTRLEN);
	printf("Client connecting to %s\n",ip);
	data = "TYPE 1";

	if(send(sockfd, data, strlen(data), 0) == -1)
		perror("client : send");
	
	printf("sent from client %s\n",data);
	
	if((i = recv(sockfd, buf, 128, 0) == -1))
		perror("client : recieve"); 
	
	printf("UDP port received from server %s\n", buf);
	close(sockfd);
	printf("TCP Connection Closed.\n");
	sleep(0.5);

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("client : socket ");
		exit(1);
	}
	printf("Enter message to send by UDP - ");
	// gets(udp_buf);
	scanf("%[^\n]%*c",udp_buf);
	// fgets(temp_buf,1024,stdin);
	// int l = strlen(temp_buf);
	// if((l > 0) && (temp_buf[l-1]) == '\n')
	// {
	// 	l--;
	// 	// temp_buf[l-1] = '\0';
	// }
	// memcpy(udp_buf,temp_buf,l);
	// udp_buf[l] = '\0';
	// // i = 0;
	// while((ch = getchar()) != '\n')
	// {
	// 	temp_buf[i] = ch;
	// 	i++;
	// }
	// temp_buf[i] = '\0';
	// strncpy(udp_buf,temp_buf,i+1);
	((struct sockaddr_in *)temp->ai_addr)->sin_port = htons(atoi(buf));
	
	if((i = sendto(sockfd, udp_buf, strlen(udp_buf), 0, (struct sockaddr *)temp->ai_addr, sizeof(struct sockaddr_storage )) == -1))
	{
		perror("client : sendto ");
		exit(1);
	}
	printf("sent data '%s' to server by udp port.\n", udp_buf);
	memset(udp_buf,0,sizeof(udp_buf));
	int len = sizeof(struct sockaddr *);
	
	if((i = recvfrom(sockfd, udp_buf, 1024, 0, (struct sockaddr *)temp->ai_addr, &len)) == -1)
	{
		perror("Client : recvfrom ");
		exit(1);
	}

	printf("data received by client by udp port is - %s\n",udp_buf);
	printf("UDP Connection Closed.\n");
	sleep(1);
	close(sockfd);
	freeaddrinfo(res);
	return 0;
}