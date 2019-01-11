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
# 文   件   名: libbitcoin_server.mk
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
LOCAL_TARGET_NAME := libbitcoin_server.so

#*********************************************************************************************************
# Source list
#*********************************************************************************************************
LOCAL_SRCS := \
./src/addrdb.cpp  \
./src/addrman.cpp  \
./src/bloom.cpp  \
./src/blockencodings.cpp  \
./src/chain.cpp  \
./src/checkpoints.cpp  \
./src/consensus/tx_verify.cpp  \
./src/httprpc.cpp  \
./src/httpserver.cpp  \
./src/init.cpp  \
./src/dbwrapper.cpp  \
./src/merkleblock.cpp  \
./src/miner.cpp  \
./src/net.cpp  \
./src/net_processing.cpp  \
./src/noui.cpp  \
./src/policy/fees.cpp  \
./src/policy/policy.cpp  \
./src/policy/rbf.cpp  \
./src/pow.cpp  \
./src/pos.cpp  \
./src/rest.cpp  \
./src/rpc/blockchain.cpp  \
./src/rpc/mining.cpp  \
./src/rpc/misc.cpp  \
./src/rpc/net.cpp  \
./src/rpc/rawtransaction.cpp  \
./src/rpc/server.cpp  \
./src/script/sigcache.cpp  \
./src/script/ismine.cpp  \
./src/timedata.cpp  \
./src/torcontrol.cpp  \
./src/txdb.cpp  \
./src/txmempool.cpp  \
./src/ui_interface.cpp  \
./src/validation.cpp  \
./src/validationinterface.cpp  \
./src/versionbits.cpp  \
./src/qtum/qtumstate.cpp  \
./src/qtum/qtumtransaction.cpp  \
./src/qtum/qtumDGP.cpp  \
./src/consensus/consensus.cpp  \
./src/qtum/storageresults.cpp

#*********************************************************************************************************
# Header file search path (eg. LOCAL_INC_PATH := -I"Your header files search path")
#*********************************************************************************************************
LOCAL_INC_PATH := \
-I"./src" \
-I"./include" \
-I"./src/config"  \
-I"./src/leveldb/include" \
-I"./src/leveldb/helpers/memenv" \
-I"./src/cpp-ethereum/utils" \
-I"./src/secp256k1/include" \
-I"./src/univalue/include" \
-I"./src/cpp-ethereum" \
-I"../libboost/src" \
-I"./src/compat" \
-I"../libdb/src/config" \
-I"../libevent/src/include" \
-I"$(SYLIXOS_BASE_PATH)/openssl/openssl/include" \
-I"../libboost/src" \
-I"./src/qtum/" \
-I"./src/include"

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
LOCAL_CXXFLAGS := -std=c++11 -lpthread #-fvisibility=hidden #-pipe -O2  -fPIE -fpermissive
#$(MINIUPNPC_CPPFLAGS)

#*********************************************************************************************************
# Depend library (eg. LOCAL_DEPEND_LIB := -la LOCAL_DEPEND_LIB_PATH := -L"Your library search path")
#*********************************************************************************************************
LOCAL_DEPEND_LIB      := \
-lbitcoin_consensus \
-lbitcoin_common \
-lcextern

LOCAL_DEPEND_LIB_PATH :=  \
-L"./$(Output)"

#*********************************************************************************************************
# Depend target
#*********************************************************************************************************
LOCAL_DEPEND_TARGET := ./$(Output)/libbitcoin_consensus.so
LOCAL_DEPEND_TARGET += ./$(Output)/libbitcoin_common.so

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
