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


void check_tests(){
	const char *filename = "./share/zwlr-layer-shell-v1.h";
	size_t file_size = 0;
	uint8_t *buffer;
	
	int status = read_source(filename, NULL, 0, &file_size);
	
	printf("(%s) : fileread status: %d and file_size : %lu.\n", __func__, status, file_size);
	
	if (status < 0) return;

	buffer = calloc(file_size, 1);
	
	if (!buffer){
		printf("(%s) buffer is failed to allocate.\n", __func__);
		return;
	}
	
	status = read_source(filename, buffer, file_size, &file_size);
	
	printf("(%s) status => %d\n", __func__, status);
	
	if (status < 0){
		printf("(%s) file is not read.\n", __func__);
		return;
	}

	printf("(%s) ----------- file content : -----------\n%s\n", __func__, buffer);

	free(buffer);
}


