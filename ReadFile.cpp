
#include "ReadFile.h"

ReadFile::ReadFile(char* readThis)
{
	myFile.open(readThis);

	if (myFile != NULL)
	{
		mySymbolTable = new SymbolTable();
		myRelocationTable = new RelocationTable();
		afterEndSymbolIndex = -1;

		sectionArray = new GenericSection[1000];
		sectionNum = -1;

		doFirstPass = false;
		doSecondPass = false;
		locationCounter = 0;
		outOfRangeError = false;
		relocationSkip = false;
		endIt = false;

		commandMap["add"] = 0;
		commandMap["sub"] = 1;
		commandMap["mul"] = 2;
		commandMap["div"] = 3;
		commandMap["cmp"] = 4;
		commandMap["and"] = 5;
		commandMap["or"] = 6;
		commandMap["not"] = 7;
		commandMap["test"] = 8;
		commandMap["ldr"] = 9;
		commandMap["str"] = 10;
		commandMap["je"] = 11;
		commandMap["jne"] = 12;
		commandMap["jge"] = 13;
		commandMap["jg"] = 14;
		commandMap["jle"] = 15;
		commandMap["jl"] = 16;
		commandMap["jp"] = 17;
		commandMap["jn"] = 18;
		commandMap["jo"] = 19;
		commandMap["jno"] = 20;
		commandMap["call"] = 21;
		commandMap["ret"] = 22;
		commandMap["iret"] = 22;
		commandMap["jmp"] = 22;
		commandMap["push"] = 23;
		commandMap["pop"] = 24;
		commandMap["movtosfr"] = 25;
		commandMap["movfromsfr"] = 26;
		commandMap["mov"] = 27;
		commandMap["in"] = 28;
		commandMap["out"] = 29;
		commandMap["shr"] = 30;
		commandMap["shl"] = 31;

		regMap["r0"] = 0;
		regMap["r1"] = 1;
		regMap["r2"] = 2;
		regMap["r3"] = 3;
		regMap["r4"] = 4;
		regMap["r5"] = 5;
		regMap["r6"] = 6;
		regMap["r7"] = 7;
		regMap["pc"] = 12;
		//probno
		regMap["sp"] = 10;
		regMap["psw"] = 8;
		regMap["pmt"] = 9;
	}

}

ReadFile::~ReadFile()
{
	delete [] sectionArray;
	delete mySymbolTable;
	delete myRelocationTable;
}

void ReadFile::close()
{
	myFile.close();
}

bool ReadFile::firstPass()
{
	doFirstPass = true;

	bool returnValue = assembleThis();

	doFirstPass = false;

	return !returnValue;
}

bool ReadFile::secondPass()
{
	doSecondPass = true;

	bool returnValue = assembleThis();

	doSecondPass = false;
	sectionNum++;

	return !returnValue;
}

bool ReadFile::assembleThis()
{
	goBackToTheBeginning();

	char* nextLine = getNextLine();

	while (!myFile.eof())
	{
		if (nextLine != 0)
		{
			char* toAdd = strtok(nextLine, " \t");

			if (doFirstPass)
				testFirst(toAdd);

			else if(doSecondPass)
				testSecond(toAdd);

			else return 0;
		}
			if (outOfRangeError)
				return 0;

			if (endIt)
				return 1;

			nextLine = getNextLine();
	}
	return 1;
}

char* ReadFile::getNextLine()
{
	std::string line = "";

	while ( (line.empty()) && (!myFile.eof()) )
		std::getline(myFile, line);

	if (line.empty())
		return 0;
	else
		return SymbolTable::convertToChar(line);
}

void ReadFile::testFirst(char* toAdd)
{
	if (checkIfSection(toAdd))
		return;

	else if (checkIfAlign(toAdd))
		return;

	else if (checkIfSkip(toAdd))
		return;

	else if(checkIfEnd(toAdd))
		afterEndSymbolIndex = getAfterEnd();

	else if (toAdd[strlen(toAdd) - 1] == ':')
		mySymbolTable -> add(toAdd, currentSection, locationCounter);
	
	else if ((toAdd != NULL) && (toAdd[0] != '.'))
		increaseLocationCounter(toAdd);
}

