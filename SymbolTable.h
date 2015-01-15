#ifndef _SYMBOLTABLE_H
#define _SYMBOLTABLE_H

#include <string>
#include <cstring>
#include <iostream>

typedef struct 
{	
	std::string label;
	std::string section;
	int offset;
	std::string local;
} SymbolStruct;

class SymbolTable
{
public:
	SymbolTable();

	void add(char* label, std::string section, int offset);

	int add(char* label);

	static char* convertToChar(std::string convertThis);

	int isInTable(std::string checkLabel, std::string checkSection);

	void printOut();

	void convertToGlobal(std::string toConvert, std::string currentSection);
	void convertToExtern(std::string toConvert, std::string currentSection);

	SymbolStruct* getSymbol();
	int getSize();

	~SymbolTable();

private:
	SymbolStruct* symbolTable;

	int num;
};

#endif