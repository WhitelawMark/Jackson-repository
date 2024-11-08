 /*
 * ymodem.h
 *
 *  Created on: 2022年10月19日
 *      Author: lwp edit
 */


#include "app_lib.h"



/*
********************************************************************************
********************************************************************************
*/ 
u32_t  LocalRxTotSize=0;
extern uint8_t FileName[];
/*
********************************************************************************
********************************************************************************
*/ 
// 加密相关参数
// {
// 升级文件头魔术字
static const uint8_t s_gu8HeadMagic[] = "YQI.";

// 解密主密钥
// 解密过程：主密钥加密文件头中的随机值生成解密数据的密钥，使用该密钥来解密数据
static const uint8_t s_gu8MainKey[] = "YQI.. $DOGCOLLA$";
// 数据解密密钥
static uint8_t s_gu8DataKey[16];

// 数据加密起始位置/结束位置
static uint32_t s_u32EncryptStartAddr, s_u32EncryptEndAddr;
// }
/*
********************************************************************************
********************************************************************************
*/ 
// 转换为字
static uint32_t CovertToHex(uint8_t *pu8Data)
{
    return (((uint32_t)pu8Data[3] << 24)
            | ((uint32_t)pu8Data[2] << 16)
            | ((uint32_t)pu8Data[1] << 8)
            | ((uint32_t)pu8Data[0]));
}
/*
********************************************************************************
********************************************************************************
*/ 
/**
  * @brief  解密数据包
  * @param  pu8Packet: 数据包
  * @param  u32Offset: 数据包起始位置
  * @param  u32PacketLen: 数据包长度
  * @retval 0:成功 -1:失败
  */
int8_t Decrypt_Packet(uint8_t *pu8Packet, uint32_t u32Offset, uint32_t u32PacketLen)
{
    uint32_t u32StartAddr, u32EndAddr;

    if (u32PacketLen == 0)return 0;
    if ((u32Offset % 128) != 0)return -1;
    if ((u32PacketLen % 128) != 0)return -1;
    
    if (u32Offset == 0){
        if (u32PacketLen < 40)return -1;
        if (memcmp(pu8Packet, s_gu8HeadMagic, 4) != 0)return -1;
        u32StartAddr = CovertToHex(&pu8Packet[12]);
        u32EndAddr = CovertToHex(&pu8Packet[16]);
        if ((u32StartAddr % 16) != 0)return -1;
        if ((u32EndAddr % 16) != 0)return -1;
        if (u32EndAddr < u32StartAddr)return -1;
        memset(&pu8Packet[12], 0, 8);
        s_u32EncryptStartAddr = u32StartAddr + APPLICATION_HEAD_SIZE;
        s_u32EncryptEndAddr = u32EndAddr + APPLICATION_HEAD_SIZE;
        aes_init((unsigned int *)s_gu8MainKey);
        aes_encrypt((unsigned int *)&pu8Packet[24], NULL, (unsigned int *)s_gu8DataKey);
        aes_init((unsigned int *)s_gu8DataKey);
    }

    u32StartAddr = u32Offset;
    u32EndAddr = u32Offset + u32PacketLen;
    if ((u32StartAddr < s_u32EncryptEndAddr)
        && (u32EndAddr > s_u32EncryptStartAddr)){
        if (u32StartAddr < s_u32EncryptStartAddr){
            u32StartAddr = s_u32EncryptStartAddr;
        }
        if (u32EndAddr > s_u32EncryptEndAddr){
            u32EndAddr = s_u32EncryptEndAddr;
        }
        u32StartAddr -= u32Offset;
        u32EndAddr -= u32Offset;
        while (u32StartAddr < u32EndAddr){
            aes_decrypt((unsigned int *)&pu8Packet[u32StartAddr], NULL, (unsigned int *)&pu8Packet[u32StartAddr]);
            u32StartAddr += 16;
        }
    }
    return 0;
}

/**
  * @brief  Receive byte from sender
  * @param  c: Character
  * @param  timeout: Timeout
  * @retval 0: Byte received
  *        -1: Timeout
  */ 
void Bl_SetLocalDownLoad(void)
{
    FlashWriteByte(FLASH_LBERR_FLG,1); 
}

/**
  * @brief  Receive byte from sender
  * @param  c: Character
  * @param  timeout: Timeout
  * @retval 0: Byte received
  *        -1: Timeout
  */ 
