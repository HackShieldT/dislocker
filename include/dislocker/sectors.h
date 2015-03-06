/* -*- coding: utf-8 -*- */
/* -*- mode: c -*- */
/*
 * Dislocker -- enables to read/write on BitLocker encrypted partitions under
 * Linux
 * Copyright (C) 2012-2013  Romain Coltel, Hervé Schauer Consultants
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */
#ifndef SECTORS_H
#define SECTORS_H

#include <pthread.h>

#include "dislocker/metadata/datums.h"
#include "dislocker/metadata/metadata.h"
#include "dislocker/encryption/encommon.h"




/**
 * Structure used for operation on disk (encryption/decryption)
 */
typedef struct _data {
	/* BitLocker-volume's metadata */
	bitlocker_information_t* information;
	
	/* The volume header, 512 bytes */
	volume_header_t* volume_header;
	
	/* The VMK */
	datum_key_t* vmk;
	
	/* The FVEK */
	datum_key_t* fvek;
	
	/*
	 * Virtualized regions are presented as zeroes when queried from the NTFS
	 * layer. In these virtualized regions, we find the 3 BitLocker metadata
	 * headers, the area where NTFS boot sectors are backed-up for W$ 7&8, and
	 * an area I don't know about yet for W$ 8.
	 * This last area is used only when BitLocker's state is 2.
	 */
	size_t              nb_virt_region;
	dis_regions_t       virt_region[5];
	
	/* Size (in bytes) of the NTFS backed-up sectors */
	off_t               virtualized_size;
	
	/* Extended info which may be present (NULL otherwise) */
	extended_info_t*    xinfo;
	
	/* Where the real partition begins */
	off_t               part_off;
	/* Volume sector size */
	uint16_t            sector_size;
	/* Volume size, in bytes */
	uint64_t            volume_size;
	/* File descriptor to access the volume */
	int                 volume_fd;
	
	/* Structure used to encrypt or decrypt */
	dis_crypt_t         crypt;
	
	/* Volume's state is kept here */
	int                 volume_state;
	
	/* Function to decrypt a region of the volume */
	int(*decrypt_region)(
		struct _data* io_data,
		size_t nb_read_sector,
		uint16_t sector_size,
		off_t sector_start,
		uint8_t* output
	);
	/* Function to encrypt a region of the volume */
	int(*encrypt_region)(
		struct _data* io_data,
		size_t nb_write_sector,
		uint16_t sector_size,
		off_t sector_start,
		uint8_t* input
	);
	
	/*
	 * FUSE uses threads. We need to protect our "lseek/read" and "lseek/write"
	 * sequences
	 */
	pthread_mutex_t     mutex_lseek_rw;
} dis_iodata_t;



/*
 * Functions prototypes
 */
int read_decrypt_sectors(
	dis_iodata_t* io_data,
	size_t nb_read_sector,
	uint16_t sector_size,
	off_t sector_start,
	uint8_t* output
);
int encrypt_write_sectors(
	dis_iodata_t* io_data,
	size_t nb_write_sector,
	uint16_t sector_size,
	off_t sector_start,
	uint8_t* input
);

#endif /* SECTORS_H */
