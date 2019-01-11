/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: spc_slave.c
**
** 创   建   人: Zhang.ZhaoGe (张兆阁)
**
** 文件创建日期: 2018 年 07 月 15 日
**
** 描        述: spc 从机端代码 (模拟了载荷端功能)
*********************************************************************************************************/
#include "SylixOS.h"
#include "pthread.h"
#include "stdlib.h"
#include "string.h"
#include "getopt.h"
#include <stdio.h>
#include <endian.h>
#include "hwrtc/hwrtc.h"
#include "can/can.h"
#include "json/json.h"
#include <iostream>
#include <string>
/*********************************************************************************************************
  数据类型定义
  [start flag 0x5A][2B long][xB data][end flag 0xA5][...]
*********************************************************************************************************/
typedef struct {
    UINT8     ucStartFlag;
    UINT16    usDataLong;                                               /*  最大是 496 字节             */
    UINT8     ucData[1];
} __attribute__((packed)) ProtocolDataHead;
/*********************************************************************************************************
  宏定义
*********************************************************************************************************/
#define IMPORTPRIVKEYLEN          43                                    /*  导入私钥命令长度            */
#define PRIVKEYLEN                52                                    /*  冷钱包私钥字节长度          */
#define RCVBUFLEN                 50                                    /*  接收数据缓冲区打小          */
#define ALLRCVBUFLEN              500                                   /*  接收数据缓冲区打小          */
#define SENDBUFLEN                198                                   /*  发送数据缓冲区打小          */
#define ALLSENDBUFLEN             2048                                  /*  发送数据缓冲区打小          */
#define CANBUAD                   500000                                /*  can 波特率                  */
#define CANDEV                    "/dev/can0"                           /*  can 设备文件                */
#define SIGCMDSTR                 "/apps/qtum/qtum-cli signrawtransaction "
                                                                        /*  交易签名命令                */
#define STARTSTR                  " ["                                  /*  txid key 开始字符串         */
#define TXIDSTR                   "{\\\"txid\\\":\\\""                  /*  txid key 字符串             */
#define VOUTSTR                   "\\\",\\\"vout\\\":"                  /*  vout key 字符串             */
#define PUBKEYSTR                 ",\\\"scriptPubKey\\\":\\\""          /*  pubkey key 字符串           */
#define REDEEMSCRIPTSTR           "\\\",\\\"redeemScript\\\":\\\""      /*  amount key 字符串           */
#define ENDSTR                    "\\\"},"                              /*  end 字符串                  */
#define ENDENDSTR                 "]"                                   /*  end end 字符串              */
#define PRIVKEYSTARTSTR           " [\\\""                              /*  priv key start 字符串       */
#define PRIVKEYENDSTR             "\\\"]"                               /*  priv key end 字符串         */
#define PRIVKEYFILEDIC            "/root/"                              /*  priv key 文件路径           */
#define IMPORTPRIVKEYSTR          "/apps/qtum/qtum-cli importprivkey "
                                                                        /*  导入私钥命令                */
/*********************************************************************************************************
  全局变量定义
*********************************************************************************************************/
static UINT32 _G_usRestartCnt          = 0;                             /*  记录从机重启计数的          */

static UCHAR  _G_ucData[CAN_MAX_DATA]  = {0xFA, 0xCE, 0xFA, 0xCE,
                                         0xFA, 0xCE, 0xFA, 0xCE};       /*  主机发送复位命令的负载数据  */
