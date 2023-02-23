#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2019 Western Digital Corporation or its affiliates.
#

# Compiler pre-processor flags
platform-cppflags-y =

# C Compiler and assembler flags.
platform-cflags-y =
platform-asflags-y =

# Linker flags: additional libraries and object files that the platform
# code needs can be added here
platform-ldflags-y =

# Space separated list of object file names to be compiled for the platform
platform-objs-y += platform.o

#
# If the platform support requires a builtin device tree file, the name of
# the device tree compiled file should be specified here. The device tree
# source file be in the form <dt file name>.dts
#
# platform-objs-y += <dt file name>.o

# Firmware load address configuration. This is mandatory.
FW_TEXT_START=0x80000000

# Optional parameter for path to external FDT
# FW_FDT_PATH="path to platform flattened device tree file"

#
# Jump firmware configuration.
# Optional parameters are commented out. Uncomment and define these parameters
# as needed.
#
FW_JUMP=y
# This needs to be 4MB aligned for 32-bit support
# This needs to be 2MB aligned for 64-bit support
# ifeq ($(PLATFORM_RISCV_XLEN), 32)
# FW_JUMP_ADDR=0x80400000
# else
# FW_JUMP_ADDR=0x80200000
# endif
# FW_JUMP_FDT_ADDR=0x82200000
#
# 该地址不使用, 编译要求定义填0
FW_JUMP_ADDR=0x0
