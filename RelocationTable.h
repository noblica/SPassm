#ifndef _RELOCATIONTABLE_H
#define _RELOCATIONTABLE_H

#include <string>
#include <iostream>

typedef struct 
{	
	std::string section;
	int offset;
	std::string type;
	int numSymbolTable;
} RelocStruct;

class RelocationTable
{
public:

	RelocationTable();

	void add(std::string section, int offset, std::string type, int numSymbolTable);

	RelocStruct* getRel();

	int getSize();

	void printOut();

	~RelocationTable();

private:

	RelocStruct* relocTable;

	int num;
};

#endif