# Need to use the exact version of gcc that was used to compile the kernel
# It's in /usr/bin, so put it at the front of the path
export PATH=/usr/bin:$PATH
SRC_PATH=/lib/modules/`uname -r`/build
echo "Looking for kernel source files in $SRC_PATH"
if [ ! -d "$SRC_PATH" ]
then
  SRC_PATH=/usr/src/kernels/`uname -r`
  echo "Looking for kernel source files in $SRC_PATH"
fi

if [ ! -d "$SRC_PATH" ]
then
  echo
  echo "Error: Failed to find kernel source files.  The PCIe driver requires"
  echo "headers and Makefiles for your current Linux kernel, but these could"
  echo "not be found on your system.  Please install these on the machine."
  echo
  echo "   For example:"
  echo "      On RedHat: sudo yum install kernel-devel"
  echo "      On Ubuntu: sudo apt-get source linux"
  echo
  exit 1
fi

echo "Using kernel source files from  $SRC_PATH"

if [ $# -ne 1 ]
then
  echo "*** Incorrect number of args: make_all.sh <name>"
  exit 1
fi

BSP_NAME=$1

BSP_NAME_FROM_HEADER=`grep ACL_BOARD_PKG_NAME hw_pcie_constants.h |cut -f2 -d\"`

if [ "$BSP_NAME" != "$BSP_NAME_FROM_HEADER" ]
then
  echo "BSP name $BSP_NAME in board_env.xml must match ACL_BOARD_PKG_NAME $BSP_NAME_FROM_HEADER set in hw_pcie_constants.h"
  exit 1
fi
echo Building driver for BSP with name $BSP_NAME

make -C $SRC_PATH M=`pwd` BSP_NAME=$BSP_NAME modules
