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
#ifndef __PTP_H__
#define __PTP_H__

#include <Usb.h>
#include "ptpconst.h"
#include "ptpmsgstr.h"
#include "ptpdebug.h"
#include "ptpcallback.h"

// Buffer size should NEVER be less than USB packet size!!!!!!!!!!!!!!!!!!!!!
#define PTP_MAX_RX_BUFFER_LEN	64
#define PTP_MAX_EV_BUFFER_LEN	 8

// States
#define PTP_STATE_DEVICE_DISCONNECTED		1
#define PTP_STATE_SESSION_NOT_OPENED		2
#define PTP_STATE_SESSION_OPENED			3
#define PTP_STATE_DEVICE_INITIALIZED		4
#define PTP_STATE_DEVICE_NOT_RESPONDING		5
#define PTP_STATE_DEVICE_BUSY				6

#define FAILED(rc)(rc != PTP_RC_OK)

class PTP;
class PTPReadParser;
class PTPDataSupplier;

class PTPStateHandlers
{
public:
	virtual void OnDeviceDisconnectedState(PTP *ptp);
	virtual void OnSessionNotOpenedState(PTP *ptp);
	virtual void OnSessionOpenedState(PTP *ptp);
	virtual void OnDeviceInitializedState(PTP *ptp);
	virtual void OnDeviceNotRespondingState(PTP *ptp);
	virtual void OnDeviceBusyState(PTP *ptp);
};


class PTP : public USBDeviceConfig
{
	uint8_t		theState;

protected:
	void SetInitialState();
	void Task();

private:
	typedef uint16_t transaction_id_t;

	transaction_id_t		idTransaction;			// Transaction ID
	uint16_t			idSession;				// Session ID

	PTPStateHandlers	*stateMachine;

protected:
	bool				bPollEnable;			// Poll enable flag. If true - device polling is enabled;
	uint8_t				devAddress;				// Device address
	uint8_t				numConf;				// Number of the configuration
	USB					*pUsb;					// USB class instance pointer

	static const uint8_t	epDataInIndex;		// DataIN  endpoint index
	static const uint8_t	epDataOutIndex;		// DataOUT endpoint index
	static const uint8_t	epInterruptIndex;	// Interrupt endpoint index

	EpInfo				epInfo[4];

	struct OperFlags
	{
		uint16_t	opParams	:	3;			// 7	- maximum number of operation parameters
		uint16_t	rsParams	:	3;			// 7	- maximum number of response parameters
		uint16_t	txOperation	:	1;			// I->R operation if the flag is set
		uint16_t	dataStage	:	1;			// operation has data stage if the flag is set
		uint16_t	typeOfVoid	:	2;			// 0 - NULL, 1 - PTPReadParser/PTPDataSupplyer, 2 - WRITEPARSER, 3 - buffer pointer
		uint16_t	dataSize	:	6;			// size of data buffer (64 bytes maximum)
	};
	typedef void (*READPARSER)(const uint16_t len, const uint8_t *pbuf, const uint32_t &offset);

	void FillEPRecords(USB_ENDPOINT_DESCRIPTOR *pep);

	void ZerroMemory(uint8_t size, uint8_t *mem) { for (uint8_t i=0; i<size; i++) mem[i] = 0; };

	// waits for any event to occur
	// returns event on success or error code if timeout elapsed
	bool EventWait(uint8_t size, uint8_t *event_buf, uint16_t timeout);

	// returns true if event occured
	// the actual data is stored in a buffer pointed by buf
	bool CheckEvent(uint8_t size, uint8_t *buf);

	uint16_t Transaction(uint16_t opcode, OperFlags *flags, uint32_t *params, void *pVoid);

public:
	PTP(USB *pusb, PTPStateHandlers *s);

	//void PrintTransactionId() { Serial.println(idTransaction,HEX); };

	// USBDeviceConfig implementation
	virtual uint8_t Init(uint8_t parent, uint8_t port, bool lowspeed);
	virtual uint8_t Release();
	virtual uint8_t Poll();
	virtual uint8_t GetAddress() { return devAddress; };

