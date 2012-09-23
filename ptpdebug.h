#ifndef __PTPDEBUG_H__
#define __PTPDEBUG_H__

#include <inttypes.h>
#include <avr/pgmspace.h>
#include "../USBHost/printhex.h"
#include "../USBHost/hexdump.h"
#include "../USBHost/message.h"

//#define PTPDEBUG

//#define Message(m,r) (ErrorMessage<uint16_t>((m),(r)))

#if defined( PTPDEBUG )
#define PTPTRACE(s)(Notify(PSTR((s))))
#define PTPTRACE2(s,r)(ErrorMessage<uint16_t>(PSTR((s)),(r)))
#else
#define PTPTRACE(s)((void)0)
#define PTPTRACE2(s,r)(delay(1))	// necessary for some PowerShot cameras to work properly
#endif

#endif // __PTPDEBUG_H__