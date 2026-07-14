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

#include "../include/evnc-wayland-egl.h"
#include "../include/evnc-utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>


float screen_buffer_coordinates[] = {
	-0.5, -0.5, 1.0, 0.0, 0.0,
	 0.5, -0.5, 0.0, 1.0, 0.0,
	-0.5,  0.5, 0.0, 0.0, 1.0,
	-0.5,  0.5, 0.0, 0.0, 1.0,
	 0.5,  0.5, 1.0, 1.0, 1.0,
	 0.5, -0.5, 0.0, 1.0, 0.0
};

const char *uniforms[] = {
	"u_time",
	"u_resolution",
	"u_mouse"
};

enum uniform_locations{
	U_TIME = 0,
	U_RESOLUTION = 1,
	U_MOUSE = 2
};


static int check_status(EGLBoolean status){
	switch(status){
		case EGL_FALSE:
			printf("EGL_FALSE\n");
			return -1;
		case EGL_BAD_DISPLAY:
			printf("EGL_BAD_DISPLAY\n");
			return -1;
		case EGL_NOT_INITIALIZED:
			printf("EGL_NOT_INITIALIZED\n");
			return -1;
		case EGL_BAD_PARAMETER:
			printf("EGL_BAD_PARAMETER\n");
			return -1;
		case EGL_BAD_ATTRIBUTE:
			printf("EGL_BAD_ATTRIBUTE\n");
			return -1;
		default:
			printf("EGL_TRUE\n");
	}

	return 0;
}


static void logEGLConfig(EGLDisplay e_display, EGLConfig config){
	EGLint r, g, b, a, d, s;
	eglGetConfigAttrib(e_display, config, EGL_RED_SIZE, &r);
	eglGetConfigAttrib(e_display, config, EGL_GREEN_SIZE, &g);
	eglGetConfigAttrib(e_display, config, EGL_BLUE_SIZE, &b);
	eglGetConfigAttrib(e_display, config, EGL_ALPHA_SIZE, &a);
	eglGetConfigAttrib(e_display, config, EGL_DEPTH_SIZE, &d);
	eglGetConfigAttrib(e_display, config, EGL_STENCIL_SIZE, &s);

	printf("(%s) [EGL_RED_SIZE] : [%d]\n", __func__, r);
	printf("(%s) [EGL_GREEN_SIZE]: [%d]\n", __func__, g);
	printf("(%s) [EGL_BLUE_SIZE]: [%d]\n", __func__, b);
	printf("(%s) [EGL_ALPHA_SIZE]: [%d]\n", __func__, a);
	printf("(%s) [EGL_DEPTH_SIZE]: [%d]\n", __func__, d);
	printf("(%s) [EGL_STENCIL_SIZE]: [%d]\n", __func__, s);
}


int egl_create_surface(
		struct evnc_primitives *primitives,
		struct egl_primitives *eprimitives
	){

	struct wl_surface *wl_surface = primitives->wl_surface;
	if (wl_surface == NULL){
		printf("(%s) wl_surface not created.\n", __func__);
		return -1;
	}

	wl_EGLWindow egl_window = wl_egl_window_create(
			wl_surface, primitives->width,
			primitives->height);

	printf("(%s) egl_window creation request dimensions : [%dx%d]\n",
			__func__, primitives->width, primitives->height);

	if(!egl_window){
		printf("(%s): failed to create egl_window.\n", __func__);
		return -1;
	}

	EGLSurface egl_surface = eglCreateWindowSurface(
					eprimitives->e_display,
					eprimitives->e_config,
					(EGLNativeWindowType)egl_window,
					NULL
				);

	if (!egl_surface){
		printf("(%s) egl surface creation failed.\n", __func__);
		return -1;
	}

	printf("(%s) created egl_window and egl_surface successfully.\n", __func__);
	eprimitives->wl_e_window = egl_window;
	eprimitives->e_surface = egl_surface;

	return 0;
}

EGLBoolean egl_set_current(struct egl_primitives *e_primitives){
	EGLBoolean status = 
		eglMakeCurrent(
			e_primitives->e_display,
			e_primitives->e_surface,
			e_primitives->e_surface,
			e_primitives->e_context
		      );

	return status;
}

