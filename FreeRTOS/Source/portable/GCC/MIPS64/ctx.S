/*
	FreeRTOS V8.1.2 - Copyright (C) 2014 Real Time Engineers Ltd.
	All rights reserved

	VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

	***************************************************************************
	 *                                                                       *
	 *    FreeRTOS provides completely free yet professionally developed,    *
	 *    robust, strictly quality controlled, supported, and cross          *
	 *    platform software that has become a de facto standard.             *
	 *                                                                       *
	 *    Help yourself get started quickly and support the FreeRTOS         *
	 *    project by purchasing a FreeRTOS tutorial book, reference          *
	 *    manual, or both from: http://www.FreeRTOS.org/Documentation        *
	 *                                                                       *
	 *    Thank you!                                                         *
	 *                                                                       *
	***************************************************************************

	This file is part of the FreeRTOS distribution.

	FreeRTOS is free software; you can redistribute it and/or modify it under
	the terms of the GNU General Public License (version 2) as published by the
	Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

	>>!   NOTE: The modification to the GPL is included to allow you to     !<<
	>>!   distribute a combined work that includes FreeRTOS without being   !<<
	>>!   obliged to provide the source code for proprietary components     !<<
	>>!   outside of the FreeRTOS kernel.                                   !<<

	FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
	WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE.  Full license text is available from the following
	link: http://www.freertos.org/a00114.html

	1 tab == 4 spaces!

	***************************************************************************
	 *                                                                       *
	 *    Having a problem?  Start by reading the FAQ "My application does   *
	 *    not run, what could be wrong?"                                     *
	 *                                                                       *
	 *    http://www.FreeRTOS.org/FAQHelp.html                               *
	 *                                                                       *
	***************************************************************************

	http://www.FreeRTOS.org - Documentation, books, training, latest versions,
	license and Real Time Engineers Ltd. contact details.

	http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
	including FreeRTOS+Trace - an indispensable productivity tool, a DOS
	compatible FAT file system, and our tiny thread aware UDP/IP stack.

	http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
	Integrity Systems to sell under the OpenRTOS brand.  Low cost OpenRTOS
	licenses offer ticketed support, indemnification and middleware.

	http://www.SafeRTOS.com - High Integrity Systems also provide a safety
	engineered and independently SIL3 certified version for use in safety and
	mission critical applications that require provable dependability.

	1 tab == 4 spaces!
*/

#include <mips/regdef.h>
#include <mips/asm.h>
#include <mips/cpu.h>
#include <mips/hal.h>

/* Save the hard context to a gp_ctx pointed to by k1.
   Leave the value of C0_STATUS in v0/$2.
   Leave the value of k1 unchanged.  */

.macro _gpctx_save
	/* Save general registers.  */
	REG_S   $1, CTX_REG(1) (k1)
	REG_S   $2, CTX_REG(2) (k1)
	REG_S   $3, CTX_REG(3) (k1)
	REG_S   $4, CTX_REG(4) (k1)
	REG_S   $5, CTX_REG(5) (k1)
	REG_S   $6, CTX_REG(6) (k1)
	REG_S   $7, CTX_REG(7) (k1)
	REG_S   $8, CTX_REG(8) (k1)
	REG_S   $9, CTX_REG(9) (k1)
	REG_S   $10, CTX_REG(10) (k1)
	REG_S   $11, CTX_REG(11) (k1)
	REG_S   $12, CTX_REG(12) (k1)
	REG_S   $13, CTX_REG(13) (k1)
	REG_S   $14, CTX_REG(14) (k1)
	REG_S   $15, CTX_REG(15) (k1)
	REG_S   $16, CTX_REG(16) (k1)
	REG_S   $17, CTX_REG(17)(k1)
	REG_S   $18, CTX_REG(18)(k1)
	REG_S   $19, CTX_REG(19)(k1)
	REG_S   $20, CTX_REG(20)(k1)
	REG_S   $21, CTX_REG(21)(k1)
	REG_S   $22, CTX_REG(22)(k1)
	REG_S   $23, CTX_REG(23)(k1)
	REG_S   $24, CTX_REG(24)(k1)
	REG_S   $25, CTX_REG(25)(k1)
	REG_S   $26, CTX_REG(26)(k1)
	/* $27/k1 must be saved prior to using this macro.  */
	REG_S   $28, CTX_REG(28)(k1)
	REG_S   $29, CTX_REG(29)(k1)
	REG_S   $30, CTX_REG(30)(k1)
	REG_S   $31, CTX_REG(31)(k1)
	PTR_S   $0, CTX_LINK(k1) 	/* Clear the link field.  */

	/* Check ISA release and optionally restore HI/LO registers.  */
