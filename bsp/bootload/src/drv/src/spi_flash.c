
#include "board.h"
#include "spi_flash.h"


/*
********************************************************************************
********************************************************************************
*/ 
/*有关外部flash控制相关命令的宏定义如下*/
#define WriteEnable             0x06
#define WriteDisable            0x04
#define ReadStatusReg           0x05
#define WriteStatusReg          0x01
#define ReadData                0x03
#define FastReadData            0x0B
#define FastReadDual            0x3B
#define PageProgram             0x02
#define BlockErase              0xD8
#define SectorErase             0x20   //
#define ChipErase               0xC7   //0x60
#define PowerDown               0xB9
// #define ReleasePowerDown        0xAB
#define DeviceID                0xAB
#define ManufactDeviceID        0x90
#define JedecDeviceID           0x9F


uint8_t const W25X_WriteEnable      =      0x06;
uint8_t const W25X_WriteDisable     =      0x04;
uint8_t const W25X_ReadStatusReg    =      0x05;
uint8_t const W25X_WriteStatusReg   =      0x01;
uint8_t const W25X_ReadData         =      0x03;
uint8_t const W25X_ReadData4Byte    =      0x13;//? 这边可能要改因为容量只有4M
uint8_t const W25X_FastReadData     =      0x0B;
uint8_t const W25X_FastReadDual     =      0x3B;
uint8_t const W25X_PageProgram      =      0x02;
uint8_t const W25X_PageProgram4Byte =      0x12;//? 这边可能要改因为容量只有4M
 // uint8_t const W25X_PageProgram4Byte =      0x38;

uint8_t const W25X_BlockErase       =      0xD8;
// uint8_t const W25X_BlockErase4Byte  =      0xDC;//
uint8_t const W25X_SectorErase      =      0x20;
uint8_t const W25X_SectorErase4Byte =      0x21;//? 这边可能要改因为容量只有4M 地址宽度24位
uint8_t const W25X_ChipErase        =      0xC7;
uint8_t const W25X_PowerDown        =      0xB9;
// uint8_t const W25X_ReleasePowerDown =      0xAB; //?
uint8_t const W25X_DeviceID         =      0xAB;
uint8_t const W25X_ManufactDeviceID =      0x90;
uint8_t const W25X_JedecDeviceID    =      0x9F;
uint8_t const W25X_ResetEnable      =      0x66;
uint8_t const W25X_ResetMemory      =      0x99;

uint8_t const W25X_FastReadQuad     =      0x6B;
uint8_t const W25X_EraseSuspend     =      0x75;
uint8_t const W25X_EraseResume      =      0x7A;

/*
********************************************************************************
********************************************************************************
*/ 
#define FLASH_BUF_MAX_LEN        (4096+30)      //内部静态缓存定义

#define W25QX_Enable()           HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,   GPIO_PIN_RESET);//PB0
#define W25QX_Disable()          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,   GPIO_PIN_SET);
/*
********************************************************************************
********************************************************************************
*/ 
/*FLASH内部使用的一个缓存区*/    
static unsigned char FlashTempBuf[ FLASH_BUF_MAX_LEN ];  

