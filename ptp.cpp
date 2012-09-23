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
#include "ptpconst.h"
#include "ptp.h"
#include "ptpdebug.h"

void PTP::SetInitialState()
{
	idSession = 0;
	idTransaction = ~((transaction_id_t)0);
	SetState(PTP_STATE_SESSION_NOT_OPENED);
}

void PTPStateHandlers::OnDeviceDisconnectedState(PTP *ptp)
{
}

void PTPStateHandlers::OnSessionNotOpenedState(PTP *ptp)
{
	if (ptp->OpenSession() == PTP_RC_OK)
	{
		PTPTRACE("Session opened\r\n");
		ptp->SetState(PTP_STATE_SESSION_OPENED);
	}
}

void PTPStateHandlers::OnSessionOpenedState(PTP *ptp)
{
	ptp->SetState(PTP_STATE_DEVICE_INITIALIZED);
}

void PTPStateHandlers::OnDeviceInitializedState(PTP *ptp)
{
}

void PTPStateHandlers::OnDeviceNotRespondingState(PTP *ptp)
{
}

void PTPStateHandlers::OnDeviceBusyState(PTP *ptp)
{
}

void uint16_to_char(uint16_t integer, unsigned char *data)
{
	data[0] = (integer & 0xff);
	data[1] = ((integer >> 8) & 0xff);
}

void uint32_to_char(uint32_t integer, unsigned char *data)
{
	data[0] = (integer & 0xff);
	data[1] = ((integer >> 8) & 0xff);
	data[2] = ((integer >> 16) & 0xff);
	data[3] = ((integer >> 24) & 0xff);
}

const uint8_t		PTP::epDataInIndex		= 1;
const uint8_t		PTP::epDataOutIndex		= 2;
const uint8_t		PTP::epInterruptIndex	= 3;

PTP::PTP(USB *pusb, PTPStateHandlers *s) : 
	pUsb(pusb),
	theState(0),
	idTransaction(~((transaction_id_t)0)), 
	idSession(0), 
	devAddress(0),
	numConf(0),
	stateMachine(s)
{
    // Control EP
	epInfo[0].epAddr		= 0;
	epInfo[0].maxPktSize	= 8;
	epInfo[0].epAttribs		= 0;
	epInfo[0].bmNakPower	= USB_NAK_MAX_POWER;

	// Data-In EP
	epInfo[1].epAddr		= 0;
	epInfo[1].maxPktSize	= 8;
	epInfo[1].epAttribs		= 0;
    
    // Data-Out EP
	epInfo[2].epAddr		= 0;
	epInfo[2].maxPktSize	= 8;
	epInfo[2].epAttribs		= 0;
    
    // Interrupt EP
	epInfo[3].epAddr		= 0;
	epInfo[3].maxPktSize	= 8;
	epInfo[3].epAttribs		= 0;

	if (pUsb)
		pUsb->RegisterDeviceClass(this);
};


