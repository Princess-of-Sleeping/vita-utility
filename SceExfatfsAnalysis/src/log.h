/*
 * SceExfatfs Analysis
 * Copyright (C) 2021 Princess of Sleeping
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef _EXFATFS_ANALYSIS_LOG_H_
#define _EXFATFS_ANALYSIS_LOG_H_

int LogIsOpened(void);
int LogOpen(const char *path);
int LogWrite(const char *fmt, ...);
int LogClose(void);

#endif /* _EXFATFS_ANALYSIS_LOG_H_ */
