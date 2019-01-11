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
** 文   件   名: spc_master.c
**
** 创   建   人: Zhang.ZhaoGe (张兆阁)
**
** 文件创建日期: 2018 年 07 月 15 日
**
** 描        述: spc 主机端代码 (模拟了 OBC 端功能)
*********************************************************************************************************/
#include "SylixOS.h"
#include "pthread.h"
#include "stdlib.h"
#include "string.h"
#include "getopt.h"
#include <stdio.h>
#include <endian.h>
#include "../SylixOS/system/device/can/can.h"
/*********************************************************************************************************
  宏定义
*********************************************************************************************************/
#define SENDBUFLEN                      50                              /*  发送数据缓冲区打小          */
#define RCVBUFLEN                       198                             /*  接收数据缓冲区打小          */
#define CANBUAD                         500000                          /*  can 波特率                  */
#define CANDEV                          "/dev/can0"                     /*  can 设备文件                */
/*********************************************************************************************************
  数据类型声明
*********************************************************************************************************/
typedef struct spc_send_data {
    CHAR   *pcTransaction;
    UCHAR   ucTxidArrayNum;
    CHAR   *pcTxid[5];
    UCHAR   ucVout[5];
    CHAR   *pcScriptPubKey[5];
    CHAR   *pcRedeemScript[5];
    UINT16  uiPrivKeyIndex;
} SPC_SEND_DATA_T;
/*********************************************************************************************************
  全局变量定义
*********************************************************************************************************/
static UCHAR _G_ucSendBuf[SENDBUFLEN];                                  /*  发送数据缓冲区              */
static UCHAR _G_ucRcvBuf[RCVBUFLEN];                                    /*  接收数据缓冲区              */
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
** 补充信息: 需要注意输入字符串字符个数是奇数的情况
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
            // 奇数，放一个字节的高四位
            *(ucOutData + iOutIndex) |= (spcCharToHex(*(cData + iInIndex)) << 4);
        } else {
            // 偶数，放一个字节的低四位
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
    UCHAR  *ucData = ucInData;
    INT     iInIndex = 0;
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
** 函数名称: spcMasterSendToSlave
** 功能描述: 主机向从机发送注入数据
** 输　入  : iFd：     can 总线设备文件
**           pucBuf：  发送数据缓冲区
**           iDataLen：发送数据长度，最大50
** 输　出  :
** 附加信息: 每次发送 50 字节，1个首帧(5B) + 6个中间帧(7B) + 1个尾帧(3B)
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  spcMasterSendToSlave (INT  iFd, UCHAR  *pucBuf, INT  iDataLen)
{
    CAN_FRAME   canframe;
    ssize_t     stLen;
    ssize_t     stFrameNum;
    INT         i;
    UCHAR       ucIndex = 0;

    /*
     * 复制数据到本地缓冲区中
     */
    if (iDataLen > SENDBUFLEN) {
        printf("data too long\n");
        return;
    }
    memset(_G_ucSendBuf, 0, SENDBUFLEN);
    memcpy(_G_ucSendBuf, pucBuf, iDataLen);

    /*
     * 首帧数据打包并发送
     */
    canframe.CAN_bRtr       = 0;                                        /*  非远程帧                    */
    canframe.CAN_bExtId     = 0;                                        /*  非扩展帧                    */
    canframe.CAN_uiId       = 0x2D;
    canframe.CAN_ucLen      = 8;

    canframe.CAN_ucData[0]  = 0x01;
    canframe.CAN_ucData[1]  = 0x08;
    canframe.CAN_ucData[2]  = 0x5A;

    memcpy(&canframe.CAN_ucData[3], _G_ucSendBuf + ucIndex, 5);
    ucIndex += 5;

    stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
    ioctl(iFd, CAN_DEV_STARTUP, 0);

    stFrameNum = stLen / sizeof(CAN_FRAME);

    if (stFrameNum != 1) {
        printf("failed to send can frame!, abort sending\n");
        return;
    }

    /*
     * 中间帧数据打包并发送
     */
    for (i = 0; i < 6; i++) {
        canframe.CAN_uiId       = 0x2E;
        canframe.CAN_ucLen      = 8;

        canframe.CAN_ucData[0]  = i + 2;

        if (ucIndex > (SENDBUFLEN - 7)) {
            return;
        }
        memcpy(&canframe.CAN_ucData[1], _G_ucSendBuf + ucIndex, 7);
        ucIndex += 7;

        stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
        ioctl(iFd, CAN_DEV_STARTUP, 0);

        stFrameNum = stLen / sizeof(CAN_FRAME);

        if (stFrameNum != 1) {
            printf("failed to send can frame!, abort sending\n");
            return;
        }
    }

    /*
     * 尾帧数据打包并发送
     */
    canframe.CAN_uiId       = 0x2F;
    canframe.CAN_ucLen      = 4;
    canframe.CAN_ucData[0]  = 0x08;                                     /*  包序号                      */

    if (ucIndex > (SENDBUFLEN - 3)) {
        return;
    }

    memcpy(&canframe.CAN_ucData[1], _G_ucSendBuf + ucIndex, 3);
    ucIndex += 3;

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
** 功能描述: 主机向从机发送请求遥测数据命令
** 输　入  : iFd：can 总线设备文件
** 输　出  :
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  spcMasterRequestTelemetryData (INT  iFd)
{
    CAN_FRAME   canframe;
    ssize_t     stLen;
    ssize_t     stFrameNum;

    canframe.CAN_bRtr       = 0;                                        /*  非远程帧                    */
    canframe.CAN_bExtId     = 0;                                        /*  非扩展帧                    */
    canframe.CAN_uiId       = 0x30;
    canframe.CAN_ucLen      = 1;
    canframe.CAN_ucData[0]  = 0x1A;

    stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
    ioctl(iFd, CAN_DEV_STARTUP, 0);

    stFrameNum = stLen / sizeof(CAN_FRAME);

    if (stFrameNum != 1) {
        printf("failed to send can frame!, abort sending\n");
        return;
    }

    return ;
}
/*********************************************************************************************************
** 函数名称: spcMasterRequestTestTelemetryData
** 功能描述: 主机向从机发送请求测试遥测数据命令
** 输　入  : iFd：can 总线设备文件
** 输　出  :
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  spcMasterRequestTestTelemetryData (INT  iFd)
{
    CAN_FRAME   canframe;
    ssize_t     stLen;
    ssize_t     stFrameNum;

    canframe.CAN_bRtr       = 0;                                        /*  非远程帧                    */
    canframe.CAN_bExtId     = 0;                                        /*  非扩展帧                    */
    canframe.CAN_uiId       = 0x30;
    canframe.CAN_ucLen      = 1;
    canframe.CAN_ucData[0]  = 0x1B;

    stLen = write(iFd, &canframe, sizeof(CAN_FRAME));
    ioctl(iFd, CAN_DEV_STARTUP, 0);

    stFrameNum = stLen / sizeof(CAN_FRAME);

    if (stFrameNum != 1) {
        printf("failed to send can frame!, abort sending\n");
        return;
    }

    return ;
}
/*********************************************************************************************************
** 函数名称: spcMasterBroadcastTimeData
** 功能描述: 主机广播时间数据
** 输　入  : iFd：can 总线设备文件
** 输　出  :
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID spcMasterBroadcastTimeData (INT iFd)
{
    struct timespec tv;
    time_t          tTime;
    CAN_FRAME       canframe;
    ssize_t         stLen;
    ssize_t         stFrameNum;

    if (lib_clock_gettime(CLOCK_REALTIME, &tv) < 0) {                   /*  获得系统时钟                */
        return;
    }

    /*
     * 2018 年 1 月 1 日 0 时 0 分 0 秒（1514736000）开始的秒数
     */
    tTime = tv.tv_sec - 1514736000;
    HTOBE32(tTime);

    canframe.CAN_bRtr       = 0;                                        /*  非远程帧                    */
    canframe.CAN_bExtId     = 0;                                        /*  非扩展帧                    */
    canframe.CAN_uiId       = 0x18;
    canframe.CAN_ucLen      = 6;

    memset(&canframe.CAN_ucData[0], 0, 6);
    memcpy(&canframe.CAN_ucData[0], (UCHAR *)&tTime, 4);

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
** 函数名称: spcPacketSendData
** 功能描述: 打包发送数据
** 输　入  : pSendData：需要发送的原始数据
** 输　出  : ucData   ：打包的数据结果
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  spcPacketSendData (UCHAR  *ucData, SPC_SEND_DATA_T  *pSendData)
{
    INT              i;
    INT              sRet;
    UINT16           usHexLong;
    UINT16           usIndex = 0;

    ucData[usIndex++] = 0xa1;
    usIndex += 2;

    ucData[usIndex++] = 0xba;

    sRet = spcStringToHex(ucData + usIndex + 2, pSendData->pcTransaction, strlen(pSendData->pcTransaction));

    usHexLong = (sRet / 2) + (sRet % 2);
    if (sRet % 2) {
        usHexLong |= 0x8000;
    }

    memcpy(ucData + usIndex, &usHexLong, 2);
    usIndex = usIndex + 2 + (usHexLong & 0x7FFF);

    ucData[usIndex++] = 0xca;
    ucData[usIndex++] = 0;
    ucData[usIndex++] = 0;
    ucData[usIndex++] = pSendData->ucTxidArrayNum;

    for (i = 0; i < pSendData->ucTxidArrayNum; i++) {

        sRet = spcStringToHex(ucData + usIndex + 1, pSendData->pcTxid[i], strlen(pSendData->pcTxid[i]));

        usHexLong = (sRet / 2) + (sRet % 2);
        if (sRet % 2) {
            usHexLong |= 0x80;
        }

        *(ucData + usIndex) = usHexLong & 0xFF;
        usIndex = usIndex + 1 + (usHexLong & 0x7F);

        ucData[usIndex++] = pSendData->ucVout[i];

        sRet = spcStringToHex(ucData + usIndex + 2, pSendData->pcScriptPubKey[i], strlen(pSendData->pcScriptPubKey[i]));

        usHexLong = (sRet / 2) + (sRet % 2);
        if (sRet % 2) {
            usHexLong |= 0x8000;
        }

        memcpy(ucData + usIndex, &usHexLong, 2);
        usIndex = usIndex + 2 + (usHexLong & 0x7FFF);

        sRet = spcStringToHex(ucData + usIndex + 2, pSendData->pcRedeemScript[i], strlen(pSendData->pcRedeemScript[i]));

        usHexLong = (sRet / 2) + (sRet % 2);
        if (sRet % 2) {
            usHexLong |= 0x8000;
        }

        memcpy(ucData + usIndex, &usHexLong, 2);
        usIndex = usIndex + 2 + (usHexLong & 0x7FFF);
    }

    memcpy(ucData + usIndex, &pSendData->uiPrivKeyIndex, 2);
    usIndex += 2;

    ucData[usIndex] = 0x1a;
    memcpy(ucData + 1, &usIndex, 2);

    return (0);
}
/*********************************************************************************************************
** 函数名称: pthreadTestSend
** 功能描述: CAN 驱动测试 pthread 发送线程
** 输　入  : pvArg         线程参数
** 输　出  : 线程返回值
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID *pthreadTestSend(VOID  *pvArg)
{
    INT       iFd;
    INT       iCnt    = 0;
    char     *devname = (char *)pvArg;

    iFd = open(devname, O_RDWR, 0666);
    if (iFd < 0) {
        printf("failed to open %s!\n", devname);
        return  (LW_NULL);
    }

    if (ioctl(iFd, CAN_DEV_SET_BAUD, CANBUAD)) {
        printf("set baud to %d error.\n", CANBUAD);
        close (iFd);
        return  (LW_NULL);
    }
    ioctl(iFd, FIOFLUSH, 0);


    /*
     * 主机请求测试遥测数据
     */
    spcMasterRequestTestTelemetryData(iFd);
    usleep(500000);

    /*
     * 同步主机时间到从机
     */
    spcMasterBroadcastTimeData (iFd);
    usleep(500000);

    {
        INT              i;
        CHAR            *cHexData         = "0200000001eadd31b0f532542b4fda3f276ef84248ecd04bbdc254a102317ae8445704818600000000b500483045022100cc400cfb26c884d0388227c23b9018e5f05995bc5541948d0f1943d092eb100902201f48932b64db8a885784b3c6dd7001fe5dfbd1703a42544a0b010af193704439014c69522103f622b4e144f901f6efc9ba3f57be34534155386574e5a04e388c0de22da554f12103b903caea072beb172ca4fc44c6b1e3ade6451dad90384778528fdcf8f3a0786e210315015bd030f0e08a3f6ae73e9beeb3634cb71239998d826520bdb7d33b6adcb453aeffffffff018033023b000000001976a9147aa23e65228b9ee440dd4d1b3f3c0710a18a179b88ac00000000";
        CHAR            *cTxidData        = "8681045744e87a3102a154c2bd4bd0ec4842f86e273fda4f2b5432f5b031ddea";
        CHAR            *cPubKey          = "a914647ef6664d6b36e236d000a234c7f26ee0e5014a87";
        CHAR            *cRedeemScryptKey = "522103f622b4e144f901f6efc9ba3f57be34534155386574e5a04e388c0de22da554f12103b903caea072beb172ca4fc44c6b1e3ade6451dad90384778528fdcf8f3a0786e210315015bd030f0e08a3f6ae73e9beeb3634cb71239998d826520bdb7d33b6adcb453ae";
        UCHAR            ucData[2048]     = {0};
        SPC_SEND_DATA_T  spcSendData;

        spcSendData.pcTransaction     = cHexData;
        spcSendData.ucTxidArrayNum    = 1;

        spcSendData.pcTxid[0]         = cTxidData;
        spcSendData.ucVout[0]         = 0;
        spcSendData.pcScriptPubKey[0] = cPubKey;
        spcSendData.pcRedeemScript[0] = cRedeemScryptKey;

        /*
         * spcSendData.pcTxid[1]         = cTxidData;
         * spcSendData.ucVout[1]         = 1;
         * spcSendData.pcScriptPubKey[1] = cPubKey;
         * spcSendData.pcRedeemScript[1] = cRedeemScryptKey;
         */

        spcSendData.uiPrivKeyIndex    = 0;

        spcPacketSendData (ucData, &spcSendData);

        for (i = 0; i < 10; i++) {
            spcMasterSendToSlave (iFd, &ucData[i * 50], 50);
            usleep(500000);
            printf("i = %d\n", i);
        }

    }

    sleep(3);                                                           /*  等待主机计算签名结果        */

    for (iCnt = 0; iCnt < 4; iCnt++) {                                  /*  主机请求从机遥测数据        */
        sleep(1);
        spcMasterRequestTelemetryData (iFd);
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

    close(iFd);

    return  (NULL);
}
/*********************************************************************************************************
** 函数名称: pthreadTestRecv
** 功能描述: CAN 驱动测试 pthread 接收线程
** 输　入  : pvArg        线程参数
** 输　出  : 线程返回值
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  *pthreadTestRecv(VOID  *pvArg)
{
    INT         iFd;
    CAN_FRAME   canframe;
    ssize_t     stLen;
    ssize_t     stFrameNum;
    char       *devname = (char *)pvArg;

    memset(&canframe, 0, sizeof(CAN_FRAME));

    iFd = open(devname, O_RDWR, 0666);
    if (iFd < 0) {
        printf("failed to open %s!\n", devname);
        return  (LW_NULL);
    }

    if (ioctl(iFd, CAN_DEV_SET_BAUD, CANBUAD)) {
        printf("set baud to %d error.\n", CANBUAD);
        close (iFd);
        return  (LW_NULL);
    }

    while(1) {
        /*
         * 开始接收数据
         */
        stLen = read(iFd, &canframe, sizeof(CAN_FRAME));
        stFrameNum = stLen / sizeof(CAN_FRAME);

        if (stFrameNum != 1) {
            /*
             * 接收数据失败
             */
            printf("failed to recv can frame, abort recving!\n");

        } else {
            if ((canframe.CAN_uiId == 0x431) || (canframe.CAN_uiId == 0x432)
               || (canframe.CAN_uiId == 0x433)) {
                /*
                 * 返回正常的遥测数据，返回多帧数据
                 * 每次 198 字节，1个首帧(5B) + 27个中间帧(7B) + 1个尾帧(4B)
                 */
                static UCHAR ucIndex = 0;
                static UCHAR ucIsTestData;

                if (canframe.CAN_uiId == 0x431) {
                    /*
                     * 收到首帧数据
                     */
                    ucIndex = 0;

                    if (canframe.CAN_ucData[2] == 0x1B) {
                        ucIsTestData = 1;
                    } else if (canframe.CAN_ucData[2] == 0x1A){
                        ucIsTestData = 0;
                    } else {
                                                                        /*  NOP                         */
                    }

                    memset(_G_ucRcvBuf, 0, RCVBUFLEN);
                    memcpy(_G_ucRcvBuf + ucIndex, &canframe.CAN_ucData[3], 5);
                    ucIndex += 5;

                } else if (canframe.CAN_uiId == 0x432) {
                    if (ucIndex > (RCVBUFLEN - 7)) {
                        return  (NULL);
                    }

                    memcpy(_G_ucRcvBuf + ucIndex, &canframe.CAN_ucData[1], 7);
                    ucIndex += 7;

                } else if (canframe.CAN_uiId == 0x433) {
                    /*
                     * 如果接收到了尾帧，则给 OBC 返回接收到的数据字节数
                     */
                    if (ucIndex > (RCVBUFLEN - 4)) {
                        return  (NULL);
                    }

                    if (ucIsTestData == 1) {
                        memcpy(_G_ucRcvBuf + ucIndex, &canframe.CAN_ucData[1], 3);
                        ucIndex += 3;
                    } else {
                        memcpy(_G_ucRcvBuf + ucIndex, &canframe.CAN_ucData[1], 4);
                        ucIndex += 4;
                    }

                    {
                        /*
                         * 打印接收到的数据
                         */
                        INT  i;
                        printf("\n");
                        for (i = 0; i < ucIndex; i++) {
                            printf("%02x", _G_ucRcvBuf[i]);
                        }
                        printf("\n");
                    }

                } else {

                }
            }
        }
    }

    close(iFd);

    return  (NULL);
}
/*********************************************************************************************************
** 函数名称: spcMasterProtocolTestStart
** 功能描述: CAN 测试
** 输　入  : NONE
** 输　出  : ERROR_CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  spcMasterProtocolTestStart (char *pcdevname)
{
    pthread_t   tid_recv;
    pthread_t   tid_send;
    INT         iError = ERROR_NONE;

    iError = pthread_create(&tid_send, NULL, pthreadTestSend, pcdevname);
    pthread_detach(tid_send);

    iError = pthread_create(&tid_recv, NULL, pthreadTestRecv, pcdevname);
    pthread_join(tid_recv, NULL);

    return  (iError == 0 ? ERROR_NONE : PX_ERROR);
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
int  main (int  argc, char  *argv[])
{
    spcMasterProtocolTestStart((CHAR *)CANDEV);

    return  (0);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
