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
** 文   件   名: rm_tool.c
**
** 创   建   人: Yang.Zhuang (杨壮)
**
** 文件创建日期: 2017 年 12 月 27 日
**
** 描        述: 递归删除工具
*********************************************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

#define VERSION         "beta-1.0"                                      /*  版本号                      */
/*********************************************************************************************************
**全局变量iRecursiveTimes，避免递归深度过深导致栈溢出
*********************************************************************************************************/
static INT  iRecursiveTimes = 256;                                      /*  最大递归次数                */
/*********************************************************************************************************
** 函数名称: usage
** 功能描述: 打印工具使用方法
** 输　入  : NONE
** 输　出  : 工具使用方法
** 返  回  : NONE
*********************************************************************************************************/
static VOID usage (VOID)
{
    fprintf(stderr,"rm_tool [-r R] <target>\n");

}
/*********************************************************************************************************
** 函数名称: __unlink_recursive
** 功能描述: 删除文件，当文件是多层目录时，递归删除文件
** 输　入  : pcFileName      文件名
** 输　出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
static INT __unlink_recursive (CPCHAR   pcFileName)
{
    struct stat    stFile;
    struct dirent *pdDirectoryInfo;
    DIR           *pdDir;
    CHAR           cPathLen[PATH_MAX];
    BOOL           bIsFail             =  LW_FALSE;

    /*
     *  判断目标文件属性
     */
    if (lstat(pcFileName, &stFile) < 0) {
        return  (PX_ERROR);
    }

    /*
     *  非目录的话直接删除
     */
    if (!S_ISDIR(stFile.st_mode)) {
        unlink(pcFileName);

        return  (ERROR_NONE);
    }

    /*
     *  判断递归次数
     */
    if (0 == iRecursiveTimes) {
        fprintf(stderr,"NOTICE: Path is too long!!! \n");
        errno = EFBIG;
        return  (PX_ERROR);
    }

    /*
     *  文件夹
     */
    pdDir = opendir(pcFileName);                                        /*  获取已打开单目录信息        */
    if (NULL == pdDir) {
        return  (PX_ERROR);
    }

    pdDirectoryInfo = readdir(pdDir);
    while (NULL != pdDirectoryInfo) {                                   /*  对目录进行递归              */
        iRecursiveTimes--;
        if (!strcmp(pdDirectoryInfo->d_name, "..") || !strcmp(pdDirectoryInfo->d_name, ".")) {
            continue;
        }

        sprintf(cPathLen, "%s/%s", pcFileName, pdDirectoryInfo->d_name);

        if (__unlink_recursive(cPathLen) < 0) {
            bIsFail = LW_TRUE;
            break;
        }
        pdDirectoryInfo = readdir(pdDir);
    }

    if (closedir(pdDir) < 0) {                                          /*  关闭目录句柄                */
        return  (PX_ERROR);
    }

    if (bIsFail) {
        return  (PX_ERROR);
    } else {
        rmdir(pcFileName);                                              /*  删除目标目录                */
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: main主函数
** 功能描述: 递归删除文件
** 输　入  : 直接填写文件名或者选择递归选项加文件名
** 输　出  : NONE
** 返  回  : ERROR_CODE
*********************************************************************************************************/
INT main (INT iArgc, CHAR *pcArgv[])
{
    INT  i          = 1;
    INT  iRet       = 0;
    BOOL bRecursive = LW_FALSE;

    if ((iArgc < 2) || (iArgc > 3)) {                                   /*  参数个数判断                */
        usage();

        return  (ERROR_NONE);
    }

    if ((iArgc >= 2) && (!strcmp(pcArgv[1], "-r") ||                     /*  参数判断，是否递归删除      */
                         !strcmp(pcArgv[1], "-R"))) {
        if (2 == iArgc) {                                               /*  排除只跟 -r 或 -R 参数的情况*/
            usage();

            return  (ERROR_NONE);
        }

        bRecursive = LW_TRUE;                                           /*  置是否递归为 1              */
        i          = 2;
    }

    for (; i < iArgc; i++) {                                            /*  删除操作                    */
        iRet = bRecursive ? __unlink_recursive(pcArgv[i]) : unlink(pcArgv[i]);
        if (iRet < 0) {
            fprintf(stderr, "can not remove this file, error: %s\n", strerror(errno));

            return  (PX_ERROR);
        }
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