void egl_cleanup(struct egl_primitives *eprimitives){
	if (eprimitives->e_surface) 	eglDestroySurface(eprimitives->e_display, eprimitives->e_surface);
	if (eprimitives->wl_e_window)	wl_egl_window_destroy(eprimitives->wl_e_window);
	if (eprimitives->e_context) 	eglDestroyContext(eprimitives->e_display, eprimitives->e_context);
	if (eprimitives->e_display) 	eglTerminate(eprimitives->e_display);
}


void egl_get_info(){
	const char *vendor = (const char *)glGetString(GL_VENDOR);
	const char *renderer = (const char *)glGetString(GL_RENDERER);
	const char *version = (const char *)glGetString(GL_VERSION);
	const char *glslv = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);

	printf("\e[3;33m(%s) vendor : [%s]\e[00m.\n", 
			__func__, vendor);
	printf("\e[3;33m(%s) renderer : [%s].\e[00m\n", 
			__func__, renderer);
        printf("\e[3;33m(%s) version : [%s].\e[00m\n", 
			__func__, version);
	printf("\e;[3;33m(%s) glsl version : [%s]\e[00m\n", 
			__func__, glslv);	
}


int egl_init(
		struct evnc_primitives *primitives,
		struct egl_primitives *eprimitives
	){
	
	
	EGLint count, n;
	EGLint major, minor;


	EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_DEPTH_SIZE, 24,
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_NONE
	};

	EGLint context_attribs[] = {
		EGL_CONTEXT_MAJOR_VERSION, 3,
		EGL_CONTEXT_MINOR_VERSION, 3,
		EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
		EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
		EGL_NONE
	};
	
	if(!primitives->wl_display){
		printf("(%s): wl_display not initialized.\n", __func__);
		return -1;
	}

	struct wl_display *wl_display = primitives->wl_display;

	EGLDisplay e_display = eglGetDisplay((EGLNativeDisplayType)wl_display);
	
	if (e_display == EGL_NO_DISPLAY){
		printf("(%s): egl display initialization failed.\n", __func__);
		return -1;
	}

	printf("(%s): egl display is created [%p].\n", __func__, e_display);

	if (!eglInitialize(e_display, &major, &minor)){
		printf("(%s) EGL initialization failed.\n", __func__);
		return -1;
	}
	printf("(%s) eglInitialize() works version [%d.%d]\n", __func__, major, minor);
	EGLBoolean status = eglBindAPI(EGL_OPENGL_API);
	if (!status){
		printf("(%s) eglBindAPI() failed to initialize OPENGL_API [status : %u].\n", __func__, status);
	} 
	printf("(%s) eglBindAPI() finished.\n", __func__);
	
	status = eglChooseConfig(e_display, config_attribs, NULL, 0, &count);
	printf("(%s) eglGetConfigs() =>", __func__);
	
	status = check_status(status);
	if (status == -1) return -1;

	printf("(%s) found total %d EGLConfigs\n", __func__, count);
	if(count <= 0){
		printf("(%s) EGLConfig is not available on selected device ...\n", __func__);
		return -1;
	}
	
	EGLConfig *configs = (EGLConfig *)calloc(count, sizeof(EGLConfig));

	if (configs == NULL){
		printf("(%s) config allocation failed.\n", __func__);
		return -1;
	}

	status = eglChooseConfig(e_display, config_attribs, configs, count, &n);
	
	printf("(%s) eglChooseConfig()=>", __func__);
	
	status = check_status(status);
	
	if (status == -1) return -1;


	for(int i = 0 ; i < count ; i++){
		printf("(%s)----------eglconfig #0x%04x-------------\n", __func__, i);	
		logEGLConfig(e_display, configs[i]);
		printf("(%s)----------------------------------------\n", __func__);
	}

	EGLConfig egl_config = configs[0];
	free(configs);

	EGLContext egl_context = eglCreateContext(
				e_display,
				egl_config,
				EGL_NO_CONTEXT,
				context_attribs
			);
	printf("(%s) egl_context : %p\n", __func__, egl_context);
	if (!egl_context){
		printf("(%s): EGL Context creation failed.\n", __func__);
		return -1;
	}


	eprimitives->e_display = 	e_display;
	eprimitives->e_config = 	egl_config;
	eprimitives->e_context =	egl_context;


	status = egl_create_surface(primitives, eprimitives);

	if (status < 0){
		printf("(%s) egl_surface creation failed.\n", __func__);
		return status;;

	}

	EGLBoolean state = egl_set_current(eprimitives);

	if (state != EGL_TRUE){
		printf("(%s) context set failed.\n", __func__);
		return -1;
	}

	printf("(%s) context has been set.\n", __func__);

	if (!gladLoadEGLLoader((GLADloadproc)eglGetProcAddress)) {
		printf("(%s) glad initialization failed.\n", __func__);
		return -1;
	}

	eprimitives->initialized = 1;

	egl_get_info();

	return 0;
}



