#if !defined(__PTPCALLBACK_H__)
#define __PTPCALLBACK_H__

#include <inttypes.h>
#include <hexdump.h>

// Base class for incomming data parser
class PTPReadParser
{
public:
	virtual void Parse(const uint16_t len, const uint8_t *pbuf, const uint32_t &offset) = 0;
};

// Base class for outgoing data supplier
class PTPDataSupplier
{
public:
	virtual uint32_t GetDataSize() = 0;
	virtual void GetData(const uint16_t len, uint8_t *pbuf) = 0;
};

typedef HexDumper<PTPReadParser, uint16_t, uint32_t>	HexDump;

#endif // __PTPCALLBACK_H__