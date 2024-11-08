/*
 *  utext_flash.c
 *  
 *  Created on: 2019年5月9日
 *      Author: root
 */
#include <rtthread.h>
#include "utest.h"
#include "app_lib.h"


/*
********************************************************************************
********************************************************************************
*/


/*
********************************************************************************
*Function    : test_wifi
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void test_wifi(void)
{

    uassert_true(1);
}
 /*
********************************************************************************
*Function    : utest_tc_init
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static rt_err_t utest_tc_init(void)
{
    return RT_EOK;
}
/*
********************************************************************************
*Function    : utest_tc_cleanup
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static rt_err_t utest_tc_cleanup(void)
{
    return RT_EOK;
}
/*
********************************************************************************
*Function    : testcase
*Description :
*Input       :
*Output      :
*Return      :
*Others      :
********************************************************************************
*/
static void testcase(void)
{
    UTEST_UNIT_RUN(test_wifi);
}
UTEST_TC_EXPORT(testcase, "utest-->wifi.", utest_tc_init, utest_tc_cleanup, 10);