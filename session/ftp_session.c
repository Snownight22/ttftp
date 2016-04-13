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

#include "ftp_err.h"
#include "ftp_session.h"

int ftp_session_create(char *ftpDomain, int ftpPort)
{
	int ftpfd;
	struct hostent *ftphost;
	struct sockaddr_in ftpaddr;

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

	fprintf(stdout, "Connected to %s (%s)\n", ftpDomain, inet_ntoa(ftpaddr.sin_addr));
	return ftpfd; 
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
