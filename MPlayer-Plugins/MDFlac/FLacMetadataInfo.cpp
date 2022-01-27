// FLacMetadataInfo.cpp: implementation of the CFLacMetadataInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "../../base/base.h"

extern "C"
{
#include "FLAC/all.h"
#include "plugin_common/all.h"
};

#include "MDFlac.h"
#include "FLacMetadataInfo.h"

struct FlacStreanInfoAndTag
{
	IMediaInput	*m_pInput;

	bool		bTagError, bStreamInfoError;

	FLAC__StreamMetadata	*pStreamInfo;
	FLAC__StreamMetadata	*pTags;

	FlacStreanInfoAndTag()
	{
		m_pInput = NULL;
		bTagError = bStreamInfoError = false;
		pStreamInfo = NULL;
		pTags = NULL;
	}
};

// libFLAC callbacks
static FLAC__StreamDecoderReadStatus MetadataInfo_ReadCallback(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], unsigned *bytes, void *client_data)
{
	IMediaInput		*pInput = ((FlacStreanInfoAndTag *)client_data)->m_pInput;

	*bytes = pInput->Read(buffer, *bytes);
	if (*bytes == 0)
		return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
	else
		return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

static FLAC__StreamDecoderSeekStatus MetadataInfo_SeekCallback(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data)
{
	IMediaInput		*pInput = ((FlacStreanInfoAndTag *)client_data)->m_pInput;

	if (pInput->Seek(absolute_byte_offset, SEEK_SET) == ERR_OK)
		return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
	else
		return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
}

static FLAC__StreamDecoderTellStatus MetadataInfo_TellCallback(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
	IMediaInput		*pInput = ((FlacStreanInfoAndTag *)client_data)->m_pInput;

	*absolute_byte_offset = pInput->GetPos();
	return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

static FLAC__StreamDecoderLengthStatus MetadataInfo_LengthCallback(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data)
{
	IMediaInput		*pInput = ((FlacStreanInfoAndTag *)client_data)->m_pInput;
	uint32			nSize;

	if (pInput->GetSize(nSize) == ERR_OK)
	{
		*stream_length = nSize;
		return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
	}
	else
		return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
}

static FLAC__bool MetadataInfo_EofCallback(const FLAC__StreamDecoder *, void *client_data)
{
	IMediaInput		*pInput = ((FlacStreanInfoAndTag *)client_data)->m_pInput;

	return pInput->IsEOF();
}

static FLAC__StreamDecoderWriteStatus MetadataInfo_WriteCallback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void MetadataInfo_MetadataCallback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	FlacStreanInfoAndTag *pInfo = (FlacStreanInfoAndTag*)client_data;
	// AUDIO_INFO	&m_audioInfo = pInfo->m_audioInfo;

	FlacStreanInfoAndTag *pFlacTag = (FlacStreanInfoAndTag *)client_data;

	/*
	 * we assume we only get here when the one metadata block we were
	 * looking for was passed to us
	 */
	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
	{
		if (!pInfo->pStreamInfo)
		{
			pInfo->pStreamInfo = FLAC__metadata_object_clone(metadata);
			if (!pInfo->pStreamInfo)
				pInfo->bStreamInfoError = true;
		}
	}
	else if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT)
	{
		if (!pInfo->pTags)
		{
			pInfo->pTags = FLAC__metadata_object_clone(metadata);
			if (!pInfo->pTags)
				pInfo->bTagError = true;
		}
	}
// 
// 	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
// 	{
// 		FLAC__ASSERT(metadata->data.stream_info.total_samples < 0x100000000); /* this plugin can only handle < 4 gigasamples */
// 
// 		m_audioInfo.nMediaFileSize = 0;
// 		pMDFlac->m_pInput->GetSize(m_audioInfo.nMediaFileSize);
// 		m_audioInfo.nTotalSamples = (unsigned)(metadata->data.stream_info.total_samples&0xfffffffful);
// 		m_audioInfo.nBps = metadata->data.stream_info.bits_per_sample;
// 		m_audioInfo.nChannels = metadata->data.stream_info.channels;
// 		m_audioInfo.nSampleRate = metadata->data.stream_info.sample_rate;
// 
// 		m_audioInfo.nMediaLength = (uint32)((double)m_audioInfo.nTotalSamples / (double)m_audioInfo.nSampleRate * 1000.0 + 0.5);
// 		m_audioInfo.nBitRate = 
// 			 int((double)m_audioInfo.nMediaFileSize * 8 * 1000 / m_audioInfo.nMediaLength);
// 
// 		pMDFlac->m_nBytesPerSample = m_audioInfo.nChannels * m_audioInfo.nBps / 8;
// 
// 		if (m_audioInfo.nBps != 8 && m_audioInfo.nBps != 16 && m_audioInfo.nBps != 24)
// 		{
// 			// FLAC_plugin__show_error("This plugin can only handle 8/16/24-bit samples.");
// 			pMDFlac->m_bKillThread = true;
// 		}
// 	}


// 	else if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) 
// 	{
// 		const FLAC__StreamMetadata_VorbisComment *comments = &metadata->data.vorbis_comment;
// 		tchar_t	s[256];
//     
// 		if (p->Format.Comment.Node) 
// 		{
// 			uint32_t No;
// 			for (No = 0; No < comments->num_comments; ++No) 
// 			{
// 				UTF8ToTcs(s, TSIZEOF(s), (const char *)comments->comments[No].entry);
// 				p->Format.Comment.Node->Set(p->Format.Comment.Node, p->Format.Comment.No, s, sizeof(s));
// 			}
// 		}    
// 	}
}