	void SetState(uint8_t state) { theState = state; };
	uint8_t GetState() { return theState; };

	virtual uint16_t EventCheck(PTPReadParser *parser);

	// Simple PTP operation which has no data stage. Any number of uint32_t params can be supplied.
	uint16_t Operation(uint16_t opcode, uint8_t nparams = 0, uint32_t *params = NULL);

	uint16_t CaptureImage();

	uint16_t OpenSession();
	uint16_t CloseSession();
	uint16_t ResetDevice();
	uint16_t PowerDown();
	uint16_t SelfTest(uint16_t type);
	uint16_t GetDeviceInfo(PTPReadParser *parser);
	uint16_t GetDevicePropDesc(const uint16_t pcode, PTPReadParser *parser);
	uint16_t GetDevicePropValue(const uint16_t pcode, PTPReadParser *parser);
	uint16_t GetDevicePropValue(const uint16_t pcode, uint8_t  &val);
	uint16_t GetDevicePropValue(const uint16_t pcode, uint16_t &val);
	uint16_t GetDevicePropValue(const uint16_t pcode, uint32_t &val);
	uint16_t GetDevicePropValue(const uint16_t pcode, int8_t  &val);
	uint16_t GetDevicePropValue(const uint16_t pcode, int16_t &val);
	uint16_t GetDevicePropValue(const uint16_t pcode, int32_t &val);
	uint16_t SetDevicePropValue(const uint16_t pcode, uint8_t  val);
	uint16_t SetDevicePropValue(const uint16_t pcode, uint16_t val);
	uint16_t SetDevicePropValue(const uint16_t pcode, uint32_t val);
	uint16_t SetDevicePropValue(const uint16_t pcode, int8_t  val);
	uint16_t SetDevicePropValue(const uint16_t pcode, int16_t val);
	uint16_t SetDevicePropValue(const uint16_t pcode, int32_t val);
	uint16_t ResetDevicePropValue(const uint16_t pcode);
	uint16_t GetStorageIDs(PTPReadParser *parser);
	uint16_t GetStorageIDs(uint8_t bufsize, uint8_t *pbuf);
	uint16_t GetStorageInfo(uint32_t storage_id, PTPReadParser *parser);
	uint16_t GetObjectHandles(uint32_t storage_id, uint16_t format, uint16_t assoc, PTPReadParser *parser);
	uint16_t GetObjectInfo(uint32_t handle, PTPReadParser *parser);
	uint16_t FormatStore(uint32_t storage_id, uint32_t fsformat);
	uint16_t GetObject(uint32_t handle, PTPReadParser *parser);
	uint16_t GetThumb(uint32_t handle, PTPReadParser *parser);

	uint16_t GetNumObjects(uint32_t &retval, uint32_t storage_id = 0xffffffff, uint16_t format = 0, uint32_t assoc = 0);
	uint16_t DeleteObject(uint32_t handle, uint16_t format = 0);
	uint16_t SetObjectProtection(uint32_t handle, uint16_t attrib);
	uint16_t MoveObject(uint32_t handle, uint32_t storage_id, uint32_t parent);
	uint16_t CopyObject(uint32_t handle, uint32_t storage_id, uint32_t parent, uint32_t &new_handle);
	uint16_t InitiateOpenCapture(uint32_t storage_id = 0, uint16_t format = 0);
	uint16_t TerminateOpenCapture(uint32_t trans_id);
	uint16_t InitiateCapture(uint32_t storage_id = 0, uint16_t format = 0);

	// To be implemented in future releases
	//uint16_t GetPartialObject(uint32_t handle, PTPReadParser *parser);
	//uint16_t SendObjectInfo(uint32_t handle, PTPDataSupplier *sup);
	//uint16_t SendObject(uint32_t handle, PTPDataSupplier *sup);
};

#endif // __PTP_H__