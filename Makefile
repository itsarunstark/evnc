# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2026 Arun Kumar
# <23u02086@iiitbhopal.ac.in>
# <bg47msva@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of version 2 of the GNU General Public License
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#

CC=gcc
CFLAGS= -g -Wall -I ./
LDFLAGS= -lwayland-client -lm -lEGL -lwayland-egl -lGL 
BIN=./bin
SRC=./src
LIB=./lib
EXT=ext
PREFIX=/usr/local


target:	${BIN}/evnc
	echo "Build Finished!"

install:
	cp -rv ${BIN}/evnc ${PREFIX}/${BIN}/evnc

uninstall:
	rm -rv ${PREFIX}/bin/evnc


${BIN}/evnc : pre-create \
		${LIB}/evnc.o \
		${LIB}/evnc-wayland.o \
		${LIB}/evnc-wayland-events.o \
		${LIB}/evnc-utils.o \
		${LIB}/${EXT}/zwlr-layer-shell-v1.o \
		${LIB}/${EXT}/xdg-shell.o \
		${LIB}/evnc-wayland-egl.o \
		${LIB}/${EXT}/glad_egl.o
	${CC} ${LDFLAGS} 				\
		${LIB}/evnc.o 				\
		${LIB}/evnc-wayland.o 			\
		${LIB}/evnc-wayland-events.o 		\
		${LIB}/evnc-utils.o			\
		${LIB}/evnc-wayland-egl.o		\
		${LIB}/${EXT}/zwlr-layer-shell-v1.o	\
		${LIB}/${EXT}/xdg-shell.o		\
		${LIB}/${EXT}/glad_egl.o		\
		-o $@

pre-create:
	mkdir -pv ./bin
	mkdir -pv ./lib
	mkdir -pv ./lib/ext


${LIB}/evnc-wayland.o:	${SRC}/evnc-wayland.c
	${CC} ${CFLAGS} -c $? -o $@

${LIB}/evnc-wayland-events.o:	${SRC}/evnc-wayland-events.c
	${CC} ${CFLAGS} -c $? -o $@

${LIB}/${EXT}/zwlr-layer-shell-v1.o:	${SRC}/${EXT}/zwlr-layer-shell-v1.c
	${CC} ${CFLAGS} -c $? -o $@

${LIB}/${EXT}/glad_egl.o:	${SRC}/${EXT}/glad_egl.c
	${CC} ${CFLAGS} -c $? -o $@

${LIB}/${EXT}/xdg-shell.o:	${SRC}/${EXT}/xdg-shell.c
	${CC} ${CFLAGS} -c $? -o $@

${LIB}/evnc.o:	${SRC}/evnc.c
	${CC} ${CFLAGS} -c $? -o $@


${LIB}/evnc-utils.o:	${SRC}/evnc-utils.c
	${CC} ${CFLAGS} -c $? -o $@

${LIB}/evnc-wayland-egl.o:	${SRC}/evnc-wayland-egl.c
	${CC} ${CFLAGS} -c $? -o $@

clean:
	rm -rf ${BIN}/*
	rm -rf ${LIB}/*
	rm -r ${BIN}
	rm -r ${LIB}

run:	${BIN}/evnc
	exec ${BIN}/evnc -f ./share/fragment2.glsl -v ./share/vertex.glsl -t 75.00

test:	${BIN}/evnc
	valgrind ${BIN}/evnc

.PHONY:	clean
