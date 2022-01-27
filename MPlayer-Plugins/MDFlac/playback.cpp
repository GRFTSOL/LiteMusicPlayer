/* in_flac - Winamp2 FLAC input plugin
 * Copyright (C) 2000,2001,2002,2003,2004,2005  Josh Coalson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "../../base/base.h"

#include "playback.h"
extern "C"
{
#include "../flac/src/plugin_common/dither.h"
};
#include "MDFlac.h"

void AudioInfoFromStreamInfo(AUDIO_INFO &audioInfo, const FLAC__StreamMetadata_StreamInfo *streaminfo);

// libFLAC callbacks
static FLAC__StreamDecoderReadStatus ReadCallback(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], unsigned *bytes, void *client_data)
{
	IMediaInput		*pInput = ((CMDFlac *)client_data)->m_pInput;

	*bytes = pInput->Read(buffer, *bytes);
	if (*bytes == 0)
		return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
	else
		return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

static FLAC__StreamDecoderSeekStatus SeekCallback(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data)
{
	IMediaInput		*pInput = ((CMDFlac *)client_data)->m_pInput;

	if (pInput->Seek(absolute_byte_offset, SEEK_SET) == ERR_OK)
		return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
	else
		return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
}

static FLAC__StreamDecoderTellStatus TellCallback(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
	IMediaInput		*pInput = ((CMDFlac *)client_data)->m_pInput;

	*absolute_byte_offset = pInput->GetPos();
	return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

static FLAC__StreamDecoderLengthStatus LengthCallback(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data)
{
	IMediaInput		*pInput = ((CMDFlac *)client_data)->m_pInput;
	uint32			nSize;

	if (pInput->GetSize(nSize) == ERR_OK)
	{
		*stream_length = nSize;
		return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
	}
	else
		return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
}

static FLAC__bool EofCallback(const FLAC__StreamDecoder *, void *client_data)
{
	IMediaInput		*pInput = ((CMDFlac *)client_data)->m_pInput;

	return pInput->IsEOF();
}

static FLAC__StreamDecoderWriteStatus WriteCallback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
	CMDFlac			*pMDFlac = (CMDFlac*)client_data;
	int				nSize = frame->header.blocksize * pMDFlac->m_nBytesPerSample;
	IFBuffer		*pBuf = pMDFlac->m_pMemAllocator->AllocFBuffer(nSize);
	ASSERT(pBuf);
	if (!pBuf)
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	int16			*pBuffer = (int16*)pBuf->data();
	int				nSamples = frame->header.blocksize;

	if (pMDFlac->m_bKillThread)
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

	nSize = FLAC__plugin_common__pack_pcm_signed_little_endian(
		(FLAC__byte*)pBuf->data(),
		buffer,
		nSamples,
		frame->header.channels,
		frame->header.bits_per_sample,
		frame->header.bits_per_sample
		);

/*	int				i;
	if (frame->header.channels == 1)
	{
		for (i = 0; i < nSamples; i++) 
			pBuffer[i] = (int16)buffer[0][i];
	}
	else
	{
		// Interleave the channels
		unsigned c;
		for (i = 0; i < nSamples; i++)
			for (c = 0; c < frame->header.channels; c++)
			{
				*(pBuffer++) = ((buffer[c][i] >> 8) | ((buffer[c][i] & 0xFF) << 8));
				// data[1] = (FLAC__byte)(sample >> 8);
				// data[0] = (FLAC__byte)sample;
				// *(pBuffer++) = (int16)buffer[c][i];
			}
	}*/
	pBuf->resize(nSize);

	if (!pMDFlac->OutputWrite(pBuf, frame->header.bits_per_sample,
		frame->header.channels, frame->header.sample_rate))
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void MetadataCallback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	CMDFlac *pMDFlac = (CMDFlac*)client_data;
	AUDIO_INFO	&m_audioInfo = pMDFlac->m_audioInfo;

	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
	{
		FLAC__ASSERT(metadata->data.stream_info.total_samples < 0x100000000); /* this plugin can only handle < 4 gigasamples */

		m_audioInfo.nMediaFileSize = 0;
		pMDFlac->m_pInput->GetSize(m_audioInfo.nMediaFileSize);

		AudioInfoFromStreamInfo(m_audioInfo, &(metadata->data.stream_info));

		pMDFlac->m_nBytesPerSample = m_audioInfo.nChannels * m_audioInfo.nBps / 8;

		if (m_audioInfo.nBps != 8 && m_audioInfo.nBps != 16 && m_audioInfo.nBps != 24)
		{
			// FLAC_plugin__show_error("This plugin can only handle 8/16/24-bit samples.");
			pMDFlac->m_bKillThread = true;
		}
	}
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