GLuint egl_load_shader(const char *filename, GLenum shadertype){
	int status = 0;
	GLint shader = 0;
	GLint source_size;
	char *buffer = NULL;
	static char compile_logs[250];
	//GLint log_size;


	shader = glCreateShader(shadertype);
	status = read_source(filename, NULL, 0, &source_size);
	
	if (status < 0){
		printf("(%s) file_read failed for %s.\n", __func__, filename);
		glDeleteShader(shader);
		return -1;
	}

	if (source_size <= 0){
		printf("(%s) source size is less than 0 as %s\n", __func__, filename);
		glDeleteShader(shader);
		return -1;
	}

	buffer = (char *)calloc(source_size, 1);
	const char *buff = buffer;

	if (buffer == NULL){
		printf("(%s) buffer allocation failed %s.\n", __func__, filename);
		glDeleteShader(shader);
		return -1;
	}

	status = read_source(filename, buffer, source_size, &source_size);
	if (status < 0){
		printf("(%s) file_read failed for %s\n", __func__, filename);
		glDeleteShader(shader);
		free(buffer);
		return 0;
	}

	glShaderSource(shader, 1, &buff, &source_size);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (!status){
		printf("(%s) shader compilation failed for %s\n", __func__, filename);
		glGetShaderInfoLog(shader, 250, NULL, compile_logs);
		printf("(%s) ------- compiled logs------------\n", __func__);
	       	printf("%s\n", compile_logs);
		printf("(%s) ---------------------------------\n", __func__);

		glDeleteShader(shader);
		free(buffer);
		
		return 0;
	}

	printf("(%s) shader_compilation finished for %s. \n" , __func__, filename);
	
	free(buffer);
	return shader;
}



GLuint egl_link_program(GLuint vertex_shader, GLuint fragment_shader, GLuint shader_program){
	
	static char link_logs[250];
	int status = 0;

	if (shader_program == 0) {
		shader_program = glCreateProgram();
		printf("(%s) shader program is not present created new shader program [%u].\n",
				__func__, shader_program);
	}

	else {
		printf(
		"(%s) shader_program %u is already present no need to create new shader program",
			__func__, shader_program);
	}


	if (shader_program == 0){
		printf("(%s) shader program failed to initialize (glCreateProgram()=>0)\n", __func__);
		return 0;
	}

	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);
	glGetProgramiv(shader_program, GL_LINK_STATUS, &status);
	
	if (!status){
		printf("(%s) shader linking failed.\n", __func__);
		glGetProgramInfoLog(shader_program, 250, NULL, link_logs);
		printf("(%s) ===== link logs ======\n%s\n==================\n", __func__, link_logs);
		glDeleteProgram(shader_program);
		return 0;
	}

	glDetachShader(shader_program, vertex_shader);
	glDetachShader(shader_program, fragment_shader);

	return shader_program;
}

