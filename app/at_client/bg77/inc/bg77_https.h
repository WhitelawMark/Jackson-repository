 /*
 *  bg77_https.h
 *
 *  Created on: 2022年08月08日
 *      Author: root
 */
#ifndef _BG77_HTTPS_H_
#define _BG77_HTTPS_H_
 
/*
********************************************************************************
********************************************************************************
*/

typedef struct {
    int  type;
    char *url;
    char *ca_file;
    char *client_file;
    char *key_file;
} https_msg_t;
/*
********************************************************************************
********************************************************************************
*/
int __bg77_https_download(socket_t *socket, https_msg_t *msg);
int __bg77_https_upload(socket_t *socket, https_msg_t *msg);
/*
********************************************************************************
********************************************************************************
*/
int bg77_https_download(int sd,char *url,char *ca_file,char *client_file, char *key_file,
                     void(*res_cb)(int res, void *arg), void *arg);

int bg77_https_upload(int sd,char *url,char *ca_file,char *client_file, char *key_file,
                     void(*res_cb)(int res, void *arg), void *arg);

#endif /* _BG77_HTTPS_H_ */

