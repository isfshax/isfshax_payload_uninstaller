/*
*	It's a Project! linux-loader
*
*	Copyright (C) 2017          Ash Logan <quarktheawesome@gmail.com>
*	Copyright (C) 2020          Roberto Van Eeden <rwrr0644@gmail.com>
*
*	Based on code from the following contributors:
*
*	Copyright (C) 2016          SALT
*	Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
*
*	Copyright (C) 2008, 2009    Haxx Enterprises <bushing@gmail.com>
*	Copyright (C) 2008, 2009    Sven Peter <svenpeter@gmail.com>
*	Copyright (C) 2008, 2009    Hector Martin "marcan" <marcan@marcansoft.com>
*	Copyright (C) 2009          Andre Heider "dhewg" <dhewg@wiibrew.org>
*	Copyright (C) 2009          John Kelley <wiidev@kelley.ca>
*
*	(see https://github.com/Dazzozo/minute)
*
*	This code is licensed to you under the terms of the GNU GPL, version 2;
*	see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#include "lolserial.h"
#include "system/gpio.h"
#include "common/utils.h"
#include "system/latte.h"
#include <sys/iosupport.h>
#include <string.h>


static int enable = 1;
static char suspend_buf[4096];
static int suspend_len = 0;

extern void lolserial_lprint(const char *s, size_t len);

void lolserial_suspend(void)
{
    memset(suspend_buf, 0, sizeof(suspend_buf));
    suspend_len = 0;
    enable = 0;
}

void lolserial_resume(void)
{
    enable = 1;
    lolserial_lprint(suspend_buf, suspend_len);
}

static ssize_t lolserial_write(struct _reent *r, void *fd, const char *ptr, size_t len)
{
    if (enable) {
        lolserial_lprint(ptr, len);
    } else {
        for(size_t i = 0; (i < len) && (suspend_len < sizeof(suspend_buf)); i++) 
            suspend_buf[suspend_len++] = ptr[i];
    }

    return len;
}

static devoptab_t lolserial_dotab =
{
    .name = "lolserial",
    .write_r = &lolserial_write,
};

void lolserial_init() {
    devoptab_list[STD_OUT] = &lolserial_dotab;
    devoptab_list[STD_ERR] = &lolserial_dotab;
}