void load_uniforms(struct egl_primitives *eprimitives){
	GLuint shader_program = eprimitives->shader_program;
	if (shader_program <= 0) return;

	eprimitives->u_mouse = glGetUniformLocation(shader_program, 
							uniforms[U_MOUSE]);
	eprimitives->u_time = glGetUniformLocation(shader_program,
							uniforms[U_TIME]);
	eprimitives->u_resolution = glGetUniformLocation(shader_program,
							uniforms[U_RESOLUTION]);

	printf("(%s) uniform location [%s]:%d\n", __func__, 
			uniforms[U_MOUSE], eprimitives->u_mouse);
	printf("(%s) uniform location [%s]:%d\n", __func__, 
			uniforms[U_TIME],  eprimitives->u_time);
	printf("(%s) uniform location [%s]:%d\n", __func__, 
			uniforms[U_RESOLUTION], eprimitives->u_resolution);
}


void update_uniforms(
		struct evnc_primitives *primitives,
		struct  egl_primitives *eprimitives){
	float width = 	1.0*primitives->width;
	float height =  1.0*primitives->height;
	float mouse_x = 1.0*primitives->mouse_x;
	float mouse_y = 1.0*primitives->mouse_y;
	float current_time = 
			1.0*primitives->timedelta;

	if (eprimitives->u_mouse >= 0) 
		glUniform2f(eprimitives->u_mouse, mouse_x, mouse_y);
	if (eprimitives->u_time >= 0)
		glUniform1f(eprimitives->u_time, current_time);
	if (eprimitives->u_resolution >= 0)
		glUniform2f(eprimitives->u_resolution, width, height);
}

void egl_clean_shader(GLuint shader){
	if (shader > 0) glDeleteShader(shader);
}

void egl_clean_program(GLuint program){
	if (program > 0) glDeleteProgram(program);
}


GLuint egl_create_vao(struct egl_primitives *eprimitives){
	GLuint VAO;
	egl_set_current(eprimitives);
	glGenVertexArrays(1, &VAO);
	return VAO;
}

GLuint egl_upload_mesh(GLuint VAO, float *mesh, size_t mesh_size, uint32_t n, int attribute_location){
	GLuint VBO;
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, mesh_size, (void *)mesh, GL_STATIC_DRAW);
	glVertexAttribPointer(
			attribute_location,	// attribute location (in 0)
		       	n, 			// total number of parameters ? (vec2)
			GL_FLOAT, 		// data type of buffer (float)
			GL_FALSE,		// normalisation required ? (no)
			sizeof(float)*5,	// stride (8 bytes)
			(void *)0		// offset from original array (none)
	);

	glVertexAttribPointer(
			attribute_location+1,
			3,
			GL_FLOAT,
			GL_FALSE,
			sizeof(float)*5,
			(void *)(sizeof(float)*n)
	);


	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(0);
	return VBO;
}


GLuint egl_upload_screen_buffer(GLuint VAO){
	return egl_upload_mesh(
		VAO,
		screen_buffer_coordinates,
		sizeof(screen_buffer_coordinates),
		2,0);
}

void egl_destroy_vao(GLuint vao){
	glDeleteVertexArrays(1, &vao);
}


GLuint egl_load_files(
		GLuint shader_program,
		const char *vertex_source,
		const char *fragment_source){

	GLuint vertex_shader, fragment_shader;
	
	vertex_shader = egl_load_shader(vertex_source, GL_VERTEX_SHADER);
	if (vertex_shader <= 0){
		printf("(%s) vertex shader failed to load.\n", __func__);
		return 0;
	}
	
	fragment_shader = egl_load_shader(fragment_source, GL_FRAGMENT_SHADER);
	if (fragment_shader <= 0){
		printf("(%s) fragment shader failed to load.\n", __func__);
		glDeleteShader(vertex_shader);
		return 0;
	}
	
	shader_program = egl_link_program(
				vertex_shader,
				fragment_shader,
				shader_program
			);

	printf("(%s) cleaning cached compiled shader objects.\n", __func__);
	egl_clean_shader(vertex_shader);
	egl_clean_shader(fragment_shader);

	if (shader_program <= 0){
		return 0;
	}

	return shader_program;
}

void egl_resize(
		struct egl_primitives *eprimitives,
		uint32_t width, uint32_t height){
	wl_egl_window_resize(eprimitives->wl_e_window, width, height, 0, 0);
	glViewport(0, 0, width, height);
	// TODO update dimensions of uniforms
}







