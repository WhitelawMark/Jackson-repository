 /*
 *  bg77_mqtt.h
 *
 *  Created on: 2022年08月08日
 *      Author: root
 */
#ifndef _BG77_MQTT_H_
#define _BG77_MQTT_H_
 
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
int bg77_qmt_open(int sd, at_addr_t *addr);
int bg77_mqtt_open(int connect_id, at_addr_t *addr, const char *clientid, const char *username, const char *password);
int __bg77_qmt_close(int sd);

void __bg77_urc_qmtrecv(const char *data, int len);
void __bg77_urc_qmtstat(const char *data, int len);

int __bg77_mqtt_unsubscribe(socket_t *socket, sub_msg_t *msg);
int __bg77_mqtt_subscribe(socket_t *socket, sub_msg_t *msg);
int __bg77_mqtt_publish(socket_t *socket, mqtt_msg_t *msg);
/*
********************************************************************************
********************************************************************************
*/
int bg77_mqtt_connect(int sd, const char *peer_ip, unsigned short peer_port,
                     const char *clientid, const char *username, const char *password,
                     void(*res_cb)(int res, void *arg), void *arg);
int bg77_mqtt_unsubscribe(int sd, int msgid, mqtt_topic_t *topics, int count, void(*res_cb)(int res, void *arg), void *arg);
int bg77_mqtt_subscribe(int sd, int msgid, mqtt_topic_t *topics, int count, void(*res_cb)(int res, void *arg), void *arg);

int bg77_mqtt_publish(int sd, int msgid, int qos, int retain,
                      const char *topic,
                      const char *payload, int payload_len,
                      void(*res_cb)(int res, void *arg), void *arg);

at_ssize_t bg77_mqtt_recv(int sd, int *msgid, void *topic, at_size_t topic_len,
                          void *payload, at_size_t payload_len);
#endif /* _BG77_MQTT_H_ */

