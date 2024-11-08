#! /usr/bin/env python
# -*- coding: utf-8 -*-
#
# =============================================================================
# Copyright (c), Fujian XXXX Tech Co., Ltd.
# All rights reserved.
#
# Change Logs:
# Date			Author		Action
# 2019/01/02	emb_wlq		Create this file
# =============================================================================
# 

"""Image process tool
用于镜像的加工处理，产生可烧片和升级的安全镜像

usage: create_image.py <prj_workroot> <prj_onfig> <boot_file_subpath> <raw_bin_filename_only>
其中：
	1、%prj_workroot%位工程项目归档存储的root路径，而不是IDE的工程路径
	2、%prjConfig%典型取值为Debug/Release
	3、%boot_file_subpath%为bootloader目录下的sub path，即.\common\bootloader下面的路径信息
	4、%raw_bin_filename_only%为编译生成的原始bin可执行文件的名称，不包括后缀名，一般直接取项目名

典型的例子如下：
	cmd /c "python $PROJ_DIR$\..\..\tools\create_image.py $PROJ_DIR$\..\.. Debug bootloader.bin $PROJ_FNAME$"

	执行此脚本后，将会在output\%prjConfig%\下面生成以下文件：
	1、security_fullimage_%raw_bin_filename_only%.bin，此文件是开启安全的、包括boot、app的完整镜像，用于工厂的烧片用途
	2、cipher_firmware_%raw_bin_filename_only%.bin，加密升级文件，用于升级用途

	其中，cipher_firmware_%raw_bin_filename_only%.bin采用
	AES128 ECB模式加密，加密主秘钥见下文中的mainKey定义
	实际使用时，采用一次一密机制，采用工作秘钥进行加密，见下文的workKey定义
	分散算法为：用主密钥加密用16位的随机数，得到工作秘钥
	这部分数据，存储在可执行镜像的头部，成为img header，此格式为一个TLV格式，具体数据组织见
	下文的imgHeader说明
"""

__author__ = 'emb_wlq'

import sys
import os
import zipfile
import binascii
import struct
from Crypto.Cipher import AES
from Crypto import Random