static UCHAR  _G_ucSendBuf[SENDBUFLEN];                                 /*  发送数据缓冲区              */
static UCHAR  _G_ucAllSendBuf[ALLSENDBUFLEN];                           /*  发送数据总缓冲区            */
static UINT   _G_uiAllSendDataIndex    = 0;                             /*  发送数据总缓冲索引          */
static UCHAR  _G_ucRcvBuf[RCVBUFLEN];                                   /*  接收数据缓冲区              */
static UCHAR  _G_ucAllRcvBuf[ALLRCVBUFLEN];                             /*  接收数据总缓冲区            */
static UINT   _G_uiAllRcvDataIndex     = 0;                             /*  接收数据总缓冲索引          */
/*********************************************************************************************************
** 函数名称: spcStringStrip
** 功能描述: 去除缓冲区中的回车和空格
** 输　入  : cString ：需要格式化的缓冲区
**         : iLen    ：字符串长度
** 输　出  : 格式化后的字符串首地址
** 全局变量:
** 调用模块:
*********************************************************************************************************/
CHAR  *spcStringStrip(CHAR  *cString, INT  iLen)
{
    INT   i     = 0;
    CHAR *cRet  = cString;
    CHAR *cDest = cString;

    while (((' ' == *cString) || ('\n' == *cString) || ('\r' == *cString)) ?
            (*cString++) :
            (*cDest++ = *cString++)) {
        if (++i >= iLen) {
            *cDest = '\0';
            break;
        }
    }

    return cRet;
}
/*********************************************************************************************************
** 函数名称: spcSlaveReturnTelemetryData
** 功能描述: 从机给主机返回遥测数据
** 输　入  : iFd：     can 总线设备文件
**           pucBuf：  发送数据缓冲区
**           iDataLen：发送数据长度，最大198
** 输　出  :
** 附件信息: 每次 198 字节 = 1个首帧(5B) + 27个中间帧(7B) + 1个尾帧(4B)
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  spcSlaveReturnTelemetryData (INT  iFd, UCHAR  *pucBuf, INT  iDataLen)
{
    CAN_FRAME   canframe;
    ssize_t     stLen;
    ssize_t     stFrameNum;
    UCHAR       ucCnt = 0;
    INT         iIndex;

    if (iDataLen > SENDBUFLEN) {
        printf("data too long\n");
        return;
    }

    memset(_G_ucSendBuf, 0, SENDBUFLEN);
    memcpy(_G_ucSendBuf, pucBuf, iDataLen);                             /*  复制数据到本地缓冲区中      */

    /*
     * 打包首帧数据
     */
    canframe.CAN_bRtr       = 0;                                        /*  非远程帧                    */
    canframe.CAN_bExtId     = 0;                                        /*  非扩展帧                    */
    canframe.CAN_uiId       = 0x431;
    canframe.CAN_ucLen      = 8;

    canframe.CAN_ucData[0]  = 0x01;
    canframe.CAN_ucData[1]  = 29;
    canframe.CAN_ucData[2]  = 0x1A;

    memcpy(&canframe.CAN_ucData[3], _G_ucSendBuf + ucCnt, 5);           /*  首帧有效负载数据长度为5字节 */
    ucCnt += 5;

    stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
    ioctl(iFd, CAN_DEV_STARTUP, 0);

    stFrameNum = stLen / sizeof(CAN_FRAME);

    if (stFrameNum != 1) {
        printf("failed to send can frame!, abort sending\n");
        return;
    }

    /*
     * 打包中间帧数据
     */
    canframe.CAN_uiId = 0x432;

    for (iIndex = 2; iIndex <= 28; iIndex++) {
        canframe.CAN_ucData[0] = iIndex;

        memcpy(&canframe.CAN_ucData[1], _G_ucSendBuf + ucCnt, 7);
        ucCnt += 7;

        stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
        ioctl(iFd, CAN_DEV_STARTUP, 0);

        stFrameNum = stLen / sizeof(CAN_FRAME);

        if (stFrameNum != 1) {
            printf("failed to send can frame!, abort sending\n");
            return;
        }
    }

    /*
     * 打包尾帧数据
     */
    canframe.CAN_uiId      = 0x433;
    canframe.CAN_ucLen     = 5;
    canframe.CAN_ucData[0] = 29;

    memcpy(&canframe.CAN_ucData[1], _G_ucSendBuf + ucCnt, 4);           /*  尾帧有效数据长度是 4 字节   */

    stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
    ioctl(iFd, CAN_DEV_STARTUP, 0);

    stFrameNum = stLen / sizeof(CAN_FRAME);

    if (stFrameNum != 1) {
        printf("failed to send can frame!, abort sending\n");
        return;
    }

    return;
}
/*********************************************************************************************************
** 函数名称: spcMasterRequestTelemetryData
** 功能描述: 从机处理主机发送的请求遥测数据命令
** 输　入  : pvArg：创建线程时传入的参数
** 输　出  :
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  *spcMasterRequestTelemetryData(INT  iFd)
{
    UCHAR  ucBuf[SENDBUFLEN];

    ioctl(iFd, FIOFLUSH, 0);

    memcpy(ucBuf, _G_ucAllSendBuf + _G_uiAllSendDataIndex, SENDBUFLEN);
    spcSlaveReturnTelemetryData(iFd, ucBuf, SENDBUFLEN);
    _G_uiAllSendDataIndex += SENDBUFLEN;

    printf("send Telemetry Data\n");

    if (_G_uiAllSendDataIndex >= ALLSENDBUFLEN) {
        _G_uiAllSendDataIndex = 0;
    }

    /*
     * 等待数据全部发送完成
     */
    while (1) {
        INT  iNum;

        if (ioctl(iFd, FIONWRITE, &iNum)) {
            printf("error");
            break;
        }

        if (iNum == 0) {
            break;
        }
    }

    ioctl(iFd, CAN_DEV_STARTUP, 0);

    return  (NULL);
}
/*********************************************************************************************************
** 函数名称: spcReturnTestTelemetryData
** 功能描述: 从机处理主机发送的请求测试数据命令
** 输　入  : pvArg：创建线程时传入的参数
** 输　出  :
** 附件信息: 每次 50 字节 = 1个首帧(5B) + 6个中间帧(7B) + 1个尾帧(3B)
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  spcReturnTestTelemetryData (INT  iFd, UCHAR  *pucBuf, INT  iDataLen)
{
    CAN_FRAME   canframe;
    ssize_t     stLen;
    ssize_t     stFrameNum;
    UCHAR       ucCnt = 0;
    INT         iIndex;

    if (iDataLen > SENDBUFLEN) {
        printf("data too long\n");
        return;
    }

    memset(_G_ucSendBuf, 0, SENDBUFLEN);
    memcpy(_G_ucSendBuf, pucBuf, iDataLen);                             /*  复制数据到本地缓冲区中      */

    /*
     * 打包首帧数据
     */
    canframe.CAN_bRtr       = 0;                                        /*  非远程帧                    */
    canframe.CAN_bExtId     = 0;                                        /*  非扩展帧                    */
    canframe.CAN_uiId       = 0x431;
    canframe.CAN_ucLen      = 8;

    canframe.CAN_ucData[0]  = 0x01;
    canframe.CAN_ucData[1]  = 8;
    canframe.CAN_ucData[2]  = 0x1B;

    memcpy(&canframe.CAN_ucData[3], _G_ucSendBuf + ucCnt, 5);           /*  首帧有效负载数据长度为5字节 */
    ucCnt += 5;

    stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
    ioctl(iFd, CAN_DEV_STARTUP, 0);

    stFrameNum = stLen / sizeof(CAN_FRAME);

    if (stFrameNum != 1) {
        printf("failed to send can frame!, abort sending\n");
        return;
    }

    /*
     * 打包中间帧数据
     */
    canframe.CAN_uiId = 0x432;

    for (iIndex = 2; iIndex <= 7; iIndex++) {
        canframe.CAN_ucData[0] = iIndex;

        memcpy(&canframe.CAN_ucData[1], _G_ucSendBuf + ucCnt, 7);
        ucCnt += 7;

        stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
        ioctl(iFd, CAN_DEV_STARTUP, 0);

        stFrameNum = stLen / sizeof(CAN_FRAME);

        if (stFrameNum != 1) {
            printf("failed to send can frame!, abort sending\n");
            return;
        }
    }

    /*
     * 打包尾帧数据
     */
    canframe.CAN_uiId      = 0x433;
    canframe.CAN_ucLen     = 4;
    canframe.CAN_ucData[0] = 8;

    memcpy(&canframe.CAN_ucData[1], _G_ucSendBuf + ucCnt, 3);           /*  尾帧有效数据长度是 3 字节   */

    stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
    ioctl(iFd, CAN_DEV_STARTUP, 0);

    stFrameNum = stLen / sizeof(CAN_FRAME);

    if (stFrameNum != 1) {
        printf("failed to send can frame!, abort sending\n");
        return;
    }

    return;
}
/*********************************************************************************************************
** 函数名称: spcMasterRequestTestData
** 功能描述: 从机处理主机发送的请求测试数据命令
** 输　入  : pvArg：创建线程时传入的参数
** 输　出  :
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  *spcMasterRequestTestData(INT  iFd)
{
    UCHAR             ucBuf[50] = {0};
    UCHAR             ucIndex;
    UINT32            uiStartTime;
    CHAR             *pcRetData = "SpacechainSupportQtumMultisigTransactions";

    ioctl(iFd, FIOFLUSH, 0);

    uiStartTime = (UINT32)(Lw_Time_Get() / Lw_Time_GetFrequency());

    memcpy(ucBuf, pcRetData, strlen(pcRetData) + 1);

    ucIndex = 42;
    ucBuf[ucIndex++]  = uiStartTime & 0xFF;
    ucBuf[ucIndex++]  = uiStartTime & 0xFF00;
    ucBuf[ucIndex++]  = uiStartTime & 0xFF0000;
    ucBuf[ucIndex++]  = uiStartTime & 0xFF000000;

    ucBuf[ucIndex++]  = _G_usRestartCnt & 0xFF;
    ucBuf[ucIndex++]  = _G_usRestartCnt & 0xFF00;
    ucBuf[ucIndex++]  = _G_usRestartCnt & 0xFF0000;
    ucBuf[ucIndex++]  = _G_usRestartCnt & 0xFF000000;

    spcReturnTestTelemetryData (iFd, ucBuf, 50);

    /*
     * 等待数据全部发送完成
     */
    while (1) {
        INT  iNum;

        if (ioctl(iFd, FIONWRITE, &iNum)) {
            printf("error");
            break;
        }

        if (iNum == 0) {
            break;
        }
    }

    ioctl(iFd, CAN_DEV_STARTUP, 0);

    return  (NULL);
}
/*********************************************************************************************************
** 函数名称: spcSlaveReturnRcvDataNum
** 功能描述: 从机返回当前已经接收到的数据字节数
** 输　入  : 创建线程时传入的参数
** 输　出  :
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  *spcSlaveReturnRcvDataNum(INT  iFd, UCHAR  ucDataNum)
{
    CAN_FRAME   canframe;
    ssize_t     stLen;
    ssize_t     stFrameNum;

    ioctl(iFd, FIOFLUSH, 0);

    /*
     * 打包帧数据
     */
    canframe.CAN_bRtr       = 0;                                        /*  非远程帧                    */
    canframe.CAN_bExtId     = 0;                                        /*  非扩展帧                    */
    canframe.CAN_uiId       = 0x42C;
    canframe.CAN_ucLen      = 1;

    canframe.CAN_ucData[0]  = ucDataNum;

    stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
    ioctl(iFd, CAN_DEV_STARTUP, 0);

    stFrameNum = stLen / sizeof(CAN_FRAME);

    if (stFrameNum != 1) {
        printf("failed to send can frame!, abort sending\n");
        return  (LW_NULL);
    }

    /*
     * 等待数据全部发送完成
     */
    while (1) {
        INT iNum;

        if (ioctl(iFd, FIONWRITE, &iNum)) {
            printf("error");
            break;
        }

        if (iNum == 0) {
            break;
        }
    }

    ioctl(iFd, CAN_DEV_STARTUP, 0);

    return  (NULL);
}
/*********************************************************************************************************
** 函数名称: spcCharToHex
** 功能描述: 把字符转成二进制数
** 输　入  : cIn
** 输　出  : 结果
** 全局变量:
** 调用模块:
*********************************************************************************************************/
UCHAR  spcCharToHex (CHAR  cIn)
{
    if ((cIn >= '0') && (cIn <= '9')) {
        return (cIn - '0');
    } else if ((cIn >= 'a') && (cIn <= 'f')) {
        return (cIn - 'a' + 10);
    } else if ((cIn >= 'A') && (cIn <= 'F')) {
        return (cIn - 'A' + 10);
    } else {
        return (-1);                                                    /*  NOP                         */
    }
}
/*********************************************************************************************************
** 函数名称: spcHexToChar
** 功能描述: 把字符转成二进制数
** 输　入  : cIn
** 输　出  : 结果
** 全局变量:
** 调用模块:
*********************************************************************************************************/
CHAR  spcHexToChar (UCHAR  ucIn)
{
    if ((ucIn >= 0) && (ucIn <= 9)) {
        return (ucIn + '0');
    } else if ((ucIn >= 10) && (ucIn <= 15)) {
        return (ucIn - 10 + 'a');
    } else {
        return (-1);                                                    /*  NOP                         */
    }
}
/*********************************************************************************************************
** 函数名称: spcStringToHex
** 功能描述: 把 hash 字符串转换成二进制数据格式（"0123456789abcdef" --- 0x0123456789abcdef）
** 输　入  : cString    ：字符串地址
**         : iLen       ：字符串长度
**         : ucOutData  ：转换结果
** 输　出  : 成功转换的字符个数
** 补充信息: 需要注意输入字符串字符个数是奇数的情况以及 ucOutData 在调用的时候需要清空一下
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  spcStringToHex (UCHAR  *ucOutData, CHAR  *cString, INT  iLen)
{
    CHAR  *cData = cString;
    INT    iInIndex  = 0;
    INT    iOutIndex = 0;

    for (iInIndex = 0; iInIndex < iLen; iInIndex++) {
        if((iInIndex % 2) == 0) {
            /*
             * 奇数，放一个字节的高四位
             */
            *(ucOutData + iOutIndex) |= (spcCharToHex(*(cData + iInIndex)) << 4);
        } else {
            /*
             * 偶数，放一个字节的低四位
             */
            *(ucOutData + iOutIndex) |= (spcCharToHex(*(cData + iInIndex)));
            iOutIndex++;
        }
    }

    return (iInIndex);
}
/*********************************************************************************************************
** 函数名称: spcHexToString
** 功能描述: 把二进制数据格式转换成 hash 字符串（0x0123456789abcdef --- "0123456789abcdef"）
** 输　入  : cString     ：二进制数据地址
**         : iLen        ：有效的字节个数
**         : bIs         ：ucInData 最后一个字节数据是否只有一半有效（输出字符个数是否为奇数个）
**         : cOutString  ：转换结果
** 输　出  : 字符串长度
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  spcHexToString (CHAR  *cOutString, UCHAR  *ucInData, INT  iLen, BOOL bIs)
{
    UCHAR  *ucData    = ucInData;
    INT     iInIndex  = 0;
    INT     iOutIndex = 0;

    for (iInIndex = 0; iInIndex < iLen; iInIndex++) {
        *(cOutString + iOutIndex) = spcHexToChar(((*(ucData + iInIndex)) & 0xF0) >> 4);
        iOutIndex++;
        *(cOutString + iOutIndex) = spcHexToChar((*(ucData + iInIndex)) & 0x0F);
        iOutIndex++;
    }

    if (bIs) {
        iOutIndex--;
    }
    *(cOutString + iOutIndex) = '\0';

    return (iOutIndex);
}
/*********************************************************************************************************
** 函数名称: spcRcvDataToCmdString
** 功能描述: 把接收到的压缩数据转换成签名时执行的字符串命令
** 输　入  : ucDest：存储结果的缓冲区
**         : ucSrc ：接收的压缩数据
**         : iLen  ：ucDest 缓冲区长度
** 输　出  : ERROR_CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  spcRcvDataToCmdString (CHAR  *ucDest, INT  iLen, UCHAR  *ucSrc)
{
    INT               ret;
    INT               iIndex     = 0;
    INT               iTxidIndex = 2;
    ProtocolDataHead *pData      = (ProtocolDataHead *)ucSrc;

    memset(ucDest, '\0', iLen);

    if (pData->ucStartFlag == 0xba) {                                   /*  交易编码数据                */
        memcpy(ucDest + iIndex, SIGCMDSTR, strlen(SIGCMDSTR));
        iIndex += strlen(SIGCMDSTR);
        ret = spcHexToString(ucDest + iIndex, &pData->ucData[0],
                            pData->usDataLong & 0x7FFF, pData->usDataLong & 0x8000);
        iIndex += ret;
        pData = (ProtocolDataHead *)((UCHAR *)pData + (3 + (pData->usDataLong & 0x7FFF)));
    }

    if (pData->ucStartFlag == 0xca) {                                   /*  txid 数组数据               */
        UCHAR     i;
        INT       iFd;
        ssize_t   ssReadNum;
        CHAR      cPrivKeyIndex[6 + strlen(PRIVKEYFILEDIC)] = {0};

        iTxidIndex = 1;
        memcpy(ucDest + iIndex, STARTSTR, strlen(STARTSTR));
        iIndex += strlen(STARTSTR);

        for (i = 0; i < pData->ucData[0]; i++) {
            /*
             * TXID
             */
            memcpy(ucDest + iIndex, TXIDSTR, strlen(TXIDSTR));
            iIndex += strlen(TXIDSTR);
            ret = spcHexToString(ucDest + iIndex, &pData->ucData[iTxidIndex + 1],
                              pData->ucData[iTxidIndex] & 0x7F, pData->ucData[iTxidIndex] & 0x80);
            iIndex += ret;
            iTxidIndex += (1 + (pData->ucData[iTxidIndex]  & 0x7F));

            /*
             * VOUT
             */
            memcpy(ucDest + iIndex, VOUTSTR, strlen(VOUTSTR));
            iIndex += strlen(VOUTSTR);

            sprintf(ucDest + iIndex, "%d", pData->ucData[iTxidIndex]);
            do {
                iIndex += 1;
                pData->ucData[iTxidIndex] /= 10;
            } while (pData->ucData[iTxidIndex] > 0);
            iTxidIndex += 1;

            /*
             * scriptPubKey
             */
            memcpy(ucDest + iIndex, PUBKEYSTR, strlen(PUBKEYSTR));
            iIndex += strlen(PUBKEYSTR);
            ret = spcHexToString(ucDest + iIndex, &pData->ucData[iTxidIndex + 2],
                              (pData->ucData[iTxidIndex] | (pData->ucData[iTxidIndex + 1] << 8)) & 0x7FFF,
                              pData->ucData[iTxidIndex + 1] & 0x80);
            iIndex += ret;
            iTxidIndex += (2 + ((pData->ucData[iTxidIndex] | (pData->ucData[iTxidIndex + 1] << 8)) & 0x7FFF));

            /*
             * redeemScript
             */
            memcpy(ucDest + iIndex, REDEEMSCRIPTSTR, strlen(REDEEMSCRIPTSTR));
            iIndex += strlen(REDEEMSCRIPTSTR);
            ret = spcHexToString(ucDest + iIndex, &pData->ucData[iTxidIndex + 2],
                              (pData->ucData[iTxidIndex] | (pData->ucData[iTxidIndex + 1] << 8)) & 0x7FFF,
                              pData->ucData[iTxidIndex + 1] & 0x80);
            iIndex += ret;
            iTxidIndex += (2 + ((pData->ucData[iTxidIndex] | (pData->ucData[iTxidIndex + 1] << 8)) & 0x7FFF));

            /*
             * ENDENDSTR
             */
            memcpy(ucDest + iIndex, ENDSTR, strlen(ENDSTR));
            iIndex += strlen(ENDSTR);
        }

        iIndex -= 1;                                                    /*  去掉 ENDSTR 中的 ','        */
        memcpy(ucDest + iIndex, ENDENDSTR, strlen(ENDENDSTR));
        iIndex += strlen(ENDENDSTR);

        memcpy(ucDest + iIndex, PRIVKEYSTARTSTR, strlen(PRIVKEYSTARTSTR));
        iIndex += strlen(PRIVKEYSTARTSTR);

        /*
         * 读取索引值，然后到文件中找到相应的秘钥，拼接到命令中
         */
        memcpy(cPrivKeyIndex, PRIVKEYFILEDIC, strlen(PRIVKEYFILEDIC));
        sprintf(cPrivKeyIndex + strlen(PRIVKEYFILEDIC), "%d", pData->ucData[iTxidIndex] |
                                                             (pData->ucData[iTxidIndex + 1] << 8));
        iFd = open(cPrivKeyIndex, O_RDONLY, 0666);
        if (iFd < 0) {
            printf("failed to open %s!\n", cPrivKeyIndex);
            return (-1);
        }

        lseek(iFd, 0, SEEK_SET);
        ssReadNum = read(iFd, ucDest + iIndex, PRIVKEYLEN);
        if (ssReadNum != PRIVKEYLEN) {
            printf("read private key %s file failed\n", cPrivKeyIndex);
            return (-1);
        }
        close(iFd);

        iIndex     += PRIVKEYLEN;
        iTxidIndex += 2;
        memcpy(ucDest + iIndex, PRIVKEYENDSTR, strlen(PRIVKEYENDSTR));
    }

    return ((pData->ucData[iTxidIndex] == 0x1a) ? 0 : -1);
}
/*********************************************************************************************************
** 函数名称: spcProcessingJsonData
** 功能描述: 处理 json 数据
** 输　入  : cJsonStr ：json 字符串数据
**         : sOutLen  ：ucOutData 缓冲区长度
**         : sLen     ：cJsonStr 字符串长度
** 输　出  : ucOutData：压缩后的 hex 字段数据
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  spcProcessingJsonData (UCHAR  *ucOutData, size_t  sOutLen, CHAR  *cJsonStr, size_t  stLen)
{
    UINT16          sHexLong;
    INT             sRet;
    INT             iIndex;
    std::string     sHex;
    std::string     sComplete;
    std::string     strJson     = cJsonStr;
    Json::Value     tempVal;
    Json::Reader   *pJsonParser = new Json::Reader(Json::Features::strictMode());

    if(!pJsonParser->parse(strJson, tempVal))
    {
        return (-1);
    }

    sHex = tempVal["hex"].asString();
    sComplete = tempVal["complete"].asString();

    if (!memcmp(sComplete.c_str(), "true", strlen("true"))) {
        sRet = spcStringToHex (ucOutData + 3, (CHAR *)sHex.c_str(), sHex.length());
    } else {
        if (stLen > sOutLen) {
            stLen = sOutLen;
        }
        memcpy(ucOutData, cJsonStr, stLen);
        spcStringStrip((CHAR *)ucOutData, sOutLen);                     /*  签名失败，格式化数据并返回  */
        return (-1);
    }

    iIndex = 0;
    ucOutData[iIndex++] = 0xc1;

    sHexLong = (sRet / 2) + (sRet % 2);
    if (sRet % 2) {
        sHexLong |= 0x8000;
    }

    memcpy(&ucOutData[iIndex], &sHexLong, sizeof(sHexLong));
    iIndex = iIndex + 2 + (sHexLong & 0x7FFF);

    ucOutData[iIndex] = 0x1c;

    delete pJsonParser;

    return (1);
}
/*********************************************************************************************************
** 函数名称: spcImportPrivKey
** 功能描述: 导入冷钱包私钥
** 输　入  : cPrivKey  ：私钥字符串地址
**         : ucPrivNum ：私钥个数
** 输　出  : 操作结果字符串长度（含'\0'），结果存储在了 _G_ucAllSendBuf 中
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  spcImportPrivKey (CHAR  *cPrivKey, UCHAR  ucPrivNum)
{
    UCHAR     ucIndex;
    CHAR      cKey[IMPORTPRIVKEYLEN + PRIVKEYLEN + 1];
    FILE     *sigResult;
    size_t    sSigDataNum = 0;

    cKey[IMPORTPRIVKEYLEN + PRIVKEYLEN] = '\0';
    memcpy(cKey, IMPORTPRIVKEYSTR, IMPORTPRIVKEYLEN);

    for (ucIndex = 0; ucIndex < ucPrivNum; ucIndex++) {
        memcpy(cKey + IMPORTPRIVKEYLEN, cPrivKey + (ucIndex * PRIVKEYLEN), PRIVKEYLEN);

        printf("\n%s\n", cKey);

        sigResult = popen((const CHAR *)cKey, "r");
        if(sigResult == NULL){
            return (-1);
        }

        if ((sizeof(_G_ucAllSendBuf) - sSigDataNum) > 0) {
            sSigDataNum += fread(_G_ucAllSendBuf + sSigDataNum, sizeof(CHAR),
                                sizeof(_G_ucAllSendBuf) - sSigDataNum, sigResult);
                                                                        /*  导入私钥操作结果预留 100B   */
        }

        pclose(sigResult);
