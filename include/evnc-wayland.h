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


#ifndef _EVNC_WAYLAND
#define _EVNC_WAYLAND
#include "./evnc-core.h"
#include "./evnc-wayland-egl.h"

int connect_display(struct evnc_primitives *);
void disconnect_display(struct evnc_primitives *);
int init_window(struct evnc_primitives *, uint32_t min_width, uint32_t min_height);
int event_poll(struct evnc_primitives *);
int bind_buffers(struct evnc_primitives *);
void unbind_buffers(struct evnc_primitives *);
int bind_buffers_egl(struct evnc_primitives *, struct egl_primitives *);
void unbind_buffers_egl(struct evnc_primitives *, struct egl_primitives *);
int bind_inputs(struct evnc_primitives *);

#endif // _EVNC_WAYLAND
