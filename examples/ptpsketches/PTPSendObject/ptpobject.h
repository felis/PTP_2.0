/* Copyright (C) 2012 Circuits At Home, LTD. All rights reserved.

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

#ifndef __PTPOBJECT_H__
#define __PTPOBJECT_H__

#include <ptp.h>
#include <ptpconst.h>
#include "ptpdebug.h"
#include "ptpcallback.h"

// ObjectInfo packed structure, for ops like SendObjectInfo
struct _ObjectInfoPacked {
	uint32_t StorageID;
	uint16_t ObjectFormat;
	uint16_t ProtectionStatus;
	uint32_t ObjectCompressedSize;
	uint16_t ThumbFormat;
	uint32_t ThumbCompressedSize;
	uint32_t ThumbPixWidth;
	uint32_t ThumbPixHeight;
	uint32_t ImagePixWidth;
	uint32_t ImagePixHeight;
	uint32_t ImageBitDepth;
	uint32_t ParentObject;
	uint16_t AssociationType;
	uint32_t AssociationDesc;
	uint32_t SequenceNumber;
        // void * filename;
};

typedef struct _ObjectInfoPacked ObjectInfoPacked;

// FileInfo supplier
class PTPFileInfoSupplier : public PTPDataSupplier
{
	char * filename;
	uint8_t filename_len;
        uint32_t file_size;
        uint32_t storage_id;
        uint32_t parent_handle;
	
public:
  PTPFileInfoSupplier(uint32_t storageId, uint32_t parentHandle,
          const char * fn, uint32_t file_size);
	virtual ~PTPFileInfoSupplier() { free(filename); };
	virtual uint32_t GetDataSize();
	virtual void GetData(const uint16_t len, uint8_t *pbuf);
};

class PTPTextFileSupplier : public PTPDataSupplier
{
	char * data;
	uint32_t file_size;
	
public:
	PTPTextFileSupplier();
	virtual ~PTPTextFileSupplier();
	void SetText(char * text);
	virtual uint32_t GetDataSize();
	virtual void GetData(const uint16_t len, uint8_t *pbuf);
};


#endif // __PTPOBJECT_H__
