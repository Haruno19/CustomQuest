SRC = src/main.cpp
CC = g++
CFLAGS = -std=c++17
INSTALL_DIR = /usr/local/share
BIN_DIR = /usr/local/bin

all: customquest

customquest: 
	${CC} ${CFLAGS} ${SRC} -o customquest

install: customquest
	rm -rf ${INSTALL_DIR}/customquest
	mkdir -p ${INSTALL_DIR}/customquest
	cp -rf ./res/* ${INSTALL_DIR}/customquest/
	chmod 711 customquest
	cp -rf customquest ${INSTALL_DIR}/customquest/customquest
	rm -rf ${BIN_DIR}/customquest
	ln -s ${INSTALL_DIR}/customquest/customquest ${BIN_DIR}/customquest