uint8_t PTP::Init(uint8_t parent, uint8_t port, bool lowspeed)
{
	const uint8_t constBufSize = 39;

	uint8_t		buf[constBufSize];
	uint8_t		rcode;
	UsbDevice	*p = NULL;
	EpInfo		*oldep_ptr = NULL;
	uint8_t		len = 0;
	uint16_t	cd_len = 0;

	uint8_t		num_of_conf;	// number of configurations
	uint8_t		num_of_intf;	// number of interfaces

	AddressPool	&addrPool = pUsb->GetAddressPool();

	PTPTRACE("PTP Init\r\n");

	if (devAddress)
		return USB_ERROR_CLASS_INSTANCE_ALREADY_IN_USE;

	// Get pointer to pseudo device with address 0 assigned
	p = addrPool.GetUsbDevicePtr(0);

	if (!p)
		return USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL;

	if (!p->epinfo)
	{
		PTPTRACE("epinfo\r\n");
		return USB_ERROR_EPINFO_IS_NULL;
	}

	// Save old pointer to EP_RECORD of address 0
	oldep_ptr = p->epinfo;

	// Temporary assign new pointer to epInfo to p->epinfo in order to avoid toggle inconsistence
	p->epinfo = epInfo;

	// Get device descriptor
	rcode = pUsb->getDevDescr( 0, 0, 8, (uint8_t*)buf );

	if  (!rcode)
		len = (buf[0] > 32) ? 32 : buf[0];

	if( rcode ) 
	{
		// Restore p->epinfo
		p->epinfo = oldep_ptr;

		PTPTRACE("getDevDesc\r\n");
		return rcode;
	}

	// Extract device class from device descriptor
	// If device class is not a hub return
	if (   ((USB_DEVICE_DESCRIPTOR*)buf)->bDeviceClass != 0 
		&& ((USB_DEVICE_DESCRIPTOR*)buf)->bDeviceClass != USB_CLASS_IMAGE 
		&& ((USB_DEVICE_DESCRIPTOR*)buf)->bDeviceSubClass != 0x01
		&& ((USB_DEVICE_DESCRIPTOR*)buf)->bDeviceProtocol != 0x01)

		return USB_DEV_CONFIG_ERROR_DEVICE_NOT_SUPPORTED;

	// Allocate new address according to device class
	devAddress = addrPool.AllocAddress(parent, false, port);

	if (!devAddress)
		return USB_ERROR_OUT_OF_ADDRESS_SPACE_IN_POOL;

	// Extract Max Packet Size from the device descriptor
	epInfo[0].maxPktSize = (uint8_t)((USB_DEVICE_DESCRIPTOR*)buf)->bMaxPacketSize0; 

	// Assign new address to the device
	rcode = pUsb->setAddr( 0, 0, devAddress );

	if (rcode)
	{
		// Restore p->epinfo
		p->epinfo = oldep_ptr;
		addrPool.FreeAddress(devAddress);
		devAddress = 0;
		PTPTRACE2("setAddr:",rcode);
		return rcode;
	}

	PTPTRACE2("Addr:", devAddress);

	// Restore p->epinfo
	p->epinfo = oldep_ptr;

	if (len)
		rcode = pUsb->getDevDescr( devAddress, 0, len, (uint8_t*)buf );

	if(rcode) 
		goto FailGetDevDescr;

	num_of_conf = ((USB_DEVICE_DESCRIPTOR*)buf)->bNumConfigurations;

	// Assign epInfo to epinfo pointer
	rcode = pUsb->setEpInfoEntry(devAddress, 4, epInfo);

	if (rcode)
		goto FailSetDevTblEntry;

	for (uint8_t i=0; i<num_of_conf; i++)
	{
		// Read configuration Descriptor in Order To Obtain Proper Configuration Value
		rcode = pUsb->getConfDescr(devAddress, 0, 8, i, buf);

		if (!rcode)
		{
			cd_len = ((USB_CONFIGURATION_DESCRIPTOR*)buf)->wTotalLength;
			cd_len = (cd_len > constBufSize) ? constBufSize : cd_len;
			rcode = pUsb->getConfDescr(devAddress, 0, cd_len, i, buf);
		}
		if (rcode)
			goto FailGetConfDescr;

		USB_INTERFACE_DESCRIPTOR	*pIntf = (USB_INTERFACE_DESCRIPTOR*)(buf + sizeof(USB_CONFIGURATION_DESCRIPTOR));

		PTPTRACE2("NI:", ((USB_CONFIGURATION_DESCRIPTOR*)buf)->bNumInterfaces);

		if (((USB_CONFIGURATION_DESCRIPTOR*)buf)->bNumInterfaces > 0 
			&& pIntf->bInterfaceClass == 6 && pIntf->bInterfaceSubClass == 1 && pIntf->bInterfaceProtocol == 1) 
		{
			FillEPRecords((USB_ENDPOINT_DESCRIPTOR*)(buf + sizeof(USB_CONFIGURATION_DESCRIPTOR) + sizeof(USB_INTERFACE_DESCRIPTOR)));
			numConf = ((USB_CONFIGURATION_DESCRIPTOR*)buf)->bConfigurationValue;
			break;
		}
	} // for
	if (!numConf)
		return USB_DEV_CONFIG_ERROR_DEVICE_NOT_SUPPORTED;

	// Set Configuration Value
	rcode = pUsb->setConf(devAddress, 0, numConf);

	if (rcode)
		goto FailSetConfDescr;

	// Set PTP state machine initial state 
	SetInitialState();

	PTPTRACE("PTP configured\r\n");

	bPollEnable = true;
	return 0;

FailGetDevDescr:
	PTPTRACE("getDevDescr:");
	goto Fail;

FailSetDevTblEntry:
	PTPTRACE("setDevTblEn:");
	goto Fail;

FailGetConfDescr:
	PTPTRACE("getConf:");
	goto Fail;

FailSetConfDescr:
	PTPTRACE("setConf:");
	goto Fail;

Fail:
	Serial.println(rcode, HEX);
	return rcode;
}

void PTP::FillEPRecords(USB_ENDPOINT_DESCRIPTOR *pep)
{
	uint8_t index;

	for (uint8_t i=0; i<3; i++)
	{
		if ((pep[i].bmAttributes & 0x03) == 3)
			index = epInterruptIndex;
		else if ((pep[i].bmAttributes & 0x03) == 2 && (pep[i].bEndpointAddress & 0x80) == 0x80)
			index = epDataInIndex;
		else
			index = epDataOutIndex;

		epInfo[index].epAddr		= (pep[i].bEndpointAddress & 0x0F);
		epInfo[index].maxPktSize	= (uint8_t)pep[i].wMaxPacketSize;
		epInfo[index].epAttribs		= 0;
		epInfo[index].bmNakPower	= (index == epInterruptIndex) ? 0 : USB_NAK_MAX_POWER;
	}	
}

