/*
 * SceExfatfs Analysis
 * Copyright (C) 2021 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/io/fcntl.h>
#include "log.h"

SceUID log_fd = 0;
SceSize log_pos = 0;
char log_buff[0x400] __attribute__((aligned(0x40)));

int LogIsOpened(void){

	if(log_fd < 0)
		return 0;

	return (log_fd == 0) ? 0 : 1;
}

int LogOpen(const char *path){

	if(log_fd == 0){
		log_fd = ksceIoOpen(path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0666);
		log_pos = 0;
	}else{
		return -1;
	}

	return 0;
}

int LogWrite(const char *fmt, ...){

	va_list args;
	int len;
	char string[0x200];

	if(log_fd > 0){
		memset(string, 0, sizeof(string));

		va_start(args, fmt);
		len = vsnprintf(string, sizeof(string) - 1, fmt, args);
		va_end(args);

		if((SceSize)(log_pos + len) >= sizeof(log_buff)){
			ksceIoWrite(log_fd, log_buff, log_pos);
			memset(log_buff, 0, log_pos);
			log_pos = 0;
		}

		memcpy(&log_buff[log_pos], string, len);
		log_pos += len;
	}

	return 0;
}

int LogClose(void){
	if(log_fd > 0){
		if(log_pos != 0){
			ksceIoWrite(log_fd, log_buff, log_pos);
			memset(log_buff, 0, log_pos);
			log_pos = 0;
		}

		ksceIoClose(log_fd);
		log_fd = 0;
	}
	return 0;
}
