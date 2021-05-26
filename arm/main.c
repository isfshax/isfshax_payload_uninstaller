/*
 *  minute - a port of the "mini" IOS replacement for the Wii U.
 *
 *  Copyright (C) 2021          rw-r-r-0644 <rwrr0644@gmail.com>
 *  Copyright (C) 2017          Ash Logan <quarktheawesome@gmail.com>
 *
 *  Copyright (C) 2016          SALT
 *  Copyright (C) 2016          Daz Jones <daz@dazzozo.com>
 *
 *  Copyright (C) 2008, 2009    Haxx Enterprises <bushing@gmail.com>
 *  Copyright (C) 2008, 2009    Sven Peter <svenpeter@gmail.com>
 *  Copyright (C) 2008, 2009    Hector Martin "marcan" <marcan@marcansoft.com>
 *  Copyright (C) 2009          Andre Heider "dhewg" <dhewg@wiibrew.org>
 *  Copyright (C) 2009          John Kelley <wiidev@kelley.ca>
 *
 *  This code is licensed to you under the terms of the GNU GPL, version 2;
 *  see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "common/types.h"
#include "common/utils.h"
#include "system/exception.h"
#include "system/memory.h"
#include "system/irq.h"
#include "system/smc.h"
#include "system/latte.h"
#include "storage/sd/sdcard.h"
#include "storage/sd/fatfs/elm.h"
#include "storage/sd/fatfs/ff.h"
#include "storage/nand/nand.h"
#include "storage/nand/isfs/isfs.h"
#include "storage/nand/isfs/super.h"
#include "storage/nand/isfs/volume.h"
#include "storage/nand/isfs/isfshax.h"
#include "crypto/crypto.h"
#include "crypto/sha.h"
#include "lolserial.h"


int uninstall_isfshax(void)
{
    isfshax_info isfshax;
    isfs_ctx *slc = isfs_get_volume(ISFSVOL_SLC);
    u16* fat;
    int i, rc;

    /* load isfshax slot allocation from the newest isfshax superblock */
    puts("Loading latest isfshax superblock...\n");
    rc = isfs_load_super(slc, ISFSHAX_GENERATION_FIRST, 0xffffffff);
    if (rc < 0) {
        printf("ERROR: Failed to find isfshax superblock (%d)\n", rc);
        return -1;
    }

    memcpy(&isfshax, slc->super + ISFSHAX_INFO_OFFSET, sizeof(isfshax));
    if (isfshax.magic != ISFSHAX_MAGIC) {
        printf("ERROR: Bad isfshax data magic %08lX\n", isfshax.magic);
        return -2;
    }

    /* load normal isfs superblock to free up good isfshas slots */
    rc = isfs_load_super(slc, 0, ISFSHAX_GENERATION_FIRST);
    if (rc < 0) {
        printf("ERROR: Failed to find an unpatched isfs superblock (%d)\n", rc);
        return -3;
    }
    fat = isfs_get_fat(slc);

    for (i = 0; i < ISFSHAX_REDUNDANCY; i++) {
        u32 slot = isfshax.slots[i] & ~ISFSHAX_BAD_SLOT;
        u32 block = BLOCK_COUNT - (slc->super_count - slot) * ISFSSUPER_BLOCKS;
        u32 cluster = block * BLOCK_CLUSTERS;
        u32 offs = 0;

        /* (attempt to) erase isfshax superblocks */
        printf("Erasing isfshax slot %d (isfs slot %lu, block %lu-%lu)\n", i, slot, block+0, block+1);
        nand_erase_block(block+0);
        nand_erase_block(block+1);

        if (isfshax.slots[i] & ISFSHAX_BAD_SLOT)
            continue;

        /* remove bad slot mark from good slots */
        for (offs = 0; offs < ISFSSUPER_CLUSTERS; offs++)
            fat[cluster + offs] = FAT_CLUSTER_RESERVED;
    }

    /* write two copies of the updated ISFS superblock, just to be sure */
    puts("Writing updated isfs superblock");
    for (i = 0; i < 2; i++) {
        rc = isfs_commit_super(slc);
        if (rc < 0) {
            printf("ERROR: Failed to commit updated superblock (%d)\n", rc);
            return -4;
        }
    }

    puts("\nSUCCESS.");
    return 0;
}

void NORETURN _main(void* base) {
    lolserial_init();
    exception_initialize();
    mem_initialize();
    irq_initialize();
    crypto_initialize();
    nand_initialize();

    uninstall_isfshax();

    irq_disable(IRQ_SD0);
    nand_deinitialize();
    irq_shutdown();
    mem_shutdown();
    smc_shutdown(false);
}


