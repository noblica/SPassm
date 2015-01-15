#include "SymbolTable.h"

SymbolTable::SymbolTable()
{
	symbolTable = new SymbolStruct[1000];
	num = 0;
}

SymbolTable::~SymbolTable()
{
	delete [] symbolTable;
}

void SymbolTable::add(char* label, std::string section, int offset)
{
	symbolTable[num].label = std::string(label, strlen(label) - 1);
	symbolTable[num].section = section;
	symbolTable[num].offset = offset;
	symbolTable[num++].local = "local";
}

int SymbolTable::add(char* label)
{
	symbolTable[num].label = std::string(label, strlen(label) - 1);
	symbolTable[num].section = "?";
	symbolTable[num].offset = -1;
	symbolTable[num].local = "global";
	return num++;
}

char* SymbolTable::convertToChar(std::string convertThis)
{
	char* toReturn = new char[convertThis.size() + 1];
	std::copy(convertThis.begin(), convertThis.end(), toReturn);

	toReturn[convertThis.size()] = '\0';
	return toReturn;
}

int SymbolTable::isInTable(std::string checkLabel, std::string checkSection)
{
	int n = num;

	for (int i = 0; i < n; i++)
		if (	checkLabel.compare(symbolTable[i].label) == 0
			&&	checkSection.compare(symbolTable[i].section) == 0)
			return symbolTable[i].offset;

	return -1;
}

void SymbolTable::printOut()
{
	int n = num;

	for (int i = 0; i < n; i++)
		std::cout << i << ". " << symbolTable[i].label << " | " << symbolTable[i].section << " | " << symbolTable[i].offset << " | " << symbolTable[i].local << std::endl;
}

void SymbolTable::convertToGlobal(std::string toConvert, std::string currentSection)
{
	for (int i = 0; i < num; i++)
		if (	toConvert.compare(symbolTable[i].label) == 0
			&&	currentSection.compare(symbolTable[i].section) == 0)
		{
			symbolTable[i].local = "global";
			break;
		}
}

void SymbolTable::convertToExtern(std::string toConvert, std::string currentSection)
{
	/*for (int i = 0; i < num; i++)
		if (	toConvert.compare(symbolTable[i].label) == 0
			&&	currentSection.compare(symbolTable[i].section) == 0)
			symbolTable[i].local = "extern";*/
}

SymbolStruct* SymbolTable::getSymbol()
{
	return symbolTable;
}

int SymbolTable::getSize()
{
	return num;
}