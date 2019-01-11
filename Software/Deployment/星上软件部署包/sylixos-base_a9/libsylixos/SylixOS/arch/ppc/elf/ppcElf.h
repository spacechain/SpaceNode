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
** 文   件   名: ppcElf.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2015 年 12 月 15 日
**
** 描        述: 实现 PowerPC 体系结构的 ELF 文件重定位.
*********************************************************************************************************/

#ifndef __ARCH_PPCELF_H
#define __ARCH_PPCELF_H

#ifdef LW_CFG_CPU_ARCH_PPC                                              /*  PowerPC 体系结构            */

#define ELF_CLASS       ELFCLASS32
#define ELF_ARCH        EM_PPC

#endif                                                                  /*  LW_CFG_CPU_ARCH_PPC         */
#endif                                                                  /*  __ARCH_PPCELF_H             */
/*********************************************************************************************************
  END
*********************************************************************************************************/
