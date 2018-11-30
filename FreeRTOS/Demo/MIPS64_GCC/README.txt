To build the generic RTOS demo for MIPS64/I6500:

Set MIPS_ELF_ROOT to point to the mips tool chain e.g.
export MIPS_ELF_ROOT=/opt/imgtec/Toolchains/mips-img-elf/2017.10-08/bin

Add to the path so the tools can be found e.g.
export PATH=$MIPS_ELF_ROOT/bin/:$PATH

Then build the demo.
cd freertos/FreeRTOS/Demo/MIPS64_GCC

make clean

Prepere build for i6500:
make I6500_boston

Default optimization level is -O3
To build for I6500 (mips64r6):
make

To build demo with -O0 run:
make DEBUG=1

