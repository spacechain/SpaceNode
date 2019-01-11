/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: lib_gmttime.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2008 年 08 月 30 日
**
** 描        述: 系统库.

** BUG:
2010.01.04  使用新的时区信息.
2010.07.10  修正 lib_localtime_r() 返回值.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
#include "lib_local.h"
/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
#if LW_CFG_RTC_EN > 0

struct tm  *lib_gmtime_r(const time_t *time, struct tm *ptm);
/*********************************************************************************************************
** 函数名称: lib_localtime_r
** 功能描述: 
** 输　入  : 
** 输　出  : 
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
struct tm  *lib_localtime_r (const time_t *time, struct tm *ptm)
{
    time_t     timeTmp;

    if (!time || !ptm) {
        return  (LW_NULL);
    }
    
    timeTmp = UTC2LOCAL(*time);
    
    return  (lib_gmtime_r(&timeTmp, ptm));
}
/*********************************************************************************************************
** 函数名称: lib_localtime
** 功能描述: 
** 输　入  : 
** 输　出  : 
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
struct tm  *lib_localtime (const time_t *time)
{
    static struct tm  timeBuffer;
    
    return  (lib_localtime_r(time, &timeBuffer));
}

#endif                                                                  /*  LW_CFG_RTC_EN               */
/*********************************************************************************************************
  END
*********************************************************************************************************/
