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
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "../include/evnc-wayland.h"
#include "../include/evnc-core.h"
#include "../include/evnc-utils.h"
#include "../include/evnc-wayland-egl.h"
#include "../include/evnc-wayland-events.h"


#define MIN_WIDTH 	0
#define MIN_HEIGHT 	0

const double timestep = 1./60;

void callback_render(
		void *data,
		struct wl_callback *wl_callback,
		uint32_t callback_data);

const static struct wl_callback_listener 
wl_callback_listener = {
	.done=callback_render
};


struct evnc_primitives primitives = {0};
struct egl_primitives eprimitives = {0};

int cleanup(struct evnc_primitives *primitives, struct egl_primitives *egl_primitives, int status){
	unbind_buffers(primitives);
	egl_cleanup(egl_primitives);
	disconnect_display(primitives);
	return status;
}

static void interrupt_handler(int signum){
	printf("(%s) signal recieved by user : %d\n", __func__, signum);
	primitives.running = 0;
}

void install_handlers(){
	struct sigaction sa;
	sa.sa_handler = interrupt_handler;
	sigemptyset(&sa.sa_mask);
	if(sigaction(SIGINT, &sa, NULL) == -1){
		printf("(%s) SIGINT handler installation failed.\n", __func__);
		return;
	}
	printf("(%s) All handlers installed.\n", __func__);
}


int render_frame(struct evnc_primitives *pPrimitives){
	uint32_t *buffer = pPrimitives->implicit_buffer;
	uint32_t width = pPrimitives->width;
	uint32_t height = pPrimitives->height;
	
	if(!buffer) return -1;

	for(int i = 0; i < height ; i++){
		for (int j=0; j<width;j++){
			buffer[width*i + j] = 0xff00ff00;
		}
	}
	
	wl_surface_damage(pPrimitives->wl_surface, 0, 0, width, height);
	wl_surface_commit(pPrimitives->wl_surface);
	
	return 0;
}

