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

#include <wayland-client.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <error.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/file.h>
#include "../include/evnc-wayland.h"
#include "../include/evnc-core.h"
#include "../include/evnc-wayland-events.h"
#include "../include/evnc-utils.h"
#define DISPLAY_NAME NULL

static const char * const db_lock_file = "/tmp/evnc-db.lock";
static const char * const boot_id_file = "/proc/sys/kernel/random/boot_id";
	
struct evnc_primitives v;

static const struct wl_registry_listener
registry_listener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove,
};

static const struct wl_buffer_listener
wl_buffer_listener = {
	.release=wl_buffer_release,
};

static const struct wl_seat_listener
wl_seat_listener = {
	.name=seat_name,
	.capabilities=get_capabilities
};

static const struct zwlr_layer_surface_v1_listener zwlr_layer_surface_listener = {
	.configure=layer_surface_configure,
	.closed=layer_surface_closed
};


int connect_display(struct evnc_primitives *primitives){
	int fd = aquire_lock(db_lock_file);
	primitives->lock_fd = fd;
	if(fd < 0) return -1;
	primitives->wl_display = wl_display_connect(DISPLAY_NAME);
	if (!primitives->wl_display) {
		printf("(%s) display connection failed ... exiting ...\n", __func__);
		return -1;
	}
	printf("(%s) display connected ...\n", __func__);
	return 0;
}


void disconnect_display(struct evnc_primitives *primitives){
	struct wl_registry *registry = 		
					primitives->wl_registry;
	
	struct wl_display *display = 		
					primitives->wl_display;
	
	struct wl_compositor *wl_compositor = 	
					primitives->wl_compositor;
	
	struct zwlr_layer_shell_v1 *layer_shell = 
					primitives->layer_shell;
	
	struct zwlr_layer_surface_v1 *zwlr_layer_surface = 
					primitives->zwlr_layer_surface;
	
	struct wl_surface *wl_surface = 
					primitives->wl_surface;

	struct wl_seat	*wl_seat = 
					primitives->wl_seat;
	struct wl_pointer *wl_pointer = 
					primitives->wl_pointer;

	struct wl_callback *wl_callback =
	       				primitives->wl_callback;	

	if (wl_callback) wl_callback_destroy(wl_callback);	
	if (wl_pointer){
		wl_pointer_destroy(wl_pointer);
		printf("(%s) destroyed wl_pointer interface [%p].\n", __func__, wl_pointer);
	}

	
	if (wl_seat){
		wl_seat_destroy(wl_seat);
		printf("(%s) destroyed wl_seat interface [%p].\n", __func__, wl_seat);
	}

	if (zwlr_layer_surface){
		zwlr_layer_surface_v1_destroy(zwlr_layer_surface);
		printf("(%s): destroyed zwlr_layer_surface_v1 [%p].\n", __func__, zwlr_layer_surface);
	}

	if (wl_surface){
		wl_surface_destroy(wl_surface);
		printf("(%s): destroyed the wl_surface [%p].\n", __func__, wl_surface);
	}

	if (layer_shell){
		zwlr_layer_shell_v1_destroy(layer_shell);
		printf("(%s): destroyed zwlr_layer_shell_v1 [%p].\n", __func__, layer_shell);
	}

	if(wl_compositor){
		wl_compositor_destroy(wl_compositor);
	}
	if (registry){
		wl_registry_destroy(registry);
		printf("(%s) destroyed registry interface [%p].\n", __func__, registry);
	}


	if(primitives->lock_fd > -1) close(primitives->lock_fd);
	if (display){
		wl_display_disconnect(display);
		printf("(%s) : disconnected display %p\n", __func__, display);
		return;
	}


	printf("(%s) : display not initialized... ignoring request\n", __func__);


}


int create_surfaces(struct evnc_primitives *primitives){
	
	struct wl_surface *wl_surface = NULL;
	struct zwlr_layer_surface_v1 *zwlr_layer_surface = NULL;
	struct wl_compositor *compositor = primitives->wl_compositor;
	struct zwlr_layer_shell_v1 *layer_shell = primitives->layer_shell;
	enum zwlr_layer_shell_v1_layer layer_role = ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND;
	//struct wl_region *wl_region;
	
	printf("(%s) passed primitive pointer %p\n", __func__, primitives);
	
	wl_surface = wl_compositor_create_surface(compositor);
	//wl_surface2 = wl_compositor_create_surface(compositor);

	if(!wl_surface){
		printf("(%s): wl_surface creation failed.\n", __func__);
		return -1;
	}

	printf("(%s): wl_surface created [%p] layer_shell [%p].\n", __func__, wl_surface, layer_shell);
	
	
	//wl_region = wl_compositor_create_region(compositor);
	//wl_surface_set_input_region(wl_surface, wl_region);

	zwlr_layer_surface = zwlr_layer_shell_v1_get_layer_surface(
			layer_shell, wl_surface,
			NULL, layer_role, "random_window");
	
	//struct zwlr_layer_surface_v1 *zwlr_layer_surface2 = zwlr_layer_shell_v1_get_layer_surface(
	//		layer_shell, wl_surface2,
	//		NULL, layer_role, "random_window1");

	if(!zwlr_layer_surface){
		printf("(%s): zwlr_layer_surface creation failed.\n", __func__);
		return -1;
	}

	printf("(%s) wl_layer_surface -> zwlr_layer_surface done.\n", __func__);
	//zwlr_layer_surface_v1_set_size(zwlr_layer_surface2, primitives->min_width, primitives->min_height);
	zwlr_layer_surface_v1_set_exclusive_zone(zwlr_layer_surface, -1);
	zwlr_layer_surface_v1_set_anchor(zwlr_layer_surface, 
			ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM	|
			ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT	|
			ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT	|
			ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP	);
	zwlr_layer_surface_v1_add_listener(zwlr_layer_surface, &zwlr_layer_surface_listener, primitives);
	//zwlr_layer_surface_v1_add_listener(zwlr_layer_surface2, &zwlr_layer_surface_listener, primitives);
	
	primitives->wl_surface = wl_surface;
	//primitives->wl_surface2 = wl_surface2;
	primitives->zwlr_layer_surface = zwlr_layer_surface;
	//primitives->zwlr_layer_surface2 = zwlr_layer_surface2;
	printf("(%s): commited the surfaces\n", __func__);
	return 0;
}



