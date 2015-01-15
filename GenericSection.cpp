#include "GenericSection.h"

GenericSection::GenericSection()
{
	genericSection = new SectionStruct[1000];
	num = 0;
	alignValue = 0;
	startOffset = 0;
}

GenericSection::~GenericSection()
{
	delete [] genericSection;
}

void GenericSection::setSectionName(std::string toName)
{
	sectionName = toName;
}

void GenericSection::setAlignValue(int value)
{
	alignValue = value;
}

int GenericSection::getAlignValue()
{
	return alignValue;
}

void GenericSection::add(int offset, unsigned short hexEntry, std::string textEntry)
{
	genericSection[num].offset = offset;
	genericSection[num].hexEntry = hexEntry;
	genericSection[num++].textEntry = textEntry;
}

void GenericSection::printOut()
{
	int n = num;

	for (int i = 0; i < n; i++)
		std::cout << i << ". " << genericSection[i].offset << " | " << genericSection[i].hexEntry << " | " << genericSection[i].textEntry << std::endl;
}

SectionStruct* GenericSection::getSection()
{
	return genericSection;
}

int GenericSection::getSize()
{
	return num;
}

std::string GenericSection::getSectionName()
{
	return sectionName;
}

int GenericSection::getStartOffset()
{
	return startOffset;
}

void GenericSection::setStartOffset(int offset)
{
	startOffset = offset;
}