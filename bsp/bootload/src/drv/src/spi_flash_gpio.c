
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
#define SectorErase             0x20
#define ChipErase               0xC7
#define PowerDown               0xB9
#define ReleasePowerDown        0xAB
#define DeviceID                0xAB
#define ManufactDeviceID        0x90
#define JedecDeviceID           0x9F
/*
********************************************************************************
********************************************************************************
*/ 
#define FLASH_TIME_OUT            3             //引脚延时定义
#define FLASH_BUF_MAX_LEN        (4096+30)      //内部静态缓存定义

#define Flash_TimeOut(...) 

#define Spi_FlashCsEnable()      HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,   GPIO_PIN_RESET);
#define Spi_FlashCsDisable()     HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,   GPIO_PIN_SET);

#define Spi_FlashClkSet()        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,   GPIO_PIN_SET);
#define Spi_FlashClkClr()        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,   GPIO_PIN_RESET);

// MOSI
#define Spi_FlashDioSet()        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,   GPIO_PIN_SET);
#define Spi_FlashDioClr()        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,   GPIO_PIN_RESET);
//    Spi_FlashMISORead
#define Spi_FlashDoRead()        HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6)
/*
********************************************************************************
********************************************************************************
*/ 
/*FLASH内部使用的一个缓存区*/    
static unsigned char FlashTempBuf[ FLASH_BUF_MAX_LEN ];  
  // unsigned char chacktmp[4096+30];
/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
**  函数名称:  Flash_PutChar
**  功能描述:  向flash输出一个字符
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
static void Flash_PutChar( unsigned char date )
{
    unsigned char i;
     
    for( i=0 ; i<8 ; i++ ){//高位先发
         if( date & 0x80 ){
            Spi_FlashDioSet();
         }else
            Spi_FlashDioClr();
         date = date<<1;
         Flash_TimeOut(FLASH_TIME_OUT);
         Spi_FlashClkSet();
         Flash_TimeOut(FLASH_TIME_OUT);
         Spi_FlashClkClr();
   }
} 