static SPI_HandleTypeDef hspi;
/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
**  函数名称:  MX_SPI1_Init
**  功能描述:  
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
static void MX_SPI1_Init(void)
{
  
/*    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio_init_struct;
    gpio_init_struct.Pin = GPIO_PIN_0;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull = GPIO_PULLUP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio_init_struct);
  */
  
    /* SPI3 parameter configuration*/
    hspi.Instance = SPI1;
    hspi.Init.Mode = SPI_MODE_MASTER;
    hspi.Init.Direction = SPI_DIRECTION_2LINES;
    hspi.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi.Init.NSS = SPI_NSS_SOFT;
    hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi.Init.CRCPolynomial = 7;

    if (HAL_SPI_Init(&hspi) != HAL_OK)
    {
       // Error_Handler();
    }
}
/*
********************************************************************************
**  函数名称:  W25QXXReadSR
**  功能描述:  
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
static uint8_t W25QXXReadSR(void)
{
    uint8_t value;
    uint8_t cmd = W25X_ReadStatusReg;

    W25QX_Enable();

    HAL_SPI_Transmit(&hspi,&cmd,1,1000);

    HAL_SPI_Receive(&hspi,&value, 1, 1000);

    W25QX_Disable();

    return value;
}
/*
********************************************************************************
**  函数名称:  W25QXXWriteSR
**  功能描述:  
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void W25QXXWriteSR(uint8_t SR)
{
    uint8_t cmd[2];

    cmd[0]=W25X_ReadStatusReg;
    cmd[1]=SR;

    HAL_SPI_Transmit(&hspi,cmd,2,1000);
}
/*
********************************************************************************
**  函数名称:  W25QXXWaitBusy
**  功能描述:  
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
static void W25QXXWaitBusy(void)
{
    while ((W25QXXReadSR() & 0x01) == 0x01){
    }
}
/*
********************************************************************************
**  函数名称:  W25QXXWriteEnable
**  功能描述:  
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
static void W25QXXWriteEnable(void)
{
    W25QX_Enable();

    HAL_SPI_Transmit(&hspi,(uint8_t *)&W25X_WriteEnable,1,1000);

    W25QX_Disable();

    W25QXXWaitBusy();
}
/*
********************************************************************************
**  函数名称:  Flash_EraseSector                                            
**  功能描述:  擦除一个扇区                                                 
**  输入参数:  sector_num 擦除的扇区号                                      
**  输出参数:  无                                                           
**  返回参数:  实行结果                                                     
********************************************************************************
*/
unsigned char Flash_EraseSector( unsigned short SectorNum )
{
    uint32_t Addr = SectorNum * W25QXX_SECTOR_SIZE;
/*    uint8_t cmd[5];

    cmd[0]=W25X_SectorErase4Byte;
    cmd[1]=(uint8_t)((Addr) >> 24);
    cmd[2]=(uint8_t)((Addr) >> 16);
    cmd[3]=(uint8_t)((Addr) >> 8);
    cmd[4]=(uint8_t)Addr;*/

#if 1
    uint8_t cmd[4];
    cmd[0] = W25X_SectorErase;/*0x13命令+后面加要读的地址*/
    cmd[1] = (uint8_t)(Addr >> 16);/*先发高位在发低位*/
    cmd[2] = (uint8_t)(Addr >> 8);
    cmd[3] = (uint8_t)(Addr);
#endif
    W25QXXWriteEnable();

    W25QX_Enable();
    
    HAL_SPI_Transmit(&hspi,(uint8_t*)cmd,4,1000);

    W25QX_Disable();

    W25QXXWaitBusy();

    return FLASH_NONE_ERR;   
}
/*
********************************************************************************
**  函数名称:  Flash_EraseBlock                                            
**  功能描述:  擦除一个块                                                 
**  输入参数:  sector_num 擦除的扇区号                                      
**  输出参数:  无                                                           
**  返回参数:  实行结果                                                     
********************************************************************************
*/

