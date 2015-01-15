#ifndef _WRITETOFILE_H
#define _WRITETOFILE_H

#include <string>
#include <fstream>
#include <iostream>
#include "SymbolTable.h"
#include "GenericSection.h"
#include "RelocationTable.h"

#define EI_NIDENT 16

#define	EI_MAG0			0	/* e_ident[] indexes */
#define	EI_MAG1			1
#define	EI_MAG2			2
#define	EI_MAG3			3
#define	EI_CLASS		4	/* File class */
#define	EI_DATA			5	/* Data encoding */
#define	EI_VERSION		6	/* File version */
#define	EI_OSABI		7	/* Operating system/ABI identification */
#define	EI_ABIVERSION	8	/* ABI version */
#define	EI_PAD			9	/* Start of padding bytes */

#define	ELFMAG0		0x7f		/* EI_MAG */
#define	ELFMAG1		'E'
#define	ELFMAG2		'L'
#define	ELFMAG3		'F'

#define	ET_NONE		0		/* e_type */
#define	ET_REL		1
#define	ET_EXEC		2
#define	ET_DYN		3
#define	ET_CORE		4
#define	ET_LOPROC	0xff00		/* processor specific range */
#define	ET_HIPROC	0xffff

#define	EM_NONE		0		/* e_machine */
#define	EM_M32		1		/* AT&T WE 32100 */
#define	EM_SPARC	2		/* Sun SPARC */
#define	EM_386		3		/* Intel 80386 */
#define	EM_68K		4		/* Motorola 68000 */
#define	EM_88K		5		/* Motorola 88000 */
#define	EM_486		6		/* Intel 80486 */
#define	EM_860		7		/* Intel i860 */
#define	EM_MIPS		8		/* MIPS RS3000 Big-Endian */
#define	EM_S370		9		/* IBM System/370 Processor */
#define	EM_MIPS_RS3_LE	10		/* MIPS RS3000 Little-Endian */

#define	EV_NONE		0		/* e_version, EI_VERSION */
#define	EV_CURRENT	1

typedef unsigned char Elf16_Half;
typedef unsigned short Elf16_Word;
typedef unsigned short Elf16_Addr;
typedef unsigned short Elf16_Off;

typedef struct 
{
	unsigned char e_ident[EI_NIDENT];
	Elf16_Half 		e_type;				
	Elf16_Half 		e_machine;
	Elf16_Word 		e_version;
	Elf16_Addr 		e_entry;
	Elf16_Off 		e_phoff;
	Elf16_Off		e_shoff;
	Elf16_Word		e_flags;
	Elf16_Half		e_ehsize;
	Elf16_Half		e_phentsize;
	Elf16_Half		e_phnum;
	Elf16_Half		e_shentsize;
	Elf16_Half		e_shnum;
	Elf16_Half		e_shstrndx;

} ElfHeader;

#define	SHN_UNDEF	0		/* special section numbers */
#define	SHN_LORESERVE	0xff00
#define	SHN_LOPROC	0xff00		/* processor specific range */
#define	SHN_HIPROC	0xff1f
#define	SHN_ABS		0xfff1
#define	SHN_COMMON	0xfff2
#define	SHN_HIRESERVE	0xffff

#define	SHT_NULL		0		/* sh_type */
#define	SHT_PROGBITS	1
#define	SHT_SYMTAB		2
#define	SHT_STRTAB		3
#define	SHT_RELA		4
#define	SHT_HASH		5
#define	SHT_DYNAMIC		6
#define	SHT_NOTE		7
#define	SHT_NOBITS		8
#define	SHT_REL			9
#define	SHT_SHLIB		10
#define	SHT_DYNSYM		11

#define	SHT_LOPROC	0x70000000	/* processor specific range */
#define	SHT_HIPROC	0x7fffffff

#define	SHT_LOUSER	0x80000000
#define	SHT_HIUSER	0xffffffff