void Bl_ClrLocalDownLoad_Flg(void)
{
    FlashWriteByte(FLASH_LBERR_FLG,0xff); 
}

/**
  * @brief  读取0x00007000 处的值并返回
  * @param  c: Character
  * @param  timeout: Timeout
  * @retval 0: Byte received
  *        -1: Timeout
  */
u8_t Bl_GetLocalDownLoad_Flg(void)
{
     return FlashReadByte(FLASH_LBERR_FLG);/*0x00007000*/
}

/**
  * @brief  Receive byte from sender
  * @param  c: Character
  * @param  timeout: Timeout
  * @retval 0: Byte received
  *        -1: Timeout
  */
static  int32_t Receive_Byte (uint8_t *c, uint32_t timeout)
{
    while (timeout-- > 0){
        if (DrvUartGetChar(c) == 1){
            return 0;
        }
    }
    delay_1ms(1);
    return -1;
}

/**
  * @brief  Send a byte
  * @param  c: Character
  * @retval 0: Byte sent
  */
static uint32_t Send_Byte (uint8_t c)
{
    SerialPutChar(c);
    return 0;
}

/**
  * @brief  Receive a packet from sender
  * @param  data
  * @param  length
  * @param  timeout
  *     0: end of transmission
  *    -1: abort by sender
  *    >0: packet length
  * @retval 0: normally return
  *        -1: timeout or packet error
  *         1: abort by user
  */
static int32_t Receive_Packet (uint8_t *data, int32_t *length, uint32_t timeout)
{
    uint16_t i, packet_size;
    uint8_t c;
    *length = 0;

    if (Receive_Byte(&c, timeout) != 0){
        return -1;
    }
    switch (c){
        case SOH:
            packet_size = PACKET_SIZE;
            break;
        case STX:
            packet_size = PACKET_1K_SIZE;
            break;
        case EOT:
            return 0;
        case CA:
            if ((Receive_Byte(&c, timeout) == 0) && (c == CA)){
                *length = -1;
                return 0;
            }else{
                return -1;
            }
        case ABORT1:
        case ABORT2:
            return 1;
        default:
            return -1;
    }
    *data = c;
    for (i = 1; i < (packet_size + PACKET_OVERHEAD); i ++){
        if (Receive_Byte(data + i, timeout) != 0){
            return -1;
        }
    }
    if (data[PACKET_SEQNO_INDEX] != ((data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff)){
        return -1;
    }
    *length = packet_size;
    return 0;
}

/**
  * @brief  Receive a file using the ymodem protocol.
  * @param  buf: Address of the first byte.
  * @retval The size of the file.
  */
int32_t Ymodem_Receive (uint8_t *buf)
{
    uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD], file_size[FILE_SIZE_LENGTH], *file_ptr, *buf_ptr;
    int32_t i, packet_length, session_done, file_done, packets_received, errors, session_begin, size = 0;
    uint32_t flashdestination, ramsource;
    uint32_t timeout;

    timeout=0;
    /* Initialize flashdestination variable */
    flashdestination = APPLICATION_ADDRESS;
    kprintf("Ymodem_Receive APPLICATION_ADDRESS%x",APPLICATION_ADDRESS);

    for (session_done = 0, errors = 0, session_begin = 0; ;)
    {
        for (packets_received = 0, file_done = 0, buf_ptr = buf; ;)
        {
            switch (Receive_Packet(packet_data, &packet_length, NAK_TIMEOUT))
            {
                case 0:
                    errors = 0;
                    switch (packet_length)
                    {
                        /* Abort by sender */
                        case - 1:
                            Send_Byte(ACK);
                            return 0;
                        /* End of transmission */
                        case 0:
                            Send_Byte(ACK);
                            file_done = 1;
                        break;
                        /* Normal packet */
                        default:
                            if ((packet_data[PACKET_SEQNO_INDEX] & 0xff) != (packets_received & 0xff)){
                                Send_Byte(NAK);
                            }else{
                                if (packets_received == 0){
                                    /* Filename packet */
                                    if (packet_data[PACKET_HEADER] != 0){
                                        /* Filename packet has valid data */
                                        for (i = 0, file_ptr = packet_data + PACKET_HEADER; (*file_ptr != 0) && (i < FILE_NAME_LENGTH);){
                                            FileName[i++] = *file_ptr++;
                                        }
                                        FileName[i++] = '\0';
                                        for (i = 0, file_ptr ++; (*file_ptr != ' ') && (i < FILE_SIZE_LENGTH);){
                                            file_size[i++] = *file_ptr++;
                                        }
                                        file_size[i++] = '\0';
                                        Str2Int(file_size, &size);

                                        /* Test the size of the image to be sent */
                                        /* Image size is greater than Flash size */
                                        if (size > (USER_FLASH_SIZE + 1)){
                                            /* End session */
                                            Send_Byte(CA);
                                            Send_Byte(CA);
                                            return -1;
                                        }
                                        /* erase user application area */
                                        LocalRxTotSize=0;
                                        FLASH_If_Erase(APPLICATION_ADDRESS);
                                        // Bl_SetLocalDownLoad();/*这个写FLASH 烧写标志*/
                                        Send_Byte(ACK);
                                        Send_Byte(CRC16);
                                    }else{ /* Filename packet is empty, end session */
                                        Send_Byte(ACK);
                                        file_done = 1;
                                        session_done = 1;
                                        break;
                                    }
                                }
                            /* Data packet */
                            else{
                                memcpy(buf_ptr, packet_data + PACKET_HEADER, packet_length);
                                ramsource = (uint32_t)buf;
                                if (Decrypt_Packet(buf_ptr, LocalRxTotSize, packet_length) != 0){
                                    /* End session */
                                    Send_Byte(CA);
                                    Send_Byte(CA);
                                    return -2;
                                }
                                LocalRxTotSize+=packet_length;

                                /* Write received data in Flash */
                                if (FLASH_If_Write(flashdestination, (uint32_t*) buf, (uint16_t) packet_length/4)  == PASSED){
                                    flashdestination+=packet_length;
                                    Send_Byte(ACK);
                                }else{ /* An error occurred while writing to Flash memory */
                                    /* End session */
                                    Send_Byte(CA);
                                    Send_Byte(CA);
                                    return -2;
                                }
                            }
                            packets_received ++;
                            session_begin = 1;
                        }
                    }
                    break;
                case 1:
                    Send_Byte(CA);
                    Send_Byte(CA);
                    return -3;
                default:
                    if (session_begin > 0){
                        errors ++;
                    }
                    if (errors > MAX_ERRORS){
                        Send_Byte(CA);
                        Send_Byte(CA);
                        return 0;
                    }
                    timeout++;
                    if(timeout > 10000){
                        return 0;
                    }
                    Send_Byte(CRC16);
                    break;
            }
            if (file_done != 0){
                break;
            }
        }
        if (session_done != 0){
            break;
        }
    }
    return (int32_t)size;
}

