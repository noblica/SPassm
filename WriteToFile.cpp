#include "WriteToFile.h"

WriteToFile::WriteToFile(char* name, GenericSection* mySectionArray, int sectionNum, SymbolTable* symbolTable, RelocationTable* relocationTable, int endIndex)
{
	fileName = name;
	outputFile.open(name);

	sectionArray = mySectionArray;

	numOfSections = sectionNum;
	numOfHeaders = getNumOfHeaders() + sectionNum;

	headerInfo = new SectionInfo[numOfHeaders];
	headerArray = new SectionHeader[numOfHeaders];

	mySymbolTable = symbolTable;
	afterEndSymbolIndex = endIndex;
	myRelTable = relocationTable;
	relNum = 0;

	symTable = new Elf16_Sym[1000];

	relTable = new Elf16_Rel[255];

	strNum = 0;
	pointer = 0;
	stringPointer = getStringPointer();
	stringStartPointer = stringPointer;
}

int WriteToFile::getNumOfHeaders()
{
	int numNoBss = 0;
	for (int i = 0; i < numOfSections; i++)
	{
		std::string sectionName = sectionArray[i].getSectionName();
		if (sectionName.compare(0, 3, "bss") != 0)
			numNoBss++;
	}
	return numNoBss + 2; //symTable + stringTable
}	

WriteToFile::~WriteToFile()
{
	delete fileName;
	delete [] symTable;
	delete [] relTable;
	delete [] mySymbolTable;
	delete [] myRelTable;
	delete [] headerInfo;
	delete [] headerArray;
	delete [] sectionArray;
}

void WriteToFile::writeFileHeader(Elf16_Off sectionHeaderStart)
{
	fileHeader.e_ident[EI_MAG0] = ELFMAG0;
	fileHeader.e_ident[EI_MAG1] = ELFMAG1;
	fileHeader.e_ident[EI_MAG2] = ELFMAG2;
	fileHeader.e_ident[EI_MAG3] = ELFMAG3;

	fileHeader.e_type = ET_REL;
	fileHeader.e_machine = EM_386;
	fileHeader.e_version = EV_CURRENT;
	fileHeader.e_entry = getEndIndex();				//virtualna adresa ulazne tackep programa
	fileHeader.e_phoff = 0;							//offset u fajlu (u bajtovima) do zaglavlja programa, ili 0
	fileHeader.e_shoff = sectionHeaderStart;		//offset u fajlu u bajtovima do zaglavja sekcija, ili 0
	fileHeader.e_flags = 0;
	fileHeader.e_ehsize = sizeof(ElfHeader);		//velicina elf zaglavlja (u bajtovima)
	fileHeader.e_phentsize = 0;						//velicina zaglavlja programa (jedan ulaz u tabeli zaglavlja)
	fileHeader.e_phnum = numOfHeaders;							//broj ulaza u tabeli zagljava programa, ili 0 ako nema tabele
	fileHeader.e_shentsize = sizeof(SectionHeader);	//velicina zaglavlja sekcije (jedan ulaz u tabeli zagljava sekcije)
	fileHeader.e_shnum = numOfHeaders;				//broj ulaza u tabeli zagljava sekcija, ili 0 ako tabela nije prisutna
	fileHeader.e_shstrndx = stringStartPointer;		//indeks ulaza u tabeli sekcija koji odgovara section name string table

	outputFile.write((char*)&fileHeader, sizeof(fileHeader));
	pointer += sizeof(ElfHeader);
}

Elf16_Addr WriteToFile::getEndIndex()
{
	Elf16_Addr offset = 0;

	if (afterEndSymbolIndex != -1)
	{
		offset += afterEndSymbolIndex * sizeof(Elf16_Sym);	//svi pre

		for (int i = 0; i < numOfSections; i++)
			offset += getSectionSize(i);
	}

	return offset;
}

Elf16_Off WriteToFile::getSectionHeaderStart()
{
	Elf16_Off sectionHeaderStart = sizeof(ElfHeader);
	for (int i = 0; i < numOfSections; i++)
		if (sectionArray[i].getSectionName().compare(0, 3, "bss") != 0)
			sectionHeaderStart += getSectionSize(i);

	sectionHeaderStart += getSymbolSize();

	sectionHeaderStart += getRelSize();

	sectionHeaderStart += getStringTableSize();

	return sectionHeaderStart;
}

void WriteToFile::writeSection(int index)
{
	headerInfo[index].name = sectionArray[index].getSectionName();
	headerInfo[index].start = pointer;

	int temp = 0;
	headerInfo[index].alignment = sectionArray[index].getAlignValue();
	headerInfo[index].memStart = sectionArray[index].getStartOffset();

	SectionStruct* readSection = sectionArray[index].getSection();
	int size = sectionArray[index].getSize();

	if (!headerInfo[index].name.compare(0, 3, "bss") == 0)
	//{
		for (int i = 0; i < size; i++)
		{
			outputFile.write((char*)&readSection[i].hexEntry, sizeof(unsigned short));
			outputFile.write((char*)&readSection[i].offset, sizeof(unsigned short));
		}

		pointer += getSectionSize(index);
		headerInfo[index].size = pointer - headerInfo[index].start;
	//}
}