#if (__mips_isa_rev < 6)
	mfhi	$9
	REG_S	$9, CTX_HI0(k1)
	mflo	$10
	REG_S	$10, CTX_LO0(k1)
#endif

	/* Save CP0 registers.  */
	PTR_MFC0 $31, C0_EPC
	REG_S	$31, CTX_EPC(k1)
	mfc0	$10, C0_STATUS
	sw	$10, CTX_STATUS(k1)
.endm

/* Restores a gp_ctx pointed to by a0.  Leaves interrupts disabled and
   C0_EPC ready to eret.  */

.macro _gpctx_load

	/* Check ISA release and optionally restore HI/LO registers.  */
#if (__mips_isa_rev < 6)
	REG_L	$9, CTX_HI0(a0)
	REG_L	$10, CTX_LO0(a0)
	mthi	$9
	mtlo	$10
#endif

	/* Restore the general registers.  */
	REG_L	$1, CTX_REG(1)(a0)
	REG_L	$2, CTX_REG(2)(a0)
	REG_L	$3, CTX_REG(3)(a0)
	/* Do not restore $4 until the end.  */
	REG_L	$5, CTX_REG(5)(a0)
	REG_L	$6, CTX_REG(6)(a0)
	REG_L	$7, CTX_REG(7)(a0)
	REG_L	$8, CTX_REG(8)(a0)
	REG_L	$9, CTX_REG(9)(a0)
	REG_L	$10, CTX_REG(10)(a0)
	REG_L	$11, CTX_REG(11)(a0)
	REG_L	$12, CTX_REG(12)(a0)
	REG_L	$13, CTX_REG(13)(a0)
	REG_L	$14, CTX_REG(14)(a0)
	REG_L	$15, CTX_REG(15)(a0)
	REG_L	$16, CTX_REG(16)(a0)
	REG_L	$17, CTX_REG(17)(a0)
	REG_L	$18, CTX_REG(18)(a0)
	REG_L	$19, CTX_REG(19)(a0)
	REG_L	$20, CTX_REG(20)(a0)
	REG_L	$21, CTX_REG(21)(a0)
	REG_L	$22, CTX_REG(22)(a0)
	REG_L	$23, CTX_REG(23)(a0)
	REG_L	$24, CTX_REG(24)(a0)
	REG_L	$25, CTX_REG(25)(a0)

	/* Restore CP0 registers, kernel registers and stack with
	   interrupts disabled.  */
	di
	lw	$27, CTX_STATUS(a0)
	REG_L	$26, CTX_EPC(a0)
	mtc0	$27, C0_STATUS
	PTR_MTC0 $26, C0_EPC
	ehb

	REG_L	$26, CTX_REG(26)(a0)
	REG_L	$27, CTX_REG(27)(a0)
	REG_L	$28, CTX_REG(28)(a0)
	REG_L	$29, CTX_REG(29)(a0)
	REG_L	$30, CTX_REG(30)(a0)
	REG_L	$31, CTX_REG(31)(a0)

	/* Finally restore a0/$4.  */
	REG_L	$4, CTX_REG(4)(a0)

.endm
