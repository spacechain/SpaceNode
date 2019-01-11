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
# 文   件   名: libbitcoin_util.mk
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
LOCAL_TARGET_NAME := libbitcoin_util.so

#*********************************************************************************************************
# Source list
#*********************************************************************************************************
LOCAL_SRCS := \
./src/support/lockedpool.cpp  \
./src/chainparamsbase.cpp  \
./src/clientversion.cpp  \
./src/compat/glibc_sanity.cpp  \
./src/compat/glibcxx_sanity.cpp  \
./src/compat/strnlen.cpp  \
./src/fs.cpp  \
./src/random.cpp  \
./src/rpc/protocol.cpp  \
./src/support/cleanse.cpp  \
./src/sync.cpp  \
./src/threadinterrupt.cpp  \
./src/util.cpp  \
./src/utilmoneystr.cpp  \
./src/utilstrencodings.cpp  \
./src/utiltime.cpp  \
./src/compat/glibc_compat.cpp

#*********************************************************************************************************
# Header file search path (eg. LOCAL_INC_PATH := -I"Your header files search path")
#*********************************************************************************************************
LOCAL_INC_PATH := \
-I"./src/config"  \
-I"./src" \
-I"./include" \
-I"./src/leveldb/include" \
-I"./src/leveldb/helpers/memenv" \
-I"./src/cpp-ethereum/utils" \
-I"./src/secp256k1/include" \
-I"./src/univalue/include" \
-I"./src/cpp-ethereum" \
-I"./src/include" \
-I"./src/compat" \
-I"./src/cpp-ethereum" \
-I"$(SYLIXOS_BASE_PATH)/openssl/openssl/include" \
-I"../libboost/src"


#*********************************************************************************************************
# Pre-defined macro (eg. -DYOUR_MARCO=1)
#*********************************************************************************************************
LOCAL_DSYMBOL := -DHAVE_CONFIG_H -DQTUM_BUILD -DHAVE_BUILD_INFO -D__STDC_FORMAT_MACROS \
				   -D_FILE_OFFSET_BITS=64 -DETH_FATDB=1 -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 \
				   -DQTUM_BUILD -DBOOST_SP_USE_STD_ATOMIC -DBOOST_AC_USE_STD_ATOMIC 

#*********************************************************************************************************
# Compiler flags
#*********************************************************************************************************
LOCAL_CFLAGS   := #-pipe -O2 
LOCAL_CXXFLAGS := -std=c++11 -lpthread #-fvisibility=hidden #-pipe -O2  -fPIE

#*********************************************************************************************************
# Depend library (eg. LOCAL_DEPEND_LIB := -la LOCAL_DEPEND_LIB_PATH := -L"Your library search path")
#*********************************************************************************************************
LOCAL_DEPEND_LIB      := \
-lboost_filesystem-mt \
-lboost_system-mt \
-lboost_thread-mt \
-lboost_program_options-mt \
-lunivalue \
-lcextern \
-lssl

LOCAL_DEPEND_LIB_PATH :=  \
-L"./$(Output)" \
-L"$(SYLIXOS_BASE_PATH)/openssl/$(Output)" \
-L"../libboost/$(Output)"

#*********************************************************************************************************
# Depend target
#*********************************************************************************************************
LOCAL_DEPEND_TARGET := ../libboost/$(Output)/libboost_filesystem.so
LOCAL_DEPEND_TARGET += ../libboost/$(Output)/libboost_system.so
LOCAL_DEPEND_TARGET += ../libboost/$(Output)/libboost_thread-mt-sd.so
LOCAL_DEPEND_TARGET += ../libboost/$(Output)/libboost_program_options-d.so
LOCAL_DEPEND_TARGET += $(SYLIXOS_BASE_PATH)/openssl/$(Output)/libssl.so
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

include $(LIBRARY_MK)

#*********************************************************************************************************
# End
#*********************************************************************************************************