static void MetadataInfo_ErrorCallback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
}

int MetadataInfo_get_streaminfo_and_tags(IMediaInput *pInput, FLAC__StreamMetadata **streaminfo, FLAC__StreamMetadata **tags)
{
	FLAC__StreamDecoder		*m_pFlacDecoder;
	FLAC__StreamDecoderInitStatus state;
	FlacStreanInfoAndTag		infoAndTags;

	m_pFlacDecoder = FLAC__stream_decoder_new();
	if (!m_pFlacDecoder)
		return ERR_DECODER_INNER_ERROR;

	infoAndTags.m_pInput = pInput;

	FLAC__stream_decoder_set_md5_checking(m_pFlacDecoder, false);

	state = FLAC__stream_decoder_init_stream(m_pFlacDecoder,
		MetadataInfo_ReadCallback, MetadataInfo_SeekCallback, 
		MetadataInfo_TellCallback, MetadataInfo_LengthCallback, 
		MetadataInfo_EofCallback, MetadataInfo_WriteCallback, 
		MetadataInfo_MetadataCallback, MetadataInfo_ErrorCallback, &infoAndTags);
	if (state != FLAC__STREAM_DECODER_INIT_STATUS_OK)
	{
		FLAC__stream_decoder_finish(m_pFlacDecoder);
		FLAC__stream_decoder_delete(m_pFlacDecoder);
		return ERR_PLAYER_INVALID_FILE;
	}

	if (!FLAC__stream_decoder_process_until_end_of_metadata(m_pFlacDecoder))
	{
		if (infoAndTags.pStreamInfo)
			FLAC__metadata_object_delete(infoAndTags.pStreamInfo);
		if (infoAndTags.pTags)
			FLAC__metadata_object_delete(infoAndTags.pTags);
		FLAC__stream_decoder_finish(m_pFlacDecoder);
		FLAC__stream_decoder_delete(m_pFlacDecoder);
		return ERR_PLAYER_INVALID_FILE;
	}

	if (infoAndTags.pStreamInfo) {
		*streaminfo = infoAndTags.pStreamInfo;
	}

	if (infoAndTags.pTags) {
		*tags = infoAndTags.pTags;
	}

	FLAC__stream_decoder_finish(m_pFlacDecoder);
	FLAC__stream_decoder_delete(m_pFlacDecoder);

	return ERR_OK;
}
