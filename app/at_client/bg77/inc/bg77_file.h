 /*
 *  BG77_FILE.h
 *
 *  Created on: 2022年08月08日
 *      Author: root
 */
#ifndef _BG77_FILE_H_
#define _BG77_FILE_H_
 
/*
********************************************************************************
********************************************************************************
*/
#define SEED_SET       0       
#define SEED_CUR       1  
#define SEED_END       2   

#define FILE_UPL       1  
#define FILE_DWL       2   
#define FILE_IMP       3  
#define FILE_EMP       4  

/*
********************************************************************************
********************************************************************************
*/

typedef struct {
    int  type;
    char *to;
    char *from;
} file_msg_t;
/*
********************************************************************************
********************************************************************************
*/
int bg77_qfupl(char *filename,char *buf,int size);

int bg77_qfdel(char *filename);
int bg77_qfopen(char *filename,int mode);
int bg77_qfclose(int handle);

int bg77_qfposition(int handle);

int bg77_qfseed(int handle,int offset,int position);
int bg77_qfread(int handle,int size,char *buf,int bufsize);
int bg77_qfwrite(int handle,int timeout,char *buf,int size);
/*
********************************************************************************
********************************************************************************
*/
int __bg77_file_operation(socket_t *socket, file_msg_t *msg);
/*
********************************************************************************
********************************************************************************
*/
int bg77_file_export(int sd,char *to,char *from, void(*res_cb)(int res, void *arg), void *arg);
int bg77_file_import(int sd,char *to,char *from, void(*res_cb)(int res, void *arg), void *arg);

#endif /* _BG77_FILE_H_ */

