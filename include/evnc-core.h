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


#ifndef _EVNC_CORE_H
#define _EVNC_CORE_H

#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include "ext/zwlr-layer-shell-v1.h"
#include <stdint.h>


struct evnc_primitives{
	struct wl_display 		*wl_display;
	struct wl_registry 		*wl_registry;
	struct wl_compositor 		*wl_compositor;
	struct wl_shm 			*wl_shm;
	struct wl_buffer 		*wl_buffer;
	struct wl_surface 		*wl_surface;
	struct wl_surface		*wl_surface2;
	struct wl_pointer 		*wl_pointer;
	struct wl_seat 			*wl_seat;
	struct zwlr_layer_shell_v1 	*layer_shell;
	struct zwlr_layer_surface_v1 	*zwlr_layer_surface, *zwlr_layer_surface2;
	struct wl_region 		*wl_pointer_region;
	struct wl_callback		*wl_callback;
	int 				running;
	int 				wl_fd;
	uint32_t 			*implicit_buffer;
	size_t 				buffer_size;
	int 				bind_buffer;
	uint32_t 			width, height, min_width, min_height;
	void 				*user_data;
	int (*init_callback)(struct evnc_primitives *primitives);
	double mouse_x, mouse_y, timestep, timedelta;
	char const *vertex_source;
	char const *fragment_source;
	uint32_t events;
	uint32_t render;
	int lock_fd;
};

typedef struct evnc_primitives * EVNCPrimitives;

#endif //_EVNC_CORE_H
