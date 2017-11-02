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

#define PENDING_CONNECTIONS 10

// IPv6 handling and random UDP for every connection.
void *get_ip(struct sockaddr *s)
{
	return &(((struct sockaddr_in *)s)->sin_addr);
}

int main(int argc, char **argv)
{
	int sockfd, newfd, udpfd, temp1, yes = 1;
	struct addrinfo base, *res, *temp;
	struct sockaddr_storage socket_addr,socket_addr1;
	struct sockaddr_in sock_addr;
	socklen_t len = sizeof(socket_addr);
	char ip[INET6_ADDRSTRLEN], *port, buf[128],udp_buf[1024], udp_port[6];
	memset(&base, 0 ,sizeof(base) );
	port = argv[1];

	base.ai_family = AF_INET;
	base.ai_socktype = SOCK_STREAM;
	base.ai_flags = AI_PASSIVE;

	if(temp1 = getaddrinfo(NULL, port, &base, &res) != 0)
	{
		fprintf(stderr, "error in getaddrinfo %s\n", gai_strerror(temp1));
		return 0;
	}

	for(temp = res; temp != NULL; temp = temp->ai_next)
	{
		if((sockfd = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol)) == -1)
		{
			perror("Error in socket forming!");
			continue;
		}

		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		{
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, temp->ai_addr, temp->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("Binding Error!");
			continue;
		}
		break;
	}
	freeaddrinfo(res);
	if(temp == NULL)
	{
		fprintf(stderr, "Server failed to bind. Exiting.\n");
		exit(1);
	}

	if(listen(sockfd,PENDING_CONNECTIONS) == -1)
	{
		perror("Listening Error! Exiting!");
		exit(1);
	}

	printf("waiting for connections ...\n");

	while(1)
	{
		if((newfd = accept(sockfd, (struct sockaddr *)&socket_addr, &len)) == -1)
		{
			perror("Server : accept ");
			continue;
		}
		inet_ntop(socket_addr.ss_family, get_ip((struct sockaddr *)&socket_addr), ip, INET6_ADDRSTRLEN);
		printf("Received connection from %s\n",ip);

		if(!fork())
		{
			close(sockfd);
			
			if((temp1 = recv(newfd, buf, 128, 0) == -1))
			{
				perror("server : receive");
			}

			if((udpfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
			{
				perror("server : socket UDP ");
				exit(1);
			}

			sock_addr.sin_family = AF_INET;
			sock_addr.sin_port = htons(0);
			sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
			
			if(bind(udpfd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1)
			{
				close(udpfd);
				perror("server : bind UDP ");
				exit(1);
			}

			printf("server received '%s' from %s.\n", buf,ip);
			getsockname(udpfd,(struct sockaddr *)&sock_addr, &len);
			snprintf(udp_port, sizeof(udp_port), "%d", ntohs(sock_addr.sin_port));
			printf("udp port %s sent to %s.\n",udp_port,ip);
			
			if(send(newfd, udp_port, sizeof(udp_port), 0) == -1)
			{
				perror("server : send");
			}
			
			close(newfd);
			printf("For %s TCP connection Closed. Created UDP Connection.\n",ip);
			
			if((temp1 = recvfrom(udpfd, udp_buf, 1024, 0, (struct sockaddr *)&socket_addr1, &len)) == -1)
			{
				perror("server : recvfrom ");
				exit(1);
			}
			udp_buf[temp1] = '\0';
			inet_ntop(socket_addr1.ss_family, get_ip((struct sockaddr *)&socket_addr1), ip, INET6_ADDRSTRLEN);
			printf("received data '%s' by udp port from %s.\n",udp_buf,ip);
			char *data = "Closing UDP Connection...";
			
			if((temp1 = sendto(udpfd, data, strlen(data), 0, (struct sockaddr *)&socket_addr1, sizeof(socket_addr1))) == -1)
			{
				perror("Server : sendto ");
				exit(1);
			}
			
			printf("sent data '%s' by udp port to %s.\n",data,ip);
			sleep(1);
			close(udpfd);
			printf("UDP Connection Closed for %s.\n",ip);
			exit(0);
		}

		close(newfd);
	}

	return 0;
}