//        sleep(1);                                                       /*  每导入一个密钥等待1秒       */
    }

    return (sSigDataNum + ucIndex);
}
/*********************************************************************************************************
** 函数名称: spcSlaveProcessRcvData
** 功能描述: 从机处理从主机接收到的数据
** 输　入  : canframe：一个 can 帧数据
** 输　出  :
** 全局变量:
** 调用模块:
*********************************************************************************************************/
void  spcSlaveProcessRcvData(INT  iFd, CAN_FRAME  canframe)
{
    if ((canframe.CAN_uiId == 0x30) && (canframe.CAN_ucLen == 1) && (canframe.CAN_ucData[0] == 0x1A)) {
        /*
         * 请求正式遥测数据验证通过，返回遥测数据
         */
        printf("request Telemetry Data\n");

        spcMasterRequestTelemetryData(iFd);

    } else if ((canframe.CAN_uiId == 0x30) && (canframe.CAN_ucLen == 1) && (canframe.CAN_ucData[0] == 0x1B)) {
        /*
         * 请求测试遥测数据验证通过，返回遥测数据
         */
        printf("id = 0x%x, len = %d, data = %02x\n", canframe.CAN_uiId, canframe.CAN_ucLen, canframe.CAN_ucData[0]);

        spcMasterRequestTestData(iFd);

    } else if ((canframe.CAN_uiId == 0x24) && (canframe.CAN_ucLen == 8)
            && (memcmp(canframe.CAN_ucData, _G_ucData, CAN_MAX_DATA) == 0)) {
        /*
         * 收到复位 can 控制器命令
         */
        if (ioctl(iFd, CAN_DEV_REST_CONTROLLER, NULL)) {
            printf("Restart CAN Controler failed.\n");
            return;
        }

        _G_uiAllSendDataIndex    = 0;
        _G_uiAllRcvDataIndex     = 0;

        printf("Reset the can's controller Success\n");

    } else if ((canframe.CAN_uiId == 0x18) && (canframe.CAN_ucLen == 6)) {
        /*
         * 收到同步时间数据帧
         */
        uint32_t            uiTime;
        struct timespec     tv;

        uiTime = (time_t)be32toh(*((uint32_t *)&canframe.CAN_ucData[0]));

        /*
         * 当前时间 = 2018 年 1 月 1 日 0 时 0 分 0 秒（1514736000） + 上位机发送的时间秒数
         */
        tv.tv_sec  = (time_t)(1514736000) + uiTime;
        tv.tv_nsec = 0;

        lib_clock_settime(CLOCK_REALTIME, &tv);
        API_SysToRtc();

    } else if (canframe.CAN_uiId == 0x2C) {
        /*
         * 单帧注入数据，直接打印
         */
        printf("id = 0x%x, len = %d, data = %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x\n",
                canframe.CAN_uiId,
                canframe.CAN_ucLen,
                canframe.CAN_ucData[0],
                canframe.CAN_ucData[1],
                canframe.CAN_ucData[2],
                canframe.CAN_ucData[3],
                canframe.CAN_ucData[4],
                canframe.CAN_ucData[5],
                canframe.CAN_ucData[6],
                canframe.CAN_ucData[7]);

        /*
         * 给 OBC 返回载荷板一共接收到的数据字节数
         */
        spcSlaveReturnRcvDataNum(iFd, canframe.CAN_ucLen);

    } else if ((canframe.CAN_uiId == 0x2D) || (canframe.CAN_uiId == 0x2E) || (canframe.CAN_uiId == 0x2F)) {
        /*
         * 多帧注入数据，每次接收 50 字节，1个首帧(5B) + 6个中间帧(7B) + 1个尾帧(3B)
         */
        static UCHAR ucRcvNum = 0;
        static UCHAR ucIndex  = 0;

        if (canframe.CAN_uiId == 0x2D) {                                /*  收到首帧数据                */
            ucRcvNum = 0;
            ucIndex  = 0;

            memset(_G_ucRcvBuf, 0, RCVBUFLEN);
            memcpy(_G_ucRcvBuf + ucIndex, &canframe.CAN_ucData[3], 5);

            ucIndex  += 5;
            ucRcvNum += (canframe.CAN_ucLen - 3);                       /*  累加接收数据字节计数值      */

        } else if (canframe.CAN_uiId == 0x2E) {

            if (ucIndex > (RCVBUFLEN - 7)) {                            /*  收到中间帧数据              */
                return;
            }

            memcpy(_G_ucRcvBuf + ucIndex, &canframe.CAN_ucData[1], 7);

            ucIndex  += 7;
            ucRcvNum += (canframe.CAN_ucLen - 1);                       /*  累加接收数据字节计数值      */

        } else if (canframe.CAN_uiId == 0x2F) {
            /*
             * 收到尾帧数据
             */
            if (ucIndex > (RCVBUFLEN - 3)) {
                return;
            }

            memcpy(_G_ucRcvBuf + ucIndex, &canframe.CAN_ucData[1], 3);

            ucIndex  += 3;
            ucRcvNum += (canframe.CAN_ucLen - 1);                       /*  累加接收数据字节计数值      */

            /*
             * 向主机发送已经接收到的数据字节数
             */
            spcSlaveReturnRcvDataNum(iFd, ucRcvNum);

            memcpy(_G_ucAllRcvBuf + _G_uiAllRcvDataIndex, _G_ucRcvBuf, RCVBUFLEN);
            _G_uiAllRcvDataIndex += RCVBUFLEN;                          /*  50B 缓冲区到 500B 缓冲区    */

            if (_G_uiAllRcvDataIndex >= ALLRCVBUFLEN) {                 /*  接受数据达到了 500 字节     */
                ProtocolDataHead *data = (ProtocolDataHead *)_G_ucAllRcvBuf;

                _G_uiAllRcvDataIndex = 0;

                if ((data->ucStartFlag == 0xa1) && (data->ucData[data->usDataLong - 3] == 0x1a)) {
                                                                        /*  数据合法计算并保存签名结果  */
                    FILE           *sigResult;
                    FILE           *logSave;
                    void           *cSigDataBuf;
                    size_t          sSigDataNum;

                    spcRcvDataToCmdString((CHAR *)_G_ucAllSendBuf, ALLSENDBUFLEN, data->ucData);
                                                                        /*  处理接收到的500字节         */
                    sigResult = popen((const CHAR *)_G_ucAllSendBuf, "r");
                    if(sigResult == NULL){
                        return;
                    }

                    memset(_G_ucAllSendBuf, '\0', ALLSENDBUFLEN);
                    sSigDataNum = fread(_G_ucAllSendBuf, sizeof(CHAR), sizeof(_G_ucAllSendBuf), sigResult);
                                                                        /*  获取json格式的操作结果      */
                    pclose(sigResult);

                    cSigDataBuf = malloc(sSigDataNum);
                    memcpy(cSigDataBuf, _G_ucAllSendBuf, sSigDataNum);
                    memset(_G_ucAllSendBuf, '\0', ALLSENDBUFLEN);
                    spcProcessingJsonData(_G_ucAllSendBuf, ALLSENDBUFLEN, (CHAR *)cSigDataBuf, sSigDataNum);
                                                                        /*  处理json数据并生成压缩数据  */
                    free(cSigDataBuf);

                    {
                        INT   iIndex;

                        printf("\n");
                        for (iIndex = 0; iIndex < ALLSENDBUFLEN; iIndex++) {
                            printf("%02x", _G_ucAllSendBuf[iIndex]);
                        }
                        printf("\n");
                    }

                    logSave = fopen( "/root/spc_log.txt", "a");         /*  记录执行的每次命令到文件    */
                    if(logSave == NULL){
                        printf("open /root/spc_log.txt file failed\n");
                           return;
                    }

                    fwrite(&data->ucData[data->usDataLong], 1, sizeof(data->ucData[data->usDataLong]), logSave);
                    fwrite(_G_ucAllSendBuf, 1, sizeof(_G_ucAllSendBuf), logSave);

                    fclose(logSave);

                } else if ((data->ucStartFlag == 0xb1) && (data->ucData[data->usDataLong] == 0x1b)) {

                    if ((data->ucData[0] * PRIVKEYLEN) == (data->usDataLong - 1)) {
                                                                        /*  数据合法，执行私钥导入操作  */
                        spcImportPrivKey((CHAR *)&data->ucData[1], data->ucData[0]);
                    } else {
                                                                        /*  NOP                         */
                    }

                } else {
                                                                        /*  NOP                         */
                }
            }

            {
                /*
                 * 打印接收到的数据
                 */
                INT  i;
                printf("\n");
                for (i = 0; i < RCVBUFLEN; i++) {
                    printf("%02x", _G_ucRcvBuf[i]);
                }
                printf("\n");
            }



        } else {                                                        /*  NOP                         */

        }
    } else {
        printf("The data is error\n");
    }

    return ;
}
/*********************************************************************************************************
** 函数名称: spcPthreadRecv
** 功能描述: CAN 驱动测试 pthread 接收线程
** 输　入  : pvArg         线程参数
** 输　出  : 线程返回值
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  *spcPthreadRecv(VOID  *pvArg)
{
    INT         iFd;
    CAN_FRAME   canframe;
    ssize_t     stLen;
    ssize_t     stFrameNum;
    CHAR       *devname = (CHAR *)pvArg;

    memset(&canframe, 0, sizeof(CAN_FRAME));

    iFd = open(devname, O_RDWR, 0666);
    if (iFd < 0) {
        printf("failed to open %s!\n", devname);
        return  (LW_NULL);
    }
    printf("Successed to open %s!\n", devname);

    if (ioctl(iFd, CAN_DEV_SET_BAUD, CANBUAD)) {
        printf("set baud to %d error.\n", CANBUAD);
        close (iFd);
        return  (LW_NULL);
    }

    printf("start %s recv test with baud %d.\n", devname, CANBUAD);

    while (1) {
        stLen = read(iFd, &canframe, sizeof(CAN_FRAME));
        stFrameNum = stLen / sizeof(CAN_FRAME);

        if (stFrameNum != 1) {                                          /*  接收数据失败                */
            printf("failed to recv can frame, abort recving!\n");
            continue;
        } else {                                                        /*  接收数据成功，处理接收数据  */
            spcSlaveProcessRcvData(iFd, canframe);
        }
    }

    close(iFd);

    return  (NULL);
}
/*********************************************************************************************************
** 函数名称: spcSlaveProtocolRecvStart
** 功能描述: CAN 测试
** 输　入  : NONE
** 输　出  : ERROR_CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  spcSlaveProtocolRecvStart (char  *pcdevname)
{
    pthread_t   tid_recv;
    INT         iError = ERROR_NONE;

    iError = pthread_create(&tid_recv, NULL, spcPthreadRecv, pcdevname);
    pthread_join(tid_recv, NULL);

    return  (iError == 0 ? ERROR_NONE : PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: spcRestartNum
** 功能描述: 处理设备重启次数
** 输　入  : NONE
** 输　出  : ERROR_CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  spcRestartNum (VOID)
{
    INT      iFramFd;

    iFramFd = open("/dev/fram", O_RDWR, 0666);
    if (iFramFd < 0) {
        printf("open /dev/fram fail\n");
    }

    ioctl(iFramFd, 0, 0);
    read(iFramFd, &_G_usRestartCnt, 4);

    _G_usRestartCnt++;

    write(iFramFd, &_G_usRestartCnt, 4);
    close(iFramFd);

    return (0);
}
/*********************************************************************************************************
** 函数名称: main
** 功能描述: 主函数
** 输　入  : argc：命令行参数个数
**           argv：命令行参数首地址
** 输　出  :
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  main (INT  argc, CHAR  *argv[])
{
    spcRestartNum();
    spcSlaveProtocolRecvStart((CHAR *)CANDEV);

    return  (0);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
