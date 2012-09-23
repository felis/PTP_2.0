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
#include "eoseventparser.h"

bool EOSEventParser::EventRecordParse(uint8_t **pp, uint16_t *pcntdn)
{
	switch (nRecStage)
	{
	case 0:
		// Retrieves the size of the event record
		if (!valueParser.Parse(pp, pcntdn))
			return false;

		nRecSize = (uint16_t)varBuffer;

		// calculates the number of event parameters ( size / 4 - 1 )
		paramCountdown	= (nRecSize >> 2) - 1;
                
		paramCount		=  1;
		nRecSize		-= 4;
		nRecStage ++;
	case 1:
		for (; paramCountdown; paramCountdown--, paramCount++, nRecSize -= 4)
		{
			if (!valueParser.Parse(pp, pcntdn))
				return false;

			switch (paramCount)
			{
			// Event Code
			case 1:
				eosEvent.eventCode = (uint16_t)varBuffer;
				break;
			// Property Code
			case 2:
				eosEvent.propCode = (uint32_t)varBuffer;
				break;
			// C189 - Property Value, C18A - Enumerator Type
			case 3:
				eosEvent.propValue = varBuffer;
                                        
				if (pHandler)
				{
					if (eosEvent.eventCode == EOS_EC_DevPropChanged)
							pHandler->OnPropertyChanged(&eosEvent);
					if (eosEvent.eventCode == EOS_EC_ObjectCreated)
							pHandler->OnObjectCreated(&eosEvent);
				}
				break;
			// C18A/enumType == 3 - Size of enumerator array
			case 4:
				if (eosEvent.eventCode == EOS_EC_DevPropValuesAccepted)
					if (pHandler)
						pHandler->OnAcceptedListSize(&eosEvent, (const uint16_t)varBuffer);
				break;
			// C18A/enumType == 3 - Enumerator Values
			default:
				if (eosEvent.eventCode == EOS_EC_DevPropValuesAccepted)
					if (pHandler)
						pHandler->OnPropertyValuesAccepted(&eosEvent, paramCount-5, varBuffer);
			} // switch (paramCount)
		} // for
		nRecStage ++;
	case 2:
		if (nRecSize)
			if (!byteSkipper.Skip(pp, pcntdn, nRecSize))
				return false;

		nRecSize = 0;
		nRecStage = 0;
	}
	return true;
}

void EOSEventParser::InitEOSEventStruct()
{
	eosEvent.eventCode	= constInitialEventCode;
	eosEvent.propCode	= 0;
	eosEvent.propValue	= 0;
}


void EOSEventParser::Parse(const uint16_t len, const uint8_t *pbuf, const uint32_t &offset)
{
	uint8_t		*p	= (uint8_t*) pbuf;
	uint16_t	cntdn	= len;

	switch (nStage)
	{
	case 0:
		p		+= 12;
		cntdn	-= 12;
		nStage	++;
	case 1:
		theBuffer.valueSize = 4;
		valueParser.Initialize(&theBuffer);
		InitEOSEventStruct();
		nStage ++;
	case 2:
		while (1)
		{
			if (!EventRecordParse(&p, &cntdn))
				return;
			if (IsLastEventRecord())
				break;
			InitEOSEventStruct();
		}
		nStage = 0;
	}
}