uint8_t PTP::Release()
{
	pUsb->GetAddressPool().FreeAddress(devAddress);

	theState = PTP_STATE_DEVICE_DISCONNECTED;
	stateMachine->OnDeviceDisconnectedState(this);

	devAddress = 0;
	idSession = 0;
	idTransaction = ~((transaction_id_t)0);

	return 0;
}

uint8_t PTP::Poll() 
{
	if (bPollEnable)
		Task();
};

void PTP::Task()
{
	switch (theState)
	{
	//case PTP_STATE_DEVICE_DISCONNECTED:
//		idSession = 0;
//		idTransaction = ~((trasaction_id_t)0);
//		if (stateMachine)
	//		stateMachine->OnDeviceDisconnectedState(this);
	//	break;
	case PTP_STATE_SESSION_NOT_OPENED:
		if (stateMachine)
			stateMachine->OnSessionNotOpenedState(this);
		break;
	case PTP_STATE_SESSION_OPENED:
		if (stateMachine)
			stateMachine->OnSessionOpenedState(this);
		break;
	case PTP_STATE_DEVICE_INITIALIZED:
		if (stateMachine)
			stateMachine->OnDeviceInitializedState(this);
		break;
	case PTP_STATE_DEVICE_NOT_RESPONDING:
		if (stateMachine)
			stateMachine->OnDeviceNotRespondingState(this);
		break;
	case PTP_STATE_DEVICE_BUSY:
		if (stateMachine)
			stateMachine->OnDeviceBusyState(this);
		break;
	// Error state
	default:
		;
	}
}

