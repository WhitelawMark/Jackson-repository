/*
 * quectel_mqtt.h
 *
 *  Created on: 2023年07月17日
 *      Author: lwp
 */
 
#ifndef _QUECTEL_MQTT_H_
#define _QUECTEL_MQTT_H_
#include "bg77.h" 
/*
********************************************************************************
********************************************************************************
*/ 
typedef struct
{
    char recdata[500];
    char snddata[500];
    char topicbuf[50];
    char *pubtopic;
    char *subtopic;
    uint32_t waittime;
    char retrycount;
    char sendflag;
    char connect;
    char sub;
    char alarm;
}MQTT_BufType;

/*
********************************************************************************
********************************************************************************
*/ 
int quectel_mqtt_init(void);
int quectel_mqtt_send(u8_t *pmsg, int ulen);

int quectel_https_download(char *url);
int quectel_https_upload(char *url);

int quectel_file_download(char *srcfile,char *dscfile);
int quectel_file_upload(char *srcfile,char *dscfile);
#endif 
/* _QUECTEL_MQTT_H_ */

