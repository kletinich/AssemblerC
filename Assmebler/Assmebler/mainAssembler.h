#pragma once


#include "data.h"

int secondLoop(FILE*, int, struct SymbolTable*, struct Image*, char* []);
void makeObjectFile(char*, struct Image*, struct Image*, int, int);
void makeEntryFile(char*, struct SymbolTable*);
void makeExternFile(FILE*, int, char*, struct SymbolTable*, char* []);
int updateInstructionNode(struct SymbolTable*, struct Image*, char*, int, char* []);
struct SymbolTable* getSymbol(struct SymbolTable*, char*);
struct SymbolTable* newSymbol(char*, int, int);
struct Image* newDataImage(int);
struct Image* newInstructionImage(int*);
int commandLine(struct SymbolTable*, char*, char*, char*, int, int, char* []);
int entryExternLine(struct SymbolTable*, char*, char*, char*, int, int, char* []);
int dataStringLine(char*, char*, char*, struct SymbolTable*, int, int);
struct SymbolTable* insertSymbol(struct SymbolTable*, struct SymbolTable*);
struct Image* insertImage(struct Image*, struct Image*);
void convertInstructionToBinary(int*, char*, char*, char* []);
void makeObjectFile(char*, struct Image*, struct Image*, int, int);