# usage: create_image.py <prj_workroot> <prj_onfig> <boot_file_subpath> <raw_bin_filename_only>
# cmd /c "python $PROJ_DIR$\..\tools\create_image.py $PROJ_DIR$\..\ Debug output\bootload.bin $PROJ_FNAME$ $CONFIG_NAME$"
def main(argv):
	print ''
	# Collect arguments
	if len(sys.argv) != 6:
		print 'usage: create_image.py <prj_workroot> <prj_onfig> <boot_file_subpath> <raw_bin_filename_only>'
		sys.exit(2)
	prj_workroot = argv[0]
	prj_onfig = argv[1]
	boot_file_subpath = argv[2]
	raw_bin_filename_only = argv[3]
	config_name = argv[4]

	# Change work dir
	os.chdir(prj_workroot)

	# Set relative paths
	binToEncryptFilename = 'output\\iar\\' + prj_onfig + '\\' + raw_bin_filename_only + '.bin'
	cipherFirmwareFilename = 'output\\iar\\' + prj_onfig + '\\yq_' + config_name + '_' + raw_bin_filename_only + '.bin'
	securityFullImageForBurnFilename = 'output\\iar\\' + prj_onfig + '\\fullimage_yq_' + config_name + '_' + raw_bin_filename_only + '.bin'
	bootFilename = 'bsp\\bootload\\' + boot_file_subpath
	
	print 'prj_workroot is %s' % prj_workroot
	print ''
	
	if os.path.exists('output\\iar\\' + prj_onfig) == False:
		os.mkdir('output\\iar\\' + prj_onfig)
	#bin file size
	src_fp_size = os.path.getsize(binToEncryptFilename)

	# Open raw img bin file
	try:
		binToEncryptFile = open(binToEncryptFilename, 'rb')
	except IOError:
		print 'cannot open raw bin file %s' % binToEncryptFilename, '\n'
		sys.exit(2)
	data = binToEncryptFile.read()

	crcval = crc16(data)
	printstr = "crcval 0x%x 0x%x 0x%x length %x"%(crcval,crcval&0x00ff,(crcval>>8)&0x00ff,src_fp_size);
    
	print(printstr)
    
	# Make TLV-formated img header
	# magicCode(4 Bytes) + tagHeader(2 Bytes) + lenHeader (2 Bytes)
	#                                                               + tagAddr(2) + lenAddr(2) + addrInfo(8)
	#                                                               + tagRnd(2) + lenRnd(2) + Rnd(16)

	imgHeader = bytearray(512)
	imgHeader[0:4] = ['Y', 'Q', 'I', '.']
	imgHeader[4:6] = [1, 0]
	imgHeader[6:8] = [32, 0]
	# addrInfo: offset + length
	imgHeader[8:10] = [2, 0]
	imgHeader[10:12] = [8, 0]
	imgHeader[12:16] = [0, 0, 0, 0]
	imgHeader[16:20] = [64, 0, 0, 0]
	# random
	imgHeader[20:22] = [2, 0]
	imgHeader[22:24] = [16, 0]
	imgHeader[24:40] = Random.new().read(AES.block_size)
	imgHeader[40:44] = [0, 0, 0, 0]
	imgHeader[44:48] = [crcval&0x00ff,(crcval>>8)&0x00ff]
	imgHeader[48:52] = [src_fp_size&0x00ff,(src_fp_size>>8)&0x00ff,(src_fp_size>>16)&0x00ff,(src_fp_size>>24)&0x00ff]
	
	imgHeader[52:512] = [0xFF] * 460
	# for test: imgHeader[24:40] = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
	print 'img header is', binascii.b2a_hex(imgHeader[:40]), '\n'
	
	# Make cipher bin img for update...
	# Encrypt img region, use AES128 ECB mode
	# Make workkey
	# mainKey = b'Bfqz r&d studio.'
	mainKey = b'YQI.. $DOGCOLLA$'
	# for test: mainKey = b'Sixteen byte key'
	iv = "".join(map(chr, imgHeader[24:40]))
	print 'iv is', binascii.b2a_hex(iv)
	cipher = AES.new(mainKey, AES.MODE_ECB, iv)
	workKey = cipher.encrypt(iv)
	workCipher = AES.new(workKey, AES.MODE_ECB, iv)
	print 'workkey is', binascii.b2a_hex(workKey)
	# use workkey to encrypt, then save cipher img bin, for update
	encryptAddr = (imgHeader[15] << 24) + (imgHeader[14] << 16) + (imgHeader[13] << 8) + imgHeader[12]
	encryptLength = (imgHeader[19] << 24) + (imgHeader[18] << 16) + (imgHeader[17] << 8) + imgHeader[16]
	print 'encryptAddr is', encryptAddr
	print 'encryptLength is', encryptLength
	
	cipherFirmwareFile = open(cipherFirmwareFilename, 'w+b')
	cipherFirmwareFile.seek(0, 0)
	cipherFirmwareFile.write(imgHeader)
	
	binToEncryptFile.seek(0, 0)
	data = binToEncryptFile.read(encryptAddr - 0)
	cipherFirmwareFile.write(data)
	
	index = 0
	while index < encryptLength:
		index += AES.block_size
		data = binToEncryptFile.read(AES.block_size)
		print 'plain data block is', binascii.b2a_hex(data[:AES.block_size])
		bytesBlock = workCipher.encrypt(data)
		print 'cipher data block is', binascii.b2a_hex(bytesBlock[:AES.block_size])
		cipherFirmwareFile.write(bytesBlock)
	data = binToEncryptFile.read()
	cipherFirmwareFile.write(data)
	cipherFirmwareFile.close()
	print 'saved cipher firmware file for update is', cipherFirmwareFilename
	print ''

	# Make security full image, for burn, for factory...
	securityFullImageForBurnFile = open(securityFullImageForBurnFilename, 'w+b')
	# TIP: 0x8000 is boot max size, our app is begin from this addr
	# firmwareBase = 0xA000
	firmwareBase = 0x20000
	fullimage = bytearray(firmwareBase + len(imgHeader) + os.path.getsize(binToEncryptFilename))
	
	# Fill with bootloader
	bootFile = open(bootFilename, 'rb')
	fullimage[0:] = bootFile.read()
	bootFile.close()
	
	# Align to firmwareBase, fill with '0xFF'
	fullimage[len(fullimage):] = [0xFF] * (firmwareBase - len(fullimage))
	
	# Fill with plain img bin file, from firmwareBase
	fullimage[firmwareBase:] = imgHeader
	binToEncryptFile.seek(0, 0)
	fullimage[(firmwareBase + len(imgHeader)):] = binToEncryptFile.read()
	
	# Save full image
	securityFullImageForBurnFile.write(fullimage)
	securityFullImageForBurnFile.close()
	print 'saved security full image for factory burn is', securityFullImageForBurnFilename
	print ''

	# Delete temp files
	binToEncryptFile.close()
	#os.remove(binToEncryptFilename)
    
def crc16(message):
    #CRC-16-CITT poly, the CRC sheme used by ymodem protocol
    poly = 0x1021
    #16bit operation register, initialized to zeros
    reg = 0x0000
    #pad the end of the message with the size of the poly
    message += '\x00\x00' 
    #for each bit in the message
    for byte in message:
        mask = 0x80
        while(mask > 0):
            #left shift by one
            reg<<=1
            #input the next bit from the message into the right hand side of the op reg
            if ord(byte) & mask:   
                reg += 1
            mask>>=1
            #if a one popped out the left of the reg, xor reg w/poly
            if reg > 0xffff:            
                #eliminate any one that popped out the left
                reg &= 0xffff           
                #xor with the poly, this is the remainder
                reg ^= poly
    return reg
if __name__ == "__main__":
	main(sys.argv[1:])