/*
********************************************************************************
**  函数名称:  Flash_GetChar
**  功能描述:  从flash中获取一个字符
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
static void Flash_GetChar(unsigned char *date)
{
    unsigned char i,pin;

    *date = 0;
    for ( i = 0 ; i < 8 ; i++ ) {
        Flash_TimeOut( FLASH_TIME_OUT );
        Spi_FlashClkSet();
        Flash_TimeOut( FLASH_TIME_OUT );
        
        //读一个位数据
        pin = 0;
        if ( Spi_FlashDoRead()) {
            pin=1;
        }

        *date = *date |( pin << (7-i) );
        Flash_TimeOut( FLASH_TIME_OUT );
        Spi_FlashClkClr();
        Flash_TimeOut( FLASH_TIME_OUT );
    }
}

/*
********************************************************************************
**  函数名称:  Flash_ReadReg
**  功能描述:  读flash的寄存器
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
static unsigned char Flash_ReadReg( unsigned char cmd )
{
    unsigned char read_data=0;

    Spi_FlashCsEnable();
    Flash_PutChar( cmd );
    Flash_GetChar( &read_data );
    Spi_FlashCsDisable();

    return read_data;   
}

/*
********************************************************************************
**  函数名称:  Flash_WriteCmd
**  功能描述:  写命令 
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
static void Flash_WriteCmd( unsigned char cmd )
{
    Spi_FlashCsEnable();
    Spi_FlashClkClr();
    Flash_PutChar( cmd );
    Spi_FlashCsDisable();
}

/*
********************************************************************************
**  函数名称:  Flash_WriteEnable
**  功能描述:  flash  写使能
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
static void Flash_WriteEnable( void )
{
    Flash_WriteCmd( WriteEnable );
}

/*
********************************************************************************
**  函数名称:  Flash_WaitBusy                                               
**  功能描述:  flash查忙                                                    
**  输入参数:  无                                                           
**  输出参数:  无                                                           
**  返回参数:  无                                                           
********************************************************************************
*/
static unsigned char Flash_WaitBusy( void )
{
    unsigned short cnt=0;

    while ( ( Flash_ReadReg( ReadStatusReg ) & 0x01) == 0x01 ) {
        cnt++;
        if(cnt>0X8FFF)return FLASH_BUSY_ERR;
    }
    return FLASH_NONE_ERR;
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
unsigned char Flash_EraseSector( unsigned short sector_num )
{
    unsigned int addr;
    
    if( sector_num > 511 ) {
        return FLASH_PARM_ERR;
    }

    if( Flash_WaitBusy() != FLASH_NONE_ERR ){
        return FLASH_BUSY_ERR;
    }

    addr = ( sector_num << 12);
    Flash_WriteEnable();
    if( Flash_WaitBusy() != FLASH_NONE_ERR ){
        return FLASH_BUSY_ERR;
    }
    Spi_FlashCsEnable();
    Spi_FlashClkClr();
    
    Flash_PutChar( SectorErase );
    Flash_PutChar( (unsigned char)(( addr & 0xFFFFFF ) >> 16) );
    Flash_PutChar( (unsigned char)(( addr & 0xFFFF) >> 8));
    Flash_PutChar( (unsigned char)( addr & 0xFF));

    Spi_FlashCsDisable();

    return FLASH_NONE_ERR;   
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
    unsigned int Temp = 0;   
    unsigned char  data;
    Spi_FlashCsEnable();
   
    Flash_PutChar( ManufactDeviceID );
    Flash_PutChar( 0x00 );
    Flash_PutChar( 0x00 );
    Flash_PutChar( 0x00 );
    Flash_GetChar( &data );  
    Temp|=data<<8;
    Flash_GetChar( &data );             

    Temp|=  data;
    Spi_FlashCsDisable();
    return Temp;
}

/*
********************************************************************************
**  函数名称:  drv_flash_init
**  功能描述:  flash的初始化
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void drv_flash_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    
    /**SPI3 GPIO Configuration
    PD14     ------> CS
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    /**SPI3 GPIO Configuration
    PB12    ------> SPI3_CS
    */
    GPIO_InitStruct.Pin = GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);    
    Spi_FlashCsDisable()  ; 

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
    
    if( Flash_WaitBusy() != FLASH_NONE_ERR ){
        return FLASH_BUSY_ERR;
    }

    Flash_WriteEnable();

    if( Flash_WaitBusy() != FLASH_NONE_ERR ){
        return FLASH_BUSY_ERR;
    }
    
    Spi_FlashCsEnable();
    Spi_FlashClkClr();
    
    Flash_PutChar( ChipErase ); 

    Spi_FlashCsDisable();

    return FLASH_NONE_ERR;   
}  
/*
********************************************************************************
**  函数名称:  FlashReadByte
**  功能描述:  flash读一个byte的数据
**  输入参数:  addr   地址
**             *pdate 存放地址
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
unsigned char FlashReadByte( unsigned int addr  )
{
    unsigned char read_data;       
    
    if( Flash_WaitBusy() != FLASH_NONE_ERR){
        return FLASH_BUSY_ERR;
    }
    Spi_FlashCsEnable();
    Spi_FlashClkClr(); 
     
    Flash_PutChar( ReadData );
    Flash_PutChar( (unsigned char)((addr & 0xFFFFFF) >> 16) );
    Flash_PutChar( (unsigned char)((addr & 0xFFFF) >> 8) );
    Flash_PutChar( (unsigned char)(addr & 0xFF) );     
    Flash_GetChar( &read_data );
    Spi_FlashCsDisable();
      
    return read_data;   
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

    unsigned int    i;          
    unsigned short  rd_len;
    unsigned int    rd_addr;
    unsigned char   *pbuf;


    if(addr > 0x1fffff)  {
        return FLASH_PARM_ERR;
    }

    rd_addr = addr ;
    pbuf    = data;
   
    
    if(ulen>256){
        if(rd_addr%256==0){
           rd_len = 256;
        }else{
           rd_len = 256-rd_addr%256;
        }
    }else{
        if((ulen+rd_addr%256)>256){
           if(ulen>(rd_addr%256)){
              rd_len = ulen-rd_addr%256;
           }else{
              rd_len = 256-rd_addr%256;
           }
           
        }else{
           rd_len = ulen;
        }
    }

    do{

        if( Flash_WaitBusy() != FLASH_NONE_ERR){
            return FLASH_BUSY_ERR;
        }
        Spi_FlashCsEnable();
        Spi_FlashClkClr(); 
        Flash_PutChar( ReadData );//?????
        Flash_PutChar( (unsigned char)((rd_addr & 0xFFFFFF) >> 16));
        Flash_PutChar( (unsigned char)((rd_addr & 0xFFFF) >> 8));
        Flash_PutChar( (unsigned char)(rd_addr & 0xFF));
        
        for(i = 0; i < rd_len; i++) {
            Flash_GetChar(pbuf+i);
        }
        Spi_FlashCsDisable();
        pbuf+=rd_len;
        rd_addr += rd_len;
        ulen-=rd_len;
        
        if(ulen>256){
            rd_len = 256;
        }else{
            rd_len = ulen;
        }
        
        
    }while(ulen);
    
    Spi_FlashCsDisable();

    return FLASH_NONE_ERR;   
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

    unsigned int wrlen,curlen;
    unsigned int offer;
    unsigned int i;
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
        if( Flash_WaitBusy() != FLASH_NONE_ERR){
            return FLASH_BUSY_ERR;
        }
        Flash_WriteEnable();
        if( Flash_WaitBusy() != FLASH_NONE_ERR){
            return FLASH_BUSY_ERR;
        }    
        Spi_FlashCsEnable();
        Spi_FlashClkClr(); 
        Flash_PutChar( PageProgram );
        Flash_PutChar( (unsigned char)((addr & 0xFFFFFF) >> 16));//    send 3 address bytes
        Flash_PutChar( (unsigned char)((addr & 0xFFFF) >> 8));
        Flash_PutChar( (unsigned char)(addr & 0xFF));

        for( i = 0 ; i < wrlen ; i++ ){ //写data
             Flash_PutChar( *(pdatabuf+i) );
        }
        
        Spi_FlashCsDisable();
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
    //unsigned int  fcnt;  
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
    return FlashWriteStr( addr, &data, 1 );
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

/*
********************************************************************************
**  函数名称:  FlashDemo
**  功能描述:   
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
********************************************************************************
*/
void FlashDemo(void)
{
    static unsigned char wrbuf[4096];
    static unsigned char rdbuf[4096];
    
    Flash_EraseSector(0);
    
    memset(wrbuf,0x0a,256);
    
    FlashWriteStr(0,wrbuf,256);
    
    FlashReadStr(0,rdbuf,256);

}
