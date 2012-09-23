#include "nkeventparser.h"

void NKEventParser::Parse(const uint16_t len, const uint8_t *pbuf, const uint32_t &offset)
{
	uint8_t		*p	= (uint8_t*) pbuf;
	uint16_t	cntdn	= len;

	switch (nStage)
	{
	case 0:
		p		+= 12;
		cntdn	-= 12;
		nStage	= 1;
	case 1:
		theBuffer.valueSize = 2;
		valueParser.Initialize(&theBuffer);
		nStage = 2;
	case 2:
		if (!valueParser.Parse(&p, &cntdn))
			return;

		numEvents = (uint16_t)varBuffer;
		eventCountdown = numEvents;

		nStage = 3;
	case 3:
		theBuffer.valueSize = sizeof(NKEvent);
		valueParser.Initialize(&theBuffer);
		nStage = 4;
	case 4:
		for (; eventCountdown; eventCountdown--)
		{
			if (!valueParser.Parse(&p, &cntdn))
				return;

			if (pHandler)
				pHandler->OnEvent((NKEvent*)varBuffer);
		}
		nStage = 0;
	}
}