void render_egl(struct evnc_primitives *pPrimitives, struct egl_primitives *peprimitives){
		
	if(!peprimitives->initialized){
		printf("(%s) waiting for egl_initialization to finish,\n", __func__);
		return;
	
	}


	pPrimitives->timedelta += pPrimitives->timestep;

	update_uniforms(pPrimitives, peprimitives);
	egl_set_current(peprimitives);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(peprimitives->shader_program);
	glBindVertexArray(peprimitives->vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	eglSwapInterval(peprimitives->e_display, 0);
	eglSwapBuffers(peprimitives->e_display, peprimitives->e_surface);
}


void attach_callback(struct evnc_primitives *pPrimitives){
	struct wl_callback *callback;
	if (pPrimitives->wl_callback){
		pPrimitives->wl_callback = NULL;
	}


	callback = wl_surface_frame(pPrimitives->wl_surface);	
	wl_callback_add_listener(callback, &wl_callback_listener, pPrimitives);
	pPrimitives->wl_callback = callback;
	wl_surface_commit(pPrimitives->wl_surface);
	
}

void load_shaders(
		struct egl_primitives *peprimitives, 
		char const *vertex_source,
		char const *fragment_source){
	
	//static const char *vertex_source = "./share/vertex.glsl";
	//static const char *fragment_source = "./share/fragment2.glsl";
	
	printf("(%s) loading shaders...\n", __func__);
	egl_set_current(peprimitives);
	
	GLuint shader_program = egl_load_files(peprimitives->shader_program, vertex_source, fragment_source);	
	
	if (shader_program <= 0){
		printf("(%s) shader load failed.\n", __func__);
		printf("(%s) continuing to use old shader program.\n", __func__);
		return;
	}
	
	
	peprimitives->shader_program = shader_program;
	
	load_uniforms(peprimitives);
	
	printf("\e[3;32m(%s) shader load finished.\e[00m\n", __func__);
}

int check_tests(struct egl_primitives *peprimitives, 
		char const *vertex_source, 
		char const *fragment_source){
	GLuint VAO, VBO;
	
	load_shaders(peprimitives, vertex_source, fragment_source);

	VAO = egl_create_vao(peprimitives);
	printf("(%s) VAO id [%u]\n", __func__, VAO);
	VBO = egl_upload_screen_buffer(VAO);
	printf("(%s) VBO id [%u]\n", __func__, VBO);
	peprimitives->vao = VAO;	
	peprimitives->vbo = VBO;

	return 0;
}




int on_init(struct evnc_primitives *pPrimitives){
	struct egl_primitives *peprimitives = (struct egl_primitives *)pPrimitives->user_data;
	int status;	
	
	if (peprimitives->initialized){
		printf("(%s) egl_init is already there .. no need to call.\n", __func__);
		egl_resize(
			peprimitives,
			pPrimitives->width,
			pPrimitives->height
		);
		return 0;
	}
	
	status = egl_init(pPrimitives, peprimitives);

	if (status < 0) return status;

	egl_resize(
		 peprimitives,
		 pPrimitives->width,
		 pPrimitives->height);

	check_tests(peprimitives, 
		pPrimitives->vertex_source, 
		pPrimitives->fragment_source);
	render_egl(pPrimitives, peprimitives);

       
	if (pPrimitives->render){
		printf("(%s) attaching render callback.\n", __func__);
		attach_callback(pPrimitives);
	}

	
	return 0;
}

void callback_render(void *data, struct wl_callback *callback, uint32_t callback_data){
	//printf("(%s) === callback render === \n", __func__);
	struct evnc_primitives* pPrimitives = (struct evnc_primitives *)data;
	if (pPrimitives == NULL){
		printf("(%s) no pPrimitives are attached not rendering.\n", __func__);
		return;
	}
	struct egl_primitives *peprimitives = (struct egl_primitives *)pPrimitives->user_data;
	if (peprimitives == NULL){
		printf("(%s) no peprimitives are attached not rendering.\n", __func__);
		return;
	}
	
	
	wl_callback_destroy(callback);
	if (callback == pPrimitives->wl_callback) pPrimitives->wl_callback = NULL;
	
		
	render_egl(pPrimitives, peprimitives);
	if (pPrimitives->render){
		attach_callback(pPrimitives);
	}
}

int parse_commandline(struct evnc_primitives *primitives,
		int argc, char const *argv[]){
	
	int counter = 1;
	primitives->vertex_source = NULL;
	primitives->fragment_source = NULL;
	primitives->timestep = 1./60;

	while((counter < argc)){
		char const *arg = argv[counter];
		counter++;

		if((strcmp(arg, "-v") == 0) || (strcmp(arg, "--vertex") == 0)){
			if(counter >= argc){
				printf("\e[31m%s: please provide the vertex-shader path.\e[00m\n", "error");
				return -1;
			}
			primitives->vertex_source = argv[counter];
		
		}

		if((strcmp(arg, "-f") == 0) || (strcmp(arg, "--fragment") == 0)){
			
			if(counter >= argc){
				printf("\e[31m%s: please provide the fragment-shader path.\e[00m\n", "error");
				return -1;
			}
			primitives->fragment_source = argv[counter];
		
		}

		if((strcmp(arg, "-t") == 0) || (strcmp(arg, "--timestep") == 0)){
			if(counter >= argc){
				printf("\e[31m%s: please provide the speed of shader u_time.\e[00m\n", "error");
				return -1;
			}
			double timestep = strtod(argv[counter], NULL);
			primitives->timestep = (timestep > 0.0) ? 1./timestep : 1./60;
		}

		counter++;
	}

	if(primitives->vertex_source && primitives->fragment_source) return 0;
	return -1;
}

void printhelp(char const *exec_name){
	char const *help_str = "%s -v file -f -file [-s speed]\n"
		"\ta shader running tool in background for wl-roots based compositor\n"
		"\t-v / --vertex   :              vertex source path\n"
		"\t-f / --fragment :              fragment source path\n"
		"\t-t / --timestep :              frame delay of the shader u_time [default : 60.00]\n"
		"\texample:\n"
		"\t\t%s -v ./vertex.glsl -f ./fragment.glsl\n"
		"for bug-report or any suggestion please create a PR or email at bg47msva@gmail.com\n";
	printf(help_str, exec_name, exec_name);
}


int main(int argc, char const *argv[]){
	
	primitives.user_data = 		(void *)&eprimitives;
	primitives.init_callback = 	on_init;
	primitives.render = 1;
	primitives.timestep = timestep;
	int status = parse_commandline(&primitives, argc, argv);

	if(status < 0){
		printhelp(argv[0]);
		return -1;
	}

	install_handlers();

	status = connect_display(&primitives);

	if (status < 0) return cleanup(&primitives, &eprimitives, status);

	status = init_window(&primitives, MIN_WIDTH, MIN_HEIGHT);
	
	if (status < 0) return cleanup(&primitives, &eprimitives, status);	
		

	status = bind_inputs(&primitives);

	if (status < 0) return cleanup(&primitives, &eprimitives, status);

	printf("(%s) argument for wl_surface_attach is wl_surface [%p], wl_buffer [%p]\n", 
			__func__, primitives.wl_surface, primitives.wl_buffer);
		

	while (primitives.running) {
		if (event_poll(&primitives) == -1) primitives.running = 0;
		
		if (primitives.events&RELOAD_SHADER){
			printf("(%s) shader reload request accepted.\n", __func__);
			primitives.events &= ~RELOAD_SHADER;
			load_shaders(
				&eprimitives, 
				primitives.vertex_source, 
				primitives.fragment_source);
		}

		if (primitives.events&FOCUS_ON){
			primitives.render = 1;
			primitives.events &= ~FOCUS_ON;
			attach_callback(&primitives);
		}

		if (primitives.events&FOCUS_OFF){
			primitives.render = 0;
			primitives.events &= ~FOCUS_OFF;
		}

		if(primitives.events){
			printf("(%s) current event flag : 0x%08x\n", 
					__func__, primitives.events);
		
		}
		usleep(1e4);
	}

	//check_tests();
	

	return cleanup(&primitives, &eprimitives, 0);
}