/**
  * @brief  check response using the ymodem protocol
  * @param  buf: Address of the first byte
  * @retval The size of the file
  */
int32_t Ymodem_CheckResponse(uint8_t c)
{
    return 0;
}

/**
  * @brief  Prepare the first block
  * @param  timeout
  *     0: end of transmission
  * @retval None
  */
void Ymodem_PrepareIntialPacket(uint8_t *data, const uint8_t* fileName, uint32_t *length)
{
    uint16_t i, j;
    uint8_t file_ptr[10];

    /* Make first three packet */
    data[0] = SOH;
    data[1] = 0x00;
    data[2] = 0xff;

    /* Filename packet has valid data */
    for (i = 0; (fileName[i] != '\0') && (i < FILE_NAME_LENGTH);i++){
        data[i + PACKET_HEADER] = fileName[i];
    }

    data[i + PACKET_HEADER] = 0x00;

    Int2Str (file_ptr, *length);
    for (j =0, i = i + PACKET_HEADER + 1; file_ptr[j] != '\0' ; ){
        data[i++] = file_ptr[j++];
    }

    for (j = i; j < PACKET_SIZE + PACKET_HEADER; j++){
        data[j] = 0;
    }
}

/**
  * @brief  Prepare the data packet
  * @param  timeout
  *     0: end of transmission
  * @retval None
  */
