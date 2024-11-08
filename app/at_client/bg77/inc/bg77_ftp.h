 /*
 *  bg77_ftp.h
 *
 *  Created on: 2022年08月08日
 *      Author: root
 */
#ifndef _BG77_FTP_H_
#define _BG77_FTP_H_
 
/*
********************************************************************************
********************************************************************************
*/
 


/*
********************************************************************************
********************************************************************************
*/
typedef struct {
    int   address;                                  /* 基地址 */
    int   maxlen;                                   /* 最大长度 */
    int (*finit)(int evtype);                       /* 初始化函数 */
    int (*fread)(int address, char *buf, int len);  /* 数据读取函数 */
    int (*fwrite)(int address, char *buf, int len); /* 数据写入函数 */
    int (*operation)(int opttype,int filelen);                  /* 操作回调函数 */ 
} ftp_device_t;

typedef struct {
    int   filesize;                                 /*文件长度 */
    at_addr_t addr;                                 /*网络地址、端口*/
    char filename[64];                              /*文件名 */
    char username[64];                              /*FTP用户名 */      
    char password[64];                              /*FTP密码 */
    ftp_device_t device;
} ftp_msg_t;

/*
********************************************************************************
********************************************************************************
*/

int __bg77_ftp_upload(socket_t *socket, ftp_msg_t *msg);
int __bg77_ftp_download(socket_t *socket, ftp_msg_t *msg);

/*
********************************************************************************
********************************************************************************
*/
int bg77_ftp_post(int type, char *ip, int port, const char *filename,
                      const char *username,const char *password, 
                      ftp_device_t *device);


int bg77_import_file(char *filename,int filesize,ftp_msg_t *msg);
int bg77_export_file(char *filename,int filesize,ftp_msg_t *msg);

#endif /* _BG77_FTP_H_ */

