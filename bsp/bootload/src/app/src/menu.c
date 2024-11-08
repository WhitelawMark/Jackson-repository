 /*
 * menu.c
 *
 *  Created on: 2022年10月19日
 *      Author: lwp edit
 */
#include "app_lib.h"
/*
********************************************************************************
********************************************************************************
*/ 
__IO uint32_t FlashProtection = 0;
uint8_t tab_1024[1024] ={0};
uint8_t FileName[FILE_NAME_LENGTH];

extern void JumpToApp(void);
void SerialDownload(void);
void SerialUpload(void);
/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
**  函数名称:  SerialDownload
**  功能描述:  串口下载程序
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void SerialDownload(void)
{
    uint8_t Number[10] = "          ";
    int32_t Size = 0;

    SerialPutString("Waiting for the file to be sent ... (press 'a' to abort)\n\r");
    Size = Ymodem_Receive(&tab_1024[0]);
  
    if (Size > 0){
        SerialPutString("\n\n\r Programming Completed Successfully!\n\r--------------------------------\r\n Name: ");
        SerialPutString(FileName);
        Int2Str(Number, Size);
        SerialPutString("\n\r Size: ");
        SerialPutString(Number);
        SerialPutString(" Bytes\r\n");
        SerialPutString("-------------------\n");
        Bl_ClrLocalDownLoad_Flg();
      //  Bl_UpdataCodeBackup();
    } else if (Size == -1) {
        SerialPutString("\n\n\rThe image size is higher than the allowed space memory!\n\r");
    } else if (Size == -2) {
        SerialPutString("\n\n\rVerification failed!\n\r");
    } else if (Size == -3) {
        SerialPutString("\r\n\nAborted by user.\n\r");
    } else {
        SerialPutString("\n\rFailed to receive the file!\n\r");
    }
}

/*
********************************************************************************
**  函数名称:  SerialUpload
**  功能描述:  串口上传任务
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void SerialUpload(void)
{
    uint8_t status = 0 ; 

    SerialPutString("\n\n\rSelect Receive File\n\r");

    if (GetKey() == CRC16) {
        /* Transmit the flash image through ymodem protocol */
        status = Ymodem_Transmit((uint8_t*)APPLICATION_ADDRESS, 
            (const uint8_t*)"UploadedFlashImage.bin", USER_FLASH_SIZE);

        if (status != 0) {
            SerialPutString("\n\rError Occurred while Transmitting File\n\r");
        } else {
            SerialPutString("\n\rFile uploaded successfully \n\r");
        }
    }
}

/*
********************************************************************************
**  函数名称:  Main_Menu
**  功能描述:  主菜单函数
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void Main_Menu(void)
{
    uint8_t key = 0;
    uint32_t timecnt = 0;
    

    SerialPutString("\r\n======================================================================");
    SerialPutString("\r\n=                  (C) COPYRIGHT YQ                                  =");
    SerialPutString("\r\n=                                                                    =");
    SerialPutString("\r\n=  STM32U575 In-Application Programming Application  (Version 1.0.0) =");
    SerialPutString("\r\n=                                                                    =");
    SerialPutString("\r\n=                     Please use YMODEM protocol software to upgrade =");
    SerialPutString("\r\n======================================================================"); 
    

    
    while (1) {
        SerialPutString("\r\n\r\n");
        SerialPutString("================== Main Menu =============================\r\n\n");
        SerialPutString("  Download Image To the STM32U575 Internal Flash ------- 1\r\n\n");
        SerialPutString("  Upload Image From the STM32U575 Internal Flash ------- 2\r\n\n");
        SerialPutString("  Erase internal flash --------------------------------- 3\r\n\n");
        SerialPutString("  Execute The New Program ------------------------------ 4\r\n\n");
        SerialPutString("==========================================================\r\n\n");

        /* Receive key */
        key = GetKey();
        if (key == 0) {
            break;
        } else if (key == 0x31) {/* Download user application in the Flash */
            SerialDownload();
        } else if (key == 0x32) {/* Upload user application from the Flash */
            SerialUpload();
        } else if (key == 0x33) { /* erase  the Flash*/
            FLASH_If_Erase(APPLICATION_ADDRESS);
            SerialPutString("Erase internal flash \r\n\n");
        } else if (key == 0x34) { /* execute the new program */
            SerialPutString("Execute the new program\r\n\n");
            JumpToApp();
        }  
        if ( timecnt++ > 3 ) {
            break;
        }
        delay_1ms(2000);
    }
}

