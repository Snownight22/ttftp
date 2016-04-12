/* FileName:ftp_main.c;
 *
 * Author:snownight;
 * Date:2016.4.6;
 *
 */
#include <stdio.h>

#include "ftp_err.h"

extern int ftp_command_proc(char *domain, int port);

int main(int argc, char *argv[])
{
	ftp_command_proc("192.168.29.152", 0);
	//ftp_session_create("192.168.29.152", 0);
	return FTP_OK;
}
