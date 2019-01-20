#ifndef __PTPOBJINFOPARSER_H__
#define __PTPOBJINFOPARSER_H__

#include <ptp.h>
#include <mtpconst.h>
#include <ptpcallback.h>

const char msgUndefined		[] PROGMEM = "Undefined";

// Ancillary formats
const char msgAssociation	[] PROGMEM = "Association";
const char msgScript		[] PROGMEM = "Script";
const char msgExecutable	[] PROGMEM = "Executable";
const char msgText			[] PROGMEM = "Text";
const char msgHTML			[] PROGMEM = "HTML";
const char msgDPOF			[] PROGMEM = "DPOF";
const char msgAIFF	 		[] PROGMEM = "AIFF";
const char msgWAV			[] PROGMEM = "WAV";
const char msgMP3			[] PROGMEM = "MP3";
const char msgAVI			[] PROGMEM = "AVI";
const char msgMPEG			[] PROGMEM = "MPEG";
const char msgASF			[] PROGMEM = "ASF";
const char msgQT			[] PROGMEM = "QT";

// Image formats
const char msgEXIF_JPEG			[] PROGMEM = "EXIF_JPEG";
const char msgTIFF_EP			[] PROGMEM = "TIFF_EP";
const char msgFlashPix			[] PROGMEM = "FlashPix";
const char msgBMP				[] PROGMEM = "BMP";
const char msgCIFF				[] PROGMEM = "CIFF";
const char msgUndefined_0x3806	[] PROGMEM = "Undefined_0x3806";
const char msgGIF				[] PROGMEM = "GIF";
const char msgJFIF				[] PROGMEM = "JFIF";
const char msgPCD				[] PROGMEM = "PCD";
const char msgPICT				[] PROGMEM = "PICT";
const char msgPNG				[] PROGMEM = "PNG";
const char msgUndefined_0x380C	[] PROGMEM = "Undefined_0x380C";
const char msgTIFF				[] PROGMEM = "TIFF";
const char msgTIFF_IT			[] PROGMEM = "TIFF_IT";
const char msgJP2				[] PROGMEM = "JP2";
const char msgJPX				[] PROGMEM = "JPX";

// MTP Object Formats
const char msgUndefined_Firmware				[] PROGMEM = "Undefined_Firmware";
const char msgWindows_Image_Format				[] PROGMEM = "Windows_Image_Format";
const char msgUndefined_Audio					[] PROGMEM = "Undefined_Audio";
const char msgWMA								[] PROGMEM = "WMA";
const char msgOGG								[] PROGMEM = "OGG";
const char msgAAC								[] PROGMEM = "AAC";
const char msgAudible							[] PROGMEM = "Audible";
const char msgFLAC								[] PROGMEM = "FLAC";
const char msgUndefined_Video					[] PROGMEM = "Undefined_Video";
const char msgWMV								[] PROGMEM = "WMV";
const char msgMP4_Container						[] PROGMEM = "MP4_Container";
const char msgMP2								[] PROGMEM = "MP2";
const char msg3GP_Container						[] PROGMEM = "3GP_Container";


/**
 * This structure is used to store ObjectInfo dataset used in ObjectInfo parser
 * as well as SendObjectInfo operation.
 */
typedef struct {

        union {
                uint8_t raw[64];

                struct {
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
                    uint32_t AssociationDescription;
                    uint32_t SequenceNumber;
                    uint8_t FilenameLength;
                    uint16_t* Filename;
                    uint8_t DateCreatedLength;
                    uint16_t* DateCreated;
                    uint8_t DateModifiedLength;
                    uint16_t* DateModified;
                    uint8_t KeywordsLength;
                    uint16_t* Keywords;
                };
        };
} ObjectInfo;

class PTPObjInfoParser : public PTPReadParser
{
	static const char* const acNames[];
	static const char* const imNames[];

	MultiValueBuffer theBuffer;
	uint32_t varBuffer;
	uint8_t	nStage;
        ObjectInfo objectinfo_ds;  // TODO: init

	MultiByteValueParser valueParser;
	PTPListParser arrayParser;

	static void PrintChar(MultiValueBuffer *p)
	{
		if (((unsigned char*)p->pValue)[0])
			Serial.print(((unsigned char*)p->pValue)[0]);
	};
	void PrintFormat(uint16_t op);

public:
	PTPObjInfoParser() : nStage(0) { theBuffer.pValue = (uint8_t*)&varBuffer; };
	virtual void Parse(const uint16_t len, const uint8_t *pbuf, const uint32_t &offset);
        uint8_t CopyObjectInfo(uint8_t* buf);    // copies ObjectInfo dataset
                                                    // into buffer provided
};

#endif // __PTPOBJINFOPARSER_H__
