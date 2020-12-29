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

#include <asm/io.h> // __raw_writel
#include "aclpci.h"
#include "hw_pcie_constants.h"
#include <linux/time.h>

#define FREEZE_STATUS_OFFSET 			0
#define FREEZE_CTRL_OFFSET			4
#define FREEZE_VERSION_OFFSET			12
#define FREEZE_BRIDGE_SUPPORTED_VERSION  	0xad000003

#define FREEZE_REQ				BIT(0)
#define RESET_REQ				BIT(1)
#define UNFREEZE_REQ				BIT(2)

#define FREEZE_REQ_DONE				BIT(0)
#define UNFREEZE_REQ_DONE			BIT(1)

#define ALT_PR_DATA_OFST			0x00
#define ALT_PR_CSR_OFST				0x04
#define ALT_PR_VER_OFST				0x08

#define ALT_PR_CSR_PR_START			BIT(0)
#define ALT_PR_CSR_STATUS_SFT			1
#define ALT_PR_CSR_STATUS_MSK			(7 << ALT_PR_CSR_STATUS_SFT)
#define ALT_PR_CSR_STATUS_NRESET		(0 << ALT_PR_CSR_STATUS_SFT)
#define ALT_PR_CSR_STATUS_BUSY	    		(1 << ALT_PR_CSR_STATUS_SFT)
#define ALT_PR_CSR_STATUS_PR_IN_PROG		(2 << ALT_PR_CSR_STATUS_SFT)
#define ALT_PR_CSR_STATUS_PR_SUCCESS		(3 << ALT_PR_CSR_STATUS_SFT)
#define ALT_PR_CSR_STATUS_PR_ERR	    	(4 << ALT_PR_CSR_STATUS_SFT)

#define PLL_OFFSET_VERSION_ID         		0x000
#define PLL_OFFSET_ROM                		0x400
#define PLL_OFFSET_RECONFIG_CTRL_S10  		0x800
#define PLL_OFFSET_COUNTER            		0x100
#define PLL_OFFSET_RESET              		0x110
#define PLL_OFFSET_LOCK               		0x120

const unsigned int PLL_M_HIGH_REG_S10             	= 260;
const unsigned int PLL_M_LOW_REG_S10              	= 263;
const unsigned int PLL_M_BYPASS_ENABLE_REG_S10    	= 261;
const unsigned int PLL_M_EVEN_DUTY_ENABLE_REG_S10 	= 262;

const unsigned int PLL_N_HIGH_REG_S10             	= 256;
const unsigned int PLL_N_LOW_REG_S10              	= 258;
const unsigned int PLL_N_BYPASS_ENABLE_REG_S10    	= 257;
const unsigned int PLL_N_EVEN_DUTY_ENABLE_REG_S10 	= 257;

const unsigned int PLL_C0_HIGH_REG_S10             	= 283;
const unsigned int PLL_C0_LOW_REG_S10              	= 286;
const unsigned int PLL_C0_BYPASS_ENABLE_REG_S10    	= 284;
const unsigned int PLL_C0_EVEN_DUTY_ENABLE_REG_S10 	= 285;

const unsigned int PLL_C1_HIGH_REG_S10             	= 287;
const unsigned int PLL_C1_LOW_REG_S10              	= 290;
const unsigned int PLL_C1_BYPASS_ENABLE_REG_S10    	= 288;
const unsigned int PLL_C1_EVEN_DUTY_ENABLE_REG_S10 	= 289;

const unsigned int PLL_LF_REG_S10  			= 266;

const unsigned int PLL_CP1_REG_S10  			= 257;
const unsigned int PLL_CP2_REG_S10  			= 269;

const unsigned int PLL_REQUEST_CAL_REG_S10 		= 329;
const unsigned int PLL_ENABLE_CAL_REG_S10 		= 330;

/* Re-configure FPGA kernel partition with given bitstream via PCIe.
 * Support for Arria 10 devices and higher */