#define	SHF_WRITE				0x01		/* sh_flags */
#define	SHF_ALLOC				0x02
#define	SHF_EXECINSTR			0x04
#define	SHF_MASKPROC	0xf0000000	/* processor specific values */

typedef struct
{
	Elf16_Word		sh_name;
	Elf16_Word		sh_type;
	Elf16_Word		sh_flags;
	Elf16_Addr		sh_addr;
	Elf16_Off		sh_offset;
	Elf16_Word		sh_size;
	Elf16_Word		sh_link;
	Elf16_Word		sh_info;
	Elf16_Word		sh_addralign;
	Elf16_Word		sh_entsize;

} SectionHeader;

#define	STB_LOCAL	0		/* BIND */
#define	STB_GLOBAL	1
#define	STB_WEAK	2
#define	STB_LOPROC	13		/* processor specific range */
#define	STB_HIPROC	15

#define	STT_NOTYPE	0		/* TYPE */
#define	STT_OBJECT	1
#define	STT_FUNC	2
#define	STT_SECTION	3
#define	STT_FILE	4

#define	STT_LOPROC	13		/* processor specific range */
#define	STT_HIPROC	15

typedef struct 
{
	Elf16_Word		st_name;
	Elf16_Addr		st_value;
	Elf16_Word		st_size;		//verovatno mi ne treba, posto je size uvek 2B
	unsigned char	st_info;		//verovatno mi ne treba ???
	unsigned char	st_other;		//zasta je ovo??
	Elf16_Half		st_shndx;

} Elf16_Sym;

#define R_386_NONE 0
#define R_386_32 1
#define R_386_PC32 2


typedef struct
{
	Elf16_Addr r_offset;
	Elf16_Word r_info;
} Elf16_Rel;

typedef struct
{
	Elf16_Off start;
	Elf16_Off size;
	Elf16_Off headerStart;
	Elf16_Addr memStart;
	Elf16_Off alignment;
	std::string name;
} SectionInfo;

class WriteToFile
{
public:

	WriteToFile(char* name, GenericSection* mySectionArray, int sectionNum, SymbolTable* symbolTable, RelocationTable* relocationTable, int endIndex);

	~WriteToFile();
	
	Elf16_Off getSectionHeaderStart();
	int getNumOfHeaders();
	Elf16_Addr getEndIndex();
	void writeFileHeader(Elf16_Off sectionHeaderStart);
	void writeSection(int index);

	void writeSymTable(int index);
	int writeRelTable(int index);
	Elf16_Off getRelStringSize();
	std::string formRelHeaderInfo(std::string currentSection, int currentSectionNum);

	void addToRel(Elf16_Addr offset);
	void skipStringTable(int index);

	void formHeader(int index);

	void writeStr();
	void writeHeaders();
	void padBytes(int index);
	void padPointer(Elf16_Off power);
	int addToStr(std::string strToAdd);

	Elf16_Off getSectionIndex(std::string section);
	Elf16_Off getStringPointer();
	Elf16_Off getSectionSize(int index);
	Elf16_Off getSymbolSize();
	Elf16_Off getRelSize();
	Elf16_Off getStringTableSize();

	bool checkForOverlap();

	void close();
	void write();

private:
	char* fileName;
	std::ofstream outputFile;
	Elf16_Off pointer;
	Elf16_Half stringPointer;
	Elf16_Half stringStartPointer;

	ElfHeader fileHeader;
	GenericSection* sectionArray;

	SectionInfo *headerInfo;
	SectionHeader *headerArray;

	Elf16_Half numOfSections;
	Elf16_Half numOfHeaders;
	Elf16_Off sectionHeaderStart;

	std::string strTable[1000];
	int strNum;

	SymbolTable* mySymbolTable;
	int afterEndSymbolIndex;
	RelocationTable* myRelTable;
	int relNum;

	Elf16_Sym* symTable;
	Elf16_Rel* relTable;
};

#endif