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
#include "ptpobject.h"

static void uint16_to_char(uint16_t integer, unsigned char *data)
{
	data[0] = (integer & 0xff);
	data[1] = ((integer >> 8) & 0xff);
}

static void uint32_to_char(uint32_t integer, unsigned char *data)
{
	data[0] = (integer & 0xff);
	data[1] = ((integer >> 8) & 0xff);
	data[2] = ((integer >> 16) & 0xff);
	data[3] = ((integer >> 24) & 0xff);
}


// PTPFileInfoSupplier implements packaging object information
// TODO: rename into PTPObjectInfoSupplier
PTPFileInfoSupplier::PTPFileInfoSupplier(uint32_t storageId,
        uint32_t parentHandle,
        const char * fn,
        uint32_t file_size) : filename(0), file_size(file_size),
        storage_id(storageId), parent_handle(parentHandle)
{
  filename = (char*)malloc(strlen(fn)+1);
  strcpy(filename, fn);
  filename_len = strlen(fn)+1;
}

uint32_t PTPFileInfoSupplier::GetDataSize()
{
  // Header + string_size + string_bytes + two bytes terminating NULL
	// return 4*2 + 11*4 + 1 + strlen(filename) + 2;
    return( sizeof(ObjectInfoPacked) + 2*filename_len );
}

void PTPFileInfoSupplier::GetData(const uint16_t len, uint8_t *pbuf)
{
    
    PTPTRACE2("Get Data Length: ",len);
    PTPTRACE2("Get buf: ", *pbuf);
    
    ObjectInfoPacked* oi = (ObjectInfoPacked*)pbuf;
  
	uint32_to_char(storage_id, (unsigned char *)&oi->StorageID);
	
        uint16_to_char(PTP_OFC_EXIF_JPEG, (unsigned char *)&oi->ObjectFormat);
	
        uint16_to_char(PTP_PS_NoProtection, (unsigned char *)&oi->ProtectionStatus);
	
        uint32_to_char(file_size, (unsigned char *)&oi->ObjectCompressedSize);

	uint16_to_char(0, (unsigned char *)&oi->ThumbFormat);
	uint32_to_char(0, (unsigned char *)&oi->ThumbCompressedSize);
	uint32_to_char(0, (unsigned char *)&oi->ThumbPixWidth);
	uint32_to_char(0, (unsigned char *)&oi->ThumbPixHeight);
	uint32_to_char(0, (unsigned char *)&oi->ImagePixWidth);
	uint32_to_char(0, (unsigned char *)&oi->ImagePixHeight);
	uint32_to_char(0, (unsigned char *)&oi->ImageBitDepth);
        
	uint32_to_char(parent_handle, (unsigned char *)&oi->ParentObject);
	
        uint16_to_char(0, (unsigned char *)&oi->AssociationType);
	uint32_to_char(0, (unsigned char *)&oi->AssociationDesc);
	uint32_to_char(0, (unsigned char *)&oi->SequenceNumber);
	
#if 0

	((char*)oi->filename)[0] = strlen(filename)+1;
	strcpy((char*)((char*)oi->filename)[1], filename);
	((char*)oi->filename)[strlen(filename)+1] = 0;
	((char*)oi->filename)[strlen(filename)+2] = 0;
#endif
}

PTPTextFileSupplier::PTPTextFileSupplier() : data(0), file_size(0)
{
}

PTPTextFileSupplier::~PTPTextFileSupplier()
{
  if (data)
    free(data);
}

void PTPTextFileSupplier::SetText(char * text)
{
  if (data) {
    free(data);
    file_size = 0;
  }
  
  file_size = strlen(text);
  data = (char*)malloc(file_size+1);
  strcpy(data, text);
  
}

uint32_t PTPTextFileSupplier::GetDataSize()
{
  return file_size;
}

void PTPTextFileSupplier::GetData(const uint16_t len, uint8_t *pbuf)
{
  if (len >= file_size)
    strcpy((char*)pbuf, data);
}

