
#include <psp2kern/types.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/rtc.h>
#include <psp2kern/io/stat.h>
#include "fs_tool.h"
#include "dmass_process.h"

FapsDmassProcessContext *pProcContextTop = NULL;

int dmass_create_proc_context(SceUID pid, FapsDmassProcessContext **ppContext){

	int res;
	FapsDmassProcessContext *pContext;

	res = dmass_search_proc_context(pid, NULL);
	if(res >= 0)
		return -1;

	pContext = dmass_malloc(sizeof(*pContext));
	if(pContext == NULL)
		return -2;

	pContext->next = pProcContextTop;
	pContext->pid  = pid;

	pProcContextTop = pContext;

	if(ppContext != NULL)
		*ppContext = pContext;

	return 0;
}

int dmass_delete_proc_context(SceUID pid){

	FapsDmassProcessContext **ppContext = &pProcContextTop;

	while(*ppContext != NULL){
		if(pid == (*ppContext)->pid){

			FapsDmassProcessContext *pContext = *ppContext;

			*ppContext = (*ppContext)->next; // unlink

			dmass_free(pContext);

			return 0;
		}

		ppContext = &((*ppContext)->next);
	}

	return -2;
}

int dmass_search_proc_context(SceUID pid, FapsDmassProcessContext **ppContext){

	FapsDmassProcessContext *pContext = pProcContextTop;

	while(pContext != NULL){
		if(pid == pContext->pid){
			if(ppContext != NULL){
				*ppContext = pContext;
			}
			return 0;
		}
		pContext = pContext->next;
	}

	return -1;
}
