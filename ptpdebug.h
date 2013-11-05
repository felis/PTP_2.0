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
#ifndef __PTPDEBUG_H__
#define __PTPDEBUG_H__

#include <Usb.h>

//#define PTPDEBUG

//#define Message(m,r) (ErrorMessage<uint16_t>((m),(r)))

#if defined( PTPDEBUG )
#define PTPTRACE(s)(E_Notify(PSTR((s))),0x80)
#define PTPTRACE2(s,r)(ErrorMessage<uint16_t>(PSTR((s)),(r)))
#else
#define PTPTRACE(s)((void)0)
#define PTPTRACE2(s,r)(delay(1))	// necessary for some PowerShot cameras to work properly
#endif

#endif // __PTPDEBUG_H__