uint16_t PTP::Transaction(uint16_t opcode, OperFlags *flags, uint32_t *params = NULL, void *pVoid = NULL)
{
	uint8_t		rcode;
	{
		uint8_t		cmd[PTP_USB_BULK_HDR_LEN + 12];		// header + 3 uint32_t parameters

		ZerroMemory(PTP_USB_BULK_HDR_LEN + 12, cmd);

		// Make command PTP container header
		uint16_to_char(PTP_USB_CONTAINER_COMMAND,	(unsigned char*)(cmd + PTP_CONTAINER_CONTYPE_OFF));			// type
		uint16_to_char(opcode,						(unsigned char*)(cmd + PTP_CONTAINER_OPCODE_OFF));			// code
		uint32_to_char(++idTransaction,				(unsigned char*)(cmd + PTP_CONTAINER_TRANSID_OFF));			// transaction id
		
		uint8_t		n = flags->opParams, len;

		if (params && *params)
		{
			*((uint8_t*)cmd) = len = PTP_USB_BULK_HDR_LEN + (n << 2);

			for (uint32_t *p1 = (uint32_t*)(cmd + PTP_CONTAINER_PAYLOAD_OFF), *p2 = (uint32_t*)params; n--; p1++, p2++)
				uint32_to_char(*p2, (unsigned char*)p1);
		}
		else
			*((uint8_t*)cmd) = len = PTP_USB_BULK_HDR_LEN;
		
		rcode = pUsb->outTransfer(devAddress, epInfo[epDataOutIndex].epAddr, len, cmd);

		if (rcode)
		{
			PTPTRACE2("Transaction: Command block send error", rcode);
			return PTP_RC_GeneralError;
		}
	}
	{
		uint8_t		data[PTP_MAX_RX_BUFFER_LEN];

		if (flags->txOperation)
		{
			if (flags->typeOfVoid && !pVoid)
			{
				PTPTRACE("Transaction: pVoid is NULL\n");
				return PTP_RC_GeneralError;
			}
			ZerroMemory(PTP_MAX_RX_BUFFER_LEN, data);

			uint32_t	bytes_left =	(flags->typeOfVoid == 3) ? PTP_USB_BULK_HDR_LEN + flags->dataSize :
							((flags->typeOfVoid == 1) ? PTP_USB_BULK_HDR_LEN + ((PTPDataSupplier*)pVoid)->GetDataSize() : 12);

			// Make data PTP container header
			*((uint32_t*)data) = bytes_left;
			uint16_to_char(PTP_USB_CONTAINER_DATA,	(unsigned char*)(data + PTP_CONTAINER_CONTYPE_OFF));		// type
			uint16_to_char(opcode,					(unsigned char*)(data + PTP_CONTAINER_OPCODE_OFF));			// code
			uint32_to_char(idTransaction,			(unsigned char*)(data + PTP_CONTAINER_TRANSID_OFF));		// transaction id

			uint16_t	len;

			if (flags->typeOfVoid == 1)
				len = (bytes_left < PTP_MAX_RX_BUFFER_LEN) ? bytes_left : PTP_MAX_RX_BUFFER_LEN;
			
			if (flags->typeOfVoid == 3)
			{
				uint8_t		*p1 = (data + PTP_USB_BULK_HDR_LEN);
				uint8_t		*p2 = (uint8_t*)pVoid;

				for (uint8_t i=flags->dataSize; i; i--, p1++, p2++)
					*p1 = *p2;

				len = PTP_USB_BULK_HDR_LEN + flags->dataSize;
			}
			bool first_time = true;

			while (bytes_left)
			{
				if (flags->typeOfVoid == 1)
					((PTPDataSupplier*)pVoid)->GetData(	(first_time) ? len - PTP_USB_BULK_HDR_LEN : len, 
														(first_time) ? (data + PTP_USB_BULK_HDR_LEN) : data);
				
				rcode = pUsb->outTransfer(devAddress, epInfo[epDataOutIndex].epAddr, len, data);

				if (rcode)
				{
					PTPTRACE2("Transaction: Data block send error.", rcode);
					return PTP_RC_GeneralError;
				}

				bytes_left -= len;

				len = (bytes_left < PTP_MAX_RX_BUFFER_LEN) ? bytes_left : PTP_MAX_RX_BUFFER_LEN;

				first_time = false;
			}
		}

		// Because inTransfer does not return the actual number of bytes recieved, it should be 
		// calculated here.
		uint32_t	total = 0, data_off = 0; 	// Total PTP data packet size, Data offset
		uint8_t		inbuffer = 0;			// Number of bytes read into buffer
		uint16_t	loops = 0;			// Number of loops necessary to get all the data from device
		uint8_t		timeoutcnt = 0;

		while (1)
		{
			ZerroMemory(PTP_MAX_RX_BUFFER_LEN, data);

			uint16_t	read = PTP_MAX_RX_BUFFER_LEN;
			rcode = pUsb->inTransfer(devAddress, epInfo[epDataInIndex].epAddr, &read, data);

			if (rcode)
			{
				PTPTRACE("Fatal USB Error\r\n");

				// in some cases NAK handling might be necessary
				PTPTRACE2("Transaction: Response recieve error", rcode);
				return PTP_RC_GeneralError;
			}

			// This can occure in case of unsupported operation or successive response after data reception stage
			if ((!loops || total == data_off) && *((uint16_t*)(data + PTP_CONTAINER_CONTYPE_OFF)) == PTP_USB_CONTAINER_RESPONSE)
			{
				uint16_t	response = *((uint16_t*)(data + PTP_CONTAINER_OPCODE_OFF));

				if (response == PTP_RC_OK && *((uint32_t*)data) > PTP_USB_BULK_HDR_LEN)
				{
					// number of params = (container length - 12) / 4
					uint8_t	n = (*((uint32_t*)data) - PTP_USB_BULK_HDR_LEN) >> 2;

					// BUG: n should be checked!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					flags->rsParams = n;
					
					for (uint32_t *p1 = (uint32_t*)(data + PTP_USB_BULK_HDR_LEN), *p2 = (uint32_t*)params; n; n--, p1++, p2++)
						p2 = p1;
				}
				if (response != PTP_RC_OK)
				{
					PTPTRACE2("Transaction: Response recieve error", response);
					data_off = 0;
				}
				return response;
			}

			if (loops == 0)
			{
				total		=	*((uint32_t*)data);
				inbuffer	=	(total < PTP_MAX_RX_BUFFER_LEN) ? (uint8_t)total : PTP_MAX_RX_BUFFER_LEN;
			}
			else
				inbuffer = ((total - data_off) > PTP_MAX_RX_BUFFER_LEN) ? PTP_MAX_RX_BUFFER_LEN : (uint8_t)(total - data_off);

			if (pVoid)
			{
				if (flags->typeOfVoid == 0x01)
					((PTPReadParser*)pVoid)->Parse(inbuffer, data, (const uint32_t&)data_off);

				if (flags->typeOfVoid == 0x03)
					for (uint32_t i=0, j=data_off; i<inbuffer && j<flags->dataSize; i++, j++)
						((uint8_t*)pVoid)[j] = data[i];
			}
			data_off += inbuffer;

			loops ++;
			//delay(10);
		} // while(1)
	} // end of scope
}


