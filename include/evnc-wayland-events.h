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

#ifndef _EVNC_WAYLAND_EVENTS
#define _EVNC_WAYLAND_EVENTS
#include <stdint.h>
#include "./evnc-wayland.h"


enum evnc_events{
	RELOAD_SHADER = 0x00000001,
	FOCUS_ON = 0x00000002,
	FOCUS_OFF = 0x00000004
};


void registry_handle_global(
		void *data,
		struct wl_registry *registry,
		uint32_t name,
		const char *interface,
		uint32_t version);

void registry_handle_global_remove(
		void *data,
		struct wl_registry *registry,
		uint32_t name);

void layer_surface_configure(void *data,
		struct zwlr_layer_surface_v1 *zwlr_surface,
		uint32_t serial,
		uint32_t width,
		uint32_t height);

void layer_surface_closed(
		void *data, 
		struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1);

void wl_buffer_release(void *data, struct wl_buffer *buffer);

void seat_name(
		void *data,
		struct wl_seat *seat,
		const char *name
);

void get_capabilities(
		void *data,
		struct wl_seat *seat,
		uint32_t capabilities
);

//int bind_inputs(struct evnc_primitives *primitives);

void wl_pointer_enter(
		void *data,
		struct wl_pointer *pointer,
		uint32_t serial,
		struct wl_surface *surface,
		wl_fixed_t surface_x,
		wl_fixed_t surface_y);

void wl_pointer_leave(
		void *data,
		struct wl_pointer *pointer,
		uint32_t serial,
		struct wl_surface *surface);

void wl_pointer_motion(
		void *data,
		struct wl_pointer *wl_pointer,
		uint32_t time,
		wl_fixed_t surface_x,
		wl_fixed_t surface_y);

void wl_pointer_button(
		void *data,
		struct wl_pointer *wl_pointer,
		uint32_t serial,
		uint32_t time,
		uint32_t button,
		uint32_t state);

void wl_pointer_axis(
		void *data,
		struct wl_pointer *wl_pointer,
		uint32_t time,
		uint32_t axis,
		wl_fixed_t value);

void wl_pointer_frame(
		void *data,
		struct wl_pointer *wl_pointer);

void wl_pointer_axis_source(
		void *data,
		struct wl_pointer *wl_pointer,
		uint32_t axis_source);

void wl_pointer_axis_stop(
		void *data,
		struct wl_pointer *wl_pointer,
		uint32_t time,
		uint32_t axis);

void wl_pointer_axis_discrete(
		void *data,
		struct wl_pointer *wl_pointer,
		uint32_t axis,
		int32_t discrete);
#endif // _EVNC_WAYLAND_EVENTS
