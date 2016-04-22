/* FileName:ftp_session.c;
 *
 * Author:snownight;
 * Date:2016.4.6;
 *
 */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <semaphore.h>

#include "ftp_err.h"
#include "ftp_session.h"

//long g_local_addr = 0;
sem_t g_sem;

int ftp_session_create(char *ftpDomain, int ftpPort)
{
	int ftpfd;
	struct hostent *ftphost;
	struct sockaddr_in ftpaddr;
	struct sockaddr_in laddr;
	int addr_len;
	int ret;

	ftphost = gethostbyname(ftpDomain);
	if (NULL == ftphost)
	{
		fprintf(stderr, "Get host ip error\n");
		return FTP_GETHOST_FAIL; 
	}

	if (0 > (ftpfd = socket(AF_INET, SOCK_STREAM, 0)))
	{
		fprintf(stderr, "create socket error\n");
		return FTP_SOCKET_ERR;
	}

	ftpaddr.sin_addr.s_addr = *(unsigned long *)ftphost->h_addr;
	if (ftpPort == 0)
		ftpPort = FTP_PORT_DEFAULT;
	ftpaddr.sin_port = htons(ftpPort);
	ftpaddr.sin_family = AF_INET;
	if (-1 == (connect(ftpfd, (struct sockaddr*)(&ftpaddr), sizeof(ftpaddr))))
	{
		fprintf(stderr, "connect server fail!\n");
		close(ftpfd);
		return FTP_CONNECT_FAIL;
	}

	//ret = getsockname(ftpfd, (struct sockaddr *)(&laddr), &addr_len);
	//g_local_addr = ntohl(laddr.sin_addr.s_addr);
	fprintf(stdout, "Connected to %s (%s)\n", ftpDomain, inet_ntoa(ftpaddr.sin_addr));
	return ftpfd; 
}

int ftp_session_config(int fd, long *listenip, int *listenport)
{
	int ret;
	struct sockaddr_in dataaddr;
	int addrlen;

	ret = getsockname(fd, (struct sockaddr *)(&dataaddr), &addrlen);
	if (ret != 0)
	{
		fprintf(stderr, "get sock name error\n");
		return FTP_ERR;
	}

	*listenip = ntohl(dataaddr.sin_addr.s_addr);

	return FTP_OK;
}

void * ftp_session_transport(void *arg)
{
	int lfd = *((int *)arg);
	struct sockaddr_in faddr;
	int sfd;
	char buffer[1024] = {0};
	int ret;
	int addrlen;

	sfd = accept(lfd, (struct sockaddr *)(&faddr), &addrlen);
	while(1)
	{
		ret = recv(sfd, buffer, 1024, 0);
		if (0 >= ret)
		{
			break;
		}
		buffer[ret] = '\0';
		fprintf(stdout, "List:\n%s\n", buffer);
	}

	sem_post(&g_sem);

	return FTP_OK;
}

#define BACKLOG  (1)
int ftp_session_data(long *listenip, int *listenport)
{
	int lfd;
	int ret;
	struct sockaddr_in laddr;
	struct sockaddr_in dataaddr;
	pthread_t data_thread;
	int addrlen = sizeof(struct sockaddr);

	if (0 > (lfd = socket(AF_INET, SOCK_STREAM, 0)))
	{
		fprintf(stderr, "session data create socket error\n");
		return FTP_SOCKET_ERR;
	}

	laddr.sin_family = AF_INET;
	laddr.sin_addr.s_addr = htonl(INADDR_ANY);
	laddr.sin_port = htons(0);
	bind(lfd,  (struct sockaddr *)(&laddr), sizeof(struct sockaddr));
	listen(lfd, BACKLOG);

	ret = pthread_create(&data_thread, NULL, ftp_session_transport, &lfd);
	if (0 > ret)
	{
		return FTP_THREAD_FAIL;
	}

	ret = getsockname(lfd, (struct sockaddr *)(&dataaddr), &addrlen);
	*listenport = ntohs(dataaddr.sin_port);
	//*listenip = g_local_addr;

	return lfd;
}

int ftp_session_getreply(int fd, char *reply, int length)
{
	return recv(fd, reply, length, 0);
}

int ftp_session_command(int fd, char *command)
{
	return send(fd, command, strlen(command), 0);
}

int ftp_session_destory(int fd)
{
	close(fd);
	return FTP_OK;
}
