/* FileName:ftp_command.c;
 *
 * Author:snownight;
 * Date:2016.4.6;
 *
 */
#include <stdio.h>
#include <string.h>

#include "ftp_err.h"
#include "ftp_command.h"

stFtpContext g_ftpcontext;

extern int ftp_session_create(char *ftpDomain, int ftpPort);

int ftp_command_proc(char *domain, int port)
{
	//char *ftpDomain = domain;
	char command[128] = {0};
	int length;
	stFtpContext *fc = &g_ftpcontext;

	if (NULL != domain)
	{
		strncpy(fc->ftpDomain, domain, FTP_DOMAIN_LENGTH_MAX-1);
		fc->ftpPort = port;
		if (FTP_OK == (ftp_session_create(fc->ftpDomain, fc->ftpPort)))
		{
		    fc->isconnected = FTP_SERVER_CONNECTED;
		}
	}

	while(1)
	{
		fprintf(stdout, FTP_COMMAND_PROMPT);
		fgets(command, 127, stdin);
		length = strlen(command);
		command[length-1] = '\0';
		if (!strcmp(command, "quit"))
		{
			fprintf(stdout, "byebye\n");
			break;
		}
	}

	return FTP_OK;
}
