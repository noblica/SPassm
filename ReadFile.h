#ifndef _READFILE_H
#define _READFILE_H

#include <fstream>
#include <map>
#include "SymbolTable.h"
#include "GenericSection.h"
#include "RelocationTable.h"
#include <cstdlib>
#include <iostream>

class ReadFile
{
public:

	ReadFile(char* readThis);

	~ReadFile();

	void close();

	char* getNextLine();

	bool isOpen();

	bool assembleThis();
	bool firstPass();
	bool secondPass();
	void increaseLocationCounter(std::string type);

	void testFirst(char* toAdd);
	void testSecond(char* toAdd);

	void addToSection(char* toAdd);
	void addToText(char* toAdd);
	void addToData(char* toAdd);
	void addToBss(char* toAdd);

	void goBackToTheBeginning();

	unsigned short formBytes(int opCode, std::string opString);
	int formOperand(std::string operand);
	int formImm(char* imm, int bits);
	int formImm(int imm, int bits);

	unsigned short group1(int opCode);
	unsigned short group2(int opCode);
	unsigned short group3(int opCode);
	unsigned short group4(int opCode);
	unsigned short group5(int opCode);
	unsigned short group6(int opCode, std::string opString);
	unsigned short group7(int opCode);
	unsigned short group8(int opCode);
	unsigned short group9(int opCode);
	unsigned short group10(int opCode);
	unsigned short group11(int opcode);

	void test();
	static void testStatic();

	bool isInRange(int labelOffset);
	int isLabel(char* checkIfLabel);
	char* checkAfterLabel(char* toCheck);
	bool checkBounds(std::string typeOfVariable, int valueOfVariable);
	int convertCharToInt(char* toConvert);
	void addToReloc(char* label);

	char* getFromBrackets(char* withBrackets);

	bool checkIfSection(char* section);
	bool checkIfAlign(char* align);
	bool checkIfSkip(char* skip);
	bool checkIfEnd(char* end);

	int getAfterEnd();

	int getAfterEndSymbolIndex();

	void alignToThis(int value);
	void skipToThis(int value);

	SymbolTable* getSymbol();
	GenericSection* getSectionArray();
	RelocationTable* getRel();

	int getSymbolSize();
	int getSectionNum();

	void changeToGlobal();
	void changeToExtern();

private:

	std::ifstream myFile;

	std::string currentSection;
	int locationCounter;

	bool doFirstPass, doSecondPass;

	SymbolTable* mySymbolTable;
	RelocationTable* myRelocationTable;

	GenericSection* sectionArray;
	int sectionNum;
	int afterEndSymbolIndex;

	std::map<std::string, int> commandMap;
	std::map<std::string, int> regMap;

	bool outOfRangeError;
	bool relocationSkip;
	bool endIt;
};

#endif