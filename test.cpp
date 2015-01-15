#include <stdio.h>
#include <iostream>
#include <sstream>
#include "ReadFile.h"
#include "WriteToFile.h"


int main(int argc, char** argv)
{
	ReadFile *thisFile = new ReadFile(argv[1]);

	if (!thisFile -> isOpen())
	{
		std::cout << "Greska pri otvaranju falja!" << std::endl;
		return -1;
	}

	//thisFile -> test();

	bool stop = false;

	stop = thisFile -> firstPass();
	if (stop) 
	{
		thisFile -> close();
		return 0;
	}

	stop = thisFile -> secondPass();
	if (stop) 
	{
		thisFile -> close();
		return 0;
	}

	thisFile -> close(); 

	//thisFile -> test();

	WriteToFile *output = new WriteToFile(argv[2], thisFile -> getSectionArray(), thisFile -> getSectionNum(), 
		thisFile -> getSymbol(), thisFile -> getRel(), thisFile -> getAfterEndSymbolIndex());
	output -> write();
	output -> close();

	/*delete thisFile;
	delete output;*/

	/*std::string inString = "";
	std::cout << std::endl;

	myFile.close();
	delete tempChar;
	delete toAdd;*/

	return 1;
}
