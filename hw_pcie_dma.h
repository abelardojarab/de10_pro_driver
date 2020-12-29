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

#ifndef HW_PCIE_DMA_H
#define HW_PCIE_DMA_H

#if defined(WINDOWS)
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#else
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifndef PAGE_SIZE
#  define PAGE_SIZE                               0x1000
#endif

#if defined(WINDOWS)
#  define ACL_PCIE_DMA_PAGES_LOCKED                 4096
#else
#  define ACL_PCIE_DMA_PAGES_LOCKED                  256
#endif

// Maximum number of entries in DMA descriptor controller when using internal descriptor controller
// of Arria 10 PCIe HIP set to Avalon-MM with DMA type
#define ACL_PCIE_DMA_DESC_MAX_ENTRIES                128

// Host channel Maximum number of page entries
#define HOSTCH_MAX_PAGE_ENTRIES			  0x1000


// DMA parameters to tweak
static const unsigned int ACL_PCIE_DMA_MAX_PINNED_MEM_SIZE = ACL_PCIE_DMA_PAGES_LOCKED*PAGE_SIZE;
static const unsigned int ACL_PCIE_DMA_PAGE_ADDR_MASK = PAGE_SIZE-1;
static const unsigned int ACL_PCIE_DMA_TIMEOUT = 0x1000000;
static const unsigned int ACL_PCIE_DMA_CTRL_C_TIMEOUT = 1000;
static const unsigned int ACL_PCIE_DMA_MAX_TRANSFER_SIZE = 65536;  // PCIe DMA supports up to 512KB
static const unsigned int ACL_PCIE_DMA_POLL_SLEEP_TIME_NS = 100;

// This is log of the largest transfer size for non-aligned transfers, not aligned to 4KB.
// Max non-aligned transfer = 2^11 Bytes
static const unsigned int ACL_PCIE_DMA_NON_ALIGNED_TRANS_LOG = 11;

#if defined(WINDOWS)
PACK(
struct DMA_DESC_ENTRY {                  
    UINT32 src_addr_ldw;
    UINT32 src_addr_udw;
    UINT32 dest_addr_ldw;
    UINT32 dest_addr_udw;
    UINT32 ctl_dma_len;
    UINT32 reserved[3];
});

PACK(
struct DMA_DESC_HEADER {
    volatile UINT32 flags[ACL_PCIE_DMA_DESC_MAX_ENTRIES];
});


PACK(
struct DMA_DESC_TABLE {
    struct DMA_DESC_HEADER header;
    struct DMA_DESC_ENTRY descriptors[ACL_PCIE_DMA_DESC_MAX_ENTRIES];
});

// Host Channel
PACK(
struct HOSTCH_ENTRY {
    UINT32 page_addr_ldw;
    UINT32 page_addr_udw;
    UINT32 page_num;
    UINT32 reserved[5];
});

PACK(
struct HOSTCH_TABLE {
    struct HOSTCH_ENTRY page_entry[HOSTCH_MAX_PAGE_ENTRIES];
});

#endif // WINDOWS

#endif // HW_PCIE_DMA_H
