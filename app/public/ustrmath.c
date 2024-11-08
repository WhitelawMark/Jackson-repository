/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */

#include "type.h" 
#include <stdio.h>
#include <string.h>
/*
******************************************************************************** 
*Function    : ustraverf
*Description : 计算数据平均值（去掉一个最大值，去掉最小值）
*Input       :
*Output      :
*Return      :
*Others      :
******************************************************************************** 
*/
float ustraverf(float *pstr,u16_t len)
{
     float  tmp_f = 0;
 
     tmp_f=0;
     for(u16_t i=0 ; i<len ; i++){
         tmp_f+=pstr[i];
     }  
     tmp_f /= len;
     return tmp_f;  
}
/*
******************************************************************************** 
*Function    : ustraver
*Description : 计算数据平均值（去掉一个最大值，去掉最小值）
*Input       :
*Output      :
*Return      :
*Others      :
******************************************************************************** 
*/
u16_t ustraver(u16_t *pstr,u16_t len)
{
     u16_t  tmp_u16 = 0;
 
     tmp_u16=0;
     for(u16_t i=0 ; i<len ; i++){
         tmp_u16+=pstr[i];
     }  
     tmp_u16 /= len;
     return tmp_u16;  
}
/*
******************************************************************************** 
*Function    : str_average
*Description : 计算数据平均值（去掉一个最大值，去掉最小值）
*Input       :
*Output      :
*Return      :
*Others      :
******************************************************************************** 
*/
unsigned int str_average(unsigned int *pstr,u8_t strnum)
{
    unsigned int maxval,minval,averval,totalval;
    u8_t maxoffer,minoffer;
    u8_t i;
    
    i=0;
    maxoffer=0;
    minoffer=0;
    minval=pstr[i];
    maxval=pstr[i];
    
    for(i=0;i<strnum;i++){
        if(minval>pstr[i]){
           minval=pstr[i];
           minoffer=i;
        }
        if(maxval<pstr[i]){
           maxval=pstr[i];
           maxoffer=i;
        }     
    }
    
    totalval=0;
    for(i=0;i<strnum;i++){
        if(i==minoffer){
        //    printf("min adcval[%d] =%d\r\n",i,adcval[i]);
        }else if(i==maxoffer){
      //      printf("max adcval[%d] =%d\r\n",i,adcval[i]);
        }else{
            totalval+=pstr[i];
        }
    }
    if(minoffer==maxoffer){
        averval=totalval/(strnum-1);
    }else{
        averval=totalval/(strnum-2);
    }
    //printf("totalval %d averval %d \r\n",totalval,averval);
    return averval;
}
/*
******************************************************************************** 
*Function    : str_averageu16
*Description : 计算数据平均值（去掉一个最大值，去掉最小值）
*Input       : pStr    -- 指向字符串的首地址
*Output      : none
*Return      : 返回字符串的长度
*Others      : 字符串必须以'\0'为结束标志
******************************************************************************** 
*/
u16_t str_averageu16(u16_t *pstr,u8_t strnum)
{
    u32_t maxval,minval,averval,totalval;
    u8_t maxoffer,minoffer;
    u8_t i;
    
    i=0;
    maxoffer=0;
    minoffer=0;
    minval=pstr[i];
    maxval=pstr[i];
    
    for(i=0;i<strnum;i++){
        if(minval>pstr[i]){
           minval=pstr[i];
           minoffer=i;
        }
        if(maxval<pstr[i]){
           maxval=pstr[i];
           maxoffer=i;
        }     
    }
    totalval=0;
    for(i=0;i<strnum;i++){
        if(i==minoffer){
          
          
        }else if(i==maxoffer){

        }else{
            totalval+=pstr[i];
        }
    }
    
    
    if(minoffer==maxoffer){
        averval=totalval/(strnum-1);
    }else{
        averval=totalval/(strnum-2);
    }
    //printf("totalval %d averval %d \r\n",totalval,averval);
    return averval;
}