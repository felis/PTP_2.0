#ifndef __PTPOBJHANDLEPARSER_H__
#define __PTPOBJHANDLEPARSER_H__

#include <Usb.h>
#include "ptpcallback.h"
#include "ptpdebug.h"

class PTPObjHandleParser : public PTPReadParser
{
	MultiValueBuffer					theBuffer;
	uint32_t						varBuffer;
	uint8_t							nStage;
	PTPListParser						arrayParser;

	static void PrintHandle(MultiValueBuffer *p, uint32_t count)
	{
		Serial.print(count, DEC);
		E_Notify(PSTR("\t"), 0x80);
		PrintHex<uint32_t>(*((uint32_t*)p->pValue), 0x80);
		E_Notify(PSTR("\r\n"), 0x80);
	};

public:
	PTPObjHandleParser() : nStage(0) { theBuffer.valueSize = 4; theBuffer.pValue = &varBuffer; arrayParser.Initialize(4, 4, &theBuffer); };
	virtual void Parse(const uint16_t len, const uint8_t *pbuf, const uint32_t &offset);
};


#endif // __PTPOBJHANDLEPARSER_H__
