/* FileName:ftp_main.c;
 *
 * Author:snownight;
 * Date:2016.4.6;
 *
 */
#include <stdio.h>

#include "ftp_err.h"

#define TEST_URL    "mirrors.ustc.edu.cn"
extern int ftp_ctrl_proc(char *domain, int port);
extern int ftp_ctrl_init();

int main(int argc, char *argv[])
{
	ftp_ctrl_init();
	ftp_ctrl_proc(NULL, 0);//TEST_URL/*"192.168.29.152"*/, 0);
	//ftp_session_create("192.168.29.152", 0);
	return FTP_OK;
}
