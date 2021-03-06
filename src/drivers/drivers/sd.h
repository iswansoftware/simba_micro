/**
 * @file drivers/sd.h
 * @version 0.5.0
 *
 * @section License
 * Copyright (C) 2014-2016, Erik Moqvist
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * This file is part of the Simba project.
 */

#ifndef __DRIVERS_SD_H__
#define __DRIVERS_SD_H__

#include "simba.h"

#define SD_BLOCK_SIZE 512

/* CID, Card Identification register, table 5-2. */
struct sd_cid_t {
    uint8_t mid;		/* Manufacturer ID. */
    char oid[2];		/* OEM/Application ID. */
    char pnm[5];		/* Product name. */
    uint8_t prv; 		/* Product revision. */
    uint32_t psn;		/* Product serial number. */
    uint16_t mdt;		/* Manufacturing date. */
    uint8_t crc;		/* CRC7 checksum. */
} PACKED;

/* Merge split fields into one. */
#define SD_CCC(csd_p)                           \
    (((uint16_t)(csd_p)->ccc_high << 4)         \
     | ((uint16_t)(csd_p)->ccc_low << 0))
#define SD_C_SIZE(csd_p)                        \
    (((uint16_t)(csd_p)->c_size_high << 10)     \
     | ((uint16_t)(csd_p)->c_size_mid << 2)     \
     | ((uint16_t)(csd_p)->c_size_low << 0))
#define SD_C_SIZE_MULT(csd_p)                   \
    (((csd_p)->c_size_mult_high << 1)           \
     | ((csd_p)->c_size_mult_low << 0))
#define SD_SECTOR_SIZE(csd_p)                   \
    (((csd_p)->sector_size_high << 1)           \
     | ((csd_p)->sector_size_low << 0))
#define SD_WRITE_BL_LEN(csd_p)                  \
    (((csd_p)->write_bl_len_high << 2)          \
     | ((csd_p)->write_bl_len_low << 0))

#define SD_CSD_STRUCTURE_V1 0
#define SD_CSD_STRUCTURE_V2 1

/* CSD, Card-Specific Data register, version 0.5.00. */
struct sd_csd_v1_t {
    uint8_t reserved1 : 6;
    uint8_t csd_structure : 2;      /* CSD structure. */
    uint8_t taac;                   /* Data read access-time-1. */
    uint8_t nsac;                   /* Data read access-time-2 in CLK cycles (NSAC * 100). */
    uint8_t tran_speed;             /* Max. data transfer rate. */
    uint8_t ccc_high;               /* Card command class. */
    uint8_t read_bl_len : 4;        /* Max. read data block length. */
    uint8_t ccc_low : 4;
    uint8_t c_size_high : 2;        /* Device size. */
    uint8_t reserved2 : 2;
    uint8_t dsr_imp : 1;            /* DSR implemented. */
    uint8_t read_blk_misalign : 1;  /* Read block misalignment. */
    uint8_t write_blk_misalign : 1; /* Write block misalignment. */
    uint8_t read_bl_partial : 1;    /* Partial blocks for read allowed. */
    uint8_t c_size_mid;
    uint8_t vdd_r_curr_max : 3;     /* Max. read current @VDD max. */
    uint8_t vdd_r_curr_min : 3;     /* Max. read current @VDD min. */
    uint8_t c_size_low  : 2;
    uint8_t c_size_mult_high : 2;   /* Device size multiplier. */
    uint8_t vdd_w_curr_max : 3;     /* Max. write current @VDD max. */
    uint8_t vdd_w_curr_min : 3;     /* Max. write current @VDD min. */
    uint8_t sector_size_high : 6;   /* Erase sector size. */
    uint8_t erase_blk_en : 1;       /* Erase single block enable. */
    uint8_t c_size_mult_low : 1;
    uint8_t wp_grp_size : 7;        /* Write protect group size. */
    uint8_t sector_size_low : 1;
    uint8_t write_bl_len_high : 2;  /* Max. write data block length. */
    uint8_t r2w_factor : 3;         /* Write speed factor. */
    uint8_t reserved3 : 2;
    uint8_t wp_grp_enable : 1;      /* Write protect group enable. */
    uint8_t reserved4 : 5;
    uint8_t write_bl_partial : 1;   /* Partial blocks for write allowed. */
    uint8_t write_bl_len_low : 2;   /* Max. write data block length. */
    uint8_t reserved5 : 2;
    uint8_t file_format : 2;        /* File format. */
    uint8_t tmp_write_protect : 1;  /* Temporary write protection. */
    uint8_t perm_write_protect : 1; /* Permanent write protection. */
    uint8_t copy : 1;               /* Copy flag. */
    uint8_t file_format_grp : 1;    /* File format group. */
    uint8_t crc;                    /* CRC. */
} PACKED;