void Ymodem_PreparePacket(uint8_t *SourceBuf, uint8_t *data, uint8_t pktNo, uint32_t sizeBlk)
{
    uint16_t i, size, packetSize;
    uint8_t* file_ptr;

    /* Make first three packet */
    packetSize = sizeBlk >= PACKET_1K_SIZE ? PACKET_1K_SIZE : PACKET_SIZE;
    size = sizeBlk < packetSize ? sizeBlk :packetSize;
    if (packetSize == PACKET_1K_SIZE){
        data[0] = STX;
    }else{
        data[0] = SOH;
    }
    data[1] = pktNo;
    data[2] = (~pktNo);
    file_ptr = SourceBuf;

    /* Filename packet has valid data */
    for (i = PACKET_HEADER; i < size + PACKET_HEADER;i++){
        data[i] = *file_ptr++;
    }
    if ( size  <= packetSize){
        for (i = size + PACKET_HEADER; i < packetSize + PACKET_HEADER; i++){
            data[i] = 0x1A; /* EOF (0x1A) or 0x00 */
        }
    }
}

/**
  * @brief  Update CRC16 for input byte
  * @param  CRC input value 
  * @param  input byte
  * @retval None
  */
uint16_t UpdateCRC16(uint16_t crcIn, uint8_t byte)
{
    uint32_t crc = crcIn;
    uint32_t in = byte | 0x100;

    do{
        crc <<= 1;
        in <<= 1;
        if(in & 0x100)
            ++crc;
        if(crc & 0x10000)
            crc ^= 0x1021;
    }while(!(in & 0x10000));

    return crc & 0xffffu;
}
/**
  * @brief  Cal CRC16 for YModem Packet
  * @param  data
  * @param  length
  * @retval None
  */
uint16_t Cal_CRC16(const uint8_t* data, uint32_t size)
{
    uint32_t crc = 0;
    const uint8_t* dataEnd = data+size;

    while(data < dataEnd)
        crc = UpdateCRC16(crc, *data++);

    crc = UpdateCRC16(crc, 0);
    crc = UpdateCRC16(crc, 0);

    return crc&0xffffu;
}

/**
  * @brief  Cal Check sum for YModem Packet
  * @param  data
  * @param  length
  * @retval None
  */
uint8_t CalChecksum(const uint8_t* data, uint32_t size)
{
    uint32_t sum = 0;
    const uint8_t* dataEnd = data+size;

    while(data < dataEnd )
        sum += *data++;

    return (sum & 0xffu);
}

/**
  * @brief  Transmit a data packet using the ymodem protocol
  * @param  data
  * @param  length
  * @retval None
  */
void Ymodem_SendPacket(uint8_t *data, uint16_t length)
{
    uint16_t i;
    i = 0;
    while (i < length){
        Send_Byte(data[i]);
        i++;
    }
}

/**
  * @brief  Transmit a file using the ymodem protocol
  * @param  buf: Address of the first byte
  * @retval The size of the file
  */
