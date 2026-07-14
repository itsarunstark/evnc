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


#include "../include/evnc-wayland-events.h"
#include "../include/evnc-wayland-egl.h"
#include "../include/ext/zwlr-layer-shell-v1.h"
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <string.h>

const char *mouse_buttons[] = {
	"BTN_LEFT",
	"BTN_RIGHT",
	"BTN_MIDDLE",
	"BTN_SIDE",
	"BTN_EXTRA",
	"BTN_FORWARD",
	"BTN_BACK",
	"BTN_TASK"
};

static const struct wl_pointer_listener 
wl_pointer_listener = {
	.enter=wl_pointer_enter,
	.leave=wl_pointer_leave,
	.motion=wl_pointer_motion,
	.button=wl_pointer_button,
	.axis=wl_pointer_axis,
	.frame=wl_pointer_frame,
	.axis_source=wl_pointer_axis_source,
	.axis_stop=wl_pointer_axis_stop,
	.axis_discrete=wl_pointer_axis_discrete
};



void registry_handle_global(
		void *data,
		struct wl_registry *registry,
		uint32_t name,
		const char *interface,
		uint32_t version){

	struct evnc_primitives *primitives = (struct evnc_primitives *)data;

	if (primitives == NULL) {
		printf("(%s): primivitve argument is not passed.\n", __func__);
		return;
	}

	if (strcmp(interface, wl_compositor_interface.name) == 0){
		printf("(%s) wl_compositor with name %s.\n", __func__, interface);
		primitives->wl_compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
	}

	if (strcmp(interface, wl_shm_interface.name) == 0){
		printf("(%s) wl_shm binded with name %s.\n", __func__, interface);
		primitives->wl_shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
	}

	if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0){
		printf("(%s) zwlr_layer_shell_v1_interface found with name [%s]\n", __func__, zwlr_layer_shell_v1_interface.name);
		primitives->layer_shell = wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, 3);
		printf("(%s) zwlr_layer_shell_v1 shell %p\n", __func__, primitives->layer_shell);
	}

	if (strcmp(interface, wl_seat_interface.name) == 0){
		printf("(%s) wl_seat_interface found with name %s.\n", __func__, wl_seat_interface.name);
		primitives->wl_seat = wl_registry_bind(registry, name, &wl_seat_interface, 5);
		printf("(%s) wl_seat found at %p\n", __func__, primitives->wl_seat);
	}
	
	printf("(%s): interface [%s] name [%u] version [%u]\n", 
			__func__, interface, name, version); 
}

void wl_buffer_release(void *data, struct wl_buffer *buffer){
	struct evnc_primitives *primitives = (struct evnc_primitives *)data;
	primitives->wl_buffer = NULL;
	printf("(%s) releasing buffer %p\n", __func__, buffer);
	wl_buffer_destroy(buffer);
}

void registry_handle_global_remove(
		void *data,
		struct wl_registry *registry,
		uint32_t name){
	// TODO : implement remove logic
}

void layer_surface_configure(void *data,
		struct zwlr_layer_surface_v1 *zwlr_surface,
		uint32_t serial,
		uint32_t width,
		uint32_t height){
	
	struct evnc_primitives *primitives = (struct evnc_primitives *)data;
	int status = 0;

	primitives->width = (width > primitives->min_width) ? width : primitives->min_width;
	primitives->height = (height > primitives->min_height) ? height : primitives->min_height;
	zwlr_layer_surface_v1_ack_configure(zwlr_surface, serial);
	printf("(%s): configure request has been approved.\n", __func__);
	printf("(%s): width : [%u] x height : [%u] -- serial : [%u]\n",
		       	__func__, width, height,serial);
	printf("(%s): using buffer_size : [%ux%u]\n", __func__,
			primitives->width, primitives->height);

	if (primitives->user_data == NULL){
		printf("(%s) user_data not initialized...\n", __func__);
		return;
	}
	
	printf("(%s) user_data contains egl_primitives\n", __func__);
		
	if (primitives->init_callback) {
		status = primitives->init_callback(primitives);
		if (status < 0){
			printf("(%s) egl_init() failed.\n", __func__);
			return;
		}
		printf("(%s) first initialization completed.\n", __func__);
	}
}

void layer_surface_closed(void *data,
		struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1){
	printf("close request recieved\n");
	struct evnc_primitives *primitives = (struct evnc_primitives *)data;
	primitives->running = 0x00;
}