bool ReadFile::checkIfSection(char* section)
{
	if (strcmp(section, ".section") == 0)
	{
		section = strtok(NULL, " \t\".");
		currentSection = std::string(section);
		locationCounter = 0;

		char* startOffsetChar = strtok(NULL, " \t\".");

		if (doSecondPass)
		{
			sectionNum++;
			sectionArray[sectionNum].setSectionName(section);

			if (startOffsetChar != 0)
			{
				int startOffsetInt = convertCharToInt(startOffsetChar);
				sectionArray[sectionNum].setStartOffset(startOffsetInt);
			}
		}
		return true;
	}
	return false;
}

bool ReadFile::checkIfAlign(char* align)
{
	if (strcmp(align, ".align") == 0)
	{
		char* alignTo = strtok(NULL, " \t\".");
		int alignValue = convertCharToInt(alignTo);
		alignToThis(alignValue);
		return true;
	}
	return false;
}

void ReadFile::alignToThis(int value)
{
	while (locationCounter % value != 0)
		locationCounter++;
	
	sectionArray[sectionNum].setAlignValue(value);
}

bool ReadFile::checkIfSkip(char* skip)
{
	if (strcmp(skip, ".skip") == 0)
	{
		char* toSkip = strtok(NULL, " \t\".");
		int skipValue = convertCharToInt(toSkip);
		skipToThis(skipValue);
		return true;
	}
	return false;
}

bool ReadFile::checkIfEnd(char* end)
{
	if (strcmp(end, ".end") == 0)
		return 1;
	return 0;
}

int ReadFile::getAfterEnd()
{
	char* startWithThisChar = strtok(NULL, " \t\".:");

	if (startWithThisChar == 0)
		return -1;

	std::string startWithThisString = std::string(startWithThisChar);

	SymbolStruct* startWithThisSymbol = mySymbolTable -> getSymbol();

	for (int i = 0; i < mySymbolTable -> getSize(); i++)
		if (startWithThisString.compare(startWithThisSymbol[i].label) == 0)
			return i;
}

void ReadFile::skipToThis(int value)
{
	while ((value--) > 0)
		locationCounter++;
}

void ReadFile::testSecond(char* toAdd)
{
	if (checkIfSection(toAdd))
		return;

	else if (checkIfAlign(toAdd))
		return;

	else if(checkIfSkip(toAdd))
		return;

	else if(checkIfEnd(toAdd))
		endIt = true;

	else if (strcmp(toAdd, ".global") == 0)
		changeToGlobal();

	else if (strcmp(toAdd, ".extern") == 0)	
	{
		changeToExtern();
		relocationSkip = false;
	}

	else
	{
		if (toAdd[strlen(toAdd) - 1] == ':')
			toAdd = checkAfterLabel(toAdd);

		if ((toAdd) != 0)
			addToSection(toAdd);

		if (outOfRangeError)
			return;
	}

}

char* ReadFile::checkAfterLabel(char* toCheck)
{
	char* instAfterLabel;

	instAfterLabel = strtok(NULL, " \t\".");

	if (instAfterLabel != 0)
		return instAfterLabel;

	return 0;
}

void ReadFile::addToSection(char* toAdd)
{
	if (currentSection.compare(0, 4, "text") == 0)
		addToText(toAdd);

	else if (currentSection.compare(0, 4, "data") == 0)
		addToData(toAdd);

	else if (currentSection.compare(0, 3, "bss") == 0)
		addToBss(toAdd);
}

void ReadFile::addToText(char* toAdd)
{
	std::string toCompare = std::string(toAdd);
	std::map<std::string, int>::const_iterator comIter = commandMap.begin();

	for (comIter; comIter != commandMap.end(); ++comIter)
		if (toCompare.compare(comIter -> first) == 0)
		{
			int opCode = comIter -> second;
			unsigned short formedInst = formBytes(opCode, comIter -> first);

			//ako je greska, iskacemo
			if (outOfRangeError)
				return;

			if (!relocationSkip)
				sectionArray[sectionNum].add(locationCounter, formedInst, toCompare);

			relocationSkip = false;
			locationCounter += 2;
			break;
		}
}

void ReadFile::addToData(char* toAdd)
{
	std::string isAlign = std::string(toAdd);

	std::string typeOfVariable = std::string(toAdd);
	char* valueOfVariable = strtok(NULL, " \t\".");

	int intValueOfVariable = convertCharToInt(valueOfVariable);

	if (checkBounds(typeOfVariable, intValueOfVariable))
	{
		sectionArray[sectionNum].add(locationCounter, intValueOfVariable, std::string(valueOfVariable));
		increaseLocationCounter(typeOfVariable);
	}
}

