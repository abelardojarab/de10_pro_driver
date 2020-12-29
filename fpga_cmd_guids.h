// Copyright(c) 2017, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

/**
 * @file guids.h
 * @brief FGUIDS for commands
 */

//#ifndef __FPGA_ACCESS_H__
//#define __FPGA_ACCESS_H__

//BEGIN_C_DECL

// {8EC23A39-9916-462A-B77D-ADD56019BE5B}
DEFINE_GUID(GUID_PCI_OPENCL_DISABLE_AER,
    0x8ec23a39, 0x9916, 0x462a, 0xb7, 0x7d, 0xad, 0xd5, 0x60, 0x19, 0xbe, 0x5b);

// {B90B5BD9-F857-42DD-8C4E-7B78C0FA6281}
DEFINE_GUID(GUID_PCI_OPENCL_ENABLE_AER_RETRAIN_LINK,
    0xb90b5bd9, 0xf857, 0x42dd, 0x8c, 0x4e, 0x7b, 0x78, 0xc0, 0xfa, 0x62, 0x81);

// {4EF33395-9816-4787-B4AE-FEB08423B959}
DEFINE_GUID(GUID_PCI_OPENCL_LOAD_PCI_CTRL_REG,
    0x4ef33395, 0x9816, 0x4787, 0xb4, 0xae, 0xfe, 0xb0, 0x84, 0x23, 0xb9, 0x59);

// {655D2909-254F-48D5-A854-C67C08622109}
DEFINE_GUID(GUID_PCI_OPENCL_SAVE_PCI_CTRL_REG,
    0x655d2909, 0x254f, 0x48d5, 0xa8, 0x54, 0xc6, 0x7c, 0x8, 0x62, 0x21, 0x9);

// {50DA184D-88DB-434B-BA6F-2B25488CABF1}
DEFINE_GUID(GUID_PCI_OPENCL_SYNC_CPU_BUFFERS,
    0x50da184d, 0x88db, 0x434b, 0xba, 0x6f, 0x2b, 0x25, 0x48, 0x8c, 0xab, 0xf1);

// {B3D6697A-774F-4C65-A77F-0BEE6B89B1DD}
DEFINE_GUID(GUID_PCI_OPENCL_SYNC_IO_BUFFERS,
    0xb3d6697a, 0x774f, 0x4c65, 0xa7, 0x7f, 0xb, 0xee, 0x6b, 0x89, 0xb1, 0xdd);

// {F9FF96D2-3DDC-4123-B704-D8313693BC1D}
DEFINE_GUID(GUID_PCI_OPENCL_GET_PCI_GEN,
    0xf9ff96d2, 0x3ddc, 0x4123, 0xb7, 0x4, 0xd8, 0x31, 0x36, 0x93, 0xbc, 0x1d);


// {9AE31566-E74A-4A64-A12D-FBE4AA3E3ABB}
DEFINE_GUID(GUID_PCI_OPENCL_GET_PCI_LANES,
    0x9ae31566, 0xe74a, 0x4a64, 0xa1, 0x2d, 0xfb, 0xe4, 0xaa, 0x3e, 0x3a, 0xbb);


#define GUID_TO_FPGA_GUID(x)  (*((fpga_guid *)(&x)))
	
//END_C_DECL

//#endif // __FPGA_ACCESS_H__
