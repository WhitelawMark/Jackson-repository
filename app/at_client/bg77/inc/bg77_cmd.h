 /*
 *  bg77_cmd.h
 *
 *  Created on: 2022年08月08日
 *      Author: root
 */
#ifndef _BG77_CMD_H_
#define _BG77_CMD_H_
 
/*
********************************************************************************
********************************************************************************
*/
 
#define GPS_GLONASS    1
#define GPS_BeiDou     2
#define GPS_Galileo    3
#define GPS_QZSS       4 
 
/*
********************************************************************************
********************************************************************************
*/
int bg77_cmd_line_with_len(const char *atcmd, int atcmdlen,const char *ack,
                      char *xbuf, int xbuf_size,unsigned int timeout_ms, int det_err);

int bg77_cmd_line_get(char *suffix,char *xbuf, int xbuf_size, unsigned int timeout_ms);

int bg77_cmd_line( const char *atcmd,const char *ack,char *xbuf, int xbuf_size,unsigned int timeout_ms, int det_err);

/*
********************************************************************************
********************************************************************************
*/
int bg77_file_size(const char *file);
/*
********************************************************************************
********************************************************************************
*/
int bg77_ps_attach_detect(void);
int bg77_csq_detect(void);
int bg77_sim_card_detect(void);
int bg77_iccid_detect(void);
int bg77_model_detect(void);
int bg77_at_resp_detect(void);

#endif /* _BG77_CMD_H_ */