int ReadFile::convertCharToInt(char* toConvert)
{
	if (toConvert[1] == 'x')
		return (int) strtol(toConvert, NULL, 16);
	else
		return (int) strtol(toConvert, NULL, 10);
}

void ReadFile::addToBss(char* toAdd)
{
	std::string isAlign = std::string(toAdd);

	std::string typeOfVariable = std::string(toAdd);

	char* toReserve = strtok(NULL, " \t\".");
	int intToReserve = convertCharToInt(toReserve);

	sectionArray[sectionNum].add(locationCounter, intToReserve, typeOfVariable);
	while ((intToReserve--) > 0)
		increaseLocationCounter(typeOfVariable);
}

void ReadFile::increaseLocationCounter(std::string type)
{
	char* charRepeater = 0;
	if (currentSection.compare("bss") == 0)
		charRepeater = strtok(NULL, " \t\".");

	int repeater = 1;
	if (charRepeater != 0)
		repeater = convertCharToInt(charRepeater);

	while ((repeater--) > 0)
	{
		if (type.compare(".char") == 0)
			locationCounter += 1;

		else if (type.compare(".word") == 0)
			locationCounter += 2;

		else
			locationCounter += 2;
	}
}

bool ReadFile::checkBounds(std::string type, int value)
{
	if (type.compare(".char") == 0)
		return (abs(value) < 128);

	if (type.compare(".word") == 0)
		return (abs(value) < 32768);

	return false;
}

unsigned short ReadFile::formBytes(int opCode, std::string opString)		//DORADI OVO !!!!!!!!!!!!!!!!!!!
{

	if (opCode >= 0 && opCode < 4)
		return group1(opCode);

	if (opCode == 4)
		return group2(opCode);

	if (opCode > 4 && opCode < 9)
		return group3(opCode);

	if (opCode >= 9 && opCode < 11)
		return group4(opCode);

	if (opCode >= 11 && opCode < 22)
		return group5(opCode);

	if (opCode == 22)
		return group6(opCode, opString);

	if (opCode >= 23 && opCode < 25)
		return group7(opCode);

	if (opCode == 25)
		return group8(opCode);

	if (opCode == 26)
		return group9(opCode);

	if (opCode > 26 && opCode < 30)
		return group10(opCode);

	if (opCode == 31)
		return group11(opCode);

	outOfRangeError = true;
	return 0;

}

unsigned short ReadFile::group1(int opCode)
{
	//ADD, SUB, MUL, DIV

	int dstInt = 0x0;
	int srcInt = 0x0;
	int immInt = 0x0;

	std::string dst = std::string(strtok(NULL, " \t,"));
	if (!dst.empty())
		dstInt = formOperand(dst);

	std::string src = std::string(strtok(NULL, " \t,"));
	if (!src.empty())
		srcInt = formOperand(src);

	char* imm = strtok(NULL, " \t");
	if (imm != NULL)
		immInt = formImm(imm, 5);

	if (outOfRangeError)
		return 0;

	unsigned short returnValue = 0x0;
	returnValue = returnValue | (opCode << 11) | (dstInt << 8) | (srcInt << 5) | immInt;

	return returnValue;
}

unsigned short ReadFile::group2(int opCode)
{
	//COMPARE
	//Sve je isto kao i u grupi 1(?)

	int dstInt = 0x0;
	int srcInt = 0x0;
	int immInt = 0x0;

	std::string dst = std::string(strtok(NULL, " \t,"));
	if (!dst.empty())
		dstInt = formOperand(dst);

	std::string src = std::string(strtok(NULL, " \t,"));
	if (!src.empty())
		srcInt = formOperand(src);

	char* imm = strtok(NULL, " \t");
	if (imm != NULL)
		immInt = formImm(imm, 5);

	if (outOfRangeError)
		return 0;

	unsigned short returnValue = 0x0;
	returnValue = returnValue | (opCode << 11) | (dstInt << 8) | (srcInt << 5) | immInt;

	return returnValue;
}

unsigned short ReadFile::group3(int opCode)
{
	//AND, OR, NOT, TEST

	int dstInt = 0x0;
	int srcInt = 0x0;

	std::string dst = std::string(strtok(NULL, " \t,"));
	if (!dst.empty())
		dstInt = formOperand(dst);

	std::string src = std::string(strtok(NULL, " \t,"));
	if (!src.empty())
		srcInt = formOperand(src);

	unsigned short returnValue = 0x0;
	returnValue = returnValue | (opCode << 11) | (dstInt << 8) | (srcInt << 5);

	return returnValue;
}

