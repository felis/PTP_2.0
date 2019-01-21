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
#include "ptpeventparser.h"

static void uint8_to_uint16( uint8_t* data, uint16_t* integer ) {
    
    PTPTRACE8("uint8_t data", *data );
    PTPTRACE8("uint8_t data", *(data+1));
    *integer = 0;
    *integer |= (uint16_t)*data;
    PTPTRACE2("uint16_t data0", *integer );
    *integer |= ((uint16_t)*(data+1)) << 8;
    PTPTRACE2("uint16_t data1", *integer );
}

static void uint8_to_uint32( uint8_t* data, uint32_t* integer ) {
    PTPTRACE8("uint8_t data0", *data);
    PTPTRACE8("uint8_t data1", *(data+1));
    PTPTRACE8("uint8_t data2", *(data+2));
    PTPTRACE8("uint8_t data3", *(data+3));
    *integer = 0;
    *integer = (uint32_t)*(data);
    PTPTRACE32("uint32_t data0", *integer );
    *integer |= ((uint32_t)*(data+1)) << 8;
    PTPTRACE32("uint32_t data1", *integer );
    *integer |= ((uint32_t)*(data+2)) << 16;
    PTPTRACE32("uint32_t data2", *integer );
    *integer |= ((uint32_t)*(data+3)) << 24;
    PTPTRACE32("uint32_t data3", *integer );
}

// assumes Max.packet size 8. TODO: fix
void PTPEventParser::Parse(const uint16_t len, const uint8_t *pbuf,
        const uint32_t &offset __attribute__ ((unused)))
{
	uint8_t* p = (uint8_t*)pbuf;
	uint16_t cntdn	= len;
        
        PTPTRACE2("PTPEventParser: len: ", len);
        PTPTRACE2("PTPEventParser: buf[0]: ", *pbuf);

	switch (nStage) {   
	case 0: // read container length
		// p += 12;
		// cntdn	-= 12;
            p += 4; // point past length
            cntdn -= 4;
            evt_container.length = *pbuf;
            PTPTRACE2("PTPEventParser: len ", evt_container.length);
            nStage	= 1;
	case 1: // read container type 
		// theBuffer.valueSize = 2;
		// valueParser.Initialize(&theBuffer);
            uint8_to_uint16(p, &evt_container.type);
            if (evt_container.type != PTP_USB_CONTAINER_EVENT) {
                PTPTRACE2("PTPEventParser: containter type mismatch", evt_container.type);
            }
            // PTPTRACE2("PTPEventParser: type ", evt_container.type);
            p += 2; // next field
            cntdn -= 2;
            nStage = 2;
	case 2: // read event code
            uint8_to_uint16(p, &evt_container.code);
            PTPTRACE2("PTPEventParser: code ", evt_container.code);
            p += 2;
            cntdn -= 2;
            
		// if (!valueParser.Parse(&p, &cntdn))
		//	return;

		// numEvents = (uint16_t)varBuffer;
                // PTPTRACE2("PTPEventParser: numEvents ", numEvents);
		// eventCountdown = numEvents;
            nStage = 3;
            PTPTRACE2("PTPEventParser: countdown ", cntdn);
	case 3: // read transaction ID
            uint8_to_uint32(p, &evt_container.trans_id);
            PTPTRACE32("PTPEventParser: trans.ID ", evt_container.trans_id);
            p += 4;
            cntdn -= 4;
		// theBuffer.valueSize = sizeof(PTPEvent);
		// valueParser.Initialize(&theBuffer);
            nStage = 4;
	case 4: // parameters
            uint8_t i = 0;
            uint32_t param_tmp;
            while( cntdn ) {
                uint8_to_uint32(p, &param_tmp);
                evt_container.params[i++] = param_tmp;
                PTPTRACE32("PTPEventParser: param1 ", evt_container.params[0]);
                PTPTRACE32("PTPEventParser: param2 ", evt_container.params[1]);
                PTPTRACE32("PTPEventParser: param3 ", evt_container.params[2]);
                p += 4;
                cntdn -= 4;
            }
            
            if (evt_container.code == PTP_EC_ObjectAdded ) {
                PTPTRACE("PTPEventParser: object added ");
                
            }
            nStage = 0;
	} // switch(nStage...
}
