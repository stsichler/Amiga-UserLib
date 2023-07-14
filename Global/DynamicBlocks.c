/* User.lib ®SSi 31.8.97 - DynamicBlocks.c ----------------------- */

#include <user/user.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int default_step = 1024;

int putblock(dynamicblock block, APTR data, int datalen)	/* ERROR= */
/* gibt data in den Block aus */
{
	if (block)
	{
		if (block->datalen+datalen>block->maxlen)
		{
			int newmax=block->datalen+datalen+block->step;
			char *newdata;
			
			if (newdata=malloc(newmax))
			{
				memcpy(newdata,block->data,block->datalen);
				free(block->data);
				block->data=newdata;
				block->maxlen=newmax;
			}
			else return -1;
		}
		memcpy(&block->data[block->datalen],data,datalen);
		block->datalen+=datalen;
	}
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

void clearblock(dynamicblock block)
/* löscht den Inhalt */
{
	int step=block->step;
	if (block)
	{
		if (block->data && block->maxlen) free(block->data);
		memset(block,0,sizeof(*block));
		block->step=step;
	}
}

/* ---------------------------------------------------------------------------------------------- */

dynamicblock createblock(int step)	/* ist step=0, wird ein Defaultwert verwendet */
/* erzeugt einen Block */
{
	dynamicblock blk=calloc(sizeof(*blk),1);
	if (blk) blk->step=step?step:default_step;
	return blk;
}

/* ---------------------------------------------------------------------------------------------- */

void destroyblock(dynamicblock block)
/* zerstört einen Block */
{
	clearblock(block);
	if (block) free(block);
}