int aclpci_pr (struct aclpci_dev *aclpci, void __user* core_bitstream, ssize_t len, int __user* pll_config_array) {

  struct pci_dev *dev = NULL;
  struct aclpci_dma *d = &(aclpci->dma_data);
  char *data;
  int pll_config_array_local[8];
  int i=0, j=0, idle;
  int result = -EFAULT;
  uint32_t to_send, status;
  uint32_t version;
  uint32_t time_to_wait = 0.1;
  ssize_t count=0, chunk_num=0;
  u64 startj, ej;
  u64 startdma, enddma;
  uint32_t pll_freq_khz, pll_m, pll_n, pll_c0, pll_c1, pll_lf, pll_cp, pll_rc;
  uint32_t pll_m_high, pll_m_low, pll_m_bypass_enable, pll_m_even_duty_enable;
  uint32_t pll_n_high, pll_n_low, pll_n_bypass_enable, pll_n_even_duty_enable;
  uint32_t pll_c0_high, pll_c0_low, pll_c0_bypass_enable, pll_c0_even_duty_enable;
  uint32_t pll_c1_high, pll_c1_low, pll_c1_bypass_enable, pll_c1_even_duty_enable;
  uint32_t pll_cp1, pll_cp2;

  /* Basic error checks */  
  if (aclpci == NULL) {
    ACL_DEBUG (KERN_WARNING "Need to open device before can do reconfigure!");
    return result;
  }
  if (core_bitstream == NULL) {
    ACL_DEBUG (KERN_WARNING "Programming bitstream is not provided!");
    return result;
  }
  if (len < 1000000) {
    ACL_DEBUG (KERN_WARNING "Programming bitstream length is suspiciously small. Not doing PR!");
    return result;
  }
  dev = aclpci->pci_dev;
  if (dev == NULL) {
    ACL_DEBUG (KERN_WARNING "Dude, where is PCIe device?!");
    return result;
  }

  //Copy in pll config
  result = copy_from_user(&pll_config_array_local[0], pll_config_array, sizeof(int)*8);

  startj = get_jiffies_64();

  /* freeze bridge */
  status = ioread32(aclpci->bar[ACL_PRREGIONFREEZE_BAR]+ACL_PRREGIONFREEZE_OFFSET+FREEZE_VERSION_OFFSET);
  ACL_DEBUG (KERN_DEBUG "Freeze bridge version is 0x%08X", (int) status);

  status = ioread32(aclpci->bar[ACL_PRREGIONFREEZE_BAR]+ACL_PRREGIONFREEZE_OFFSET+FREEZE_STATUS_OFFSET);
  ACL_DEBUG (KERN_DEBUG "Freeze bridge status is 0x%08X", (int) status);

  ACL_DEBUG (KERN_DEBUG "Asserting region freeze");
  iowrite32(FREEZE_REQ, aclpci->bar[ACL_PRREGIONFREEZE_BAR]+ACL_PRREGIONFREEZE_OFFSET+FREEZE_CTRL_OFFSET);
  msleep(1);

  status = ioread32(aclpci->bar[ACL_PRREGIONFREEZE_BAR]+ACL_PRREGIONFREEZE_OFFSET+FREEZE_STATUS_OFFSET);
  ACL_DEBUG (KERN_DEBUG "Freeze bridge status is 0x%08X", (int) status);

  /* PR IP write initialisation */
  status = ioread32(aclpci->bar[ACL_PRCONTROLLER_BAR]+ACL_PRCONTROLLER_OFFSET+ALT_PR_VER_OFST);
  ACL_DEBUG (KERN_DEBUG "ALT_PR_VER_OFST version is 0x%08X", (int) status);

  status = ioread32(aclpci->bar[ACL_PRCONTROLLER_BAR]+ACL_PRCONTROLLER_OFFSET+ALT_PR_CSR_OFST);
  ACL_DEBUG (KERN_DEBUG "ALT_PR_CSR_OFST status is 0x%08X", (int) status);

  to_send = ALT_PR_CSR_PR_START;
  ACL_DEBUG (KERN_DEBUG "Starting PR by writing 0x%08X to ALT_PR_CSR_OFST", (int) to_send);
  iowrite32(to_send, aclpci->bar[ACL_PRCONTROLLER_BAR]+ACL_PRCONTROLLER_OFFSET+ALT_PR_CSR_OFST); 

  /* Wait for PR to be in progress */
  status = ioread32(aclpci->bar[ACL_PRCONTROLLER_BAR]+ACL_PRCONTROLLER_OFFSET+ALT_PR_CSR_OFST);
  i=0;
  while(status != ALT_PR_CSR_STATUS_PR_IN_PROG)
  {
    msleep(1);
    i++;
    status = ioread32(aclpci->bar[ACL_PRCONTROLLER_BAR]+ACL_PRCONTROLLER_OFFSET+ALT_PR_CSR_OFST);
  };
  ACL_DEBUG (KERN_DEBUG "PR IP initialization took %d ms, ALT_PR_CSR_OFST status is 0x%08X", i, (int) status);

  /* Get version ID */
  version = ioread32(aclpci->bar[ACL_VERSIONID_BAR]+ACL_VERSIONID_OFFSET);
  ACL_DEBUG (KERN_DEBUG "VERSION_ID is 0x%08X", (int) version);


  // ---------------------------------------------------------------
  // Legacy PR using PIO 
  // ---------------------------------------------------------------
  if(version == (unsigned int)ACL_VERSIONID) {
 
    /* PR IP write bitstream */
    data = (char __user*)core_bitstream;
    count = len;
    ACL_DEBUG (KERN_DEBUG "Size of PR RBF is 0x%08X", (int) count);

    /* Write out the complete 32-bit chunks */
    /* Wait for a designated amount of time between 4K chunks */
    while (count >= 4) {
      result = copy_from_user (&to_send, data + i, sizeof(to_send));
      i=i+4;
      iowrite32(to_send, aclpci->bar[ACL_PRCONTROLLER_BAR]+ACL_PRCONTROLLER_OFFSET+ALT_PR_DATA_OFST);
      j++;
      count=count-4;
      if (j >= 1024)
      {
        chunk_num++;
        j = 0;
        msleep(time_to_wait);
      }
    }

    ACL_DEBUG (KERN_DEBUG "Number of 4K chunks written: %d", (int) chunk_num);
    ACL_DEBUG (KERN_DEBUG "Number of bytes in PR bitstream remaining: %d", (int) count);

    /* Write out remaining non 32-bit chunks */
    result = copy_from_user (&to_send, data + i, sizeof(to_send));
    switch (count) {
      case 3:
        to_send = to_send & 0x00ffffff;
        iowrite32(to_send, aclpci->bar[ACL_PRCONTROLLER_BAR]+ACL_PRCONTROLLER_OFFSET+ALT_PR_DATA_OFST);
        break;
      case 2:
        to_send = to_send & 0x0000ffff;
        iowrite32(to_send, aclpci->bar[ACL_PRCONTROLLER_BAR]+ACL_PRCONTROLLER_OFFSET+ALT_PR_DATA_OFST);
        break;
      case 1:
        to_send = to_send & 0x000000ff;
        iowrite32(to_send, aclpci->bar[ACL_PRCONTROLLER_BAR]+ACL_PRCONTROLLER_OFFSET+ALT_PR_DATA_OFST);
        break;
      case 0:
        break;
      default:
        /* This will never happen */
        return -EFAULT;
    }
  }

  // ---------------------------------------------------------------
  // PR using DMA 
  // ---------------------------------------------------------------
  if(version == (unsigned int)ACL_VERSIONID_COMPATIBLE_181) {

    /* PR IP write bitstream */
    ACL_DEBUG (KERN_DEBUG "Size of PR RBF is 0x%08X, initiating DMA transfer to PR IP", (int) len);

    /* Write PR bitstream using DMA */
    status = aclpci_dma_rw (aclpci, (void*) ACL_PCIE_PR_DMA_OFFSET, core_bitstream, len, 0);

    /* Wait for DMA being idle */
    startdma = get_jiffies_64();
    idle=d->m_idle;
    i=0;
    while(idle != 1)
    {
      msleep(1);
      i++;
      idle=d->m_idle;
    };
    enddma = get_jiffies_64();
    ACL_DEBUG (KERN_DEBUG "PR DMA took %u ms, DMA idle status is %d", (jiffies_to_usecs(enddma-startdma)/1000), idle);
  }

  /* Wait for PR complete */ 
  status = ioread32(aclpci->bar[ACL_PRCONTROLLER_BAR]+ACL_PRCONTROLLER_OFFSET+ALT_PR_CSR_OFST);
  i=0;
  while(status != ALT_PR_CSR_STATUS_PR_SUCCESS && status != ALT_PR_CSR_STATUS_PR_ERR)
  {
    msleep(1);
    i++;
    status = ioread32(aclpci->bar[ACL_PRCONTROLLER_BAR]+ACL_PRCONTROLLER_OFFSET+ALT_PR_CSR_OFST);
  };
  ACL_DEBUG (KERN_DEBUG "PR completion took %d ms, ALT_PR_CSR_OFST status is 0x%08X", i, (int) status);

  if (status == ALT_PR_CSR_STATUS_PR_SUCCESS)
  {
    ACL_DEBUG (KERN_DEBUG "PR done! Status is 0x%08X", (int) status);
    result = 0;
  }
  if (status == ALT_PR_CSR_STATUS_PR_ERR) 
  {
    ACL_DEBUG (KERN_DEBUG "PR error! Status is 0x%08X", (int) status);
    result = 1;
  };

  /* dynamically reconfigure IOPLL for kernel clock */
  /* read kernel clock generation version ID */
  status = ioread32(aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_VERSION_ID);
  ACL_DEBUG (KERN_DEBUG "Kernel clock generator version ID is 0x%08X", (int) status);

  /* extract PLL settings from PLL configuration array */
  pll_freq_khz = pll_config_array_local[0];
  pll_m        = pll_config_array_local[1];
  pll_n        = pll_config_array_local[2];
  pll_c0       = pll_config_array_local[3];  
  pll_c1       = pll_config_array_local[4];
  pll_lf       = pll_config_array_local[5];
  pll_cp       = pll_config_array_local[6];
  pll_rc       = pll_config_array_local[7];
  ACL_DEBUG (KERN_DEBUG "PLL settings are %d %d %d %d %d %d %d %d", pll_freq_khz, pll_m, pll_n, pll_c0, pll_c1, pll_lf, pll_cp, pll_rc);

  // Measure kernel clock frequency
  iowrite32(0, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_COUNTER);
  msleep(100);
  status = ioread32(aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_COUNTER);
  ACL_DEBUG (KERN_DEBUG "Before reconfig, kernel clock set to approx. %d MHz", (int) status/100000);

  // extract all PLL parameters
  pll_m_high = (pll_m >> 8) & 0xFF;
  pll_m_low = pll_m & 0xFF;
  pll_m_bypass_enable = (pll_m >> 16) & 0x01;
  pll_m_even_duty_enable = (pll_m >> 17) & 0x01;

  pll_n_high = (pll_n >> 8) & 0xFF;
  pll_n_low = pll_n & 0xFF;
  pll_n_bypass_enable = (pll_n >> 16) & 0x01;
  pll_n_even_duty_enable = (pll_n >> 17) & 0x01;

  pll_c0_high = (pll_c0 >> 8) & 0xFF;
  pll_c0_low = pll_c0 & 0xFF;
  pll_c0_bypass_enable = (pll_c0 >> 16) & 0x01;
  pll_c0_even_duty_enable = (pll_c0 >> 17) & 0x01;

  pll_c1_high = (pll_c1 >> 8) & 0xFF;
  pll_c1_low = pll_c1 & 0xFF;
  pll_c1_bypass_enable = (pll_c1 >> 16) & 0x01;
  pll_c1_even_duty_enable = (pll_c1 >> 17) & 0x01;

  pll_lf = (pll_lf >> 6) & 0xFF;

  pll_cp = pll_cp & 0xFF;
  pll_cp1 = pll_cp & 0x07;
  pll_cp2 = (pll_cp >> 3) & 0x07;

  pll_rc = pll_rc & 0x03;

  /* read and write PLL settings */
  to_send = pll_m_high;
  iowrite8(to_send, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_M_HIGH_REG_S10);
  to_send = pll_m_low;
  iowrite8(to_send, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_M_LOW_REG_S10);
  to_send = pll_m_bypass_enable;
  iowrite8(to_send, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_M_BYPASS_ENABLE_REG_S10);
  to_send = (pll_m_even_duty_enable << 7);
  iowrite8(to_send, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_M_EVEN_DUTY_ENABLE_REG_S10);

  to_send = pll_n_high;
  iowrite8(to_send, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_N_HIGH_REG_S10);
  to_send = pll_n_low;
  iowrite8(to_send, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_N_LOW_REG_S10);
  to_send = (pll_n_even_duty_enable << 7) | (pll_cp1 << 4) | pll_n_bypass_enable;
  iowrite8(to_send, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_N_BYPASS_ENABLE_REG_S10);

  to_send = pll_c0_high;
  iowrite8(to_send, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_C0_HIGH_REG_S10);
  to_send = pll_c0_low;
  iowrite8(to_send, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_C0_LOW_REG_S10);
  to_send = pll_c0_bypass_enable;
  iowrite8(to_send, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_C0_BYPASS_ENABLE_REG_S10);
  to_send = (pll_c0_even_duty_enable << 7);
  iowrite8(to_send, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_C0_EVEN_DUTY_ENABLE_REG_S10);

  to_send = pll_c1_high;
  iowrite8(to_send, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_C1_HIGH_REG_S10);
  to_send = pll_c1_low;
  iowrite8(to_send, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_C1_LOW_REG_S10);
  to_send = pll_c1_bypass_enable;
  iowrite8(to_send, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_C1_BYPASS_ENABLE_REG_S10);
  to_send = (pll_c1_even_duty_enable << 7);
  iowrite8(to_send, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_C1_EVEN_DUTY_ENABLE_REG_S10);

  to_send = (pll_cp2 << 5);
  iowrite8(to_send, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_CP2_REG_S10);

  to_send = (pll_lf << 3) | (pll_rc << 1);
  iowrite8(to_send, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_LF_REG_S10);

  // start PLL calibration
  /* read/modify/write the request calibration */ 
  ACL_DEBUG (KERN_DEBUG "Requesting PLL calibration"); 
  status = ioread8(aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_REQUEST_CAL_REG_S10);
  to_send = status | 0x40;
  iowrite8(to_send, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_REQUEST_CAL_REG_S10);
  /* write 0x03 to enable calibration interface */
  to_send = 0x03;
  iowrite8(to_send, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_RECONFIG_CTRL_S10+PLL_ENABLE_CAL_REG_S10);
  ACL_DEBUG (KERN_DEBUG "PLL calibration done");

  // Measure kernel clock frequency
  iowrite32(0, aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_COUNTER);
  msleep(100);
  status = ioread32(aclpci->bar[ACL_PCIE_KERNELPLL_RECONFIG_BAR]+ACL_PCIE_KERNELPLL_RECONFIG_OFFSET+PLL_OFFSET_COUNTER);
  ACL_DEBUG (KERN_DEBUG "After reconfig, kernel clock set to approx. %d MHz", (int) status/100000);

  /* assert reset */
  ACL_DEBUG (KERN_DEBUG "Asserting region reset");
  iowrite32(RESET_REQ, aclpci->bar[ACL_PRREGIONFREEZE_BAR]+ACL_PRREGIONFREEZE_OFFSET+FREEZE_CTRL_OFFSET);
  msleep(10);

  /* unfreeze bridge */
  status = ioread32(aclpci->bar[ACL_PRREGIONFREEZE_BAR]+ACL_PRREGIONFREEZE_OFFSET+FREEZE_VERSION_OFFSET);
  ACL_DEBUG (KERN_DEBUG "Freeze bridge version is 0x%08X", (int) status);

  status = ioread32(aclpci->bar[ACL_PRREGIONFREEZE_BAR]+ACL_PRREGIONFREEZE_OFFSET+FREEZE_STATUS_OFFSET);
  ACL_DEBUG (KERN_DEBUG "Freeze bridge status is 0x%08X", (int) status);

  ACL_DEBUG (KERN_DEBUG "Removing region freeze");
  iowrite32(UNFREEZE_REQ, aclpci->bar[ACL_PRREGIONFREEZE_BAR]+ACL_PRREGIONFREEZE_OFFSET+FREEZE_CTRL_OFFSET);
  msleep(1);

  ACL_DEBUG (KERN_DEBUG "Checking freeze bridge status");
  status = ioread32(aclpci->bar[ACL_PRREGIONFREEZE_BAR]+ACL_PRREGIONFREEZE_OFFSET+FREEZE_STATUS_OFFSET);
  ACL_DEBUG (KERN_DEBUG "Freeze bridge status is 0x%08X", (int) status);

  /* deassert reset */
  ACL_DEBUG (KERN_DEBUG "Deasserting region reset");
  iowrite32(0, aclpci->bar[ACL_PRREGIONFREEZE_BAR]+ACL_PRREGIONFREEZE_OFFSET+FREEZE_CTRL_OFFSET);
  msleep(10);

  ej = get_jiffies_64();
  ACL_DEBUG (KERN_DEBUG "PR and PLL reconfiguration took %u msec", (jiffies_to_usecs(ej - startj)/1000) );

  ACL_DEBUG (KERN_DEBUG "PR completed!");
  return result;
}

