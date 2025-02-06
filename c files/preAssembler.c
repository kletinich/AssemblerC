#include "data.h"

struct mcrTable
{
	char *mcrName;
	char *mcrContent;
	struct mcrTable *next;	
};

struct mcrTable* getNode(struct mcrTable*, char*);
char *getMcrContent(FILE*, int);
struct mcrTable* newMcr(char*, char*);

FILE* preAssemble(FILE* filePointer, char* filePath, char* newFileName, char* reservedWords[])
{
	/* TO DO: rename the post macro name to the given file name*/
	FILE* newFilePointer = NULL; /* a new file for the post macro*/
	
	int fileLength; /* the length in chars of the assembly file*/
	int errorFlag = 0; /* a flag for errors*/
	int i;
	
	char* line = (char*)malloc(MAX_LINE_LENGTH); /* a line from the assembly file*/
	char* tempLine = (char*)malloc(MAX_LINE_LENGTH); /* a line from the assembly filefor strtok*/
	char* token; /* a word from the line*/	
	char* mcrContent; /* the content of a given mcr decleration*/
	
	struct mcrTable* head = NULL; /* a pointer to the head of the macro table*/
	struct mcrTable* node = NULL; /* a pointer to a node to run over the macro table*/
	
	strcat(newFileName, ".am");
	newFilePointer = fopen(newFileName, "w+");
	
	fseek(filePointer, 0, SEEK_END);
	fileLength = ftell(filePointer);
	fseek(filePointer, 0, SEEK_SET);
	
	while(ftell(filePointer) < fileLength - 1)
	{
		fgets(line, MAX_LINE_LENGTH, filePointer);
		
		/* avoiding empty lines*/
		if(strlen(line) > 1)
		{
			strcpy(tempLine, line);
			token = strtok(tempLine, " \n\t");

			/*mcr decleration found*/
			if(!strcmp(token, "mcr"))
			{
				/* check if the name of the mcr was already declared before*/
				token = strtok(NULL, " \n\t");
			
				/* mcr decleration found but no variable declared*/
				if(token == NULL)
				{
					fprintf(stderr, "\nError: mcr decleration found but no variable to store it in.\nCannot pre assemble '%s'.\n\n", filePath);
					errorFlag = 1;
				}
				
				else
				{
					for(i = 0; i < RESERVED_WORDS_COUNT - 8; i++)
					{
						if(!strcmp(token, reservedWords[i]))
						{
							fprintf(stderr, "\nError: mcr variable name can't be a command or an instruction.\nCannot pre assemble '%s'.\n\n", filePath);
							errorFlag = 1;
							break;
						}
					}
				}
				
			
				if(errorFlag)
					break;
			
				node = getNode(head, token);
			
				/* this is a declaration of a new mcr*/
				if(node == NULL)
				{
					if(head == NULL)
					{
						mcrContent = getMcrContent(filePointer, fileLength);
						head = newMcr(token, mcrContent);
					}
				
					else
					{
						node = head;
					
						while(node -> next != NULL)
							node = node->next;
					
						mcrContent = getMcrContent(filePointer, fileLength);
						node->next = newMcr(token, mcrContent); 
					}
				
					free(mcrContent);			
				}
			
				/* The new declared mcr already exist. This is an error and the pre assembler is terminated*/
				else
				{
					fprintf(stderr, "\n'%s' already declared.\nCannot pre assemble '%s'.\n\n", token, filePath);
					errorFlag = 1;
				}
				
				token = strtok(NULL, "\n");
				
				/* more text after mcr declration, this is an error and the pre assembler is terminated*/
				if(token != NULL)
				{
					fprintf(stderr, "\nIllegal text after mcr decleration.\nCannot pre assemble '%s'.\n\n", filePath);
					errorFlag = 1;
				}
			
				if(errorFlag)
					break;			
			}

			/*not an mcr decleration*/
			else
			{
				node = getNode(head, token);
			
				/* mcr was found. Now need to spawn it in the post assembler file*/
				if(node != NULL)
					fputs(node->mcrContent, newFilePointer);
			
				/* mcr was not found. copy the text to the post assembler file*/
				else
					fputs(line, newFilePointer); 
			}
		}
	}
		
	/*free all the nodes*/
	while(head != NULL)
	{
		node = head;
		free(node->mcrName);
		free(node->mcrContent);
		head = head->next;
		free(node);
	}
	
	free(line);
	free(tempLine);
	
	if(errorFlag)
	{
		fclose(newFilePointer);
		return NULL;
	}
	
	return newFilePointer;
}
	
/* This function gets a string and checks if its a name of an existing decleration of an mcr.
Return the node if it exists.*/
struct mcrTable* getNode(struct mcrTable *head, char* name)
{
	while(head != NULL)
	{
		if(!strcmp(head->mcrName, name))
			break;
			
		head = head->next;
	}
	
	return head;
}

/* This function copies the mcr decleration content for future decleration in the mcr table*/
char* getMcrContent(FILE* filePointer, int fileLength)
{
	char* mcrContent = (char*)malloc(0); /* the content of the macro*/
	char* line = (char*)malloc(MAX_LINE_LENGTH); /* a line from the file*/
	int mcrContentLength = 0; /* macro length for realloc*/
	int i;
	
	/* removing garbage values from start of mcrContent.*/
	for(i = 0; i < strlen(mcrContent); i++)
		strncpy(mcrContent + i, "\0", 1);

	while(ftell(filePointer) <= fileLength)
	{
		fgets(line, MAX_LINE_LENGTH, filePointer);
		
		if(!strncmp(line, "endmcr\n", 7) || !strncmp(line, "endmcr ", 7) || !strncmp(line, "endmcr\t", 7))
			break;
			
		mcrContentLength += strlen(line);
		mcrContent = (char*)realloc(mcrContent, mcrContentLength*4);
		
		strcpy(mcrContent + strlen(mcrContent), line);	
	}
	
	free(line);

	return mcrContent;	
}

/* This function creates a new node of a newly declared mcr.*/
struct mcrTable* newMcr(char* newMcrName, char* newMcrContent)
{
	struct mcrTable* node = (struct mcrTable*)malloc(sizeof(struct mcrTable));
	node->mcrName = (char*)malloc(MAX_LABEL_LENGTH);
	node->mcrContent = (char*)malloc(strlen(newMcrContent) + 1);
	strcpy(node->mcrName, newMcrName);
	strcpy(node->mcrContent, newMcrContent);
	node->next = NULL;
	
	return node;
}
