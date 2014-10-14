/*
 * fs.c - CC31xx/CC32xx Host Driver Implementation
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/
#include <stdio.h>
#include "fs.h"
#include "unistd.h"
#include <stdbool.h>
/*****************************************************************************/
/*  sl_FsOpen */ 
/*****************************************************************************/

FILE * g_FileHandle;
bool g_CreateFile = false;
unsigned long g_CreateFileSize = 0;
bool g_FileOpenForRead = false;
unsigned char g_FileName[127];

unsigned long _GetCreateFsMode(unsigned long maxSizeInBytes,unsigned long accessFlags)
{
	g_CreateFile = true;
	g_CreateFileSize = maxSizeInBytes;
	
	return _FS_MODE(_FS_MODE_OPEN_WRITE_CREATE_IF_NOT_EXIST,  0, 0, 0);
}


#if _SL_INCLUDE_FUNC(sl_FsOpen)
long sl_FsOpen(unsigned char *pFileName,unsigned long AccessModeAndMaxSize, unsigned long *pToken,long *pFileHandle)
{
	memcpy(g_FileName, pFileName, strlen(pFileName) + 1);
	printf("sl_FsOpen: %s\r\n", g_FileName);

	if (g_CreateFile) {
		g_CreateFile = false;
		
		unsigned char* mem = (unsigned char*)malloc(g_CreateFileSize);
		memset(mem, 0, g_CreateFileSize);
		g_FileHandle = fopen((const char *)pFileName, "wb");
		fwrite(mem, 1, g_CreateFileSize,g_FileHandle );
		fclose(g_FileHandle);
		free(mem);
	}
	
	if (access(pFileName, F_OK)) {
		return -1;
	}
	
	if (AccessModeAndMaxSize == 0) {
		g_FileHandle = fopen((const char *)pFileName, "rb");
		g_FileOpenForRead = true;
	} else {
		g_FileHandle = fopen((const char *)pFileName, "wb");
		g_FileOpenForRead = false;
	}
	
	if (g_FileHandle != NULL) {
		return 0;
	} else {
		return -1;
	}
}
#endif

/*****************************************************************************/
/* sl_FsClose */ 
/*****************************************************************************/
#if _SL_INCLUDE_FUNC(sl_FsClose)
int sl_FsClose(long FileHdl, unsigned char* pCeritificateFileName,unsigned char* pSignature ,unsigned long SignatureLen)
{
	printf("sl_FsClose: %s\r\n", g_FileName);
	fclose(g_FileHandle);
	return 0;
}
#endif


/*****************************************************************************/
/* sl_FsRead */ 
/*****************************************************************************/
#if _SL_INCLUDE_FUNC(sl_FsRead)
long sl_FsRead(long FileHdl, unsigned long Offset, unsigned char* pData, unsigned long Len)
{
	fflush(g_FileHandle);
	if (!g_FileOpenForRead) {
		fclose(g_FileHandle);

		printf("Reopen for read: %s\r\n", g_FileName);
		
		g_FileHandle = fopen((const char *)g_FileName, "rb");
		g_FileOpenForRead = true;
	}
	fseek(g_FileHandle, Offset, SEEK_SET);
	fflush(g_FileHandle);
	long RetCount = fread(pData, 1, Len, g_FileHandle);
	
	printf("READ %d:", RetCount);
	for (unsigned int i = 0; i < RetCount; i++) {
		printf("%2x_%c ", pData[i], pData[i]);
	}
	printf("\r\n");

    return (long)RetCount;
}
#endif

/*****************************************************************************/
/* sl_FsWrite */ 
/*****************************************************************************/
#if _SL_INCLUDE_FUNC(sl_FsWrite)
long sl_FsWrite(long FileHdl, unsigned long Offset, unsigned char* pData, unsigned long Len)
{
	fflush(g_FileHandle);
	if (g_FileOpenForRead) {
		fclose(g_FileHandle);
		
		printf("Reopen for write: %s\r\n", g_FileName);
		
		g_FileHandle = fopen((const char *)g_FileName, "wb");
		g_FileOpenForRead = false;
	}
	fseek(g_FileHandle, Offset, SEEK_SET);
	fflush(g_FileHandle);
	long RetCount = fwrite(pData, 1, Len, g_FileHandle);

	printf("WRITE:");
	for (unsigned int i = 0; i < RetCount; i++) {
		printf("%2x_%c ", pData[i], pData[i]);
	}
	printf("\r\n");
	
    return (long)RetCount;
}
#endif

/*****************************************************************************/
/* sl_FsGetInfo */ 
/*****************************************************************************/

#if _SL_INCLUDE_FUNC(sl_FsGetInfo)
int sl_FsGetInfo(unsigned char *pFileName,unsigned long Token,SlFsFileInfo_t* pFsFileInfo)
{
	
	FILE * pHandle = fopen((const char *)pFileName, "rb");
	if (pHandle == NULL) {
		return -1;
	}
	fseek(pHandle, 0, SEEK_END);
	
	long size = ftell(pHandle);
	fclose(pHandle);

	pFsFileInfo->flags        = 0;
    pFsFileInfo->FileLen      = size;
    pFsFileInfo->AllocatedLen = size;
    pFsFileInfo->Token[0]     = 0;
    pFsFileInfo->Token[1]     = 0;
    pFsFileInfo->Token[2]     = 0;
    pFsFileInfo->Token[3]     = 0;
	return 0;
}
#endif

/*****************************************************************************/
/* sl_FsDel */ 
/*****************************************************************************/
#if _SL_INCLUDE_FUNC(sl_FsDel)
int sl_FsDel(unsigned char *pFileName,unsigned long Token)
{
	int status = remove((const char *)pFileName);
	return  status;
}
#endif
