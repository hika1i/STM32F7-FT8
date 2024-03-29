;/* ----------------------------------------------------------------------
; * Project:      CMSIS DSP Library
; * Title:        arm_bitreversal2.S
; * Description:  arm_bitreversal_32 function done in assembly for maximum speed.
; *               Called after doing an fft to reorder the output.
; *               The function is loop unrolled by 2. arm_bitreversal_16 as well.
; *
; * $Date:        18. March 2019
; * $Revision:    V1.5.2
; *
; * Target Processor: Cortex-M cores
; * -------------------------------------------------------------------- */
;/*
; * Copyright (C) 2010-2019 ARM Limited or its affiliates. All rights reserved.
; *
; * SPDX-License-Identifier: Apache-2.0
; *
; * Licensed under the Apache License, Version 2.0 (the License); you may
; * not use this file except in compliance with the License.
; * You may obtain a copy of the License at
; *
; * www.apache.org/licenses/LICENSE-2.0
; *
; * Unless required by applicable law or agreed to in writing, software
; * distributed under the License is distributed on an AS IS BASIS, WITHOUT
; * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; * See the License for the specific language governing permissions and
; * limitations under the License.
; */

    #define LABEL
	AREA     ||.text||, CODE, READONLY, ALIGN=2
	THUMB

;/**
;  @brief         In-place bit reversal function.
;  @param[in,out] pSrc        points to the in-place buffer of unknown 32-bit data type
;  @param[in]     bitRevLen   bit reversal table length
;  @param[in]     pBitRevTab  points to bit reversal table
;  @return        none
; */
	EXPORT arm_bitreversal_32
	EXPORT arm_bitreversal_16


arm_bitreversal_32 PROC
	ADDS     r3,r1,#1
	CMP      r3,#1
	IT       LS
	BXLS     lr
	PUSH     {r4-r9}
	ADDS     r1,r2,#2
	LSRS     r3,r3,#2
arm_bitreversal_32_0 LABEL       ;/* loop unrolled by 2 */
	LDRH     r8,[r1,#4]
	LDRH     r9,[r1,#2]
	LDRH     r2,[r1,#0]
	LDRH     r12,[r1,#-2]
	ADD      r8,r0,r8
	ADD      r9,r0,r9
	ADD      r2,r0,r2
	ADD      r12,r0,r12
	LDR      r7,[r9,#0]
	LDR      r6,[r8,#0]
	LDR      r5,[r2,#0]
	LDR      r4,[r12,#0]
	STR      r6,[r9,#0]
	STR      r7,[r8,#0]
	STR      r5,[r12,#0]
	STR      r4,[r2,#0]
	LDR      r7,[r9,#4]
	LDR      r6,[r8,#4]
	LDR      r5,[r2,#4]
	LDR      r4,[r12,#4]
	STR      r6,[r9,#4]
	STR      r7,[r8,#4]
	STR      r5,[r12,#4]
	STR      r4,[r2,#4]
	ADDS     r1,r1,#8
	SUBS     r3,r3,#1
	BNE      arm_bitreversal_32_0
	POP      {r4-r9}
	BX       lr
	ENDP

arm_bitreversal_16 PROC
	ADDS     r3,r1,#1
	CMP      r3,#1
	IT       LS
	BXLS     lr
	PUSH     {r4-r9}
	ADDS     r1,r2,#2
	LSRS     r3,r3,#2
arm_bitreversal_16_0 LABEL       ;/* loop unrolled by 2 */
	LDRH     r8,[r1,#4]
	LDRH     r9,[r1,#2]
	LDRH     r2,[r1,#0]
	LDRH     r12,[r1,#-2]
	ADD      r8,r0,r8,LSR #1
	ADD      r9,r0,r9,LSR #1
	ADD      r2,r0,r2,LSR #1
	ADD      r12,r0,r12,LSR #1
	LDR      r7,[r9,#0]
	LDR      r6,[r8,#0]
	LDR      r5,[r2,#0]
	LDR      r4,[r12,#0]
	STR      r6,[r9,#0]
	STR      r7,[r8,#0]
	STR      r5,[r12,#0]
	STR      r4,[r2,#0]
	ADDS     r1,r1,#8
	SUBS     r3,r3,#1
	BNE      arm_bitreversal_16_0
	POP      {r4-r9}
	BX       lr
	ENDP

	END