uint8_t Ymodem_Transmit (uint8_t *buf, const uint8_t* sendFileName, uint32_t sizeFile)
{
    uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD];
    uint8_t filename[FILE_NAME_LENGTH];
    uint8_t *buf_ptr, tempCheckSum;
    uint16_t tempCRC;
    uint16_t blkNumber;
    uint8_t receivedC[2], CRC16_F = 0, i;
    uint32_t errors, ackReceived, size = 0, pktSize;

    errors = 0;
    ackReceived = 0;
    for (i = 0; i < (FILE_NAME_LENGTH - 1); i++){
        filename[i] = sendFileName[i];
    }
    CRC16_F = 1;

    /* Prepare first block */
    Ymodem_PrepareIntialPacket(&packet_data[0], filename, &sizeFile);

    do 
    {
        /* Send Packet */
        Ymodem_SendPacket(packet_data, PACKET_SIZE + PACKET_HEADER);

        /* Send CRC or Check Sum based on CRC16_F */
        if (CRC16_F){
            tempCRC = Cal_CRC16(&packet_data[3], PACKET_SIZE);
            Send_Byte(tempCRC >> 8);
            Send_Byte(tempCRC & 0xFF);
        }else{
            tempCheckSum = CalChecksum (&packet_data[3], PACKET_SIZE);
            Send_Byte(tempCheckSum);
        }

        /* Wait for Ack and 'C' */
        if (Receive_Byte(&receivedC[0], 10000) == 0){
            if (receivedC[0] == ACK){ /* Packet transferred correctly */
                ackReceived = 1;
            }
        }else{
            errors++;
        }
    }while (!ackReceived && (errors < 0x0A));

    if (errors >=  0x0A){
        return errors;
    }
    buf_ptr = buf;
    size = sizeFile;
    blkNumber = 0x01;
    /* Here 1024 bytes package is used to send the packets */


    /* Resend packet if NAK  for a count of 10 else end of communication */
    while (size)
    {
        /* Prepare next packet */
        Ymodem_PreparePacket(buf_ptr, &packet_data[0], blkNumber, size);
        ackReceived = 0;
        receivedC[0]= 0;
        errors = 0;
        do
        {
            /* Send next packet */
            if (size >= PACKET_1K_SIZE){
                pktSize = PACKET_1K_SIZE;
            }else{
                pktSize = PACKET_SIZE;
            }
            Ymodem_SendPacket(packet_data, pktSize + PACKET_HEADER);
            /* Send CRC or Check Sum based on CRC16_F */
            /* Send CRC or Check Sum based on CRC16_F */
            if (CRC16_F){
                tempCRC = Cal_CRC16(&packet_data[3], pktSize);
                Send_Byte(tempCRC >> 8);
                Send_Byte(tempCRC & 0xFF);
            }else{
                tempCheckSum = CalChecksum (&packet_data[3], pktSize);
                Send_Byte(tempCheckSum);
            }
          
            /* Wait for Ack */
            if ((Receive_Byte(&receivedC[0], 100000) == 0)  && (receivedC[0] == ACK)){
                ackReceived = 1;  
                if (size > pktSize){
                    buf_ptr += pktSize;  
                    size -= pktSize;
                    if (blkNumber == (USER_FLASH_SIZE/1024)){
                        return 0xFF; /*  error */
                    }else{
                        blkNumber++;
                    }
                }else{
                    buf_ptr += pktSize;
                    size = 0;
                }
            }else{
                errors++;
            }
        }while(!ackReceived && (errors < 0x0A));
        /* Resend packet if NAK  for a count of 10 else end of communication */
        if (errors >=  0x0A){
            return errors;
        }
    }
    ackReceived = 0;
    receivedC[0] = 0x00;
    errors = 0;
    do 
    {
        Send_Byte(EOT);
        /* Send (EOT); */
        /* Wait for Ack */
        if ((Receive_Byte(&receivedC[0], 10000) == 0)  && receivedC[0] == ACK){
            ackReceived = 1;  
        }else{
            errors++;
        }
    }while (!ackReceived && (errors < 0x0A));

    if (errors >=  0x0A){
        return errors;
    }

    /* Last packet preparation */
    ackReceived = 0;
    receivedC[0] = 0x00;
    errors = 0;

    packet_data[0] = SOH;
    packet_data[1] = 0;
    packet_data [2] = 0xFF;

    for (i = PACKET_HEADER; i < (PACKET_SIZE + PACKET_HEADER); i++){
        packet_data [i] = 0x00;
    }

    do 
    {
        /* Send Packet */
        Ymodem_SendPacket(packet_data, PACKET_SIZE + PACKET_HEADER);

        /* Send CRC or Check Sum based on CRC16_F */
        tempCRC = Cal_CRC16(&packet_data[3], PACKET_SIZE);
        Send_Byte(tempCRC >> 8);
        Send_Byte(tempCRC & 0xFF);

        /* Wait for Ack and 'C' */
        if (Receive_Byte(&receivedC[0], 10000) == 0)  {
            if (receivedC[0] == ACK){ 
            /* Packet transferred correctly */
            ackReceived = 1;
            }
        }else{
            errors++;
        }
    }while (!ackReceived && (errors < 0x0A));

    /* Resend packet if NAK  for a count of 10  else end of communication */
    if (errors >=  0x0A){
        return errors;
    }  

    do 
    {
        Send_Byte(EOT);
        /* Send (EOT); */
        /* Wait for Ack */
        if ((Receive_Byte(&receivedC[0], 10000) == 0)  && receivedC[0] == ACK){
            ackReceived = 1;  
        }else{
            errors++;
        }
    }while (!ackReceived && (errors < 0x0A));

    if (errors >=  0x0A){
        return errors;
    }
    return 0; /* file transmitted successfully */
}

/**
  * @}
  */

/*******************(C)COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
