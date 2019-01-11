#*********************************************************************************************************
#
#                                    中国软件开源组织
#
#                                   嵌入式实时操作系统
#
#                                SylixOS(TM)  LW : long wing
#
#                               Copyright All Rights Reserved
#
#--------------文件信息--------------------------------------------------------------------------------
#
# 文   件   名: qtum.mk
#
# 创   建   人: RealEvo-IDE
#
# 文件创建日期: 2018 年 06 月 27 日
#
# 描        述: 本文件由 RealEvo-IDE 生成，用于配置 Makefile 功能，请勿手动修改
#*********************************************************************************************************

#*********************************************************************************************************
# Clear setting
#*********************************************************************************************************
include $(CLEAR_VARS_MK)

#*********************************************************************************************************
# Target
#*********************************************************************************************************
LOCAL_TARGET_NAME := qtumd

#*********************************************************************************************************
# Source list
#*********************************************************************************************************
LOCAL_SRCS := \
./src/bitcoind.cpp

#*********************************************************************************************************
# Header file search path (eg. LOCAL_INC_PATH := -I"Your header files search path")
#*********************************************************************************************************
LOCAL_INC_PATH := \
-I"./src/include" \
-I"./src" \
-I"./src/config" \
-I"./src/compat" \
-I"./src/cpp-ethereum" \
-I"./src/univalue/include" \
-I"../libboost/src"

#*********************************************************************************************************
# Pre-defined macro (eg. -DYOUR_MARCO=1)
#*********************************************************************************************************
LOCAL_DSYMBOL := -DHAVE_CONFIG_H -DQTUM_BUILD -DHAVE_BUILD_INFO -D__STDC_FORMAT_MACROS \
				   -D_FILE_OFFSET_BITS=64 -DHAVE_BUILD_INFO -DETH_FATDB=1

#*********************************************************************************************************
# Compiler flags
#*********************************************************************************************************
LOCAL_CFLAGS   := #-pipe -O2 
LOCAL_CXXFLAGS := -std=c++11 -lpthread -static-libstdc++ #-fvisibility=hidden #-pipe -O2  \
					-fPIE -Wl,--exclude-libs,ALL  -Wl,-z,relro -Wl,-z,now -pie

#-Wstack-protector -fstack-protector-all
#*********************************************************************************************************
# Depend library (eg. LOCAL_DEPEND_LIB := -la LOCAL_DEPEND_LIB_PATH := -L"Your library search path")
#*********************************************************************************************************
LOCAL_DEPEND_LIB := \
-Wl,-Bstatic \
-lbitcoin_server \
-lbitcoin_common \
-lunivalue \
-lbitcoin_util \
-lbitcoin_consensus \
-lbitcoin_wallet \
-lbitcoin_crypto \
-lleveldb_sse42 \
-lleveldb \
-lmemenv \
-lcryptopp \
-lsecp256k1 \
-lboost_system-mt \
-lboost_filesystem-mt \
-lboost_program_options-mt \
-lboost_thread-mt \
-lboost_chrono-mt \
-lboost_random-mt \
-lbitcoin_util \
\
-lbitcoin_server \
-lbitcoin_common \
-lunivalue \
-lbitcoin_util \
-lbitcoin_consensus \
-lbitcoin_wallet \
-lbitcoin_crypto \
-lleveldb_sse42 \
-lleveldb \
-lmemenv \
-lcryptopp \
-lsecp256k1 \
-lbitcoin_util \
-lcextern \
-Wl,-Bdynamic \
-ldb \
-lssl \
-lcrypto \
-levent \
-levent_pthreads


#-Wl,-whole-archive   \
#-Wl,-no-whole-archive  \

LOCAL_DEPEND_LIB_PATH := \
-L"./src/config"  \
-L"./$(Output)" \
-L"../libevent/$(Output)" \
-L"$(SYLIXOS_BASE_PATH)/openssl/$(Output)" \
-L"../libboost/$(Output)" \
-L"../libdb/$(Output)"

#*********************************************************************************************************
# Depend target
#*********************************************************************************************************
LOCAL_DEPEND_TARGET := ./$(Output)/libsecp256k1.so
LOCAL_DEPEND_TARGET += ./$(Output)/libcryptopp.so
LOCAL_DEPEND_TARGET += ./$(Output)/libmemenv.so
LOCAL_DEPEND_TARGET += ./$(Output)/libleveldb.so
LOCAL_DEPEND_TARGET += ./$(Output)/libleveldb_sse42.so
LOCAL_DEPEND_TARGET += ./$(Output)/libbitcoin_server.so
LOCAL_DEPEND_TARGET += ./$(Output)/libbitcoin_util.so
LOCAL_DEPEND_TARGET += ./$(Output)/libbitcoin_consensus.so
LOCAL_DEPEND_TARGET += ./$(Output)/libbitcoin_wallet.so
LOCAL_DEPEND_TARGET += ./$(Output)/libbitcoin_crypto.so
LOCAL_DEPEND_TARGET += ./$(Output)/libunivalue.so

#*********************************************************************************************************
# C++ config
#*********************************************************************************************************
LOCAL_USE_CXX := yes
LOCAL_USE_CXX_EXCEPT := yes

#*********************************************************************************************************
# Code coverage config
#*********************************************************************************************************
LOCAL_USE_GCOV := no

#*********************************************************************************************************
# OpenMP config
#*********************************************************************************************************
LOCAL_USE_OMP := no

#*********************************************************************************************************
# User link command
#*********************************************************************************************************
LOCAL_PRE_LINK_CMD   := 
LOCAL_POST_LINK_CMD  := 
LOCAL_PRE_STRIP_CMD  := 
LOCAL_POST_STRIP_CMD := 

#*********************************************************************************************************
# Depend target
#*********************************************************************************************************
LOCAL_DEPEND_TARGET := 

include $(APPLICATION_MK)

#*********************************************************************************************************
# End
#*********************************************************************************************************
