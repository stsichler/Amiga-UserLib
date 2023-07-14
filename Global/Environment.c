/* User.lib ®SSi 31.8.97 - Environment.c ------------------------- */

#include <user/user.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int PutEnv(char *name, APTR buffer, int length, BOOL save) /* ERROR= */
/* speichert eine Variable in ENV: oder ENVARC: */
{
	char *filename;
	int error=-1;
	FILE *fp;
	
	if (filename=malloc(strlen(name)+8))
	{
		strcpy(filename,"ENV:"); strcat(filename,name);
		do
		{
			if (fp=fopen(filename,"wb")) { fwrite(buffer,1,length,fp); error=ferror(fp); fclose(fp); }
			if (save && filename[3]!='A') { strcpy(filename,"ENVARC:"); strcat(filename,name); }
			else save=FALSE;
		}
		while (save && !error);
		free(filename);
	}
	return error;
}


int GetEnv(char *name, APTR buffer, int length)	/* Bytes read= */
{
	char *filename;
	int ret=0;
	FILE *fp;

	if (filename=malloc(strlen(name)+8))
	{
		strcpy(filename,"ENV:"); strcat(filename,name);

		if (fp=fopen(filename,"rb")) { ret=fread(buffer,1,length,fp); fclose(fp); }
		free(filename);
	}
	return ret;
}
