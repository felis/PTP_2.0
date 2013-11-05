#ifndef __STINFOPARSER_H__
#define __STINFOPARSER_H__

#include <ptpconst.h>
#include <ptp.h>

// Storage Types
const char msgUndefined		[] PROGMEM = "Undefined";
const char msgFixedROM		[] PROGMEM = "FixedROM";
const char msgRemovableROM	[] PROGMEM = "RemovableROM";
const char msgFixedRAM		[] PROGMEM = "FixedRAM";
const char msgRemovableRAM	[] PROGMEM = "RemovableRAM";

// File System Types
const char msgGenericFlat			[] PROGMEM = "GenericFlat";
const char msgGenericHierarchical	[] PROGMEM = "GenericHierarchical";
const char msgDCF					[] PROGMEM = "DCF";

// Access Capability
const char msgReadWrite						[] PROGMEM = "ReadWrite";
const char msgReadOnly						[] PROGMEM = "ReadOnly";
const char msgReadOnly_with_Object_Deletion	[] PROGMEM = "ReadOnly with Object Deletion";


class PTPStorageInfoParser : public PTPReadParser
{
	uint8_t							nStage;
	uint8_t							strByteCountDown;

	static const char*				stNames[];
	static const char*				fstNames[];
	static const char*				acNames[];

	void PrintStorageType(uint8_t **pp, uint16_t *pcntdn);
	void PrintFileSystemType(uint8_t **pp, uint16_t *pcntdn);
	void PrintAccessCapability(uint8_t **pp, uint16_t *pcntdn);
	void PrintMaxCapacity(uint8_t **pp, uint16_t *pcntdn);
	void PrintFreeSpaceInBytes(uint8_t **pp, uint16_t *pcntdn);
	void PrintFreeSpaceInImages(uint8_t **pp, uint16_t *pcntdn);
	bool PrintString(uint8_t **pp, uint16_t *pcntdn);

public:
	PTPStorageInfoParser() : nStage(0),	strByteCountDown(0) {};

	virtual void Parse(const uint16_t len, const uint8_t *pbuf, const uint32_t &offset);
};

#endif // __STINFOPARSER_H__
