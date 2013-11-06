#ifndef __EOSEVENTHANDLERS_H__
#define __EOSEVENTHANDLERS_H__

#include <Usb.h>
#include <Max_LCD.h>
#include <canoneos.h>
#include <eoseventparser.h>

class EosEventHandlers : public EOSEventHandlers
{
public:
	virtual void OnPropertyChanged(const EOSEvent *evt);
	virtual void OnAcceptedListSize(const EOSEvent *evt, const uint16_t size);
	virtual void OnPropertyValuesAccepted(const EOSEvent *evt, const uint16_t index, const uint32_t &val);
};

#endif // __EOSEVENTHANDLERS_H__