unsigned short ReadFile::group4(int opCode)
{
	//LDR, STR

	int dstInt = 0x0;
	int srcInt = 0x0;
	int immInt = 0x0;

	std::string dst = std::string(strtok(NULL, " \t,"));
	if (!dst.empty())
	{
		if (dst.compare("pc") == 0)
			dstInt = 7;

		else dstInt = formOperand(dst);
	}

	std::string src = std::string(strtok(NULL, " \t,[]"));
	if (!src.empty())
		srcInt = formOperand(src);

	char* imm = strtok(NULL, " \t,[]+");
	if (imm != NULL)
		immInt = formImm(imm, 5);

	if (outOfRangeError)
		return 0;

	unsigned short returnValue = 0x0;
	returnValue = returnValue | (opCode << 11) | (dstInt << 8) | (srcInt << 5) | immInt;

	return returnValue;
}

unsigned short ReadFile::group5(int opCode)
{
	//JE, JNE, JGE, JG, JLE, JL, JP, JN, JO, JNO, CALL

	//ako je dst < 8 - korsitimo registre
	//visa dva bita dst (1100 = 12 ?) koristimo PC
	//visa tri bita dst = (1110 = 14 ? ) odredišna adresa se dobija čitanjem iz memorije sa adrese PC+imm

	int dstInt = 0x0;
	int immInt = 0x0;

	bool usePc = false;

	char* checkNext = strtok(NULL, " \t");

		//Ako je neposredno
		if (checkNext[0] == '[')
		{
			//Ako je tipa [123]
			if (isdigit(checkNext[1]))
			{
				dstInt = 3;
				char* noBrackets = getFromBrackets(checkNext);
				immInt = formImm(noBrackets, 9);
				usePc = true;
			}
			//Ako je tipa [r1, 123]
			else
			{
				char* tempDst;
				memcpy(tempDst, checkNext + 1, 2);
				tempDst[2] = '\0';

				dstInt = formOperand(std::string(tempDst));

				char* imm = strtok(NULL, " \t,[]");
				if (imm != NULL)
					immInt = formImm(imm, 7);
			}
		}

		//Ako nije neposredno (jedino moze da bude labela)
		else 
		{
			dstInt = 2;
			int distance = isLabel(checkNext);
			immInt = formImm(distance, 9);
			usePc = true;
		}

	if (outOfRangeError)
		return 0;

	unsigned short returnValue = 0x0;
	if (usePc)
		returnValue = returnValue | (opCode << 11) | (dstInt << 9) | immInt;

	else
		returnValue = returnValue | (opCode << 11) | (dstInt << 7) | immInt;

	return returnValue;	
}

unsigned short ReadFile::group6(int opCode, std::string opString)
{
	// RET, IRET, JMP

	int typeInt = 0x0;
	int immInt = 0x0;

	if (opString.compare("ret") == 0)
		typeInt = 1;

	else if (opString.compare("iret") == 0)
		typeInt = 0;

	else
	{
		char* checkNext = strtok(NULL, " \t");

		//Ako je neposredno (???)
		if (checkNext[0] == '[')
		{
			typeInt = 3;
			char* noBrackets = getFromBrackets(checkNext);
			immInt = formImm(noBrackets, 9);
		}

		//Ako je samo broj
		else if (isdigit(checkNext[0]))
		{
			typeInt = 2;
			immInt = formImm(checkNext, 9);
		}

		//Ako je labela
		else 
		{
			typeInt = 2;
			int distance = isLabel(checkNext);
			immInt = formImm(distance, 9);
		}
	}

	if (outOfRangeError || relocationSkip)
		return 0; 		//max vrednost za unsigned short. Nijedna operacija je ne koristi, pa je koristim za prijavu greske.
	
	unsigned short returnValue = 0x0;

	int mask = 1;
	mask = mask << 9;
	mask -= 1;

	immInt = immInt & mask;
	returnValue = returnValue | (opCode << 11) | (typeInt << 9) | immInt;

	return returnValue;
}

char* ReadFile::getFromBrackets(char* withBrackets)
{
	char* noBrackets = new char();

	memcpy(noBrackets, withBrackets + 1, strlen(withBrackets) - 2);
	noBrackets[strlen(noBrackets)] = '\0';

	return noBrackets;
}

