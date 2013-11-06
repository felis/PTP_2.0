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
#if !defined(__PTPCALLBACK_H__)
#define __PTPCALLBACK_H__

#include <Usb.h>

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