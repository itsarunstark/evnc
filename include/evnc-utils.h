/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2026 Arun Kumar
 * <23u02086@iiitbhopal.ac.in>
 * <bg47msva@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#ifndef _EVNC_UTILS_H
#define _EVNC_UTILS_H
#include <stdint.h>

void randname(char * const buf);

int create_shm_file();

int allocate_shm_file(size_t size);

uint32_t evnc_random();

int read_source(const char *filename,
		char *buff,
		int32_t buffer_size,
		int32_t *pfile_size
	       );
int aquire_lock(char const *filename);
#endif // _EVNC_UTILS_H
