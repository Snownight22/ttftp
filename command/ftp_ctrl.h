/* FileName:ftp_command.h;
 *
 * Author:snownight;
 * Date:2016.4.6;
 *
 */
#ifndef _FTP_COMMAND_H_
#define _FTP_COMMAND_H_

//#define FTP_PORT_DEFAULT          (21)

#define FTP_DOMAIN_LENGTH_MAX    (128)

#define FTP_SERVER_NOTCONNECTED    (0)
#define FTP_SERVER_CONNECTED       (1)

#define FTP_IDENTIFY_INVALID       (0)
#define FTP_IDENTIFY_VALID         (1)

typedef struct ftp_context
{
	char ftpDomain[FTP_DOMAIN_LENGTH_MAX];
	int ftpPort;
	int isconnected;
	int isidentified;
	int serverfd;
}stFtpContext;

#define FTP_COMMAND_PROMPT    "Ftp>>"

typedef int (*action_func)(void*);

typedef struct ftp_command
{
	char *command;
	char *ftpcommand;
	int connect_status;
	int identify_status;
	action_func func;
}stFtpCommand;

#endif