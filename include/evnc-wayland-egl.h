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


#ifndef _EVNC_WAYLAND_EGL_H
#define _EVNC_WAYLAND_EGL_H
#define GLEW_EGL

#include <EGL/eglplatform.h>
#include <wayland-client.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-egl.h>
#include <wayland-egl-core.h>
#include "./evnc-core.h"
#include "./glad/glad_egl.h"
#include <GLES3/gl3.h>

typedef struct wl_egl_window * wl_EGLWindow;

struct egl_primitives{
	EGLDisplay 	e_display;
	EGLConfig 	e_config;
	EGLContext 	e_context;
	EGLSurface 	e_surface;
	wl_EGLWindow 	wl_e_window;
	uint32_t 	initialized;
	GLuint 		shader_program;
	GLuint 		vao, vbo;
	GLint 		u_mouse;
	GLint 		u_time;
	GLint 		u_resolution;
};

typedef struct egl_primitives * EGLPrimitive;

int egl_init(EVNCPrimitives primitives, EGLPrimitive eprimitive);
void egl_cleanup(struct egl_primitives *eprimitives);
EGLBoolean egl_set_current(struct egl_primitives *e_primitives);
int egl_create_surface(
		struct evnc_primitives *primitives,
		struct egl_primitives *eprimitives
	);

void egl_resize(
		struct egl_primitives *eprimitives,
		uint32_t width, uint32_t height);

GLuint egl_load_shader(const char *filename, GLenum shadertype);

GLuint egl_link_program(
		GLuint vertex_shader, 
		GLuint fragment_shader, 
		GLuint shader_program
		);

void
	egl_clean_shader(GLuint shader);
void 
	egl_clean_program(GLuint program);
GLuint 
	egl_load_files(
		GLuint shader_program,
		const char *vertex_source,
		const char *fragment_source
		);

GLuint
	egl_create_vao(struct egl_primitives *eprimitives);

GLuint
	egl_upload_mesh(
		GLuint VAO,
		float *mesh,
		size_t mesh_size,
		uint32_t n,
		int attribute_location);

GLuint
	egl_upload_screen_buffer(GLuint VAO);

void load_uniforms(struct egl_primitives *eprimitives);

void update_uniforms(
		struct evnc_primitives *primitives,
		struct egl_primitives *eprimitives
		);

#endif // _EVNC_WAYLAND_EGL_H
