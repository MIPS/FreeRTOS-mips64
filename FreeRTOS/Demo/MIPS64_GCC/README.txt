
## Generic FreeRTOS Demo for mips64r6/I6500


# Build demo

Set MIPS_ELF_ROOT to point to the mips tool chain e.g.
$ export MIPS_ELF_ROOT=/opt/imgtec/Toolchains/mips-img-elf/2017.10-08/

Add to the path so the tools can be found e.g.
$ export PATH=$MIPS_ELF_ROOT/bin/:$PATH

Then build the demo.
$ cd ./FreeRTOS/Demo/MIPS64_GCC
$ make clean

Prepere build for i6500:
$ make I6500_boston

To build for I6500 (mips64r6):
$ make

Note: Default optimization level is -O3
To build demo with -O0 run:
$ make DEBUG=1


# Run demo

Run Codescape-Debugger

Add sysprobe target

Load demo (RTOSDemo.elf)

Run demo

Expected output  on Codescape-Debugger:
> MIPS:I6500-VPE0(T0) [STDOUT] prvQueueSendTask message sent
> MIPS:I6500-VPE0(T0) [STDOUT] prvQueueReceiveTask message received
> MIPS:I6500-VPE0(T0) [STDOUT] prvQueueSendTask message sent
> MIPS:I6500-VPE0(T0) [STDOUT] prvQueueReceiveTask message received
> ...

Note: Tested with Codescape-8.6 and SysProbe SP58ET
