#if !defined(__EOSEVENTDUMP_H__)
#define __EOSEVENTDUMP_H__

#include <inttypes.h>
#include <avr/pgmspace.h>
#include "ptpcallback.h"
#include "parsetools.h"

#if defined(ARDUINO) && ARDUINO >=100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

class EOSEventDump : public PTPReadParser
{
	uint32_t				ptppktSize;
	uint16_t				recordSize;
	uint8_t					parseStage;
	uint8_t					parseSubstage;

	MultiByteValueParser	valueParser;
	MultiValueBuffer		valueBuffer;
	uint32_t				theBuffer;
public:
	EOSEventDump() : ptppktSize(0), recordSize(0), parseStage(0), parseSubstage(0) 
		{ valueBuffer.valueSize = 4; valueBuffer.pValue = &theBuffer; };
	void Initialize() { ptppktSize = 0; recordSize = 0; parseStage = 0; valueParser.Initialize(&valueBuffer); };
	virtual void Parse(const uint16_t len, const uint8_t *pbuf, const uint32_t &offset);
};

#endif // __EOSEVENTDUMP_H__