/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-14     lwp          edit
 */                                                                         
#include "app_lib.h"                                                          
#include "rt_utils_tlv.h"
/*
********************************************************************************
********************************************************************************
*/

/*
********************************************************************************
********************************************************************************
*/ 
static const char g_c_hex[] = "0123456789ABCDEF";
/*
********************************************************************************
********************************************************************************
*/ 
/*
********************************************************************************
Function      : rt_utils_mem_nibble_swap
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
void rt_utils_mem_nibble_swap(uint8_t* bytes, uint16_t len)
{
    uint8_t temp;
    while (len != 0) {
        len--;
        temp = bytes[len];
        bytes[len] = ((temp >> 4) & 0x0F) | ((temp << 4) & 0xF0);
    }
}

/*
********************************************************************************
Function      : rt_utils_bytes_to_hex
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
uint32_t rt_utils_bytes_to_hex(uint8_t* data, uint32_t data_len, char* buf)
{
    uint32_t i, j;

    for (i = 0, j = 0; i < data_len; i++, j += 2) {
        buf[j] = g_c_hex[(data[i] >> 4) & 0x0F];
        buf[j + 1] = g_c_hex[data[i] & 0x0F];
    }
    buf[j] = 0x00;

    return data_len * 2;
}
/*
********************************************************************************
Function      : rt_lpa_get_profile_info
Description   :  
Input         :
Output        :
Return        :
Others        :
********************************************************************************
*/
int rt_lpa_get_profile_info(char* buf, profile_info_t* pi, char* num, char max_num)
{
    int ret = 0;
    int size = 1024 * 10;
    int i;
    int offset;
    uint32_t total_info_len;
    uint32_t info_len;
    int info_off;
    int element_off;
    char channel = 0xFF;

    if (!num) {
        ret = -1;
        goto end;
    }

    /*
    -- Definition of ProfileInfoListResponse
    ProfileInfoListResponse ::= [45] CHOICE { -- Tag 'BF2D'
        profileInfoListOk SEQUENCE OF ProfileInfo,
        profileInfoListError ProfileInfoListError
    }
    ProfileInfo ::= [PRIVATE 3] SEQUENCE { -- Tag 'E3'
        iccid Iccid OPTIONAL,
        isdpAid [APPLICATION 15] OctetTo16 OPTIONAL, -- AID of the ISD-P containing the Profile, tag '4F'
        profileState [112] ProfileState OPTIONAL, -- Tag '9F70'
        profileNickname [16] UTF8String (SIZE(0..64)) OPTIONAL, -- Tag '90'
        serviceProviderName [17] UTF8String (SIZE(0..32)) OPTIONAL, -- Tag '91'
        profileName [18] UTF8String (SIZE(0..64)) OPTIONAL, -- Tag '92'
        iconType [19] IconType OPTIONAL, -- Tag '93'
        icon [20] OCTET STRING (SIZE(0..1024)) OPTIONAL, -- Tag '94', see condition in ES10c:GetProfilesInfo
        profileClass [21] ProfileClass OPTIONAL, -- Tag '95'
        notificationConfigurationInfo [22] SEQUENCE OF NotificationConfigurationInformation OPTIONAL, -- Tag 'B6'
        profileOwner [23] OperatorId OPTIONAL, -- Tag 'B7'
        dpProprietaryData [24] DpProprietaryData OPTIONAL, -- Tag 'B8'
        profilePolicyRules [25] PprIds OPTIONAL -- Tag '99'
    }

    IconType ::= INTEGER { jpg(0), png(1) }
    ProfileState ::= INTEGER { disabled(0), enabled(1) }
    ProfileClass ::= INTEGER { test(0), provisioning(1), operational(2) }
    ProfileInfoListError ::= INTEGER { incorrectInputValues(1), undefinedError(127) }
    */

    //  ProfileInfoListResponse
    offset = rt_bertlv_get_tl_length((uint8_t*)buf, NULL);

    if (rt_bertlv_get_tag((uint8_t*)buf + offset, NULL) != 0xA0) {
        ret = -1;
        goto end;
    }

    // profileInfoListOk SEQUENCE OF ProfileInfo
    offset += rt_bertlv_get_tl_length((uint8_t*)buf + offset, &total_info_len);

    if (pi != NULL) {
        for (i = 0, *num = 0; i < total_info_len; (*num)++) {
            if (*num >= max_num) {
                printf("too many profile detected (%d > %d)\n", *num, max_num);
                break;
            }
            // ProfileInfo E3 TL
            info_off = rt_bertlv_get_tl_length((uint8_t*)buf + offset + i, &info_len);

            // find ICCID
            element_off = rt_bertlv_find_tag((uint8_t*)buf + offset + i + info_off, info_len, 0x5A, 1);
            if (element_off != BER_TLV_INVALID_OFFSET) {
                // get ICCID value offset
                element_off += rt_bertlv_get_tl_length((uint8_t*)buf + offset + i + info_off + element_off, NULL);
                //rt_utils_mem_nibble_swap((uint8_t*)buf + offset + i + info_off + element_off, 10);
                rt_utils_bytes_to_hex((uint8_t*)buf + offset + i + info_off + element_off, 10, pi[*num].iccid);
            }

            // find ProfileClass
            element_off = rt_bertlv_find_tag((uint8_t*)buf + offset + i + info_off, info_len, 0x95, 1);
            if (element_off != BER_TLV_INVALID_OFFSET) {
                pi[*num].type = (uint8_t)rt_bertlv_get_integer((uint8_t*)buf + offset + i + info_off + element_off, NULL);
            }

            // find ProfileState
            element_off = rt_bertlv_find_tag((uint8_t*)buf + offset + i + info_off, info_len, 0x9F70, 1);
            if (element_off != BER_TLV_INVALID_OFFSET) {
                pi[*num].state = (uint8_t)rt_bertlv_get_integer((uint8_t*)buf + offset + i + info_off + element_off, NULL);
            }

            // get next E3 TLV
            i += rt_bertlv_get_tlv_length((uint8_t*)buf + offset + i);
        }
    }

end:

    return ret;
}
