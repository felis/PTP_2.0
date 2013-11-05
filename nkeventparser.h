/* Copyright (C) 2011 Circuits At Home, LTD. All rights reserved.

This software may be distributed and modified under the terms of the GNU
General Public License version 2 (GPL2) as published by the Free Software
Foundation and appearing in the file GPL2.TXT included in the packaging of
this file. Please note that GPL2 Section 2[b] requires that all works based
on this software must also be made publicly available under the terms of
the GPL2 ("Copyleft").

Contact information
-------------------

Circuits At Home, LTD
Web      :  http://www.circuitsathome.com
e-mail   :  support@circuitsathome.com
*/
#ifndef __NKEVENTPARSER_H__
#define __NKEVENTPARSER_H__

#include <Usb.h>
#include <nikon.h>

struct NKEvent
{
	uint16_t	eventCode;

	union
	{
		struct
		{
			uint16_t	wParam1;
			uint16_t	wParam2;
		};
		uint32_t		dwParam;
	};
};
struct NKPropertyChangedEvent
{
	uint16_t	eventCode;
	uint16_t	propCode;
	uint16_t	wordAllign;
};

struct NKObjAddedEvent
{
	uint16_t	eventCode;
	uint32_t	objHandle;
};

class NKEventHandlers
{
public:
	virtual void OnEvent(const NKEvent *evt) = 0;
	//virtual void OnPropertyChanged(const EOSEvent *evt) = 0;
	//virtual void OnAcceptedListSize(const EOSEvent *evt, const uint16_t size) = 0;
	//virtual void OnPropertyValuesAccepted(const EOSEvent *evt, const uint16_t index, const uint32_t &val) = 0;
	//void OnObjectCreated() = 0;
	//void OnCaptureComplete() = 0;
};

class NKEventParser : public PTPReadParser
{
	const uint16_t			constInitialEventCode;

	NKEventHandlers			*pHandler;

	uint8_t					paramsChanged;

	uint8_t					nStage;
	uint8_t					nRecStage;

	uint16_t				numEvents;
	uint8_t					eventCountdown;

	MultiValueBuffer		theBuffer;
	uint16_t				varBuffer[3];
	//NKEvent					nkEvent;

	MultiByteValueParser	valueParser;
	//ByteSkipper				byteSkipper;

public:
	NKEventParser(NKEventHandlers *p) :
		pHandler(p),
		constInitialEventCode(0xFFFF),
		nStage(0),
		numEvents(0)
		{
			for (uint8_t i=0; i<3; i++)
				varBuffer[i] = 0;

			theBuffer.pValue = &varBuffer;
		};

	void Reset()
	{
		nStage			= 0;

		for (uint8_t i=0; i<3; i++)
			varBuffer[i] = 0;
	};

	virtual void Parse(const uint16_t len, const uint8_t *pbuf, const uint32_t &offset);
};

#endif // __NKEVENTPARSER_H__
