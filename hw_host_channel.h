/* 
 * Copyright (c) 2019, Intel Corporation.
 * Intel, the Intel logo, Intel, MegaCore, NIOS II, Quartus and TalkBack 
 * words and logos are trademarks of Intel Corporation or its subsidiaries 
 * in the U.S. and/or other countries. Other marks and brands may be 
 * claimed as the property of others.   See Trademarks on intel.com for 
 * full list of Intel trademarks or the Trademarks & Brands Names Database 
 * (if Intel) or See www.Intel.com/legal (if Altera).
 * All rights reserved
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * BSD 3-Clause license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 *      - Neither Intel nor the names of its contributors may be 
 *        used to endorse or promote products derived from this 
 *        software without specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef HW_HOST_CHANNEL_H
#define HW_HOST_CHANNEL_H

#ifndef PAGE_SIZE
#  define PAGE_SIZE                           0x1000
#endif

/// Maximum software buffer size
//  Constrained by page table size on hardware
#define HOSTCH_MAX_BUF_SIZE                 0x100000

// Loop counter that lets kernel thread to end after
// 1ms of no activity.
#define HOSTCH_LOOP_COUNTER                    20000

/// Host Channel BAR4 Registers
//  Controls dma_to_kernel IP
#define HOSTCH_BASE                           0xc700
#define HOSTCH_CONTROL_ADDR_PUSH          0x00000000
#define HOSTCH_CONTROL_ADDR_PULL          0x00000004
#define HOSTCH_IN_FRONT_ADDR              0x00000008
#define HOSTCH_IN_END_ADDR                0x0000000C
#define HOSTCH_OUT_FRONT_ADDR             0x00000010
#define HOSTCH_OUT_END_ADDR               0x00000014
#define HOSTCH_IN_AVAIL                   0x00000018
#define HOSTCH_OUT_AVAIL                  0x0000001C

// Host Channel BAR0 Registers
#define ACL_HOST_CHANNEL_BAR                     0
#define ACL_HOST_CHANNEL_CTR_BASE           0x0000

/// Name of channel 0
//  Following values are used when
//  initializing the channel
#define ACL_HOST_CHANNEL_0 		   	 0
#define ACL_HOST_CHANNEL_0_NAME   "host_to_device"

// Handle for push channel
#define ACL_HOST_CHANNEL_0_ID                   10

// Checking if channel 0 is a Write Channel: true
#define ACL_HOST_CHANNEL_0_WRITE                 1

// Qsys address for push channel control registers
#define ACL_HOST_CHANNEL_0_TXS_ADDR_LOW     0x0020
#define ACL_HOST_CHANNEL_0_TXS_ADDR_HIGH    0x0024
#define ACL_HOST_CHANNEL_0_HOST_ENDP        0x0028
#define ACL_HOST_CHANNEL_0_LOGIC_EN         0x002C
#define ACL_HOST_CHANNEL_0_IP_ADDR_HIGH     0x0030
#define ACL_HOST_CHANNEL_0_IP_ADDR_LOW      0x0034
#define ACL_HOST_CHANNEL_0_BUF_SIZE         0x0038

// Qsys address of host channel circular buffer on FPGA
#define ACL_HOST_CHANNEL_0_DMA_ADDR    0x100000000

/// Name of channel 1
//  Following values are used when
//  initializing the channel
#define ACL_HOST_CHANNEL_1 		         1
#define ACL_HOST_CHANNEL_1_NAME   "device_to_host"

// Handle for pull channel
#define ACL_HOST_CHANNEL_1_ID                   11

// Checking if channel 1 is a Write Channel: false
#define ACL_HOST_CHANNEL_1_WRITE                 0

// Qsys address for pull channel control registers
#define ACL_HOST_CHANNEL_1_TXS_ADDR_LOW     0x0120
#define ACL_HOST_CHANNEL_1_TXS_ADDR_HIGH    0x0124
#define ACL_HOST_CHANNEL_1_HOST_FRONTP      0x0128
#define ACL_HOST_CHANNEL_1_LOGIC_EN         0x012C
#define ACL_HOST_CHANNEL_1_IP_ADDR_HIGH     0x0130
#define ACL_HOST_CHANNEL_1_IP_ADDR_LOW      0x0134
#define ACL_HOST_CHANNEL_1_BUF_SIZE         0x0138

// Qsys address of host channel circular buffer on FPGA
#define ACL_HOST_CHANNEL_1_DMA_ADDR    0x100000000

// ERROR Values
#define ERROR_CHANNEL_PREVIOUSLY_OPENED        -10
#define ERROR_CHANNEL_CLOSED                   -11
#define ERROR_INVALID_CHANNEL                  -12
#define ERROR_INCORRECT_DIRECTION              -13

#endif // HW_HOST_CHANNEL_H
