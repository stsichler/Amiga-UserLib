/* FindNameType durchsucht eine Exec-Liste nach einem Eintrag
 * mit Name=name und Typ=type und gibt dessen Adresse zurück.
 * Bei Mißerfolg 0.
 */

#include <exec/nodes.h>
#include <exec/lists.h>
#include <user/user.h>

extern int strcmp(char *,char *);

struct Node *FindNameType(list,name,type)
struct List *list;
char *name;
UBYTE type;
{
	struct Node *found=0;
	struct Node *node=(struct Node *)list;
	if (name)
		while ((node=node->ln_Succ)->ln_Succ && !found)
			if (node->ln_Name) 
				if (node->ln_Type==type && !strcmp(name,node->ln_Name))
					found=node;
	return(found);
}		
