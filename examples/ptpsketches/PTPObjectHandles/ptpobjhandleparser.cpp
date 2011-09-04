#include "ptpobjhandleparser.h"


void PTPObjHandleParser::Parse(const uint16_t len, const uint8_t *pbuf, const uint32_t &offset)
{
	uint16_t	cntdn	= (uint16_t)len;
	uint8_t		*p		= (uint8_t*)pbuf;

	switch (nStage)
	{
	case 0:
		p		+= 12;
		cntdn	-= 12;
		nStage	++;

	case 1:
		if (arrayParser.Parse(&p, &cntdn, (PTP_ARRAY_EL_FUNC)&PrintHandle))
			nStage = 0;
	}
}