int ReadFile::isLabel(char* checkIfLabel)
{
	int offset = mySymbolTable -> isInTable(std::string(checkIfLabel), currentSection);

	if (offset == -1)
	{
		addToReloc(checkIfLabel);
		return 0;
	}

	else if (!isInRange(offset))
	{
		std::cout << "GRESKA !!!\n Labela nije unutar opsega" << std::endl;
		outOfRangeError = true;

		return 0;	//imm = 0? Ili treba da idem da "skocim" na sledecu liniju?
	}

	return offset - locationCounter;
}

void ReadFile::addToReloc(char* label)
{
	relocationSkip = true;

	int num = mySymbolTable -> add(label);
	myRelocationTable -> add(currentSection, locationCounter, "R_386_PC16", num);
}

unsigned short ReadFile::group7(int opCode)
{
	//PUSH, POP

	int regInt = 0x0;

	std::string reg = std::string(strtok(NULL, " \t,[]"));
	if (!reg.empty())
		regInt = formOperand(reg);

	unsigned short returnValue = 0x0;
	returnValue = returnValue | (opCode << 11) | (regInt << 7);

	return returnValue;
}

unsigned short ReadFile::group8(int opCode)
{
	//MOVTOSFR

	int srcInt = 0x0;
	int dstInt = 0x0;

	std::string dst = std::string(strtok(NULL, " \t,"));
	if (!dst.empty())
	{
		dstInt = formOperand(dst);
		switch(dstInt)
		{
			case 12:				//PC
				dstInt = 0;
				break;
			case 10:				//SP
				dstInt = 1;
				break;
			case 8:					//PSW
				dstInt = 2;
				break;
			case 9:					//PMT
				dstInt = 3;
				break;
		}
	}

	std::string src = std::string(strtok(NULL, " \t,"));
	if (!src.empty())
		srcInt = formOperand(src);

	unsigned short returnValue = 0x0;
	returnValue = returnValue | (opCode << 11) | (srcInt << 8) | (dstInt << 6);

	return returnValue;
}

unsigned short ReadFile::group9(int opCode)
{
	//MOVFROMSFR

	int dstInt = 0x0;
	int srcInt = 0x0;

	std::string dst = std::string(strtok(NULL, " \t,"));
	if (!dst.empty())
		dstInt = formOperand(dst);

	std::string src = std::string(strtok(NULL, " \t,"));
	if (!src.empty())
	{
		srcInt = formOperand(src);
		switch(srcInt)
		{
			case 12:				//PC
				srcInt = 0;
				break;
			case 10:				//SP
				srcInt = 1;
				break;
			case 8:					//PSW
				srcInt = 2;
				break;
			case 9:					//PMT
				srcInt = 3;
				break;
		}
	}

	unsigned short returnValue = 0x0;
	returnValue = returnValue | (opCode << 11) | (dstInt << 8) | (srcInt << 6);

	return returnValue;
}

unsigned short ReadFile::group10(int opCode)
{
	//MOV, IN, OUT

	int dstInt = 0x0;
	int srcInt = 0x0;

	std::string dst = std::string(strtok(NULL, " \t,"));
	if (!dst.empty())
		dstInt = formOperand(dst);

	std::string src = std::string(strtok(NULL, " \t,[]"));
	if (!src.empty())
		srcInt = formOperand(src);

	unsigned short returnValue = 0x0;
	returnValue = returnValue | (opCode << 11) | (dstInt << 8) | (srcInt << 5);

	return returnValue;
}

unsigned short ReadFile::group11(int opCode)
{
	//SHR, SHL

	int dstInt = 0x0;
	int srcInt = 0x0;
	int immInt = 0x0;

	std::string dst = std::string(strtok(NULL, " \t,"));
	if (!dst.empty())
		dstInt = formOperand(dst);

	std::string src = std::string(strtok(NULL, " \t,[]"));
	if (!src.empty())
		srcInt = formOperand(src);

	char* imm = strtok(NULL, " \t");
	if (imm != NULL)
		immInt = formImm(imm, 4);

	if (abs(immInt) > 7)
	{
		outOfRangeError = true;
		return 0;
	}

	unsigned short returnValue = 0x0;
	returnValue = returnValue | (opCode << 11) | (dstInt << 8) | (srcInt << 5) | (immInt << 1);

	return returnValue;
}

int ReadFile::formOperand(std::string operand)
{
	std::map<std::string, int>::const_iterator regIter = regMap.begin();

		for (regIter; regIter != commandMap.end(); ++regIter)
			if (operand.compare(regIter -> first) == 0)
				return regIter -> second;
	return 0;
}