void seat_name(void *data, struct wl_seat *seat, const char *name){
	printf("(%s) seat name for seat %p is [%s].\n", __func__, seat, name);
}


void get_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities){
	struct evnc_primitives *primitives = (struct evnc_primitives *)data;
	if (!data){
		printf("(%s) no evnc_primitive is attached returning..\n", __func__);
		return;
	}
	if (capabilities & WL_SEAT_CAPABILITY_POINTER){
		if (primitives->wl_pointer) wl_pointer_destroy(primitives->wl_pointer);
		primitives->wl_pointer = wl_seat_get_pointer(wl_seat);
		printf("(%s) grabbed pointer with pointer id : %p\n", 
				__func__, primitives->wl_pointer);
		wl_pointer_add_listener(primitives->wl_pointer, &wl_pointer_listener, data);
	}

	if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD){
		printf("(%s) keyboard support is also present.\n", __func__);
	}

}

void wl_pointer_enter(
		void *data,
		struct wl_pointer *pointer,
		uint32_t serial,
		struct wl_surface *surface,
		wl_fixed_t surface_x,
		wl_fixed_t surface_y){

	struct evnc_primitives *primitives = (struct evnc_primitives *)data;
	printf("(%s) pointer has been entered.\n", __func__);
	if(primitives->blocking) primitives->events |= FOCUS_ON;
}

void wl_pointer_leave(
		void *data,
		struct wl_pointer *pointer,
		uint32_t serial,
		struct wl_surface *surface){

	struct evnc_primitives *primitives = (struct evnc_primitives *)data;
	printf("(%s) pointer has been left.\n", __func__);
	if(primitives->blocking) primitives->events |= FOCUS_OFF;
}


void wl_pointer_motion(
		void *data,
		struct wl_pointer *wl_pointer,
		uint32_t time,
		wl_fixed_t surface_x,
		wl_fixed_t surface_y){

	struct evnc_primitives *primitives = (struct evnc_primitives *)data;
	
	double pointer_x = wl_fixed_to_double(surface_x);
	double pointer_y = wl_fixed_to_double(surface_y);

	primitives->mouse_x = pointer_x;
	primitives->mouse_y = pointer_y;

	//printf("(%s) pointer has motion x:%0.06lf y:%0.06lf.\n", 
	//		__func__, pointer_x, pointer_y);
}


void wl_pointer_button(
		void *data,
		struct wl_pointer *wl_pointer,
		uint32_t serial,
		uint32_t time,
		uint32_t button,
		uint32_t state){

	struct evnc_primitives *primitives = (struct evnc_primitives *)data;
	printf("(%s) pointer has button press button : 0x%s state : %u\n.\n",
		       	__func__, mouse_buttons[button&0x00000007], state);
	if (button == BTN_EXTRA && state == 1){
		printf("(%s) button extra is called , reload shader is requested.\n", __func__);
		primitives->events |= RELOAD_SHADER;
	}

}


void wl_pointer_axis(
		void *data,
		struct wl_pointer *wl_pointer,
		uint32_t time,
		uint32_t axis,
		wl_fixed_t value){

	struct evnc_primitives *primitives = (struct evnc_primitives *)data;
	printf("(%s) pointer axis event axis %d.\n", __func__, axis);
}


void wl_pointer_frame(
		void *data,
		struct wl_pointer *wl_pointer){

	struct evnc_primitives *primitives = (struct evnc_primitives *)data;
	//printf("(%s) ============== pointer frame finish ==============.\n", __func__);
}


void wl_pointer_axis_source(
		void *data,
		struct wl_pointer *wl_pointer,
		uint32_t axis_source){

	struct evnc_primitives *primitives = (struct evnc_primitives *)data;
	printf("(%s) pointer axis source invoked.\n", __func__);
}



void wl_pointer_axis_stop(
		void *data,
		struct wl_pointer *wl_pointer,
		uint32_t time,
		uint32_t axis){

	struct evnc_primitives *primitives = (struct evnc_primitives *)data;
	printf("(%s) pointer_axis_stop event fired.\n", __func__);

}



void wl_pointer_axis_discrete(
		void *data,
		struct wl_pointer *wl_pointer,
		uint32_t axis,
		int32_t discrete){

	struct evnc_primitives *primitives = (struct evnc_primitives *)data;
	printf("(%s) pointer axis discrete event invoked.\n", __func__);
	

}
