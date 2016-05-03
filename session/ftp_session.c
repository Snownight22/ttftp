/* FileName:ftp_session.c;
 *
 * Author:snownight;
 * Date:2016.4.6;
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <errno.h>

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
	struct sockaddr_in faddr;
	int sfd;
	char buffer[1024] = {0};
	int ret;
	int addrlen;
	int i;
	stFtpSc *sc = (stFtpSc *)arg;
	int lfd = sc->lfd;
	FILE *fp = NULL;

	sfd = accept(lfd, (struct sockaddr *)(&faddr), &addrlen);
	if (sc->mode == 1)
	{
		fp = fopen(sc->filename, "wb");
	}
	while(1)
	{
		ret = recv(sfd, buffer, 1023, 0);
		if (0 >= ret)
		{
			//fprintf(stderr, "recv ret:%d, errno:%d\n", ret, errno);
			break;
		}
		//fprintf(stdout, "recv %d char\n", ret);
		//for (i = 0;i < ret;i++)
		//{
			//fprintf(stdout, "%c", buffer[i]);
		//}
		if (sc->mode == 1)
		{
			if (fp)
				fwrite(buffer, 1, ret*sizeof(char), fp);
		}
		else
		{
		    buffer[ret] = '\0';
		    fprintf(stdout, "%s\n", buffer);
		}
	}

	if (fp)
	{
		fclose(fp);
		fp = NULL;
	}
	//sem_post(&g_sem);

	return FTP_OK;
}

#define BACKLOG  (1)
int ftp_session_data(int *clientfd, int *listenport, char *filename, int mode)
{
	int lfd;
	int ret;
	struct sockaddr_in laddr;
	struct sockaddr_in dataaddr;
	pthread_t data_thread;
	int addrlen = sizeof(struct sockaddr);
	stFtpSc *sc = (stFtpSc *)malloc(sizeof(stFtpSc));

	if (0 > (lfd = socket(AF_INET, SOCK_STREAM, 0)))
	{
		fprintf(stderr, "session data create socket error\n");
		return FTP_SOCKET_ERR;
	}
	//fprintf(stdout, "create data fd:%d\n", lfd);

	laddr.sin_family = AF_INET;
	laddr.sin_addr.s_addr = htonl(INADDR_ANY);
	laddr.sin_port = htons(0);
	bind(lfd,  (struct sockaddr *)(&laddr), sizeof(struct sockaddr));
	listen(lfd, BACKLOG);
	*clientfd = lfd;
	sc->lfd = lfd;
	sc->mode = mode;
	if (sc->mode == 1)
		strncpy(sc->filename, filename, 128);

	ret = pthread_create(&data_thread, NULL, ftp_session_transport, sc);
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