void WriteToFile::writeSymTable(int index)
{
	SymbolStruct* symStruct = mySymbolTable -> getSymbol();
	int size = mySymbolTable -> getSize();

	headerInfo[index].name = std::string(".symtab");
	headerInfo[index].start = pointer;
	headerInfo[index].alignment = 0;

	pointer += sizeof(Elf16_Sym) * size;
	headerInfo[index].size = pointer - headerInfo[index].start;

	for (int i = 0; i < size; i++)
	{
		symTable[i].st_name = stringPointer;
		stringPointer += addToStr(symStruct[i].label);

		symTable[i].st_value = symStruct[i].offset;
		symTable[i].st_size = 2;
		symTable[i].st_info = 0;
		symTable[i].st_other = 0;
		symTable[i].st_shndx = getSectionIndex(symStruct[i].section);
	}

	for (int i = 0; i < size; i++)
		outputFile.write((char*)&symTable[i], sizeof(symTable[i]));
}

int WriteToFile::writeRelTable(int index)
{
	RelocStruct* relStruct = myRelTable -> getRel();
	int size = myRelTable -> getSize();

	std::string currentSection = relStruct[0].section;
	int currentSectionNum = index;

	int numOfRelTables = 0;

	while (!currentSection.empty())
	{
		currentSection = formRelHeaderInfo(currentSection, currentSectionNum);
		currentSectionNum++;
		numOfRelTables++;
	}

	for (int i = 0; i < size; i++)
		addToRel(relStruct[i].offset);

	for (int i = 0; i < size; i++)
		outputFile.write((char*)&relTable[i], sizeof(relTable[i]));

	return numOfRelTables;
}

std::string WriteToFile::formRelHeaderInfo(std::string currentSection, int currentSectionNum)
{
	RelocStruct* relStruct = myRelTable -> getRel();
	int size = myRelTable -> getSize();

	std::string relSectionName = std::string(".rel.");
	relSectionName.append(currentSection);
		
	headerInfo[currentSectionNum].name = relSectionName;
	headerInfo[currentSectionNum].start = pointer;
	headerInfo[currentSectionNum].alignment = 0;

	int currentRelSectionSize = 0;
	int i = 0;
	bool stop = false;

	while (i < size)
		if (relStruct[i].section.compare(currentSection) == 0)
		{
			currentRelSectionSize++;
			stop = true;
			i++;
		}
		else 
			if (stop)
				break;

	pointer += sizeof(Elf16_Rel) * currentRelSectionSize;
	headerInfo[currentSectionNum].size = pointer - headerInfo[currentSectionNum].start;

	std::string newSection = "a";
	newSection.clear();
	if (currentSection.compare(relStruct[size - 1].section) != 0)
		newSection = relStruct[i].section;

	return newSection;
}

void WriteToFile::addToRel(Elf16_Addr offset)
{
	relTable[relNum].r_offset = offset;
	relTable[relNum].r_info = (R_386_PC32 << sizeof(Elf16_Word)) | relNum++;
}

void WriteToFile::skipStringTable(int index)
{
	headerInfo[index].name = std::string(".strtab");
	headerInfo[index].start = pointer;

	pointer += getStringTableSize();
	headerInfo[index].size = pointer - headerInfo[index].start;
}

void WriteToFile::formHeader(int index)
{
	headerArray[index].sh_name = stringPointer;
	stringPointer += addToStr(headerInfo[index].name);
	headerInfo[index].headerStart = pointer;

	headerArray[index].sh_type = SHT_PROGBITS;
	headerArray[index].sh_flags = SHF_ALLOC + SHF_EXECINSTR;			
	headerArray[index].sh_addr = headerInfo[index].memStart;			//virtuelna adresa prvog bajta, ako je sekcija u memoriji(fiksirana)
	headerArray[index].sh_offset = headerInfo[index].start;				//offset prvog bajta sekcije
	headerArray[index].sh_size = headerInfo[index].size;				//velicina sekcije
	headerArray[index].sh_link = 0;										
	headerArray[index].sh_info = 0; 									
	headerArray[index].sh_addralign = headerInfo[index].alignment;		//da li je zahtevano poravnanje
	headerArray[index].sh_entsize = 0;									//ako su ulazi fiksne velicine - ovde se upisuje velicina jednog ulaza u bajtovima

	//padPointer(headerInfo[index].alignment);
	pointer += sizeof(SectionHeader);
}

void WriteToFile::padPointer(Elf16_Off power)
{
	if (power == 0)
		return;

	Elf16_Off toThePowerOf = 1;

	for (int i = 0; i < power; i++)
		toThePowerOf *= 2;

	while (pointer % toThePowerOf != 0)
		pointer++;
}

