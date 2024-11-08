/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */

#ifndef __ESP_MQT_H                                                          
#define __ESP_MQT_H    

/*
********************************************************************************
********************************************************************************
*/
typedef struct {
   int qos;
   char *topic;
} mqtt_topic_t;


typedef struct {
    int msgid;
    int number;
    mqtt_topic_t *topics;
} sub_msg_t;

typedef struct {
    int msgid;
    int retain;
    int qos;
    int topic_len;
    int payload_len;
    char *topic;
    char *payload;
} mqtt_msg_t;

/*
********************************************************************************
********************************************************************************
*/
int __esp_mqtt_subscribe(socket_t *socket, sub_msg_t *msg);
int __esp_mqtt_publish(socket_t *socket, mqtt_msg_t *msg);
int __esp_mqtt_unsubscribe(socket_t *socket, sub_msg_t *msg);
int __esp_mqtt_open(int connect_id, at_addr_t *addr, const char *clientid, const char *username, const char *password);
int __esp_mqtt_close(int sd);

/*
********************************************************************************
********************************************************************************
*/
int esp_mqtt_publish(int sd, int msgid, int qos, int retain,
                      const char *topic,
                      const char *payload, int payload_len,
                      void(*res_cb)(int res, void *arg), void *arg);

int esp_mqtt_subscribe(int sd, int msgid, 
                       mqtt_topic_t *topics, int count,
                       void(*res_cb)(int res, void *arg), void *arg);

int esp_mqtt_unsubscribe(int sd, int msgid, 
                         mqtt_topic_t *topics, int count, 
                         void(*res_cb)(int res, void *arg), void *arg);

int esp_mqtt_connect(int sd, const char *peer_ip, unsigned short peer_port,
                     const char *clientid, const char *username, const char *password,
                     void(*res_cb)(int res, void *arg), void *arg);

int esp_mqtt_init(char *ssid,char *pwd);
#endif 
/* __ESP_MQT_H */
