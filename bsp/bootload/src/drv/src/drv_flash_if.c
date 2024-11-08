 /*
 * drv_flash_if.c
 *
 *  Created on: 2022年10月19日
 *      Author: lwp edit
 */
#include "app_lib.h"
 

/*
********************************************************************************
********************************************************************************
*/ 
#define FLASH_START_ADRESS    0x08000000
#define FLASH_PAGE_NBPERBANK  256
#define FLASH_BANK_NUMBER     2

/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
**  函数名称:  GetPage
**  功能描述:  
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
static uint32_t GetPage(uint32_t Addr)
{
    uint32_t page = 0;

    if (Addr < (FLASH_BASE + FLASH_BANK_SIZE)){ /* Bank 1 */
        page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
    }else{ /* Bank 2 */
        page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
    }

    return page;
}
/*
********************************************************************************
**  函数名称:  GetBank
**  功能描述:  
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
static uint32_t GetBank(uint32_t Addr)
{
    return FLASH_BANK_1;
}
/*
********************************************************************************
**  函数名称:  FLASH_If_Erase
**  功能描述:  This function does an erase of all user flash area
**  输入参数:  无
**  输出参数:  无
**  返回参数:  0: user flash area successfully erased
**             1: error occurred
********************************************************************************
*/
uint32_t FLASH_If_Erase(uint32_t start)
{
    uint32_t NbrOfPages = 0;
    uint32_t PageError = 0;
    uint32_t FirstPage = 0,LastPage = 0;
    FLASH_EraseInitTypeDef pEraseInit;
    HAL_StatusTypeDef status = HAL_OK;

    HAL_FLASH_Unlock();
    
    FirstPage = GetPage(start);
    LastPage = GetPage(USER_FLASH_END_ADDRESS);
    
    NbrOfPages = LastPage - FirstPage + 1;

    // (0x08200000-0x08020000)/(1024*8)
    pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    pEraseInit.Banks = GetBank(start);
    pEraseInit.NbPages = NbrOfPages;
    pEraseInit.Page = FirstPage;
    status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);

    HAL_FLASH_Lock();

    if (status != HAL_OK){
        /* Error occurred while page erase */
        return -1;
    }
    return 0;
}

/*
********************************************************************************
**  函数名称:  FLASH_If_Erase
**  功能描述:  This function does an erase of all user flash area
**  输入参数:  无
**  输出参数:  无
**  返回参数:  0: user flash area successfully erased
**             1: error occurred
********************************************************************************
*/
uint32_t FLASH_If_EraseBlack(uint32_t StartAddr,uint32_t Size)
{
    uint32_t FirstPage = 0, NbOfPages = 0, BankNumber = 0;
    uint32_t PageError = 0;
    FLASH_EraseInitTypeDef EraseInitStruct;

    HAL_FLASH_Unlock();

    FirstPage = GetPage(StartAddr);

    NbOfPages = GetPage(StartAddr+Size) - FirstPage + 1;

    BankNumber = GetBank(StartAddr);
    
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks       = BankNumber;
    EraseInitStruct.Page        = FirstPage;
    EraseInitStruct.NbPages     = NbOfPages;
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK){
        return 0; 
    }
    HAL_FLASH_Lock();
    
    return 0;
}
/*
********************************************************************************
**  函数名称:  FLASH_If_Write
**  功能描述:  This function writes a data buffer in flash (data are 32-bit aligned).
**  输入参数:  FlashAddress: start address for writing data buffer
**             Data: pointer on data buffer
**             DataLength: length of data buffer (unit is 32-bit word)   
**  输出参数:  无
**  返回参数:  0: Data successfully written to Flash memory
**             1: Error occurred while writing data in Flash memory
**             2: Written Data in flash memory is different from expected one
********************************************************************************
*/

uint32_t FLASH_If_Write(uint32_t destination, uint32_t* pbuf ,uint32_t length)
{
    uint32_t status = PASSED;
    uint32_t i = 0;

    HAL_FLASH_Unlock();

    
    for (i = 0; (i < length/4) && (destination <= (USER_FLASH_END_ADDRESS-8)); i++){
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, destination, (uint32_t)(pbuf+i*4)) == HAL_OK) {
            destination = destination + 16; 
        }else{
            status = 2;
            break;
        }
    }

    HAL_FLASH_Lock();
    return status;
}
/*
********************************************************************************
**  函数名称:  FLASH_If_GetWriteProtectionStatus
**  功能描述:  获取当前写保护的状态
**  输入参数:  无
**  输出参数:  无
**  返回参数:  0: No write protected sectors inside the user flash area
**             1: Some sectors inside the user flash area are write protected
********************************************************************************
*/
uint32_t FLASH_If_Read(__IO uint32_t* FlashAddress,uint32_t *pbuf,uint32_t ulen )
{
    u32_t i=0;
      i=0;
      do{ 
        pbuf[i++] = *(uint32_t*)*FlashAddress;
        *FlashAddress += 4;
    }while(i<ulen);
    return 0;
}
/*
********************************************************************************
**  函数名称:  FLASH_If_Demo
**  功能描述:  板级初始化
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
****************3****************************************************************
*/
#if 1
uint32_t QuadWord[12] = { 0x12345678,
                         0x87654321,
                         0x12344321,
                         0x56788765, 
                         0x12345678,
                         0x87654321,
                         0x12344321,
                         0x56788765,
                         0x12345678,
                         0x87654321,
                         0x12344321,
                         0x56788765
                         };

void FLASH_If_Demo(void)
{
    uint32_t destination,Address;

    uint32_t length =FLASH_PAGE_SIZE;
    
    FLASH_If_Erase(APPLICATION_ADDRESS);
      
    destination = APPLICATION_ADDRESS;
  
   // FLASH_If_EraseBlack(destination,length);
    
    FLASH_If_Write(destination, QuadWord ,sizeof(QuadWord)/4);
#if 1
    Address = ADDR_FLASH_PAGE_16;
    HAL_FLASH_Unlock();
    while (Address < ADDR_FLASH_PAGE_16+FLASH_PAGE_SIZE)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, Address, ((uint32_t)QuadWord)) == HAL_OK)
        {
          Address = Address + 16; /* increment to next quad word*/
        }
       else
        {

        }
    }  
    HAL_FLASH_Lock();
#endif
}
#endif