uint16_t PTP::EventCheck(PTPReadParser *pParser)
{
	uint8_t		data[PTP_MAX_EV_BUFFER_LEN];
	uint8_t		rcode;

	// Because inTransfer does not return the actual number of bytes recieved, it should be 
	// calculated here.
	uint32_t	total = 0, data_off = 0;
	uint8_t		inbuffer = 0;
	uint16_t	loops = 0;
	uint8_t		timeoutcnt = 0;

	while (1)
	{
		ZerroMemory(PTP_MAX_EV_BUFFER_LEN, data);

		uint16_t	read = PTP_MAX_EV_BUFFER_LEN;
		rcode = pUsb->inTransfer(devAddress, epInfo[epInterruptIndex].epAddr, &read, data);

		switch (rcode)
		{
		// In case of no event occured
		case 0xFF:
				return PTP_EC_Undefined;

		default:
			// in case of a usb error
			PTPTRACE2("EventCheck USB error: ", rcode);
			return PTP_RC_GeneralError;
		}

		if (loops == 0)
		{
			total		=	*((uint32_t*)data);
			inbuffer	=	(total < PTP_MAX_EV_BUFFER_LEN) ? (uint8_t)total : PTP_MAX_EV_BUFFER_LEN;
		}
		else
			inbuffer = ((total - data_off) > PTP_MAX_EV_BUFFER_LEN) ? PTP_MAX_EV_BUFFER_LEN : (uint8_t)(total - data_off);

		if (pParser)
			pParser->Parse(inbuffer, data, (const uint32_t&)data_off);
		
		data_off += inbuffer;

		loops ++;
		delay(50);
	} // while(1)
}

bool PTP::CheckEvent(uint8_t size, uint8_t *buf)
{
	ZerroMemory(size, buf);

	uint16_t	read = size;
	uint8_t rcode = pUsb->inTransfer(devAddress, epInfo[epInterruptIndex].epAddr, &read, buf);

	// if no interrupts pending - return false

	// there should be some error type checking involved i.e. timeout error
	if (rcode)
		return false;

	return true;
}

bool PTP::EventWait(uint8_t size, uint8_t *event_buf, uint16_t timeout)
{
	const uint16_t	milisec = 0xA0;		// must be polling interval for the interrupt pipe
	bool			occured	= false;

	while (!(occured = CheckEvent(size, event_buf)) && (timeout -= milisec))
		delay(milisec);

	return occured;
}

uint16_t PTP::OpenSession()
{
	uint32_t	params[1];
	OperFlags	flags = { 1, 0, 0, 0, 0, 0 };

	idSession		= 1;
	idTransaction	= ~((transaction_id_t)0);

	params[0]	= idSession;

	while (1)
	{
		uint16_t ret = Transaction(PTP_OC_OpenSession, &flags, params);

		if (ret == PTP_RC_SessionAlreadyOpened)
			ret == CloseSession();
		else
			return ret;
	}
}

uint16_t PTP::ResetDevice()
{
	OperFlags	flags = { 0, 0, 0, 0, 0, 0 };
	return Transaction(PTP_OC_ResetDevice, &flags);
}

uint16_t PTP::GetNumObjects(uint32_t &retval, uint32_t storage_id, uint16_t format, uint32_t assoc)
{
	uint16_t	ptp_error = PTP_RC_GeneralError;
	OperFlags	flags = { 3, 1, 0, 0, 0, 0 };
	uint32_t	params[3];

	if ( (ptp_error = Transaction(PTP_OC_GetNumObjects, &flags, params)) == PTP_RC_OK)
		retval = params[0];

	return ptp_error;
}

uint16_t PTP::GetObject(uint32_t handle, PTPReadParser *parser)
{
	OperFlags	flags = { 1, 0, 0, 1, 1, 0 };
	uint32_t	params[1];

	params[0] = handle;

	return Transaction(PTP_OC_GetObject, &flags, params, parser);
}

uint16_t PTP::GetThumb(uint32_t handle, PTPReadParser *parser)
{
	OperFlags	flags = { 1, 0, 0, 1, 1, 0 };
	uint32_t	params[1];

	params[0] = handle;

	return Transaction(PTP_OC_GetThumb, &flags, params, parser);
}

uint16_t PTP::DeleteObject(uint32_t handle, uint16_t format)
{
	OperFlags	flags = { 2, 0, 0, 0, 0, 0 };
	uint32_t	params[2];

	params[0] = handle;
	params[1] = (uint32_t)format;

	return Transaction(PTP_OC_DeleteObject, &flags, params);
}

uint16_t PTP::SetObjectProtection(uint32_t handle, uint16_t attrib)
{
	OperFlags	flags = { 2, 0, 0, 0, 0, 0 };
	uint32_t	params[2];

	params[0] = handle;
	params[1] = (uint32_t)attrib;

	return Transaction(PTP_OC_SetObjectProtection, &flags, params);
}

