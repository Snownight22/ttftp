/* FileName:ftp_command.c;
 *
 * Author:snownight;
 * Date:2016.4.6;
 *
 */
#include <stdio.h>
#include <string.h>

#include "ftp_err.h"
#include "ftp_ctrl.h"
#include "../session/ftp_session.h"

stFtpContext g_ftpcontext;

static stFtpCommand g_ctrl_commands[] = 
{
	{"open", NULL, FTP_SERVER_NOTCONNECTED, FTP_IDENTIFY_INVALID, ftp_ctrl_session},
	{"user", "USER", FTP_SERVER_CONNECTED, FTP_IDENTIFY_INVALID, ftp_ctrl_identify},
	{"password", "PASS", FTP_SERVER_CONNECTED, FTP_IDENTIFY_INVALID, NULL},
};

int ftp_ctrl_identify(void *arg1, void *arg2)
{
	stFtpContext *fc = (stFtpContext *)arg1;
	char *user = (char *)arg2;
	char command[128] = {0};
	char reply[1024] = {0};
	char pwd[32] = {0};
	int length;
	int ret;

	snprintf(command, 127, "USER %s\r\n", user);
	if (0 >= (ret = ftp_session_command(fc->serverfd, command)))
	{
		fprintf(stderr, "send user error\n");
		return FTP_OK;
	}
	if (0 <= (ret = ftp_session_getreply(fc->serverfd, reply, 1024)))
	{
		reply[ret] = '\0';
		fprintf(stdout, reply);
	}

	fprintf(stdout, "Password:");
	fgets(pwd, 31, stdin);
	pwd[strlen(pwd)-1] = '\0';
	//if (0 == length)
		//strncpy(pwd, "User@", strlen("User@"));
	snprintf(command, 127, "PASS %s\r\n", pwd);
	if (0 >= (ret = ftp_session_command(fc->serverfd, command)))
	{
		fprintf(stderr, "send pwd error\n");
		return FTP_OK;
	}
	if (0 <= (ret = ftp_session_getreply(fc->serverfd, reply, 1024)))
	{
		reply[ret] = '\0';
		fprintf(stdout, reply);
		fc->isidentified = FTP_IDENTIFY_VALID;
	}

	return FTP_OK;
}

int ftp_ctrl_session(void *arg1, void *arg2)
{
	stFtpContext *fc = (stFtpContext *)arg1;
	char command[128] = {0};
	char reply[1024] = {0};
	char user[32] = {0};
	int length;
	int ret;

	if (0 <= (ret = ftp_session_create(fc->ftpDomain, fc->ftpPort)))
	{
		fc->serverfd = ret;
		fc->isconnected = FTP_SERVER_CONNECTED;
		if (0 <= (ret = ftp_session_getreply(fc->serverfd, reply, 1024)))
		{
			reply[ret] = '\0';
			fprintf(stdout, reply);
		}

		fprintf(stdout, "Name (%s: anonymous as default):", fc->ftpDomain);
		fgets(user, 31, stdin);
		user[strlen(user)-1] = '\0';
		length = strlen(user);
		if (0 == length)
			strncpy(user, "anonymous", strlen("anonymous"));
		
		ftp_ctrl_identify(fc, user);

		return FTP_OK;
	}

	return FTP_ERR;
}

int ftp_ctrl_proc(char *domain, int port)
{
	stFtpContext *fc = &g_ftpcontext;
	stFtpCommand *ctrl = g_ctrl_commands;
	//char *ftpDomain = domain;
	char command[128] = {0};
	char reply[1024] = {0};
	char user[32] = {0};
	int length;
	int ret;
	int command_len = sizeof(g_ctrl_commands)/sizeof(stFtpCommand);
	int i;

	if (NULL != domain)
	{
		strncpy(fc->ftpDomain, domain, FTP_DOMAIN_LENGTH_MAX-1);
		fc->ftpPort = port;
		ftp_ctrl_session(fc, NULL);
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
		
		for (i = 0; i < command_len; i++)
		{
			if (!strcmp(command, ctrl[i].command))
			{
				ctrl[i].func(fc, command);
			}
		}
	}

	return FTP_OK;
}

int ftp_ctrl_init()
{
	stFtpContext *fc = &g_ftpcontext;
	memset(fc, 0, sizeof(stFtpContext));

	fc->ftpPort = FTP_PORT_DEFAULT;	
	fc->isconnected = FTP_SERVER_NOTCONNECTED;
	fc->isidentified = FTP_IDENTIFY_INVALID;
	fc->serverfd = -1;

	return FTP_OK;
}