void WriteToFile::writeStr()
{
	for (int i = 0; i < strNum; i++)
		outputFile.write(strTable[i].c_str(), sizeof(char) * strTable[i].size());

	//padding

		/*(std::string zero = "0";
		outputFile.write(zero.c_str(), 2);*/
}

void WriteToFile::writeHeaders()
{
	for (int i = 0; i < numOfHeaders; i++)
	{
		padBytes(i);
		outputFile.write((char*)&headerArray[i], sizeof(headerArray[i]));
	}
}

void WriteToFile::padBytes(int index)
{
	Elf16_Off power = headerArray[index].sh_addralign;
	if (power == 0)
		return;

	Elf16_Off toThePowerOf = 1;

	for (int i = 0; i < power; i++)
		toThePowerOf *= 2;

	while (headerArray[index].sh_offset % toThePowerOf != 0)
	{
		headerArray[index].sh_offset++;
		std::string zero = "0";
		outputFile.write(zero.c_str(), 1);
	}
}

int WriteToFile::addToStr(std::string strToAdd)
{
	strTable[strNum] = strToAdd;
	return sizeof(char) * strTable[strNum++].size();
}


Elf16_Off WriteToFile::getSectionIndex(std::string section)
{
	for (int i = 0; i < numOfSections; i++)
		if (headerInfo[i].name.compare(section) == 0)
			return headerInfo[i].start;

	return 0;
}

Elf16_Off WriteToFile::getStringPointer()
{
	int totalSectionSize = sizeof(ElfHeader);
	for (int i = 0; i < numOfSections; i++)
		totalSectionSize += getSectionSize(i);
	totalSectionSize += getSymbolSize() + getRelSize();

	return totalSectionSize;
}

Elf16_Off WriteToFile::getSectionSize(int index)
{
	if (sectionArray[index].getSectionName().compare(0, 3, "bss") == 0)
		return 0;
	return sectionArray[index].getSize() * 2 * sizeof(unsigned short);
}

Elf16_Off WriteToFile::getSymbolSize()
{
	int size = mySymbolTable -> getSize();
	return size * sizeof(Elf16_Sym);
}

Elf16_Off WriteToFile::getRelSize()
{
	int size = myRelTable -> getSize();
	return size * sizeof(Elf16_Rel);
}

Elf16_Off WriteToFile::getRelStringSize()
{
	RelocStruct* relStruct = myRelTable -> getRel();
	int size = myRelTable -> getSize();
	Elf16_Off totalSize = 0;
	std::string currentSection = "";

	for (int i = 0; i < size; i++)
		if (currentSection.compare(relStruct[i].section) != 0)
		{
			currentSection = ".rel.";
			currentSection.append(relStruct[i].section);

			totalSize += sizeof(char) * currentSection.size();

			currentSection = relStruct[i].section;
		}

	return totalSize;
}

Elf16_Off WriteToFile::getStringTableSize()
{
	Elf16_Off totalSize = 0;
	int symSize = mySymbolTable -> getSize();
	SymbolStruct* symbolTable = mySymbolTable -> getSymbol();

	for (int i = 0; i < symSize; i++)
		totalSize += sizeof(char) * symbolTable[i].label.size();

	for (int i = 0; i < numOfSections; i++)
	{
		std::string sectionName = sectionArray[i].getSectionName();
		totalSize += sizeof(char) * sectionName.size();
	}

	totalSize += getRelStringSize();

	return totalSize + sizeof(char) * (std::string(".symtab").size() + std::string(".strtab").size());
}

bool WriteToFile::checkForOverlap()
{
	
	int currentSectionOffset = sectionArray[0].getStartOffset();
	for (int i = 0; i < numOfSections; i++)
	{
		int currentSectionOffset = sectionArray[i].getStartOffset();
		int currentSectionSize = getSectionSize(i);

		if (currentSectionOffset != 0)
			for (int j = 0; j < numOfSections; j++)		
			{
				if (j != i)
				{
					int nextSectionOffset = sectionArray[j].getStartOffset();

					if (nextSectionOffset > 0)
						if ((currentSectionOffset <= nextSectionOffset) && ((currentSectionOffset + currentSectionSize) > nextSectionOffset))
							return true;
				}
			}
	}
	return false;
}

void WriteToFile::close()
{
	outputFile.close();
}

void WriteToFile::write()
{
	bool overlapError = checkForOverlap();

	if (overlapError)
	{
		std::cout << "Sekcije se preklapaju!!!" << std::endl;
		return;
	}

	sectionHeaderStart = getSectionHeaderStart();

	writeFileHeader(sectionHeaderStart);

	for (int i = 0; i < numOfSections; i++)
		writeSection(i);

	writeSymTable(numOfSections);
	int numOfRelTables = writeRelTable(numOfSections + 1);
	skipStringTable(numOfSections + 1 + numOfRelTables);

	for (int i = 0; i < numOfHeaders; i++)
		formHeader(i);

	writeStr();

	writeHeaders();
}