/* CSD, Card-Specific Data register, version 2.00. */
struct sd_csd_v2_t {
    uint8_t reserved1 : 6;
    uint8_t csd_structure : 2;
    uint8_t taac;
    uint8_t nsac;
    uint8_t tran_speed;
    uint8_t ccc_high;
    uint8_t read_bl_len : 4;
    uint8_t ccc_low : 4;
    uint8_t reserved2 : 4;
    uint8_t dsr_imp : 1;
    uint8_t read_blk_misalign : 1;
    uint8_t write_blk_misalign : 1;
    uint8_t read_bl_partial : 1;
    uint8_t c_size_high : 6;
    uint8_t reserved3 : 2;
    uint8_t c_size_mid;
    uint8_t c_size_low;
    uint8_t sector_size_high : 6;
    uint8_t erase_blk_en : 1;
    uint8_t reserved4 : 1;
    uint8_t wp_grp_size : 7;
    uint8_t sector_size_low : 1;
    uint8_t write_bl_len_high : 2;
    uint8_t r2w_factor : 3;
    uint8_t reserved5 : 2;
    uint8_t wp_grp_enable : 1;
    uint8_t reserved6 : 5;
    uint8_t write_bl_partial : 1;
    uint8_t write_bl_len_low : 2;
    uint8_t reserved7 : 2;
    uint8_t file_format : 2;
    uint8_t tmp_write_protect : 1;
    uint8_t perm_write_protect : 1;
    uint8_t copy : 1;
    uint8_t file_format_grp : 1;
    uint8_t crc;
} PACKED;

/* CSD, Card-Specific Data register. */
union sd_csd_t {
    struct sd_csd_v1_t v1;
    struct sd_csd_v2_t v2;
};

struct sd_driver_t {
    struct spi_driver_t *spi_p;
    int type;
};

/**
 * Initialize driver object.
 *
 * @param[in,out] self_p Driver object to initialize.
 *
 * @return zero(0) or negative error code.
 */
int sd_init(struct sd_driver_t *self_p,
            struct spi_driver_t *spi_p);

/**
 * Start driver.
 *
 * @param[in] self_p Initialized driver object.
 *
 * @return zero(0) or negative error code.
 */
int sd_start(struct sd_driver_t *self_p);

/**
 * Stop driver.
 *
 * @param[in] self_p Initialized driver object.
 *
 * @return zero(0) or negative error code.
 */
int sd_stop(struct sd_driver_t *self_p);

/**
 * Read card CID register. The CID contains card identification
 * information such as Manufacturer ID, Product name, Product serial
 * number and Manufacturing date.
 *
 * @param[in] self_p Initialized driver object.
 * @param[out] cid pointer to cid data store.
 *
 * @return zero(0) or negative error code.
 */
ssize_t sd_read_cid(struct sd_driver_t *self_p,
                    struct sd_cid_t* cid_p);

/**
 * Read card CSD register. The CSD contains that provides
 * information regarding access to the card's contents.
 *
 * @param[in] self_p Initialized driver object.
 * @param[out] csd pointer to csd data store.
 *
 * @return zero(0) or negative error code.
 */
ssize_t sd_read_csd(struct sd_driver_t *self_p,
                    union sd_csd_t* csd_p);

#if 0
/**
 * Erase given block interval from start to end. Returns true if
 * successful otherwise false.
 *
 * @param[in] self_p Initialized driver object.
 * @param[in] start block address.
 * @param[in] end block address.
 *
 * @return zero(0) or negative error code.
 */
int sd_erase_blocks(struct sd_driver_t *self_p,
                    uint32_t start_block,
                    uint32_t end_block);
#endif

/**
 * Read given block from SD card.
 *
 * @param[in] self_p Initialized driver object.
 * @param[in] buf_p Buffer to read into.
 * @param[in] src_block Block to read from.
 *
 * @return Number of read bytes or negative error code.
 */
ssize_t sd_read_block(struct sd_driver_t *self_p,
                      void *dst_p,
                      uint32_t src_block);

/**
 * Write data to the SD bus.
 *
 * @param[in] self_p Initialized driver object.
 * @param[in] dst_block Block to write to.
 * @param[in] src_p Buffer to write.
 *
 * @return Number of written bytes or negative error code.
 */
ssize_t sd_write_block(struct sd_driver_t *self_p,
                       uint32_t dst_block,
                       const void *src_p);

#endif
