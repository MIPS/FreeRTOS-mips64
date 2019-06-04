# MIPS FreeRTOS

This is a fork of FreeRTOS providing support for:
* mips32 compliant cores from mip32r2 to mips32r6,
* mips64r6,
* microMIPS and nanoMIPS ISA.

Requirements:

This port is dependent on features provided by Codescape SDK.
To get the latest version visit the following website:

https://www.mips.com/develop/tools/codescape-mips-sdk/

You must use SDK v1.3 or higher.

Or if you're just installing the tools on their own, it must be v2015.06-xx
or higher.

Features:
* Uses FreeRTOS V10.2.1
* Generic Demo for mips32 archs (mip32r2 to mips32r6)
* Generic Demo for mips64r6
* Specific LwIP Demos for both CI40 and Xilinx NEXYS4DDR FPGA boards

## Generic Demo - 64bit

The generic demo is tested on Boston FPGA board with bitfile supporting I6500
processor (mips64r6).

Building the demo:
```
cd FreeRTOS/Demo/MIPS64_GCC
make I6500_boston
make
```
Demo run instructions are available in [README.txt](FreeRTOS/Demo/MIPS64_GCC/README.txt)

## Generic Demo - 32bit

The generic demo is highly configurable and thus is configured via a config
file, see FreeRTOS/Demo/MIPS32_GCC/configs (any new configs added must be stored
here).

Building the demo:
```
cd FreeRTOS/Demo/MIPS32_GCC
make defaultconfig
make
```
## LWIP Based Demo

Note: The Xilinx NEXYS4DDR FPGA board must be programmed with a suitable bitfile
supporting a MIPS m14kc processor.

Building the Demo for the CI40 target:
```
cd FreeRTOS/Demo/lwIP_MIPS_GCC/CI40/
ARCH=interaptiv INTS=eic GIC_BASE_ADDRESS=0x1BDC0000 make
```

Building the Demo for the mipsFPGA target:
```
cd FreeRTOS/Demo/lwIP_MIPS_GCC/mipsFPGA/
ARCH=m14kc make
```

NOTE: The compiler must be in your path. You'll need mips-mti-elf- toolchain
for mips32r2-r5 targets and mips-img-elf- for mips32r6/mips64r6 targets.
E.g. for mips64r6:
```
export MIPS_ELF_ROOT=/opt/imgtec/Toolchains/mips-img-elf/2017.10-08/bin
export PATH=$MIPS_ELF_ROOT/bin/:$PATH
```
