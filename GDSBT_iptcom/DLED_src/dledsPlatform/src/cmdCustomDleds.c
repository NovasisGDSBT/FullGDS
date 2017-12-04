/************************************************************************/
/*  (C) COPYRIGHT 2015 by Bombardier Transportation                     */
/*                                                                      */
/*  Bombardier Transportation Switzerland Dep. PPC/EUT                  */
/************************************************************************/
/*                                                                      */
/*  PROJECT:      Remote download                                       */
/*                                                                      */
/*  MODULE:       dledsPlatform                                         */
/*                                                                      */
/*  ABSTRACT:     This file contains the platform specific parts        */
/*                for dleds.                                            */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*  REMARKS:                                                            */
/*                                                                      */
/*  DEPENDENCIES: See include list                                      */
/*                                                                      */
/*  ACCEPTED:                                                           */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/*  HISTORY:                                                            */
/*                                                                      */
/*  version  yy-mm-dd  name       depart.    ref   status               */
/*  -------  --------  ---------  -------    ----  ---------            */
/*                                                                      */
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "dleds.h"
#include "dludatarep.h"
#include "dledsVersion.h"
#include "dledsInstall.h"
#include "dleds_icd.h"
#include "dledsDbg.h"
#include "dledsCrc32.h"
#include "dledsPlatform.h"
#include "dledsPlatformDefines.h"

extern UINT8 CFavailable;
extern TYPE_DLEDS_SESSION_DATA sessionData;

const char *statemachine[15]={
    "UNDEFINED",
    "INITIALIZED",
    "WAIT_FOR_REQUEST",
    "SEND_RESPONSE",
    "SEND_STATUS",
    "STARTED",
    "DOWNLOAD_MODE",
    "RESET_TO_RUN_MODE",
    "WAIT_FOR_RESPONSE_RESULT",
    "WAIT_FOR_STATUS_RESULT",
    "PROCESS_RESULT",
    "RESET_MODE",
    "STOP",
    "TERMINATED",
    "IDLE_MODE"
};

unsigned int forced_reboot(unsigned int  input);
int getDlAllowed(void);
DLEDS_RESULT dleds_setLinuxDledsTempPath(char* filePath);
int bash_exec(int len, char *stringcommand, char *dststring);


int getDlAllowed(void)
{
//int result = FALSE;
int result = TRUE;

    return result;
}

unsigned int forced_reboot(unsigned int  input)
{
   //unsigned int result=TRUE;
   unsigned int result=FALSE;


   printf("Function %s \n",__FUNCTION__);

   return result;
}


/*******************************************************************************
 *
 * Function name: dleds_setLinuxDledsTempPath
 *
 * Abstract:      Sets filePath to temporary DLEDS files on CCU-C and MCG.
 *                If CF card is mounted, It will be used to temporary store and
 *                unpack ED package.
 *
 * Return value:  DLEDS_OK      - if OK
 *                DLEDS_ERROR   - otherwise
 *
 * Globals:       -
 */
DLEDS_RESULT dleds_setLinuxDledsTempPath(
    char* filePath)    /* IN/OUT: Path including sub directory */
{
    DLEDS_RESULT    res = DLEDS_ERROR;
#if 0
    char *          buffer;
    char *          pBuffer = NULL;
    char *          pTmp;
    size_t          result;
    FILE *          pFile;

    printf("Function %s - filePath:%s\n",__FUNCTION__,filePath);
    if (filePath == NULL)
    {
        return res;
    }

      /* CCU-C and MCG. HMI devices are not included */
    /* check if CF is mounted */
    pFile = fopen(DLEDS_LINUX_MOUNTS, "r");
    if (pFile != NULL)
    {
        /* allocate memory to contain the mounts file */
        buffer = malloc(DLEDS_MOUNT_FILE_SIZE);
        if (buffer != NULL)
        {
            memset(buffer, 0, DLEDS_MOUNT_FILE_SIZE);
            /* copy the file into the buffer */
            result = fread (buffer, 1, DLEDS_MOUNT_FILE_SIZE, pFile);
            pBuffer = strstr(buffer, DLEDS_LINUX_CF);
            if (pBuffer != NULL)
            {
                pTmp = strtok(pBuffer, " ");
                pTmp = strtok(NULL, " ");

                if (pTmp != NULL)
                {
                    strncpy(filePath, pTmp, strlen(pTmp) + 1);
                    if(filePath[strlen(pTmp)-1] != '/')
                    {
                        strncat(filePath, "/", DLEDS_MAX_PATH_LEN);
                    }
                    CFavailable = 1;
                    res = DLEDS_OK;
                }
            }
            fclose (pFile);
            free(buffer);
        }
    }

    if (CFavailable == 1)
    {
        /* Check if enough space is available on CF */
         if (dleds_diskSpaceAvailable(filePath, sessionData.requestInfo.fileSize) == DLEDS_ERROR)
         {
            DebugError1("dleds_setLinuxDledsTempPath: Not enought disk space, filesize is %d kByte", sessionData.requestInfo.fileSize);
            strcpy(filePath,"");
            res = DLEDS_ERROR;
         }
    }


    if (res == DLEDS_ERROR)
    {
        /* Use flash file system for temporary DLEDS directory */
        strcpy(filePath, FILE_SYSTEM_NAME);
        strcat(filePath,"/");
        res = DLEDS_OK;
        DebugError0("dleds_setLinuxDledsTempPath: ERROR so Use flash file system dir=/");
    }
#endif
    res = DLEDS_OK;
    return res;
}



int bash_exec(int len, char *stringcommand, char *dststring)
{

    FILE *fp;
    char path[RESULTBASH_SIZESTRING];
    char bash_cmd[256];
    int result=-1;



    memset( bash_cmd, '\0', sizeof(bash_cmd));
    strncpy(bash_cmd, stringcommand, len);

    /* Open the command for reading. */
    fp = popen(bash_cmd, "r");
    if (fp == NULL)
    {
        DebugError0("bash_exec Failed popen");

    }
    else
    {
        /* Read the output a line at a time - output it. */
        while (fgets(path, sizeof(path)-1, fp) != NULL)
        {
            DebugError1("bash_exec %s", path);
            strncpy(dststring,path,RESULTBASH_SIZESTRING);
        }

    /* close */
    pclose(fp);

    result=0;
    }

    return result;
}
