#ifndef _GENERICSECTION_H
#define _GENERICSECTION_H

#include <string>
#include <iostream>

typedef struct 
{	
	unsigned short hexEntry;
	unsigned short offset;
	std::string textEntry;
} SectionStruct;

class GenericSection
{
public:
	GenericSection();

	void setSectionName(std::string toName);

	void setAlignValue(int value);

	int getAlignValue();

	void add(int offset, unsigned short hexEntry, std::string textEntry);

	void printOut();

	SectionStruct* getSection();

	int getSize();

	std::string getSectionName();

	int getStartOffset();

	void setStartOffset(int offset);

	~GenericSection();

private:
	SectionStruct* genericSection;
	int alignValue;
	int startOffset;
	std::string sectionName;
	int num;
};

#endif