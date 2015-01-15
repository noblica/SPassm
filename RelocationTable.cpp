#include "RelocationTable.h"

RelocationTable::RelocationTable()
{
	relocTable = new RelocStruct[1000];
	num = 0;
}

RelocationTable::~RelocationTable()
{
	delete [] relocTable;
}

void RelocationTable::add(std::string section, int offset, std::string type, int numSymbolTable)
{
	relocTable[num].section = section;
	relocTable[num].offset = offset;
	relocTable[num].type = type;
	relocTable[num++].numSymbolTable = numSymbolTable;
}

void RelocationTable::printOut()
{
	for (int i = 0; i < num; i++)
		std::cout << i << ". " << relocTable[i].section << " | " << relocTable[i].offset << " | " << relocTable[i].type << " | " << relocTable[i].numSymbolTable << std::endl;
}

RelocStruct* RelocationTable::getRel()
{
	return relocTable;
}

int RelocationTable::getSize()
{
	return num;
}