int init_window(struct evnc_primitives *primitives, uint32_t min_width, uint32_t min_height){
	
	printf("(%s) : grabbing the wayland_display fd.\n", __func__);
	
	int fd = wl_display_get_fd(primitives->wl_display);
	
	if (fd < 0){
		printf("(%s) : failed to grab the file descriptor\n", __func__);
		return -1;
	}
	
	primitives->running = 		0x01;
	primitives->wl_fd = 		fd;
	primitives->min_width = 	min_width;
	primitives->min_height = 	min_height;
	primitives->width = 		min_width;
	primitives->height = 		min_height;

	printf("(%s) : grabbed [wayland] file descriptor for display %d\n", __func__, fd);
	struct wl_registry* registry = wl_display_get_registry(primitives->wl_display);

	if (registry == NULL){
		printf("(%s) : failed to get [wayland] display registry.\n", __func__);
		return -1;
	}
	primitives->wl_registry = registry;

	wl_registry_add_listener(registry, &registry_listener, (void *)primitives);
	wl_display_roundtrip(primitives->wl_display);
	
	if(create_surfaces(primitives) == -1){
		printf("(%s): surface creation failed.\n", __func__);
		return -1;
	}

	printf("(%s) : surface creation finished.\n", __func__);

	wl_surface_commit(primitives->wl_surface);
	wl_display_flush(primitives->wl_display);

	return 0;
}



int event_poll(struct evnc_primitives *primitives){
	int status = 0;
	while(wl_display_flush(primitives->wl_display));
	
	/*while(wl_display_prepare_read(primitives->wl_display) != 0){
		wl_display_dispatch_pending(primitives->wl_display);
		wl_display_flush(primitives->wl_display);
	}*/

	//printf("event poll..\n");

	if (primitives->running && primitives->wl_fd > 0){
		int fd = primitives->wl_fd;
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		struct timeval tv = {0};
		int ret = select(fd + 1, &rfds, NULL, NULL, &tv);
		if (ret > 0 && FD_ISSET(fd, &rfds)){
			status = wl_display_dispatch(primitives->wl_display);
		}
	}

	return status;
}

int bind_inputs(struct evnc_primitives *primitives){
	if (primitives->wl_seat)
		wl_seat_add_listener(primitives->wl_seat, &wl_seat_listener, primitives);
	return 0;
}


void unbind_buffers(struct evnc_primitives *primitives){
	// TODO: implement a universal wayland buffer unbind logic.
	
	struct wl_shm *wl_shm = 	primitives->wl_shm;
	struct wl_buffer *wl_buffer = 	primitives->wl_buffer;
	uint32_t *implicit_buffer = 	primitives->implicit_buffer;
	size_t buffer_size = 		primitives->buffer_size;

        if (implicit_buffer) {
		primitives->implicit_buffer = NULL;
		munmap((void *)implicit_buffer, buffer_size);
	}
	if (wl_shm) wl_shm_destroy(wl_shm);
       	if (wl_buffer) wl_buffer_destroy(wl_buffer);
}


int bind_buffers(struct evnc_primitives *primitives){
	if (primitives->wl_shm == NULL){
		printf("(%s) primitives does not have any shm handle linked.\n", __func__);
		return -1;
	}

	const uint32_t width = primitives->width;
	const uint32_t height = primitives->height;
	const uint32_t stride = width*4;
	const size_t size = stride*height;

	int fd = allocate_shm_file(size);

	printf("(%s) trying to allocate %lu bytes for screen buffer \n", __func__, size);
	
	uint32_t *buffer_raw = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	printf("(%s) allocation status %p\n", __func__, buffer_raw);

	if (buffer_raw == MAP_FAILED){
		close(fd);
		return -1;
	}

	struct wl_shm_pool *wl_shm_pool = wl_shm_create_pool(primitives->wl_shm, fd, size);
	struct wl_buffer *buffer = wl_shm_pool_create_buffer(
			wl_shm_pool, 0,
			width, height ,
			stride, WL_SHM_FORMAT_XRGB8888
			);
	
	wl_shm_pool_destroy(wl_shm_pool);
	close(fd);

	for(int i = 0; i < height; i++){
		for(int j = 0 ; j < width; j++){
			buffer_raw[i*width + j] = 0xFFFFFFFF;
		}
	}

	primitives->implicit_buffer = buffer_raw;
	primitives->buffer_size = size;
	wl_buffer_add_listener(buffer, &wl_buffer_listener, primitives);
	primitives->wl_buffer = buffer;
	printf("(%s) created wl_buffer at %p\n", __func__, primitives->wl_buffer);
	return 0;
}


int bind_buffers_egl(struct evnc_primitives *primitives, struct egl_primitives *eprimitives){
	return 0;
}

void unbind_buffers_egl(struct evnc_primitives *primitives, struct egl_primitives *eprimitives){

}

