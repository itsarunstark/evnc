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


#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <unistd.h>
#include <errno.h>
#include "../include/evnc-utils.h"


void randname(char * const buf){
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	long r = ts.tv_nsec;
	for(int i = 0; i < 6 ; i++){
		buf[i] = 'A' + (r&15) + (r&16)*2;
		r >>= 5;
	}
}

uint32_t evnc_random(){
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	long r = ts.tv_nsec;
	return r&0xffffffff;
}


int aquire_lock(const char *filename){
	int fd = open(filename, O_RDWR |O_CREAT, S_IRUSR|S_IWUSR);
	if(fd == -1){
		printf("%s: error failed to create lock_file.\n", __func__);
		return -1;
	}

	if(flock(fd, LOCK_EX | LOCK_NB) == -1){
		if(errno == EWOULDBLOCK){
			printf("\e[31m%s:error:  another evnc instance is running\e[00m.\n", __func__);
		}
		printf("%s: failed to obtain lock_file.\n", __func__);
		close(fd);
		return -1;
	}


	ftruncate(fd, 0);
	dprintf(fd, "%d\n", getpid());
	return fd;
}



int read_source(const char *filename, char *buff, int32_t buffer_size, int32_t *pfile_size){
	FILE *fp = fopen(filename, "rb");
	if (!fp) return -1;

	if (fseek(fp, 0l, SEEK_END) != 0){
		printf("(%s) fseek failed to set the SEEK_END.\n", __func__);
		fclose(fp);
		return -1;
	};

	int32_t file_size = ftell(fp);
	if (file_size == 0){
		printf("(%s) file_size of file : %s is 0.\n", __func__, filename);
		fclose(fp);
		return -1;
	}

	if(fseek(fp, 0L, SEEK_SET) != 0){
		printf("(%s) file seek failed to set SEEK_SET.\n", __func__);
		fclose(fp);
		return -1;
	}

	*pfile_size = file_size;
	
	if (buff == NULL) {
		fclose(fp);
		return 0;
	}


	int32_t read_size = file_size > buffer_size ? buffer_size : file_size;
	printf("(%s) [%s read-size : %u].\n", __func__, filename, read_size);

	while(read_size > 0){
		size_t bytes_read = fread(buff, 1, read_size, fp); 
		buff += bytes_read;
		read_size -= bytes_read;
	}

	fclose(fp);

	return 0;
}

int create_shm_file(){
	int retries = 100;
	int fd = -1;
	char name[] = "/evnc-shm-XXXXXX";
	do{
		randname(name + sizeof(name) - 7);
		printf("(%s) trying to create shm_file named (%s).\n",__func__, name); 
		fd = shm_open(name, O_CREAT|O_RDWR|O_EXCL, 0600);
		retries--;
		if (fd >= 0 ) {
			shm_unlink(name);
			return fd;
		}
	} while (retries && errno == EEXIST);
	return -1;
}

int allocate_shm_file(size_t filesize){
	int fd = create_shm_file();
	if (fd < 0) return -1;
	int ret;
	do{
		ret = ftruncate(fd, filesize);
	} while (ret < 0 && errno == EINTR);
	if (ret < 0) {
		close(fd);
		return -1;
	}
	return fd;
}






