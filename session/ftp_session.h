/* FileName:ftp_session.h;
 *
 * Author:snownight;
 * Date:2016.4.6;
 *
 */
#ifndef _FTP_SESSION_H_
#define _FTP_SESSION_H_

#define FTP_PORT_DEFAULT    (21)

int ftp_session_create(char *ftpDomain, int ftpPort);
int ftp_session_config(int fd, long *listenip, int *listenport);
int ftp_session_data(long *listenip, int *listenport);
int ftp_session_getreply(int fd, char *reply, int length);
int ftp_session_command(int fd, char *command);
int ftp_session_destory(int fd);

#endif