unsigned int Flash_EraseBlock(unsigned int u32BlockNum)
{
    unsigned int Addr = u32BlockNum * W25QXX_BLOCK_SIZE;
    unsigned char cmd[4];

    cmd[0]=W25X_BlockErase;
    cmd[1]=(unsigned char)((Addr) >> 16);
    cmd[2]=(unsigned char)((Addr) >> 8);
    cmd[3]=(unsigned char)Addr;

    W25QXXWriteEnable();

    W25QXXWaitBusy();

    W25QX_Enable();

    HAL_SPI_Transmit(&hspi,(unsigned char *)&cmd[0],4,1000);

    W25QX_Disable();

    W25QXXWaitBusy();

    return 0;
}
/*
********************************************************************************
**  函数名称:  Flash_ReadID
**  功能描述:  提供外部使用的函数
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
unsigned short FlashReadID(void)
{
    uint16_t u16Id = 0;
    uint8_t cmd[4] = {W25X_ManufactDeviceID,0x00,0x00,0x00};
    // uint8_t cmd[4] = {W25X_ManufactDeviceID,0x00,0x00,0x01};

    W25QX_Enable();

    HAL_SPI_Transmit(&hspi,cmd,4,1000);

    HAL_SPI_Receive(&hspi,(uint8_t*)&u16Id, 2, 1000);

    W25QX_Disable();

    return u16Id;
}
/*
********************************************************************************
**  函数名称:  drv_flash_init
**  功能描述:  flash的初始化 SPI1
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void drv_flash_init(void)
{
    MX_SPI1_Init();
    
    W25QX_Disable();
}
/*
********************************************************************************
**  函数名称:  FlashEraseChip
**  功能描述:  flash芯片整块擦除
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
unsigned char FlashEraseChip(void)
{
    W25QXXWriteEnable();

    W25QXXWaitBusy();

    W25QX_Enable();

    HAL_SPI_Transmit(&hspi,(uint8_t *)&W25X_ChipErase,1,1000);

    W25QX_Disable();

    W25QXXWaitBusy();
    
    return FLASH_NONE_ERR;   
}  
/*
********************************************************************************
**  函数名称:  FlashReadByte
**  功能描述:  flash读一个byte的数据
**  输入参数:  addr   地址
**  输出参数:  addr 地址上的数据
**  返回参数:  无
***************************************** ***************************************
*/
unsigned char FlashReadByte(unsigned int addr)
{
    unsigned char data;       
    uint8_t cmd[4];
#if 0
    cmd[0] = W25X_ReadData4Byte;/*0x13命令+后面加要读的地址*/
    cmd[1] = (uint8_t)(addr >> 24);/*先发高位在发低位*/
    cmd[2] = (uint8_t)(addr >> 16);
    cmd[3] = (uint8_t)(addr >> 8);
    cmd[4] = (uint8_t)(addr);
#endif
#if 1
    cmd[0] = W25X_ReadData;/*0x13命令+后面加要读的地址*/
    cmd[1] = (uint8_t)(addr >> 16);/*先发高位在发低位*/
    cmd[2] = (uint8_t)(addr >> 8);
    cmd[3] = (uint8_t)(addr);
#endif
    W25QX_Enable();

    HAL_SPI_Transmit(&hspi,cmd,4,1000);

    if (HAL_SPI_Receive(&hspi, &data,1,1000) != HAL_OK)
    { 
        W25QX_Disable();
        return -1;
    }
    W25QX_Disable();
    
    return data;   
} 
/*
********************************************************************************
**  函数名称:  Flash_ReadStr
**  功能描述:  在地址为addr、上flash读取长度为siez的数据  存放在*date中
**  输入参数:  addr    地址
**             *date   数据存放在内存的地址
**             size    数据长度
**  输出参数:  无
**  返回参数:  实行结果
********************************************************************************
*/
unsigned char Flash_ReadStr( unsigned int addr , unsigned char *data , unsigned int ulen)
{  
#if 1
    uint8_t cmd[4];
    cmd[0] = W25X_ReadData;/*0x13命令+后面加要读的地址*/
    cmd[1] = (uint8_t)(addr >> 16);/*先发高位在发低位*/
    cmd[2] = (uint8_t)(addr >> 8);
    cmd[3] = (uint8_t)(addr);
#endif
    W25QX_Enable();

    HAL_SPI_Transmit(&hspi,cmd,4,1000);

    if (HAL_SPI_Receive(&hspi, data,ulen,5000) != HAL_OK){ 
        W25QX_Disable();
        return -1;
    }
    W25QX_Disable();

    return FLASH_NONE_ERR;   
} 
/*
********************************************************************************
**  函数名称:  Flash_Write_Page
**  功能描述:  在地址为addr写flash读取长度为ulen的数据  存放在*pdata中数据
**  输入参数:  addr    地址
**             pdata   数据存放在内存的地址
**             ulen    数据长度
**  输出参数:  无
**  返回参数:  实行结果
********************************************************************************
*/

