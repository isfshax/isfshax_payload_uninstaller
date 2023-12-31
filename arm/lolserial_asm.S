/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
*	Copyright (C) 2021          Roberto Van Eeden <rwrr0644@gmail.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include "system/latte.h"

.arm

.globl lolserial_print
.globl lolserial_lprint

.section .text

/*
 * the number of timer ticks to wait
 * each bit; might need manual adjustment
 * for particoular serial adapters, usually
 * the stable value is in the +5/-5 range
 */

.equ LOLSERIAL_WAIT_TICKS, 200

.if 1
/* GP_SENSORBAR */
.equ LOLSERIAL_PIN, 0x00000100
.equ LOLSERIAL_PIN_SHIFT, 8
.else
/* GP_DEBUG0 */
.equ LOLSERIAL_PIN, 0x00010000
.equ LOLSERIAL_PIN_SHIFT, 16
.endif

lolserial_print:
    mov r1, #-1
lolserial_lprint:
    push {r4-r6}
    add r1, r1, r0

    ldr r6, =0x0D800000

    /* clear32(LT_GPIO_OWNER, LOLSERIAL_PIN) */
    ldr r5, [r6, #0x0FC]
    bic r5, r5, #LOLSERIAL_PIN
    str r5, [r6, #0x0FC]

    /* set32(LT_GPIO_ENABLE, LOLSERIAL_PIN) */
    ldr r5, [r6, #0x0DC]
    orr r5, r5, #LOLSERIAL_PIN
    str r5, [r6, #0x0DC]

    /* set32(LT_GPIO_DIR, LOLSERIAL_PIN) */
    ldr r5, [r6, #0x0E4]
    orr r5, r5, #LOLSERIAL_PIN
    str r5, [r6, #0x0E4]

    /* set32(LT_GPIO_OUT, LOLSERIAL_PIN) */
    ldr r5, [r6, #0x0E0]
    orr r5, r5, #LOLSERIAL_PIN
    str r5, [r6, #0x0E0]

    lolserial_send_string_loop:
        cmp r0, r1
        ldrneb r5, [r0], #1
        cmpne r5, #0
        beq lolserial_send_string_end    

        mov r3, #0x200
        orr r3, r3, r5, lsl #1

        lolserial_send_char_loop:
            and r4, r3, #1

            ldr r5, [r6, #0x0E0]
            bic r5, r5, #LOLSERIAL_PIN
            orr r5, r5, r4, lsl #LOLSERIAL_PIN_SHIFT
            str r5, [r6, #0x0E0]

            ldr r5, [r6, #0x010]
            adds r4, r5, #LOLSERIAL_WAIT_TICKS
            bcc timer_wait_loop
            timer_wait_overflow_loop:
                ldr r2, [r6, #0x010]
                cmp r2, r5
                bhs timer_wait_overflow_loop
            timer_wait_loop:
                ldr r5, [r6, #0x010]
                cmp r5, r4
                blo timer_wait_loop

            movs r3, r3, lsr #1
            bne lolserial_send_char_loop            
        
        b lolserial_send_string_loop
    lolserial_send_string_end:

    pop {r4-r6}
    bx lr