uint16_t PTP::MoveObject(uint32_t handle, uint32_t storage_id, uint32_t parent)
{
	OperFlags	flags = { 3, 0, 0, 0, 0, 0 };
	uint32_t	params[2];

	params[0] = handle;
	params[1] = storage_id;
	params[2] = parent;

	return Transaction(PTP_OC_MoveObject, &flags, params);
}

uint16_t PTP::CopyObject(uint32_t handle, uint32_t storage_id, uint32_t parent, uint32_t &new_handle)
{
	uint16_t	ptp_error = PTP_RC_GeneralError;
	OperFlags	flags = { 3, 1, 0, 0, 0, 0 };
	uint32_t	params[3];

	params[0] = handle;
	params[1] = storage_id;
	params[2] = parent;

	if ( (ptp_error = Transaction(PTP_OC_CopyObject, &flags, params)) == PTP_RC_OK)
		new_handle = params[0];

	return ptp_error;
}

uint16_t PTP::InitiateCapture(uint32_t storage_id, uint16_t format)
{
	uint16_t	ptp_error = PTP_RC_GeneralError;
	OperFlags	flags = { 2, 0, 0, 0, 0, 0 };
	uint32_t	params[2];

	params[0] = storage_id;
	params[1] = (uint32_t)format;

	if ( (ptp_error = Transaction(PTP_OC_InitiateCapture, &flags, params)) == PTP_RC_OK)
	{}

	return ptp_error;
}

uint16_t PTP::InitiateOpenCapture(uint32_t storage_id, uint16_t format)
{
	uint16_t	ptp_error = PTP_RC_GeneralError;
	OperFlags	flags = { 2, 0, 0, 0, 0, 0 };
	uint32_t	params[2];

	params[0] = storage_id;
	params[1] = (uint32_t)format;

	if ( (ptp_error = Transaction(PTP_OC_InitiateOpenCapture, &flags, params)) == PTP_RC_OK)
	{}

	return ptp_error;
}
	
uint16_t PTP::TerminateOpenCapture(uint32_t trans_id)
{
	OperFlags	flags = { 1, 0, 0, 0, 0, 0 };
	uint32_t	params[1];

	params[0] = trans_id;

	return Transaction(PTP_OC_TerminateOpenCapture, &flags, params);
}

uint16_t PTP::PowerDown()
{
	OperFlags	flags = { 0, 0, 0, 0, 0, 0 };
	return Transaction(PTP_OC_PowerDown, &flags);
}

uint16_t PTP::SelfTest(uint16_t type = 0)
{
	OperFlags	flags = { 1, 0, 0, 0 };
	uint32_t	params[1];
	params[0]	= type;

	return Transaction(PTP_OC_SelfTest, &flags, params);
}

uint16_t PTP::CloseSession()
{
	uint16_t	ptp_error = PTP_RC_GeneralError;
	OperFlags	flags = { 0, 0, 0, 0, 0, 0 };

	if ( (ptp_error = Transaction(PTP_OC_CloseSession, &flags)) == PTP_RC_OK)
	{
		idSession = 0;
		idTransaction = ~((transaction_id_t)0);
	}
	return ptp_error;
}

uint16_t PTP::GetDeviceInfo(PTPReadParser *parser)
{
	OperFlags	flags = { 0, 0, 0, 1, 1, 0 };
	return Transaction(PTP_OC_GetDeviceInfo, &flags, NULL, parser);
}

uint16_t PTP::GetObjectInfo(uint32_t handle, PTPReadParser *parser)
{
	OperFlags	flags = { 1, 0, 0, 1, 1, 0 };
	uint32_t	params[1];
	params[0] = handle;

	return Transaction(PTP_OC_GetObjectInfo, &flags, params, parser);
}

uint16_t PTP::GetDevicePropDesc(const uint16_t pcode, PTPReadParser *parser)
{
	OperFlags	flags		= { 1, 0, 0, 1, 1, 0 };
	uint32_t	params[1];

	params[0] = (uint32_t)pcode;

	return Transaction(PTP_OC_GetDevicePropDesc, &flags, params, parser);
}

uint16_t PTP::GetDevicePropValue(const uint16_t pcode, PTPReadParser *parser)
{
	OperFlags	flags		= { 1, 0, 0, 1, 1, 0 };
	uint32_t	params[1];

	params[0] = (uint32_t)pcode;

	return Transaction(PTP_OC_GetDevicePropValue, &flags, params, parser);
}