void Flash_Write_Page(unsigned int dst, unsigned char* pdata, unsigned int ulen)
{
    unsigned char cmd[4];
    unsigned int size;

    if ((dst % W25QXX_PAGE_SIZE) != 0)//写入的地址不是整页的地方
    {
        if ((dst / W25QXX_PAGE_SIZE) != ((dst + ulen) / W25QXX_PAGE_SIZE))//写入的长度大于一页
        {
            size = W25QXX_PAGE_SIZE - dst % W25QXX_PAGE_SIZE;//同一页剩余的没写的字节大小
            ulen -= size;//扣掉写入的字节大小，等于剩下要写的
        }
        else
        {
            size = ulen;//跟写入的地址是同一页，要写入的少于一页
            ulen = 0;
        }
        cmd[0] = W25X_PageProgram;//页编程命令
        cmd[1] = (unsigned char)(dst >> 16);
        cmd[2] = (unsigned char)(dst >> 8);
        cmd[3] = (unsigned char)(dst);

        W25QXXWriteEnable();

        W25QX_Enable();

        HAL_SPI_Transmit(&hspi,cmd,4,1000);

        if (HAL_SPI_Transmit(&hspi, pdata,size,1000) != HAL_OK)
        {
            return ;
        }
        W25QX_Disable();

        W25QXXWaitBusy();

        dst += size;// 写入的指针往后移动SIZE大小
        pdata+=size;//要写入的内容指针往后移动SIZE大小
    }
    while (ulen > 0)//如果写入的长度大于0
    {
        if (ulen < W25QXX_PAGE_SIZE)//小于1页
        {
            size = ulen;//写入的大小等于长度
            ulen = 0;
        }
        else  
        {
            size = W25QXX_PAGE_SIZE;//如果大于一页 ，要写入大小为页大小
            ulen -= W25QXX_PAGE_SIZE;//写入的长度在减去写入的大小
        }

        cmd[0] = W25X_PageProgram;
        cmd[1] = (unsigned char)(dst >> 16);
        cmd[2] = (unsigned char)(dst >> 8);
        cmd[3] = (unsigned char)(dst);

        W25QXXWriteEnable();

        W25QX_Enable();
        HAL_SPI_Transmit(&hspi,cmd,4,1000);

        if (HAL_SPI_Transmit(&hspi, pdata,size,1000) != HAL_OK)
        {
            return ;
        }
        W25QX_Disable();
        W25QXXWaitBusy();
        dst += size;//下一页flash
        pdata+=size;//写入内容向下移动
    }
}
 
