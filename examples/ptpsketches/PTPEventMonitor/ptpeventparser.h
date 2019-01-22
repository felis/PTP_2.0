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
#ifndef __PTPEVENTPARSER_H__
#define __PTPVENTPARSER_H__

#include <Usb.h>
#include <ptp.h>
// #include <nikon.h>

struct PTPEvent
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

struct PTPPropertyChangedEvent
{
	uint16_t	eventCode;
	uint16_t	propCode;
	uint16_t	wordAllign;
};

struct PTPObjAddedEvent
{
	uint16_t	eventCode;
	uint32_t	objHandle;
};

class PTPEventHandlers
{
public:
	virtual void OnEvent(const PTPUSBEventContainer* evt);
	//virtual void OnPropertyChanged(const EOSEvent *evt) = 0;
	//virtual void OnAcceptedListSize(const EOSEvent *evt, const uint16_t size) = 0;
	//virtual void OnPropertyValuesAccepted(const EOSEvent *evt, const uint16_t index, const uint32_t &val) = 0;
	//void OnObjectCreated() = 0;
	//void OnCaptureComplete() = 0;
};

class PTPStringParser : public PTPReadParser
{
    uint8_t nStage;
    uint32_t container_length;
    uint8_t ptpstring_length;
    uint8_t string_buf[64]; // kludge
    const char* pstr = reinterpret_cast<const char*>(string_buf);
    
public:
    PTPStringParser() : nStage(0), container_length(0), ptpstring_length(0) {};
    void Parse(const uint16_t len, const uint8_t *pbuf, const uint32_t &offset);
    void GetFilename();
};

class PTPEventParser : public PTPReadParser
{

    PTP* ptpdevice;
    
    const uint16_t constInitialEventCode;
    
    // uint8_t eventLength;    // total length of USB event container, transmitted
                            // in first 4 bytes of event IN data. We don't care
                            // about last 3, ever
    
    PTPUSBEventContainer evt_container;
    
    uint8_t paramsChanged;

    uint8_t nStage;
    uint8_t nRecStage;

    uint16_t numEvents;
    uint8_t eventCountdown;

    MultiValueBuffer theBuffer;
    uint16_t varBuffer[3];
    //NKEvent					nkEvent;

    MultiByteValueParser valueParser;
    //ByteSkipper				byteSkipper;
    
    

public:
	PTPEventParser(PTP* ptpdevice) :
		ptpdevice(ptpdevice),
		constInitialEventCode(0xFFFF),
                // eventLength(0),
                // evt_container(0),
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

	void Parse(const uint16_t len, const uint8_t *pbuf, const uint32_t &offset);
        void GetFilename();
};

#endif // __PTPEVENTPARSER_H__