uint16_t PTP::GetDevicePropValue(const uint16_t pcode, uint8_t &val)
{
	uint16_t	ptp_error	= PTP_RC_GeneralError;
	OperFlags	flags		= { 1, 0, 0, 0, 3, 13 };
	uint32_t	params[1];
	uint8_t		buf[13];

	params[0] = (uint32_t)pcode;

	if ( (ptp_error = Transaction(PTP_OC_GetDevicePropValue, &flags, params, buf)) == PTP_RC_OK)
		val = buf[12];

	return ptp_error;
}

uint16_t PTP::GetDevicePropValue(const uint16_t pcode, uint16_t &val)
{
	uint16_t	ptp_error	= PTP_RC_GeneralError;
	OperFlags	flags		= { 1, 0, 0, 0, 3, 14 };
	uint32_t	params[1];
	uint16_t	buf[7];

	params[0] = pcode;

	if ( (ptp_error = Transaction(PTP_OC_GetDevicePropValue, &flags, params, buf)) == PTP_RC_OK)
		val = buf[6];

	return ptp_error;
}

uint16_t PTP::GetDevicePropValue(const uint16_t pcode, uint32_t &val)
{
	uint16_t	ptp_error	= PTP_RC_GeneralError;
	OperFlags	flags		= { 1, 0, 0, 0, 3, 16 };
	uint32_t	params[1];
	uint32_t	buf[4];

	params[0] = pcode;

	if ( (ptp_error = Transaction(PTP_OC_GetDevicePropValue, &flags, params, buf)) == PTP_RC_OK)
		val = buf[3];

	return ptp_error;
}


uint16_t PTP::GetDevicePropValue(const uint16_t pcode, int8_t &val)
{
	uint16_t	ptp_error	= PTP_RC_GeneralError;
	OperFlags	flags		= { 1, 0, 0, 0, 3, 13 };
	uint32_t	params[1];
	uint8_t		buf[13];

	params[0] = (uint32_t)pcode;

	if ( (ptp_error = Transaction(PTP_OC_GetDevicePropValue, &flags, params, buf)) == PTP_RC_OK)
		val = buf[12];

	return ptp_error;
}

uint16_t PTP::GetDevicePropValue(const uint16_t pcode, int16_t &val)
{
	uint16_t	ptp_error	= PTP_RC_GeneralError;
	OperFlags	flags		= { 1, 0, 0, 0, 3, 14 };
	uint32_t	params[1];
	uint16_t	buf[7];

	params[0] = pcode;

	if ( (ptp_error = Transaction(PTP_OC_GetDevicePropValue, &flags, params, buf)) == PTP_RC_OK)
		val = buf[6];

	return ptp_error;
}

uint16_t PTP::GetDevicePropValue(const uint16_t pcode, int32_t &val)
{
	uint16_t	ptp_error	= PTP_RC_GeneralError;
	OperFlags	flags		= { 1, 0, 0, 0, 3, 16 };
	uint32_t	params[1];
	uint32_t	buf[4];

	params[0] = pcode;

	if ( (ptp_error = Transaction(PTP_OC_GetDevicePropValue, &flags, params, buf)) == PTP_RC_OK)
		val = buf[3];

	return ptp_error;
}

uint16_t PTP::SetDevicePropValue(uint16_t pcode, uint8_t val)
{
	OperFlags	flags		= { 1, 0, 1, 1, 3, 1 };
	uint32_t	params[1];
	uint8_t		value;

	params[0]	= (uint32_t)pcode;
	value		= val;

	return Transaction(PTP_OC_SetDevicePropValue, &flags, params, (void*)&value);
}

uint16_t PTP::SetDevicePropValue(uint16_t pcode, uint16_t val)
{
	OperFlags	flags		= { 1, 0, 1, 1, 3, 2 };
	uint32_t	params[1];
	uint16_t	value;

	params[0]	= (uint32_t)pcode;
	value		= val;

	return Transaction(PTP_OC_SetDevicePropValue, &flags, params, (void*)&value);
}

uint16_t PTP::SetDevicePropValue(uint16_t pcode, uint32_t val)
{
	OperFlags	flags		= { 1, 0, 1, 1, 3, 4 };
	uint32_t	params[1];
	uint32_t	value;

	params[0]	= (uint32_t)pcode;
	value		= val;

	return Transaction(PTP_OC_SetDevicePropValue, &flags, params, (void*)&value);
}

uint16_t PTP::SetDevicePropValue(uint16_t pcode, int8_t val)
{
	OperFlags	flags		= { 1, 0, 1, 1, 3, 1 };
	uint32_t	params[1];
	uint8_t		value;

	params[0]	= (uint32_t)pcode;
	value		= val;

	return Transaction(PTP_OC_SetDevicePropValue, &flags, params, (void*)&value);
}

