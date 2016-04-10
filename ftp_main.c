/* FileName:ftp_main.c;
 *
 * Author:snownight;
 * Date:2016.4.6;
 *
 */
#include <stdio.h>

#include "ftp_err.h"

extern int ftp_session_create(char *ftpDomain, int ftpPort);

int main(int argc, char *argv[])
{
	ftp_session_create("mirrors.ustc.edu.cn", 0);
	return FTP_OK;
}