static void ErrorCallback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	CMDFlac *pMDFlac = (CMDFlac*)client_data;

	if (status!=FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC)
		pMDFlac->m_bKillThread = true;
}

int FLAC_plugin__decoder_init(CMDFlac *pMDFlac)
{
	FLAC__StreamDecoderInitStatus init_status;

	FLAC__ASSERT(pMDFlac->m_pFlacDecoder);
	FLAC__stream_decoder_finish(pMDFlac->m_pFlacDecoder);
	/* init pMDFlac->m_pFlacDecoder */
	FLAC__stream_decoder_set_md5_checking(pMDFlac->m_pFlacDecoder, false);
	FLAC__stream_decoder_set_metadata_ignore_all(pMDFlac->m_pFlacDecoder);
	FLAC__stream_decoder_set_metadata_respond(pMDFlac->m_pFlacDecoder, FLAC__METADATA_TYPE_STREAMINFO);
	FLAC__stream_decoder_set_metadata_respond(pMDFlac->m_pFlacDecoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);

//	FLAC__stream_decoder_set_metadata_respond_all(pMDFlac->m_pFlacDecoder);
// 	FLAC__stream_decoder_set_metadata_ignore_all(pMDFlac->m_pFlacDecoder);
// 	FLAC__stream_decoder_set_metadata_respond(pMDFlac->m_pFlacDecoder, FLAC__METADATA_TYPE_STREAMINFO);
// 	FLAC__stream_decoder_set_metadata_respond(pMDFlac->m_pFlacDecoder, FLAC__METADATA_TYPE_VORBIS_COMMENT);
	init_status = FLAC__stream_decoder_init_stream(pMDFlac->m_pFlacDecoder,
		ReadCallback, SeekCallback, TellCallback, LengthCallback, 
		EofCallback, WriteCallback, MetadataCallback, ErrorCallback, pMDFlac);

	if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK)
	{
		// FLAC_plugin__show_error("Error while initializing decoder (%s [%s]).", FLAC__StreamDecoderInitStatusString[init_status], FLAC__stream_decoder_get_resolved_state_string(decoder));
		FLAC__stream_decoder_finish(pMDFlac->m_pFlacDecoder);
		return ERR_PLAYER_INVALID_FILE;
	}
	/* process */

	if (!FLAC__stream_decoder_process_until_end_of_metadata(pMDFlac->m_pFlacDecoder))
	{
		// FLAC_plugin__show_error("Error while processing metadata (%s).", FLAC__stream_decoder_get_resolved_state_string(pMDFlac->m_pFlacDecoder));
		FLAC__stream_decoder_finish(pMDFlac->m_pFlacDecoder);
		return ERR_PLAYER_INVALID_FILE;
	}
	/* check results */
	if (pMDFlac->m_bKillThread)
		return ERR_PLAYER_INVALID_FILE;

	/* init replaygain */
	/*stream_data->output_bits_per_sample = stream_data->has_replaygain && cfg.replaygain.enable ?
		cfg.resolution.replaygain.bps_out :
	cfg.resolution.normal.dither_24_to_16 ? min(stream_data->bits_per_sample, 16) : stream_data->bits_per_sample;

	if (stream_data->has_replaygain && cfg.replaygain.enable && cfg.resolution.replaygain.dither)
		FLAC__replaygain_synthesis__init_dither_context(&stream_data->dither_context, stream_data->bits_per_sample, cfg.resolution.replaygain.noise_shaping);
	*/

	return ERR_OK;
}


/*
 *  decode
 */

void FLAC_plugin__seek(CMDFlac *pMDFlac)
{
	AUDIO_INFO	&m_audioInfo = pMDFlac->m_audioInfo;
	FLAC__uint64 target_sample =
		(FLAC__uint64)m_audioInfo.nTotalSamples * pMDFlac->m_nSeekPos / m_audioInfo.nMediaLength;

	FLAC__stream_decoder_seek_absolute(pMDFlac->m_pFlacDecoder, target_sample);
}
