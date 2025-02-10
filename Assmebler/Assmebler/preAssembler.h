#pragma once

typedef struct mcrTable
{
	char* mcrName;
	char* mcrContent;
	struct mcrTable* next;
}mcrTable;

mcrTable* getNode(mcrTable*, char*);
char* getMcrContent(FILE*, int);
mcrTable* newMcr(char*, char*);
