/* FileName:ftp_command.c;
 *
 * Author:snownight;
 * Date:2016.4.6;
 *
 */
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

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
	{"ls", "LIST", FTP_SERVER_CONNECTED, FTP_IDENTIFY_VALID, FTP_HAS_NO_ARGS, ftp_ctrl_list},
	{"passive", "PASV", FTP_SERVER_CONNECTED, FTP_IDENTIFY_VALID, FTP_HAS_NO_ARGS, ftp_ctrl_setpassive},
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

static int reply_analysis(char *reply, int *errcode)
{
	int i = 0;
	int length = strlen(reply);
	char code[4] = {0};

	strncpy(code, reply, 3);
	*errcode = atoi(code);

	return FTP_OK;
}

int ftp_ctrl_pasvmsg(char *reply, int *errcode, long *sip, int *sport)
{
	char *cp;
	unsigned int v[6];

	cp = strchr(reply, '(');
	if (NULL == cp)
		return FTP_ERR;
	cp++;

	sscanf(cp, "%u,%u,%u,%u,%u,%u", &v[2], &v[3], &v[4], &v[5], &v[0], &v[1]);
	*sip = (v[2] << 24) + (v[3] << 16) + (v[4] << 8) + v[5];
	*sport = (v[0] << 8) + v[1];

	return FTP_OK;
}

int ftp_ctrl_setpassive(void *arg1, void *arg2)
{
	stFtpContext *fc = (stFtpContext *)arg1;

	fc->ispassive = !fc->ispassive;
	fprintf(stdout, "passive mode %s\n", fc->ispassive ? "on":"off");

	return FTP_OK;
}

int ftp_ctrl_getmsg(void *arg1, void *arg2)
{
	stFtpContext *fc = (stFtpContext *)arg1;
	char *command = (char *)arg2;
	char reply[1024] = {0};
	char send_command[128] = {0};
	int ret;
	int errcode;

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

	reply_analysis(reply, &errcode);
	if (errcode == 227)
		ftp_ctrl_pasvmsg(reply, &errcode, &fc->fdataaddr, &fc->fdataport);

	return FTP_OK;
}

//extern sem_t g_sem;
int ftp_ctrl_list(void *arg1, void *arg2)
{
	stFtpContext *fc = (stFtpContext *)arg1;
	int ret;
	char command[128] = {0};
	int errcode;
	char fip[16] = {0};
	char *p;

	if (fc->ispassive == FTP_NOT_PASSIVE)
	{
		ret = ftp_session_config(fc->serverfd, &fc->ldataaddr, &fc->ldataport);
		ret = ftp_session_data(&fc->clientfd, &fc->ldataport);
		if (0 < ret)
		{
			//sem_init(&g_sem, 0, 0);
			snprintf(command, 127, "PORT %u,%u,%u,%u,%u,%u", (fc->ldataaddr>>24)&0x000000ff, (fc->ldataaddr>>16) & 0x000000ff, (fc->ldataaddr>>8)&0x000000ff, (fc->ldataaddr&0x000000ff), (fc->ldataport >>8)&0x00ff, (fc->ldataport & 0x00ff));
			ftp_ctrl_getmsg(fc, command);
			ftp_ctrl_getmsg(fc, "LIST");
			sleep(1);
			ret = ftp_session_getreply(fc->serverfd, command, 128);
			if (0 < ret)
			{
				command[ret] = '\0';
			    fprintf(stdout, command);
			}
			//sem_wait(&g_sem);
			//sem_destroy(&g_sem);
		}
		else
		{
			fprintf(stderr, "listen data port error\n");
		}
	}
	else
	{
		fprintf(stdout, "passive mode\n");
		ftp_ctrl_getmsg(fc, "PASV");
		*p = fc->fdataaddr;
		snprintf(fip, 15, "%u.%u.%u.%u", (p[0]>>24) & 0xff, (p[1]>>16) & 0xff, (p[2]>>8) & 0xff, p[3]& 0xff);
		fprintf(stdout, "faddr:%s, port:%d\n", fip, fc->fdataport);
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
		if (length <= 1)
			continue;
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
				break;
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
