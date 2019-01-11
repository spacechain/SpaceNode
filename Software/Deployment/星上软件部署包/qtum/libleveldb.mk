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
# 文   件   名: libleveldb.mk
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
LOCAL_TARGET_NAME := libleveldb.so

#*********************************************************************************************************
# Source list
#*********************************************************************************************************
LOCAL_SRCS := \
./src/leveldb/db/builder.cpp  \
./src/leveldb/db/c.cpp  \
./src/leveldb/db/dbformat.cpp  \
./src/leveldb/db/db_impl.cpp  \
./src/leveldb/db/db_iter.cpp  \
./src/leveldb/db/dumpfile.cpp  \
./src/leveldb/db/filename.cpp  \
./src/leveldb/db/log_reader.cpp  \
./src/leveldb/db/log_writer.cpp  \
./src/leveldb/db/memtable.cpp  \
./src/leveldb/db/repair.cpp  \
./src/leveldb/db/table_cache.cpp  \
./src/leveldb/db/version_edit.cpp  \
./src/leveldb/db/version_set.cpp  \
./src/leveldb/db/write_batch.cpp  \
./src/leveldb/table/block_builder.cpp  \
./src/leveldb/table/block.cpp  \
./src/leveldb/table/filter_block.cpp  \
./src/leveldb/table/format.cpp  \
./src/leveldb/table/iterator.cpp  \
./src/leveldb/table/merger.cpp  \
./src/leveldb/table/table_builder.cpp  \
./src/leveldb/table/table.cpp  \
./src/leveldb/table/two_level_iterator.cpp  \
./src/leveldb/util/arena.cpp  \
./src/leveldb/util/bloom.cpp  \
./src/leveldb/util/cache.cpp  \
./src/leveldb/util/coding.cpp  \
./src/leveldb/util/comparator.cpp  \
./src/leveldb/util/crc32c.cpp  \
./src/leveldb/util/env.cpp  \
./src/leveldb/util/env_posix.cpp  \
./src/leveldb/util/filter_policy.cpp  \
./src/leveldb/util/hash.cpp  \
./src/leveldb/util/histogram.cpp  \
./src/leveldb/util/logging.cpp  \
./src/leveldb/util/options.cpp  \
./src/leveldb/util/status.cpp  \
./src/leveldb/port/port_posix.cpp

#*********************************************************************************************************
# Header file search path (eg. LOCAL_INC_PATH := -I"Your header files search path")
#*********************************************************************************************************
LOCAL_INC_PATH := \
-I"./src/config"  \
-I"./include" \
-I"./src/leveldb" \
-I"./src/leveldb/include" \
-I"./src/leveldb/helpers/memenv" \
-I"../libboost/src"

#*********************************************************************************************************
# Pre-defined macro (eg. -DYOUR_MARCO=1)
#*********************************************************************************************************
LOCAL_DSYMBOL := -DHAVE_CONFIG_H -DQTUM_BUILD -DHAVE_BUILD_INFO -D__STDC_FORMAT_MACROS \
				   -D_FILE_OFFSET_BITS=64 -DETH_FATDB=1 -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 \
				   -DQTUM_BUILD -DOS_LINUX -DLEVELDB_ATOMIC_PRESENT -D__STDC_LIMIT_MACROS \
				   -DLEVELDB_PLATFORM_POSIX

#*********************************************************************************************************
# Compiler flags
#*********************************************************************************************************
LOCAL_CFLAGS   := -pipe -O2 
LOCAL_CXXFLAGS := -std=c++11 #-fvisibility=hidden #-pipe -O2  -fPIE

#*********************************************************************************************************
# Depend library (eg. LOCAL_DEPEND_LIB := -la LOCAL_DEPEND_LIB_PATH := -L"Your library search path")
#*********************************************************************************************************
LOCAL_DEPEND_LIB      := \
-lleveldb_sse42

LOCAL_DEPEND_LIB_PATH := \
-L"./$(Output)"

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