/*
********************************************************************************
**  函数名称:  Flash_WriteStr
**  功能描述:  SPIflash写入一个扇区操作。(一个扇区4k)
**  输入参数:  databuf            :    写入的一个sector数据 
**             sector_num        :    sector_num编号(0-511)    
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
static unsigned char Flash_WriteStr( unsigned long addr,unsigned char *buf,unsigned int ulen )
{
    uint8_t cmd[5];
    unsigned int wrlen,curlen;
    unsigned int offer;
    unsigned char *pdatabuf;

    
    if(addr > 0x1fffff){        
        return FLASH_PARM_ERR;
    }    
    if(ulen+(addr%4096)>4096){
        return FLASH_PARM_ERR;
    }

    curlen = ulen;
    pdatabuf = buf ;
    offer =(addr%256);
    
    if(offer == 0){
        if( curlen > 256 ){
            wrlen = 256;
        }else{
            wrlen = curlen;
        }
    }else{
        wrlen = 256-offer;
        if(wrlen > curlen ){
            wrlen = curlen;
        }
    }
    do
    {    
        cmd[0] = W25X_PageProgram4Byte;
        cmd[1] = (uint8_t)(addr >> 24);
        cmd[2] = (uint8_t)(addr >> 16);
        cmd[3] = (uint8_t)(addr >> 8);
        cmd[4] = (uint8_t)(addr);

        W25QXXWriteEnable();

        W25QX_Enable();
      
        HAL_SPI_Transmit(&hspi,cmd,5,1000);
        
        if (HAL_SPI_Transmit(&hspi, pdatabuf,wrlen,5000) != HAL_OK)
        {
            W25QX_Disable();
            return FLASH_BUSY_ERR;
        }
        W25QX_Disable();

        W25QXXWaitBusy();

        curlen -= wrlen ;
        addr += wrlen ;
        pdatabuf += wrlen ;
        if( curlen > 256 ){  
            wrlen = 256;
        }else{
            wrlen = curlen;
        }
    }while(curlen);

    return FLASH_NONE_ERR;
}
/*
********************************************************************************
**  函数名称:  FlashUpdateWriteStr
**  功能描述:  SPIflash写入一个扇区操作。(一个扇区4k)
**  输入参数:  databuf     :  写入的一个sector数据
**             sector_num  :  sector_num编号(0-511)  
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
unsigned char FlashWriteStr( unsigned int addr,unsigned char *buf,unsigned int ulen )
{
    unsigned int  sector_num;
    unsigned int  i;
    unsigned char *pdatabuf;
 
    if( buf == 0 ){
        return FLASH_PARM_ERR;
    }

    /*(((flash_addr+1) % 256) ==0) 或超出最大地址范围*/
    if(addr > 0x1fffff){        
        return FLASH_PARM_ERR;
    }        
    if(ulen+(addr%4096)>4096){
        return FLASH_PARM_ERR;
    }
    //fcnt = 0;     
    pdatabuf = buf ;                

    /* erase flash */
    if (( addr & 0xFFF )==0) {
        sector_num = ( addr >> 12 );    /*sector_num = flash_addr/4096;*/    
        if(Flash_EraseSector(sector_num)!=FLASH_NONE_ERR) {
            return FLASH_OPWR_ERR;
        }
    }
    
    /* 写入flash */
    if ( Flash_WriteStr(addr,pdatabuf,ulen) != FLASH_NONE_ERR ) {
        return FLASH_OPWR_ERR;
    }

    /* 读取flash */
    if ( Flash_ReadStr( addr , FlashTempBuf , ulen)!= FLASH_NONE_ERR ) {
        return FLASH_OPRD_ERR;
    }

    /* 校验数据  */
    for ( i = 0 ; i < ulen ; i++) {
        if( buf[i] != FlashTempBuf[i] ){
            kprintf("flash chk error\r\n");
            return FLASH_OPWR_ERR;
        }
    }
    return FLASH_NONE_ERR;
}
/*
********************************************************************************
**  函数名称:  FlashUpdateWriteStr
**  功能描述:  SPIflash写入一个扇区操作。(一个扇区4k)
**  输入参数:  databuf     :  写入的一个sector数据
**             sector_num  :  sector_num编号(0-511)  
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
unsigned char FlashWriteByte(unsigned int addr,unsigned char data)
{
    // return FlashWriteStr( addr, &data, 1 );
    Flash_Write_Page( addr, &data, 1 );
    return FLASH_NONE_ERR;
}
/*
********************************************************************************
**  函数名称:  FlashReadStr
**  功能描述:   
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
unsigned char FlashReadStr( unsigned int addr , unsigned char *data , unsigned int ulen)
{
    return Flash_ReadStr(addr , data ,ulen);
}
