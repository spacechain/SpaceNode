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
** 文   件   名: dm9000.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2016 年 10 月 31 日
**
** 描        述: dm9000 网卡驱动支持.
*********************************************************************************************************/

#ifndef __DM9000_H
#define __DM9000_H

#include "netdev.h"

/*********************************************************************************************************
  dm9000 configuration
*********************************************************************************************************/
struct dm9000_netdev {
    struct netdev   netdev;                                             /*  net device                  */

    /*
     *  user MUST set following members before calling this module api.
     */
    void  (*init)(struct dm9000_netdev *dm9000);                        /*  init function               */

    addr_t base;                                                        /*  base memory                 */
    addr_t io;                                                          /*  io-offset                   */
    addr_t data;                                                        /*  data-offset                 */

    int irq;                                                            /*  irq vector                  */

    UINT8 mac[6];                                                       /*  hwaddr                      */
};

/*********************************************************************************************************
  dm9000 driver functions
*********************************************************************************************************/

INT  dm9000Init(struct dm9000_netdev *dm9000, const char *ip, const char *netmask, const char *gw);
VOID dm9000Isr(struct dm9000_netdev *dm9000);

#endif                                                                  /*  __DM9000_H                  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
