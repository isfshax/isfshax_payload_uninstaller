/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#ifndef _SMC_H
#define _SMC_H

#include "common/types.h"

enum {
    SMC_CMD_ODDPOWER_ON         = 0,
    SMC_CMD_ODDPOWER_OFF        = 1,
    SMC_CMD_REQUEST_EJECT       = 2,
    SMC_CMD_ONINDICATOR_ON      = 0x10,
    SMC_CMD_ONINDICATOR_OFF     = 0x12,
    SMC_CMD_ONINDICATOR_PULSE   = 0x13,
    SMC_CMD_CCINDICATOR_OFF     = 0x14,
    SMC_CMD_CCINDICATOR_ON      = 0x15,
    SMC_CMD_CCINDICATOR_PULSE   = 0x16,
    SMC_CMD_OFFINDICATOR_ON     = 0x1E,
    SMC_CMD_OFFINDICATOR_PULSE  = 0x1F,
    SMC_CMD_BT_RST              = 0x20,
    SMC_CMD_WIFI_RST            = 0x21,
    SMC_CMD_DWIFI_RST           = 0x22,
};


#define SMC_POWER_BUTTON 0x40
#define SMC_EJECT_BUTTON 0x20

int smc_read_register(u8 offset, u8* data);
int smc_write_register(u8 offset, u8 data);

int smc_write_raw(u8 data);

u8 smc_get_events(void);
u8 smc_wait_events(u8 mask);

int smc_set_odd_power(bool enable);
int smc_set_cc_indicator(int state);
int smc_set_on_indicator(int state);

void __attribute__((__noreturn__)) smc_shutdown(bool reset);
void __attribute__((__noreturn__)) smc_reset(void);
void __attribute__((__noreturn__)) smc_power_off(void);

void smc_get_panic_reason(char* buffer);
void smc_set_panic_reason(const char* buffer);

#endif
