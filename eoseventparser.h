#ifndef __EOSEVENTPARSER_H__
#define __EOSEVENTPARSER_H__

#include <inttypes.h>
#include <parsetools.h>
#include <canoneos.h>

struct EOSEvent
{
	uint16_t	eventCode;
	uint32_t	propCode;
	uint32_t	propValue;
};

class EOSEventHandlers
{
public:
	virtual void OnPropertyChanged(const EOSEvent *evt) = 0;
	virtual void OnAcceptedListSize(const EOSEvent *evt, const uint16_t size) = 0;
	virtual void OnPropertyValuesAccepted(const EOSEvent *evt, const uint16_t index, const uint32_t &val) = 0;
	virtual void OnObjectCreated(const EOSEvent *evt) = 0;
	//void OnCaptureComplete() = 0;
};

class EOSEventParser : public PTPReadParser
{
	const uint16_t			constInitialEventCode;

	EOSEventHandlers		*pHandler;

	uint8_t					paramsChanged;

	uint8_t					nStage;
	uint8_t					nRecStage;
	uint16_t				nRecSize;
	MultiValueBuffer		theBuffer;
	uint32_t				varBuffer;
	EOSEvent				eosEvent;
	uint16_t				paramCountdown;
	uint16_t				paramCount;

	MultiByteValueParser	valueParser;
	ByteSkipper				byteSkipper;

	bool EventRecordParse(uint8_t **pp, uint16_t *pcntdn);
	bool IsLastEventRecord() { return (eosEvent.eventCode == 0); };
	void InitEOSEventStruct();

public:
	EOSEventParser(EOSEventHandlers *p) : 
		pHandler(p),
		constInitialEventCode(0xFFFF),
		nStage(0),
		nRecStage(0),
		nRecSize(0),
		varBuffer(0),
		paramCountdown(0),
		paramsChanged(0)
		{ 
			theBuffer.pValue = &varBuffer; 
		};

	void Reset()
	{
		nStage			= 0;
		nRecStage		= 0;
		nRecSize		= 0;
		varBuffer		= 0;
		paramCountdown	= 0;
		paramsChanged	= 0;
	};

	virtual void Parse(const uint16_t len, const uint8_t *pbuf, const uint32_t &offset);
};

#endif // __EOSEVENTPARSER_H__
