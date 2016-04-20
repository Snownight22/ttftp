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
	{"open", NULL, FTP_SERVER_NOTCONNECTED, FTP_IDENTIFY_INVALID, FTP_HAS_ARGS, ftp_ctrl_session},
	{"user", "USER", FTP_SERVER_CONNECTED, FTP_IDENTIFY_INVALID, FTP_HAS_ARGS, ftp_ctrl_identify},
	{"password", "PASS", FTP_SERVER_CONNECTED, FTP_IDENTIFY_INVALID, FTP_HAS_ARGS, NULL},
	{"system", "SYST", FTP_SERVER_CONNECTED, FTP_IDENTIFY_VALID, FTP_HAS_NO_ARGS, ftp_ctrl_getmsg},
	{"list", "LIST", FTP_SERVER_CONNECTED, FTP_IDENTIFY_VALID, FTP_HAS_NO_ARGS, ftp_ctrl_list},
};

static int command_analysis(char *string, char *command, char *args)
{
	int length = strlen(string);
	int i;

	strncpy(command, string, length);
	for (i = 0;i < length; i++)
	{
		if ((string[i] == ' ') ||
			(string[i] == '\t'))
		{
			string[i] = '\0';
			strcpy(command, string);
			strcpy(args, &string[i+1]);
		}
	}

	return FTP_OK;
}

int ftp_ctrl_getmsg(void *arg1, void *arg2)
{
	stFtpContext *fc = (stFtpContext *)arg1;
	char *command = (char *)arg2;
	char reply[1024] = {0};
	char send_command[128] = {0};
	int ret;

	snprintf(send_command, 127, "%s\r\n", command);
	if (0 >= (ret = ftp_session_command(fc->serverfd, send_command)))
	{
		fprintf(stderr, "send command:%s error\n", command);
		return FTP_OK;
	}
	if (0 <= (ret = ftp_session_getreply(fc->serverfd, reply, 1024)))
	{
		reply[ret] = '\0';
		fprintf(stdout, reply);
	}

	return FTP_OK;
}

int ftp_ctrl_list(void *arg1, void *arg2)
{
	stFtpContext *fc = (stFtpContext *)arg1;
	int ret;
	char command[128] = {0};

	if (fc->ispassive == FTP_NOT_PASSIVE)
	{
		ret = ftp_session_data(&fc->ldataaddr, &fc->ldataport);
		if (0 < ret)
		{
			snprintf(command, 127, "PORT %u,%u,%u,%u,%u,%u", (fc->ldataaddr>>24)&0x000000ff, (fc->ldataaddr>>16) & 0x000000ff, (fc->ldataaddr>>8)&0x000000ff, (fc->ldataaddr&0x000000ff), (fc->ldataport >>8)&0x00ff, (fc->ldataport & 0x00ff));
			ftp_ctrl_getmsg(fc, command);
		}
		else
		{
			fprintf(stderr, "listen data port error\n");
		}
	}

	return FTP_OK;
}

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
	char *domain = (char *)arg2;
	char command[128] = {0};
	char reply[1024] = {0};
	char user[32] = {0};
	int length;
	int ret;

	if (NULL != domain)
		strncpy(fc->ftpDomain, domain, strlen(domain));
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
	stFtpCommand *ctrl;
	//char *ftpDomain = domain;
	char command[128] = {0};
	char input[128] = {0};
	char args[128] = {0};
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
		fgets(input, 127, stdin);
		length = strlen(input);
		input[length-1] = '\0';
		memset(command, 0, sizeof(command));
		memset(args, 0, sizeof(args));
		command_analysis(input, command, args);
		if (!strcmp(command, "quit"))
		{
			fprintf(stdout, "byebye\n");
			ftp_session_destory(fc->serverfd);
			break;
		}
		
		for (i = 0; i < command_len; i++)
		{
			ctrl = &g_ctrl_commands[i];
			if (!strcmp(command, ctrl->command))
			{
				if ((ctrl->connect_status == FTP_SERVER_CONNECTED) && (fc->isconnected == FTP_SERVER_NOTCONNECTED))
				{
					fprintf(stdout, "Not connected.\n");
					break;
				}
				if (ctrl->func)
				{
					if (ctrl->has_args)
						ctrl->func(fc, args);
					else
						ctrl->func(fc, ctrl->ftpcommand);
				}
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
	fc->ispassive = FTP_NOT_PASSIVE;

	return FTP_OK;
}