uint16_t PTP::SetDevicePropValue(uint16_t pcode, int16_t val)
{
	OperFlags	flags		= { 1, 0, 1, 1, 3, 2 };
	uint32_t	params[1];
	uint16_t	value;

	params[0]	= (uint32_t)pcode;
	value		= val;

	return Transaction(PTP_OC_SetDevicePropValue, &flags, params, (void*)&value);
}

uint16_t PTP::SetDevicePropValue(uint16_t pcode, int32_t val)
{
	OperFlags	flags		= { 1, 0, 1, 1, 3, 4 };
	uint32_t	params[1];
	uint32_t	value;

	params[0]	= (uint32_t)pcode;
	value		= val;

	return Transaction(PTP_OC_SetDevicePropValue, &flags, params, (void*)&value);
}

uint16_t PTP::ResetDevicePropValue(const uint16_t pcode)
{
	OperFlags	flags		= { 1, 0, 0, 0 };
	uint32_t	params[1];

	params[0] = (uint32_t)pcode;

	return Transaction(PTP_OC_ResetDevicePropValue, &flags, params);
}

uint16_t PTP::Operation(uint16_t opcode, uint8_t nparams, uint32_t *params)
{
	OperFlags	flags		= { 0, 0, 0, 0, 0, 0 };

	flags.opParams = nparams;

	return Transaction(opcode, &flags, params);
}

uint16_t PTP::GetStorageInfo(uint32_t storage_id, PTPReadParser *parser)
{
	OperFlags	flags		= { 1, 0, 0, 1, 1, 0 };

	uint32_t	params[1]; 
	params[0]	= storage_id;

	return Transaction(PTP_OC_GetStorageInfo, &flags, params, parser);
}

uint16_t PTP::FormatStore(uint32_t storage_id, uint32_t fsformat)
{
	OperFlags	flags		= { 2, 0, 0, 0, 0, 0 };

	uint32_t	params[2]; 
	params[0]	= storage_id;
	params[1]	= fsformat;

	return Transaction(PTP_OC_FormatStore, &flags, params);
}

uint16_t PTP::CaptureImage()
{
	uint16_t	ptp_error = PTP_RC_GeneralError;
	uint32_t	params[2] = {0, 0x00003801};
	OperFlags	flags = { 2, 0, 0, 0 };

	if ( (ptp_error = Transaction(PTP_OC_InitiateCapture, &flags, params)) != PTP_RC_OK)
	{
		PTPTRACE2("CaptureImage error", ptp_error);
		return ptp_error;
	}
	PTPUSBEventContainer	evnt;
	bool					occured;

	while (1)
	{
		// multiple objects can be added depending on current camera shooting mode
		if ((occured = EventWait(sizeof(PTPUSBEventContainer), (uint8_t*)&evnt, 500))/* && evnt.code == PTP_EC_ObjectAdded*/)
		//	PTPTRACE("CaptureImage: New object added.\r\n");
		{
		if (!occured)
		{
			PTPTRACE("CaptureImage: Timeout ellapsed.\r\n");
			return PTP_RC_Undefined;
		}
		switch (evnt.code)
		{
		case PTP_EC_ObjectAdded:
			PTPTRACE("CaptureImage: New object added.\r\n");
			break;
		case PTP_EC_CaptureComplete:
			PTPTRACE("CaptureImage: Image captured!\r\n");
			return PTP_RC_OK;

		case PTP_EC_StoreFull:
			PTPTRACE("CaptureImage: Storage is full.\r\n");
			return PTP_RC_StoreFull;

		default:
			PTPTRACE2("CaptureImage: Unexpected event\r\n", evnt.code);
			return PTP_RC_Undefined;
		}
		}
		//if (evnt.code == PTP_EC_CaptureComplete)
		//	break;
	} // while (1)
}

uint16_t PTP::GetStorageIDs(PTPReadParser *parser)
{
	OperFlags	flags = { 0, 0, 0, 1, 1, 0 };
	return Transaction(PTP_OC_GetStorageIDs, &flags, NULL, parser);
}

uint16_t PTP::GetStorageIDs(uint8_t bufsize, uint8_t *pbuf)
{
	OperFlags	flags = { 0, 0, 0, 1, 3, 0 };

	flags.dataSize = bufsize;

	return Transaction(PTP_OC_GetStorageIDs, &flags, NULL, pbuf);
}

uint16_t PTP::GetObjectHandles(uint32_t storage_id, uint16_t format, uint16_t assoc, PTPReadParser *parser)
{
	OperFlags	flags = { 3, 0, 0, 1, 1, 0 };
	uint32_t	params[3];

	params[0] = storage_id;
	params[1] = (uint32_t)format;
	params[2] = (uint32_t)assoc;

	return Transaction(PTP_OC_GetObjectHandles, &flags, params, parser);
}
