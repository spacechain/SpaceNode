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
** 文   件   名: CpuPower.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2014 年 07 月 21 日
**
** 描        述: 设置 CPU 能耗.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
  裁剪控制
*********************************************************************************************************/
#if LW_CFG_POWERM_EN > 0
/*********************************************************************************************************
** 函数名称: API_CpuPowerSet
** 功能描述: 设置 CPU 能耗等级.
** 输　入  : uiPowerLevel      能耗等级
** 输　出  : ERROR
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
ULONG  API_CpuPowerSet (UINT  uiPowerLevel)
{
    bspCpuPowerSet(uiPowerLevel);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: API_CpuPowerGet
** 功能描述: 获取 CPU 能耗等级.
** 输　入  : puiPowerLevel     能耗等级
** 输　出  : ERROR
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
ULONG  API_CpuPowerGet (UINT  *puiPowerLevel)
{
    if (!puiPowerLevel) {
        _ErrorHandle(EINVAL);
        return  (EINVAL);
    }

    bspCpuPowerGet(puiPowerLevel);
    
    return  (ERROR_NONE);
}

#endif                                                                  /*  LW_CFG_POWERM_EN > 0        */
/*********************************************************************************************************
  END
*********************************************************************************************************/