int ReadFile::formImm(char* imm, int bits)
{
	int immInt = convertCharToInt(imm);
	int limit = 1 << bits;

	int mask = 1;
	mask = mask << bits;
	mask -= 1;

	if (abs(immInt) > limit)
	{
		std::cout << "GRESKA !!!\n" << "Imm van dozvoljenih granica za operaciju" << std::endl;
		outOfRangeError = true;
	}

	return (immInt & mask);
}

int ReadFile::formImm(int immInt, int bits)
{
	int limit = 1 << bits;

	int mask = 1;
	mask = mask << bits;
	mask -= 1;

	if (abs(immInt) > limit)
	{
		std::cout << "GRESKA !!!\n" << "Imm van dozvoljenih granica za operaciju" << std::endl;
		outOfRangeError = true;
	}

	return (immInt & mask);
}

void ReadFile::goBackToTheBeginning()
{
	myFile.clear();					//ciscenje eof flega, obavezno
	myFile.seekg(0, myFile.beg);

	locationCounter = 0;
	currentSection.clear();
}

bool ReadFile::isInRange(int labelOffset)
{
	return (abs(labelOffset - locationCounter) < 255);
}

void ReadFile::changeToGlobal()
{
	char* toChange = strtok(NULL, " \t\".,");
	while (toChange != 0)
	{
		mySymbolTable -> convertToGlobal(std::string(toChange), currentSection);
		toChange = strtok(NULL, " \t\".,");
	}
}

void ReadFile::changeToExtern()
{
	char* toChange = strtok(NULL, " \t\".,");
	while (toChange != 0)
	{
		addToReloc(toChange);
		toChange = strtok(NULL, " \t\".,");
	}
}

SymbolTable* ReadFile::getSymbol()
{
	return mySymbolTable;
}

GenericSection* ReadFile::getSectionArray()
{
	return sectionArray;
}

int ReadFile::getSectionNum()
{
	return sectionNum;
}

RelocationTable* ReadFile::getRel()
{
	return myRelocationTable;
}


int ReadFile::getAfterEndSymbolIndex()
{
	return afterEndSymbolIndex;
}

void ReadFile::testStatic()
{
	char* toAdd = strtok(NULL, " \t");
	for (int i = 0; i < 2; i++)
	{
		std::cout << std::string(toAdd) << std::endl;
		toAdd = strtok(NULL, " \t");
	}
}

bool ReadFile::isOpen()
{
	if (myFile == 0)
		return false;
	return true;
}

void ReadFile::test()
{
	/*std::string line;

	while (!myFile.eof())
	{
		std::cout << line << std::endl;
		std::getline(myFile, line);
	}

	goBackToTheBeginning();

	std::getline(myFile, line);
	std::cout << line << std::endl;

	std::getline(myFile, line);
	std::cout << line << std::endl;

	std::getline(myFile, line);
	std::cout << line << std::endl;*/

	//--------------------------------------

	//TESTIRANJE ISPISA TABELA 

	//--------------------------------------

	std::cout << "SYMBOL TABLE :\n" << std::endl;
	mySymbolTable -> printOut();
	std::cout << std::endl;

	for (int i = 0; i < sectionNum; i++)
	{
		std::cout << sectionArray[i].getSectionName() << " TABLE :\n" << std::endl;
		sectionArray[i].printOut();
		std::cout << std::endl;
	}

	std::cout << "RELOCATION TABLE :\n" << std::endl;

	myRelocationTable -> printOut();

	//--------------------------------------

	//TESTIRANJE VELICINE STRUKTURE 

	//--------------------------------------

	/*std::cout << "Size of SymbolTable: " << sizeof(mySymbolTable) << std::endl;
	std::cout << "Size of SymbolTable*: " << sizeof(mySymbolTable[0]) << std::endl;
	std::cout << "Size of GenericSection: " << sizeof(myTextSection) << std::endl;
	std::cout << "Size of GenericSection*: " << sizeof(myTextSection[0]) << std::endl;
	std::cout << "Size of GenericStruct - struct: " << sizeof(SectionStruct) << std::endl;*/

	//--------------------------------------

	//TESTIRANJE STRTOK SA STATICNOM METODOM 

	//--------------------------------------

	/*goBackToTheBeginning();

	char* nextLine = getNextLine();
	char* toAdd = strtok(nextLine, " \t");

	for (int i = 0; i < 2; i++)
	{
		std::cout << std::string(toAdd) << std::endl;
		toAdd = strtok(NULL, " \t");
	}

	testStatic();*/

}