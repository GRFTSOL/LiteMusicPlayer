/*
** FAAD2 - Freeware Advanced Audio (AAC) Decoder including SBR decoding
** Copyright (C) 2003 M. Bakker, Ahead Software AG, http://www.nero.com
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** Any non-GPL usage of this software or parts of this software is strictly
** forbidden.
**
** Commercial non-GPL licensing of this software is possible.
** For more info contact Ahead Software through Mpeg4AAClicense@nero.com.
**
**/

#include "../../base/base.h"
#include "../../MPlayerEngine/IMPlayer.h"
#include <math.h>
#include <neaacdec.h>
#define USE_TAGGING
#include <mp4ff.h>

#include "MDMP4.h"

#include "resource.h"

extern "C"
{
#include "utils.h"
};
#include "config.h"
#include "aacinfo.h"

#define SZ_DECODER_DESC		_T("MPEG4 File decoder")

#define DECODE_BUFF_SIZE (1024 * 8)

static int res_table[] = {
    16,
    24,
    32,
    0,
    0,
    16
};

struct seek_list
{
    struct seek_list *next;
    __int64 offset;
};

struct DecoderState
{
    /* general stuff */
    NeAACDecHandle hDecoder;
    double decode_pos_ms; // current decoding position, in milliseconds
    int filetype; /* 0: MP4; 1: AAC */
    __int64 last_offset;

    /* MP4 stuff */
    mp4ff_t *mp4file;
    int mp4track;
    long numSamples;
    long sampleId;
    mp4ff_callback_t mp4cb;

    /* AAC stuff */
    long m_aac_bytes_into_buffer;
    long m_aac_bytes_consumed;
    __int64 m_file_offset;
    unsigned char *m_aac_buffer;
    int m_at_eof;
    double cur_pos_sec;
    int m_header_type;
    struct seek_list *m_head;
    struct seek_list *m_tail;
    unsigned long m_length;

    /* for gapless decoding */
    unsigned int useAacLength;
    unsigned int framesize;
    unsigned int initial;
    unsigned long timescale;

    IMediaInput	*pInput;
};

DecoderState mp4state;

/* Function definitions */
void *decode_aac_frame(DecoderState *st, NeAACDecFrameInfo *frameInfo);


uint32_t read_callback(void *user_data, void *buffer, uint32_t length)
{
	IMediaInput	*pInput = (IMediaInput*)user_data;
	return pInput->Read(buffer, length);
}

uint32_t seek_callback(void *user_data, uint64_t position)
{
	IMediaInput	*pInput = (IMediaInput*)user_data;
	if (pInput->Seek(position, SEEK_SET) == ERR_OK)
		return 0;
	else
		return -1;
}

static void show_error(LPCTSTR message, ...)
{
    if (m_show_errors)
        MessageBox(NULL, message, _T("Error"), MB_OK);
}

static void show_error(LPCSTR message, ...)
{
	tstringex	strMsg;
	ConvertStr(message, -1, strMsg);
    if (m_show_errors)
        MessageBox(NULL, strMsg.c_str(), _T("Error"), MB_OK);
}

int fill_buffer(DecoderState *st)
{
    int bread;

    if (st->m_aac_bytes_consumed > 0)
    {
        if (st->m_aac_bytes_into_buffer)
        {
            memmove((void*)st->m_aac_buffer, (void*)(st->m_aac_buffer + st->m_aac_bytes_consumed),
                st->m_aac_bytes_into_buffer*sizeof(unsigned char));
        }

        if (!st->m_at_eof)
        {
			bread = st->pInput->Read((void*)(st->m_aac_buffer + st->m_aac_bytes_into_buffer),
                st->m_aac_bytes_consumed);

            if (bread != st->m_aac_bytes_consumed)
                st->m_at_eof = 1;

            st->m_aac_bytes_into_buffer += bread;
        }

        st->m_aac_bytes_consumed = 0;

        if (st->m_aac_bytes_into_buffer > 3)
        {
            if (memcmp(st->m_aac_buffer, "TAG", 3) == 0)
                st->m_aac_bytes_into_buffer = 0;
        }
        if (st->m_aac_bytes_into_buffer > 11)
        {
            if (memcmp(st->m_aac_buffer, "LYRICSBEGIN", 11) == 0)
                st->m_aac_bytes_into_buffer = 0;
        }
        if (st->m_aac_bytes_into_buffer > 8)
        {
            if (memcmp(st->m_aac_buffer, "APETAGEX", 8) == 0)
                st->m_aac_bytes_into_buffer = 0;
        }
    }

    return 1;
}

void advance_buffer(DecoderState *st, int bytes)
{
    st->m_file_offset += bytes;
    st->m_aac_bytes_consumed = bytes;
    st->m_aac_bytes_into_buffer -= bytes;
}

int adts_parse(DecoderState *st, int *bitrate, double *length)
{
    static int sample_rates[] = {96000,88200,64000,48000,44100,32000,24000,22050,16000,12000,11025,8000};
    int frames, frame_length;
    int t_framelength = 0;
    int samplerate;
    double frames_per_sec, bytes_per_frame;

    /* Read all frames to ensure correct time and bitrate */
    for (frames = 0; /* */; frames++)
    {
        fill_buffer(st);

        if (st->m_aac_bytes_into_buffer > 7)
        {
            /* check syncword */
            if (!((st->m_aac_buffer[0] == 0xFF)&&((st->m_aac_buffer[1] & 0xF6) == 0xF0)))
                break;

            st->m_tail->offset = st->m_file_offset;
            st->m_tail->next = (struct seek_list*)malloc(sizeof(struct seek_list));
            st->m_tail = st->m_tail->next;
            st->m_tail->next = NULL;

            if (frames == 0)
                samplerate = sample_rates[(st->m_aac_buffer[2]&0x3c)>>2];

            frame_length = ((((unsigned int)st->m_aac_buffer[3] & 0x3)) << 11)
                | (((unsigned int)st->m_aac_buffer[4]) << 3) | (st->m_aac_buffer[5] >> 5);

            t_framelength += frame_length;

            if (frame_length > st->m_aac_bytes_into_buffer)
                break;

            advance_buffer(st, frame_length);
        } else {
            break;
        }
    }

    frames_per_sec = (double)samplerate/1024.0;
    if (frames != 0)
        bytes_per_frame = (double)t_framelength/(double)(frames*1000);
    else
        bytes_per_frame = 0;
    *bitrate = (int)(8. * bytes_per_frame * frames_per_sec + 0.5);
    if (frames_per_sec != 0)
        *length = (double)frames/frames_per_sec;
    else
        *length = 1;

    return 1;
}

int skip_id3v2_tag()
{
    unsigned char buf[10];
    int bread, tagsize = 0;

	bread = mp4state.pInput->Read(buf, 10);
    if (bread != 10) return -1;

    if (!memcmp(buf, "ID3", 3))
    {
        /* high bit is not used */
        tagsize = (buf[6] << 21) | (buf[7] << 14) | (buf[8] << 7) | (buf[9] << 0);

        tagsize += 10;
		mp4state.pInput->Seek(tagsize, SEEK_SET);
    } else {
		mp4state.pInput->Seek(0, SEEK_SET);
    }

    return tagsize;
}

int getsonglength(IMediaInput *pInput)
{
    long msDuration = 0;
	LPCTSTR		szFileName = pInput->getSource();
	int			nNameLen = _tcslen(szFileName);

	if (nNameLen < 4)
		return 0;

    if (_tcsicmp(szFileName + nNameLen - 3, _T("MP4")) == 0 || _tcsicmp(szFileName + nNameLen - 3, _T("M4A")) == 0)
    {
        int track;
        int64_t length;
        mp4ff_t *file;
        mp4ff_callback_t mp4cb = {0};

        mp4cb.read = read_callback;
        mp4cb.seek = seek_callback;
        mp4cb.user_data = pInput;


        file = mp4ff_open_read(&mp4cb);
        if (file == NULL)
            return 0;

        if ((track = GetAACTrack(file)) < 0)
        {
            mp4ff_close(file);
            return -1;
        }

        length = mp4ff_get_track_duration(file, track);

        msDuration = (long)((float)length*1000.0 / (float)mp4ff_time_scale(file, track) + 0.5);

        mp4ff_close(file);

        return msDuration;
    } else {
        int tagsize = 0;
        int bread = 0;
        double length = 0.;
        int bitrate = 128;
        struct seek_list *target;
        DecoderState len_state;

        memset(&len_state, 0, sizeof(DecoderState));
		len_state.pInput = pInput;

        len_state.m_at_eof = 0;

        if (!(len_state.m_aac_buffer = (unsigned char*)malloc(768*6)))
        {
            //console::error(_T("Memory allocation error."), _T("foo_mp4"));
            return 0;
        }
        memset(len_state.m_aac_buffer, 0, 768*6);

        bread = pInput->Read(len_state.m_aac_buffer, 768*6);
        len_state.m_aac_bytes_into_buffer = bread;
        len_state.m_aac_bytes_consumed = 0;
        len_state.m_file_offset = 0;

        if (bread != 768*6)
            len_state.m_at_eof = 1;

        if (!memcmp(len_state.m_aac_buffer, "ID3", 3))
        {
            /* high bit is not used */
            tagsize = (len_state.m_aac_buffer[6] << 21) | (len_state.m_aac_buffer[7] << 14) |
                (len_state.m_aac_buffer[8] <<  7) | (len_state.m_aac_buffer[9] <<  0);

            tagsize += 10;
            advance_buffer(&len_state, tagsize);
        }

        len_state.m_head = (struct seek_list*)malloc(sizeof(struct seek_list));
        len_state.m_tail = len_state.m_head;
        len_state.m_tail->next = NULL;

        len_state.m_header_type = 0;
        if ((len_state.m_aac_buffer[0] == 0xFF) && ((len_state.m_aac_buffer[1] & 0xF6) == 0xF0))
        {
            if (1) //(m_reader->can_seek())
            {
                adts_parse(&len_state, &bitrate, &length);
                pInput->Seek(tagsize, SEEK_SET);

                bread = pInput->Read(len_state.m_aac_buffer, 768*6);
                if (bread != 768*6)
                    len_state.m_at_eof = 1;
                else
                    len_state.m_at_eof = 0;
                len_state.m_aac_bytes_into_buffer = bread;
                len_state.m_aac_bytes_consumed = 0;

                len_state.m_header_type = 1;
            }
        } else if (memcmp(len_state.m_aac_buffer, "ADIF", 4) == 0) {
            int skip_size = (len_state.m_aac_buffer[4] & 0x80) ? 9 : 0;
            bitrate = ((unsigned int)(len_state.m_aac_buffer[4 + skip_size] & 0x0F)<<19) |
                ((unsigned int)len_state.m_aac_buffer[5 + skip_size]<<11) |
                ((unsigned int)len_state.m_aac_buffer[6 + skip_size]<<3) |
                ((unsigned int)len_state.m_aac_buffer[7 + skip_size] & 0xE0);

			uint32		nFileSize;
			pInput->GetSize(nFileSize);
            length = (double)nFileSize;
            if (length == -1)
                length = 0;
            else
                length = ((double)length*8.)/((double)bitrate) + 0.5;

            len_state.m_header_type = 2;
        } else {
			uint32		nFileSize;
			pInput->GetSize(nFileSize);
            length = (double)nFileSize;
            length = ((double)length*8.)/((double)bitrate*1000.) + 0.5;

            len_state.m_header_type = 0;
        }

        if (len_state.m_aac_buffer)
            free(len_state.m_aac_buffer);

        target = len_state.m_head;
        while (target)
        {
            struct seek_list *tmp = target;
            target = target->next;
            if (tmp) free(tmp);
        }

        return (int)(length*1000.);
    }
}


/* Get the title from the file */
void Mp4ffGetTags(mp4ff_t *file, IMediaInfo *media)
{
    char temp[4096];
    int some_info = 0;
    char *out = temp;//title;
    char *bound = out + sizeof(temp) - 256; //out + (MAX_PATH - 10 - 1);
    char *pVal;
	tstringex	str;

    if (mp4ff_meta_get_track(file, &pVal))
	{
		Utf8ToUCS2(pVal, -1, str);
        media->setAttribute(MA_TRACK_NUMB, str.c_str());
	}

    if (mp4ff_meta_get_artist(file, &pVal))
	{
		Utf8ToUCS2(pVal, -1, str);
        media->setAttribute(MA_ARTIST, str.c_str());
	}

	if (mp4ff_meta_get_title(file, &pVal))
	{
		Utf8ToUCS2(pVal, -1, str);
        media->setAttribute(MA_TITLE, str.c_str());
	}

	if (mp4ff_meta_get_album(file, &pVal))
	{
		Utf8ToUCS2(pVal, -1, str);
        media->setAttribute(MA_ALBUM, str.c_str());
	}

	if (mp4ff_meta_get_date(file, &pVal))
	{
		Utf8ToUCS2(pVal, -1, str);
        media->setAttribute(MA_YEAR, str.c_str());
	}

	if (mp4ff_meta_get_comment(file, &pVal))
	{
		Utf8ToUCS2(pVal, -1, str);
        media->setAttribute(MA_COMMENT, str.c_str());
	}

	if (mp4ff_meta_get_genre(file, &pVal))
	{
		Utf8ToUCS2(pVal, -1, str);
        media->setAttribute(MA_GENRE, str.c_str());
	}
}

ResultCode CMDMP4::PreDecode()
{
    int avg_bitrate, br, sr;
    unsigned char *buffer;
    unsigned int buffer_size;
    NeAACDecConfigurationPtr config;
    unsigned char header[8];

    memset(&mp4state, 0, sizeof(DecoderState));
	memset(&m_audioInfo, 0, sizeof(m_audioInfo));

	mp4state.pInput = m_pInput;

	m_pInput->GetSize(m_audioInfo.nMediaFileSize);

	m_pInput->Seek(0, SEEK_SET);
    m_pInput->Read(header, 8);
	m_pInput->Seek(0, SEEK_SET);

    if (header[4] == 'f' && header[5] == 't' && header[6] == 'y' && header[7] == 'p')
        mp4state.filetype = 0;
	else
		mp4state.filetype = 1;

    if (mp4state.filetype)
    {
        int tagsize = 0, tmp = 0, init;
        int bread = 0;
        double length = 0.;
        int bitrate = 128;

        m_bIsSeekable = 1;

        tagsize = skip_id3v2_tag();
        if (tagsize<0)
			return ERR_PLAYER_INVALID_FILE;

        if (!(mp4state.m_aac_buffer = (unsigned char*)malloc(768*6)))
        {
            show_error(_T("Memory allocation error."));
            return ERR_NO_MEM;
        }

        for (init=0; init<2; init++)
        {
            mp4state.hDecoder = NeAACDecOpen();
            if (!mp4state.hDecoder)
            {
                show_error(_T("Unable to open decoder library."));
                return ERR_DECODER_INIT_FAILED;
            }

            config = NeAACDecGetCurrentConfiguration(mp4state.hDecoder);
            config->outputFormat = m_resolution + 1;
            config->downMatrix = m_downmix;
            NeAACDecSetConfiguration(mp4state.hDecoder, config);

            memset(mp4state.m_aac_buffer, 0, 768*6);
            bread = mp4state.pInput->Read(mp4state.m_aac_buffer, 768*6);
            mp4state.m_aac_bytes_into_buffer = bread;
            mp4state.m_aac_bytes_consumed = 0;
            mp4state.m_file_offset = 0;
            mp4state.m_at_eof = (bread != 768*6) ? 1 : 0;

            if (init==0)
            {
                NeAACDecFrameInfo frameInfo;

                fill_buffer(&mp4state);

                if ((mp4state.m_aac_bytes_consumed = NeAACDecInit(mp4state.hDecoder,
                    mp4state.m_aac_buffer, mp4state.m_aac_bytes_into_buffer,
                    &m_audioInfo.nSampleRate, &m_audioInfo.nChannels)) < 0)
                {
                    show_error(_T("Can't initialize decoder library."));
                    return ERR_DECODER_INIT_FAILED;
                }
                advance_buffer(&mp4state, mp4state.m_aac_bytes_consumed);

                do {
                    memset(&frameInfo, 0, sizeof(NeAACDecFrameInfo));
                    fill_buffer(&mp4state);
                    NeAACDecDecode(mp4state.hDecoder, &frameInfo, mp4state.m_aac_buffer, mp4state.m_aac_bytes_into_buffer);
                } while (!frameInfo.samples && !frameInfo.error);

                if (frameInfo.error)
                {
                    show_error(NeAACDecGetErrorMessage(frameInfo.error));
                    return ERR_PLAYER_INVALID_FILE;
                }

                m_audioInfo.nChannels = frameInfo.channels;
                m_audioInfo.nSampleRate = frameInfo.samplerate;
                mp4state.framesize = (frameInfo.channels != 0) ? frameInfo.samples/frameInfo.channels : 0;
                /*
                sbr = frameInfo.sbr;
                profile = frameInfo.object_type;
                header_type = frameInfo.header_type;
                */

                NeAACDecClose(mp4state.hDecoder);
                mp4state.pInput->Seek(tagsize, SEEK_SET);
            }
        }

        mp4state.m_head = (struct seek_list*)malloc(sizeof(struct seek_list));
        mp4state.m_tail = mp4state.m_head;
        mp4state.m_tail->next = NULL;

        mp4state.m_header_type = 0;
        if ((mp4state.m_aac_buffer[0] == 0xFF) && ((mp4state.m_aac_buffer[1] & 0xF6) == 0xF0))
        {
            if (1) //(can_seek)
            {
                adts_parse(&mp4state, &bitrate, &length);
                mp4state.pInput->Seek(tagsize, SEEK_SET);

                bread = mp4state.pInput->Read(mp4state.m_aac_buffer, 768*6);
                if (bread != 768*6)
                    mp4state.m_at_eof = 1;
                else
                    mp4state.m_at_eof = 0;
                mp4state.m_aac_bytes_into_buffer = bread;
                mp4state.m_aac_bytes_consumed = 0;

                mp4state.m_header_type = 1;
            }
        } else if (memcmp(mp4state.m_aac_buffer, "ADIF", 4) == 0) {
            int skip_size = (mp4state.m_aac_buffer[4] & 0x80) ? 9 : 0;
            bitrate = ((unsigned int)(mp4state.m_aac_buffer[4 + skip_size] & 0x0F)<<19) |
                ((unsigned int)mp4state.m_aac_buffer[5 + skip_size]<<11) |
                ((unsigned int)mp4state.m_aac_buffer[6 + skip_size]<<3) |
                ((unsigned int)mp4state.m_aac_buffer[7 + skip_size] & 0xE0);

			length = m_audioInfo.nMediaFileSize;
            if (length == -1)
            {
                m_bIsSeekable = 0;
                length = 0;
            } else {
                length = ((double)length*8.)/((double)bitrate) + 0.5;
            }

            mp4state.m_header_type = 2;
        } else {
			length = m_audioInfo.nMediaFileSize;
            length = ((double)length*8.)/((double)bitrate*1000.) + 0.5;

            m_bIsSeekable = 1;
        }

        mp4state.m_length = (int)(length*1000.);

        fill_buffer(&mp4state);
        if ((mp4state.m_aac_bytes_consumed = NeAACDecInit(mp4state.hDecoder,
            mp4state.m_aac_buffer, mp4state.m_aac_bytes_into_buffer,
            &m_audioInfo.nSampleRate, &m_audioInfo.nChannels)) < 0)
        {
            show_error(_T("Can't initialize decoder library."));
            return ERR_DECODER_INIT_FAILED;
        }
        advance_buffer(&mp4state, mp4state.m_aac_bytes_consumed);

        if (mp4state.m_header_type == 2)
            avg_bitrate = bitrate;
        else
            avg_bitrate = bitrate*1000;
    } else {
        mp4state.hDecoder = NeAACDecOpen();
        if (!mp4state.hDecoder)
        {
            show_error(_T("Unable to open decoder library."));
            return ERR_DECODER_INIT_FAILED;
        }

        config = NeAACDecGetCurrentConfiguration(mp4state.hDecoder);
        config->outputFormat = m_resolution + 1;
        config->downMatrix = m_downmix;
        NeAACDecSetConfiguration(mp4state.hDecoder, config);

        mp4state.pInput = m_pInput;
        mp4state.mp4cb.read = read_callback;
        mp4state.mp4cb.seek = seek_callback;
        mp4state.mp4cb.user_data = mp4state.pInput;


        mp4state.mp4file = mp4ff_open_read(&mp4state.mp4cb);
        if (!mp4state.mp4file)
        {
            show_error(_T("Unable to open file."));
            NeAACDecClose(mp4state.hDecoder);
            return ERR_PLAYER_INVALID_FILE;
        }

        if ((mp4state.mp4track = GetAACTrack(mp4state.mp4file)) < 0)
        {
            show_error(_T("Unsupported Audio track type."));
            NeAACDecClose(mp4state.hDecoder);
            mp4ff_close(mp4state.mp4file);
            return ERR_PLAYER_INVALID_FILE;
        }

        buffer = NULL;
        buffer_size = 0;
        mp4ff_get_decoder_config(mp4state.mp4file, mp4state.mp4track,
            &buffer, &buffer_size);
        if (!buffer)
        {
            NeAACDecClose(mp4state.hDecoder);
            mp4ff_close(mp4state.mp4file);
            return ERR_PLAYER_INVALID_FILE;
        }

        if(NeAACDecInit2(mp4state.hDecoder, buffer, buffer_size,
            &m_audioInfo.nSampleRate, &m_audioInfo.nChannels) < 0)
        {
            /* If some error initializing occured, skip the file */
            NeAACDecClose(mp4state.hDecoder);
            mp4ff_close(mp4state.mp4file);
            if (buffer) free (buffer);
            return ERR_PLAYER_INVALID_FILE;
        }

        /* for gapless decoding */
        {
            mp4AudioSpecificConfig mp4ASC;

            mp4state.timescale = mp4ff_time_scale(mp4state.mp4file, mp4state.mp4track);
            mp4state.framesize = 1024;
            mp4state.useAacLength = 0;

            if (buffer)
            {
                if (NeAACDecAudioSpecificConfig(buffer, buffer_size, &mp4ASC) >= 0)
                {
                    if (mp4ASC.frameLengthFlag == 1) mp4state.framesize = 960;
                    if (mp4ASC.sbr_present_flag == 1) mp4state.framesize *= 2;
                }
            }
        }

        free(buffer);

        avg_bitrate = mp4ff_get_avg_bitrate(mp4state.mp4file, mp4state.mp4track);

        mp4state.numSamples = mp4ff_num_samples(mp4state.mp4file, mp4state.mp4track);
        mp4state.sampleId = 0;

        {
            double timescale_div = 1.0 / (double)mp4ff_time_scale(mp4state.mp4file, mp4state.mp4track);
            int64_t duration = mp4ff_get_track_duration_use_offsets(mp4state.mp4file, mp4state.mp4track);
            if (duration == -1)
            {
                mp4state.m_length = 0;
            } else {
                mp4state.m_length = (int)((double)duration * timescale_div * 1000.0);
            }
        }

        m_bIsSeekable = 1;
    }

    if (m_audioInfo.nChannels == 0)
    {
        show_error(_T("Number of channels not supported for playback."));
        NeAACDecClose(mp4state.hDecoder);
        if (mp4state.filetype)
            ;//fclose(mp4state.aacfile);
        else {
            mp4ff_close(mp4state.mp4file);
        }
        return ERR_PLAYER_INVALID_FILE;
    }

    if (m_downmix && (m_audioInfo.nChannels == 5 || m_audioInfo.nChannels == 6))
        m_audioInfo.nChannels = 2;

    ResultCode	nRet = m_pOutput->Open(m_audioInfo.nSampleRate, (int)m_audioInfo.nChannels,
        res_table[m_resolution]);
    if (nRet != ERR_OK)
    {
        NeAACDecClose(mp4state.hDecoder);
        if (mp4state.filetype)
            ;//fclose(mp4state.aacfile);
        else {
            mp4ff_close(mp4state.mp4file);
        }
        return nRet;
    }

    mp4state.decode_pos_ms = 0;

    br = (int)floor(((float)avg_bitrate + 500.0)/1000.0 + 0.5);
    sr = (int)floor((float)m_audioInfo.nSampleRate/1000.0 + 0.5);

    return ERR_OK;
}
/*

int getoutputtime()
{
    return mp4state.decode_pos_ms+(module.outMod->GetOutputTime()-module.outMod->GetWrittenTime());
}
*/

static void remap_channels(unsigned char *data, unsigned char *dataOut, unsigned int samples, unsigned int bps)
{
    unsigned int i;

    switch (bps)
    {
    case 8:
        {
            for (i = 0; i < samples; i += 6)
            {
                dataOut[i+2] = data[i];
                dataOut[i] = data[i+1];
                dataOut[i+1] = data[i+2];
                dataOut[i+4] = data[i+3];
                dataOut[i+5] = data[i+4];
                dataOut[i+3] = data[i+5];
            }
        }
        break;

    case 16:
    default:
        {
            unsigned short *sample_buffer = (unsigned short *)data;
            unsigned short *sample_bufferOut = (unsigned short *)dataOut;
            for (i = 0; i < samples; i += 6)
            {
                sample_bufferOut[i+2] = sample_buffer[i];
                sample_bufferOut[i] = sample_buffer[i+1];
                sample_bufferOut[i+1] = sample_buffer[i+2];
                sample_bufferOut[i+4] = sample_buffer[i+3];
                sample_bufferOut[i+5] = sample_buffer[i+4];
                sample_bufferOut[i+3] = sample_buffer[i+5];
            }
        }
        break;

    case 24:
    case 32:
        {
            unsigned int *sample_buffer = (unsigned int *)data;
            unsigned int *sample_bufferOut = (unsigned int *)dataOut;
            for (i = 0; i < samples; i += 6)
            {
                sample_bufferOut[i+2] = sample_buffer[i];
                sample_bufferOut[i] = sample_buffer[i+1];
                sample_bufferOut[i+1] = sample_buffer[i+2];
                sample_bufferOut[i+4] = sample_buffer[i+3];
                sample_bufferOut[i+5] = sample_buffer[i+4];
                sample_bufferOut[i+3] = sample_buffer[i+5];
            }
        }
        break;
    }
}

ResultCode CMDMP4::MP4PlayThreadFun()
{
    int seq_frames = 0;
    int seq_bytes = 0;

    void *sample_buffer;
    unsigned char *buffer;
    unsigned int buffer_size;
    NeAACDecFrameInfo frameInfo;

    int last_frame = 0;
    mp4state.initial = 1;

	IFBuffer	*pBuf = NULL;

    while (!m_bKillThread)
    {
        /* seeking */
        if (m_bSeekFlag)
        {
            int64_t duration;
            int32_t skip_samples = 0;

            m_pOutput->Flush();
            duration = (int64_t)(m_nSeekPos / 1000.0 * m_audioInfo.nSampleRate + 0.5);
            mp4state.sampleId = mp4ff_find_sample_use_offsets(mp4state.mp4file,
                mp4state.mp4track, duration, &skip_samples);

            mp4state.decode_pos_ms = m_nSeekPos;
            m_bSeekFlag = false;
        }

        if (last_frame)
        {
            break;
        } else {
            int rc;

            /* for gapless decoding */
            char *buf;
            long dur;
            unsigned int sample_count;
            unsigned int delay = 0;

            /* get acces unit from MP4 file */
            buffer = NULL;
            buffer_size = 0;

            dur = mp4ff_get_sample_duration(mp4state.mp4file, mp4state.mp4track, mp4state.sampleId);
            rc = mp4ff_read_sample(mp4state.mp4file, mp4state.mp4track, mp4state.sampleId++, &buffer,  &buffer_size);

            if (mp4state.sampleId == 1) dur = 0;
            if (rc == 0 || buffer == NULL)
            {
                last_frame = 1;
                sample_buffer = NULL;
                frameInfo.samples = 0;
            } else {
                sample_buffer = NeAACDecDecode(mp4state.hDecoder, &frameInfo,
                    buffer, buffer_size);
            }
            if (frameInfo.error > 0)
            {
                show_error(NeAACDecGetErrorMessage(frameInfo.error));
                last_frame = 1;
            }
            if (mp4state.sampleId >= mp4state.numSamples)
                last_frame = 1;

            if (buffer) free(buffer);

            if (mp4state.useAacLength || (mp4state.timescale != m_audioInfo.nSampleRate)) {
                sample_count = frameInfo.samples;
            } else {
                sample_count = (unsigned int)(dur * frameInfo.channels);

                if (!mp4state.useAacLength && !mp4state.initial && (mp4state.sampleId < mp4state.numSamples/2) && (dur*frameInfo.channels != frameInfo.samples))
                {
                    //fprintf(stderr, _T("MP4 seems to have incorrect frame duration, using values from AAC data.\n"));
                    mp4state.useAacLength = 1;
                    sample_count = frameInfo.samples;
                }
            }

            if (mp4state.initial && (sample_count < mp4state.framesize*m_audioInfo.nChannels) && (frameInfo.samples > sample_count))
            {
                delay = frameInfo.samples - sample_count;
            }

            if (!m_bKillThread && (sample_count > 0))
            {
                buf = (char *)sample_buffer;
                mp4state.initial = 0;

                switch (res_table[m_resolution])
                {
                case 8:
                    buf += delay;
                    break;
                case 16:
                default:
                    buf += delay * 2;
                    break;
                case 24:
                case 32:
                    buf += delay * 4;
                    break;
                case 64:
                    buf += delay * 8;
                }

				int			bufSize;
				if (res_table[m_resolution] == 24)
					bufSize = frameInfo.samples * 32 / 8;
				else
					bufSize = frameInfo.samples * res_table[m_resolution] / 8;

			    pBuf = m_pMemAllocator->AllocFBuffer(DECODE_BUFF_SIZE);
			    char	*bufData = pBuf->data() + pBuf->size();

                if (frameInfo.channels == 6 && frameInfo.num_lfe_channels)
                    remap_channels((unsigned char*)buf, (unsigned char*)bufData, sample_count, res_table[m_resolution]);
				else
					memcpy(bufData, sample_buffer, bufSize);
				pBuf->resize(bufSize);

                if (res_table[m_resolution] == 24)
                {
                    /* convert libfaad output (3 bytes packed in 4) */
                    convert3in4to3in3(bufData, sample_count);
					pBuf->resize(pBuf->size() - frameInfo.samples * frameInfo.channels);
                }
                mp4state.decode_pos_ms += (double)sample_count * 1000.0 /
                    ((double)frameInfo.samplerate * (double)frameInfo.channels);

				m_pOutput->WaitForWrite();

				if (m_bKillThread)
				{
					pBuf->Release();
					break;
				}

				if (m_bSeekFlag)
					pBuf->Release();
				else
				{
					m_pPlayer->OutputWrite(pBuf, res_table[m_resolution], frameInfo.channels, frameInfo.samplerate);
				}
 
//                 /* VBR bitrate display */
//                 if (m_vbr_display)
//                 {
//                     seq_frames++;
//                     seq_bytes += frameInfo.bytesconsumed;
//                     if (seq_frames == (int)(floor((float)frameInfo.samplerate/(float)(sample_count/frameInfo.channels) + 0.5)))
//                     {
//                         module.SetInfo((int)floor(((float)seq_bytes*8.)/1000. + 0.5),
//                             (int)floor(frameInfo.samplerate/1000. + 0.5),
//                             m_audioInfo.nChannels, 1);
// 
//                         seq_frames = 0;
//                         seq_bytes = 0;
//                     }
//                 }
            }
        }
    }

    return ERR_OK;
}

void *decode_aac_frame(DecoderState *st, NeAACDecFrameInfo *frameInfo)
{
    void *sample_buffer = NULL;

    do
    {
        fill_buffer(st);

        if (st->m_aac_bytes_into_buffer != 0)
        {
            sample_buffer = NeAACDecDecode(st->hDecoder, frameInfo,
                st->m_aac_buffer, st->m_aac_bytes_into_buffer);

            if (st->m_header_type != 1)
            {
                if (st->last_offset < st->m_file_offset)
                {
                    st->m_tail->offset = st->m_file_offset;
                    st->m_tail->next = (struct seek_list*)malloc(sizeof(struct seek_list));
                    st->m_tail = st->m_tail->next;
                    st->m_tail->next = NULL;
                    st->last_offset = st->m_file_offset;
                }
            }

            advance_buffer(st, frameInfo->bytesconsumed);
        } else {
            break;
        }

    } while (!frameInfo->samples && !frameInfo->error);

    return sample_buffer;
}

int aac_seek(DecoderState *st, int nSampleRate, double seconds)
{
    int i, frames;
    int bread;
    struct seek_list *target = st->m_head;

    if (1 /*can_seek*/ && ((st->m_header_type == 1) || (seconds < st->cur_pos_sec)))
    {
        frames = (int)(seconds*((double)nSampleRate/(double)st->framesize) + 0.5);

        for (i = 0; i < frames; i++)
        {
            if (target->next)
                target = target->next;
            else
                return 0;
        }
        if (target->offset == 0 && frames > 0)
            return 0;
        st->pInput->Seek(target->offset, SEEK_SET);
        st->m_file_offset = target->offset;

        bread = st->pInput->Read(st->m_aac_buffer, 768*6);
        if (bread != 768*6)
            st->m_at_eof = 1;
        else
            st->m_at_eof = 0;
        st->m_aac_bytes_into_buffer = bread;
        st->m_aac_bytes_consumed = 0;
        st->m_file_offset += bread;

        NeAACDecPostSeekReset(st->hDecoder, -1);

        return 1;
    } else {
        if (seconds > st->cur_pos_sec)
        {
            NeAACDecFrameInfo frameInfo;

            frames = (int)((seconds - st->cur_pos_sec)*((double)nSampleRate/(double)st->framesize));

            if (frames > 0)
            {
                for (i = 0; i < frames; i++)
                {
                    memset(&frameInfo, 0, sizeof(NeAACDecFrameInfo));
                    decode_aac_frame(st, &frameInfo);

                    if (frameInfo.error || (st->m_aac_bytes_into_buffer == 0))
                    {
                        if (frameInfo.error)
                        {
                            if (NeAACDecGetErrorMessage(frameInfo.error) != NULL)
                                show_error(NeAACDecGetErrorMessage(frameInfo.error));
                        }
                        return 0;
                    }
                }
            }

            NeAACDecPostSeekReset(st->hDecoder, -1);
        }
        return 1;
    }
}

ResultCode CMDMP4::AACPlayThreadFun()
{
    int seq_frames = 0;
    int seq_bytes = 0;
	IFBuffer	*pBuf = NULL;

    while (!m_bKillThread)
    {
        /* seeking */
        if (m_bSeekFlag)
        {
            double ms;

            ms = m_nSeekPos / 1000;
            if (aac_seek(&mp4state, m_audioInfo.nSampleRate, ms)!=0)
            {
                m_pOutput->Flush();
                mp4state.cur_pos_sec = ms;
                mp4state.decode_pos_ms = ms * 1000;
            }
            m_bSeekFlag = false;
        }

        NeAACDecFrameInfo frameInfo;
        void *sample_buffer;

        memset(&frameInfo, 0, sizeof(NeAACDecFrameInfo));

        sample_buffer = decode_aac_frame(&mp4state, &frameInfo);

        if (frameInfo.error || (mp4state.m_aac_bytes_into_buffer == 0))
        {
            if (frameInfo.error)
            {
                if (NeAACDecGetErrorMessage(frameInfo.error) != NULL)
                    show_error(NeAACDecGetErrorMessage(frameInfo.error));
            }
			break;
        }

        if (frameInfo.samples > 0)
        {
			int			bufSize;
			if (res_table[m_resolution] == 24)
				bufSize = frameInfo.samples * 32 / 8;
			else
				bufSize = frameInfo.samples * res_table[m_resolution] / 8;

			pBuf = m_pMemAllocator->AllocFBuffer(DECODE_BUFF_SIZE);
			char	*bufData = pBuf->data();
            if (frameInfo.channels == 6 && frameInfo.num_lfe_channels)
                remap_channels((unsigned char*)sample_buffer, (unsigned char*)bufData, frameInfo.samples, res_table[m_resolution]);
			else
				memcpy(bufData, sample_buffer, bufSize);
			pBuf->resize(bufSize);

            if (res_table[m_resolution] == 24)
            {
                /* convert libfaad output (3 bytes packed in 4 bytes) */
                convert3in4to3in3(bufData, frameInfo.samples);
				pBuf->resize(pBuf->size() - frameInfo.samples * frameInfo.channels);
            }

            mp4state.decode_pos_ms += (double)frameInfo.samples * 1000.0 /
                ((double)frameInfo.samplerate* (double)frameInfo.channels);


			if (m_bKillThread)
			{
				pBuf->Release();
				return ERR_OK;
			}

			if (m_bSeekFlag)
				pBuf->Release();
			else
			{
				m_pPlayer->OutputWrite(pBuf, res_table[m_resolution], frameInfo.channels, frameInfo.samplerate);
			}

            /* VBR bitrate display */
//             if (m_vbr_display)
//             {
//                 seq_frames++;
//                 seq_bytes += frameInfo.bytesconsumed;
//                 if (seq_frames == (int)(floor((float)frameInfo.samplerate/(float)(frameInfo.samples/frameInfo.channels) + 0.5)))
//                 {
//                     module.SetInfo((int)floor(((float)seq_bytes*8.)/1000. + 0.5),
//                         (int)floor(frameInfo.samplerate/1000. + 0.5),
//                         m_audioInfo.nChannels, 1);
// 
//                     seq_frames = 0;
//                     seq_bytes = 0;
//                 }
//             }
        }

        if (frameInfo.channels > 0 && m_audioInfo.nSampleRate > 0)
            mp4state.cur_pos_sec += ((double)(frameInfo.samples/frameInfo.channels))/(double)m_audioInfo.nSampleRate;
    }

    return ERR_OK;
}

////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


CMDMP4::CMDMP4()
{
	OBJ_REFERENCE_INIT;

	m_pInput = NULL;
	m_pPlayer = NULL;
	m_pOutput = NULL;
	m_bPaused = false;
	m_bKillThread = false;
	m_nSeekPos = 0;
	m_bSeekFlag = false;
	m_state = PS_STOPPED;
}

CMDMP4::~CMDMP4()
{
	if (m_pPlayer)
	{
		m_pPlayer->Release();
		m_pPlayer = NULL;
	}
}

OBJ_REFERENCE_IMP(CMDMP4)

cstr_t CMDMP4::getDescription()
{
	return SZ_DECODER_DESC;
}

// get supported file's extentions.
cstr_t CMDMP4::getFileExtentions()
{
	return _T(".m4a\0MPEG-4 Files (*.m4a)\0.aac\0AAC Files (*.aac)\0");
}

ResultCode CMDMP4::getMediaInfo(IMediaInput *pInput, IMediaInfo *media)
{
    memset(&mp4state, 0, sizeof(DecoderState));
	memset(&m_audioInfo, 0, sizeof(m_audioInfo));

	mp4state.pInput = m_pInput;

	pInput->Seek(0, SEEK_SET);
	m_audioInfo.nMediaLength = getsonglength(pInput);
	pInput->Seek(0, SEEK_SET);
    unsigned char header[8];
    pInput->Read(header, 8);

    if (header[4] == 'f' && header[5] == 't' && header[6] == 'y' && header[7] == 'p')
    {
        mp4ff_t *file;
        mp4ff_callback_t mp4cb = {0};

        mp4cb.read = read_callback;
        mp4cb.seek = seek_callback;
        mp4cb.user_data = pInput;

        file = mp4ff_open_read(&mp4cb);
        if (file == NULL)
            return ERR_FALSE;

        Mp4ffGetTags(file, media);

        mp4ff_close(file);
    }

	pInput->GetSize(m_audioInfo.nMediaFileSize);
	media->setAttribute(MA_FILESIZE, m_audioInfo.nMediaFileSize);
	media->setAttribute(MA_DURATION, m_audioInfo.nMediaLength);
// 	media->setAttribute(SZ_M_SampleRate, m_audioInfo.nSampleRate, false);
//	media->setAttribute(SZ_M_Channels, m_audioInfo.nChannels, false);
 	// media->setAttribute(SZ_M_BPS, m_audioInfo.nBps, false);

	return ERR_OK;
}

bool CMDMP4::isSeekable()
{
	return true;
}

bool CMDMP4::isUseOutputPlug()
{
	return true;
}

ResultCode CMDMP4::play(IMPlayer *pPlayer, IMediaInput *pInput)
{
	ResultCode		nRet;

	m_state = PS_STOPPED;
	m_bPaused = false;
	m_nSeekPos = 0;
	m_bSeekFlag = false;

	m_pPlayer = pPlayer;
	m_pPlayer->AddRef();

	m_pInput = pInput;
	m_pInput->AddRef();

	nRet = m_pPlayer->QueryInterface(MPIT_OUTPUT, (LPVOID *)&m_pOutput);
	if (nRet != ERR_OK)
		goto R_FAILED;

	nRet = m_pPlayer->QueryInterface(MPIT_MEMALLOCATOR, (LPVOID *)&m_pMemAllocator);
	if (nRet != ERR_OK)
		goto R_FAILED;

	nRet = PreDecode();
	if (nRet != ERR_OK)
		goto R_FAILED;

	m_state = PS_PLAYING;
	m_bKillThread = false;

	if (!m_threadDecode.Create(DecodeThread, this))
	{
		nRet = ERR_CREATE_THREAD;
		goto R_FAILED;
	}
	m_threadDecode.SetPriority(THREAD_PRIORITY_HIGHEST);
// #ifdef _WIN32_WCE
// 	HMODULE hModule = LoadLibrary(_T("Coredll.dll"));
// 	if (hModule) {
// 		BOOL (WINAPI *pCeSetThreadPriority)(HANDLE hThread, int nPriority); 
// 		(FARPROC&)pCeSetThreadPriority = GetProcAddress(hModule, _T("CeSetThreadPriority"));
// 		if (pCeSetThreadPriority) 
// 			pCeSetThreadPriority(GetCurrentThread(), 160);
// 		FreeLibrary(hModule);
// 	}
// #endif

	return ERR_OK;

R_FAILED:
	if (m_pPlayer)
	{
		m_pPlayer->Release();
		m_pPlayer = NULL;
	}
	if (m_pInput)
	{
		m_pInput->Release();
		m_pInput = NULL;
	}
	if (m_pOutput)
	{
		m_pOutput->Release();
		m_pOutput = NULL;
	}
	if (m_pMemAllocator)
	{
		m_pMemAllocator->Release();
		m_pMemAllocator = NULL;
	}
	return nRet;
}

ResultCode CMDMP4::pause()
{
	if (m_state != PS_PLAYING)
		return ERR_PLAYER_INVALID_STATE;
	m_state = PS_PAUSED;
	ASSERT(m_pOutput);
	return m_pOutput->pause(true);
}

ResultCode CMDMP4::unpause()
{
	if (m_state != PS_PAUSED)
		return ERR_PLAYER_INVALID_STATE;
	m_state = PS_PLAYING;
	ASSERT(m_pOutput);
	return m_pOutput->pause(false);
}

bool CMDMP4::IsPaused()
{
	return m_state == PS_PAUSED;
}

ResultCode CMDMP4::stop()
{
	m_bKillThread = true;

	if (m_state == PS_PAUSED)
	{
		ASSERT(m_pOutput);
		m_pOutput->Flush();
	}

	m_threadDecode.Join();

	return ERR_OK;
}

uint32 CMDMP4::getLength()
{
	return mp4state.m_length;
}

ResultCode CMDMP4::Seek(uint32 dwPos)
{
	m_bSeekFlag = true;
	m_nSeekPos = dwPos;

	ASSERT(m_pOutput);
	if (m_pOutput && m_state == PS_PLAYING)
		m_pOutput->Flush();

	return ERR_OK;
}

uint32 CMDMP4::GetPos()
{
	if (m_pOutput && !m_bSeekFlag)
		return m_nSeekPos + m_pOutput->GetPos();
	else
		return m_nSeekPos;
}

ResultCode CMDMP4::setVolume(int volume, int nBanlance)
{
	if (!m_pOutput)
		return ERR_PLAYER_INVALID_STATE;

	return m_pOutput->setVolume(volume, nBanlance);
}

void CMDMP4::DecodeThread(LPVOID lpParam)
{
	CMDMP4		*pMDRow;

	pMDRow = (CMDMP4 *)lpParam;

	pMDRow->AddRef();
	pMDRow->DecodeThreadProc();
	pMDRow->Release();
}

void CMDMP4::DecodeThreadProc()
{
    if (mp4state.filetype)
    {
        AACPlayThreadFun();
    } else {
        MP4PlayThreadFun();
    }

	//
	// when thread stop
	//
    struct seek_list *target = mp4state.m_head;

    if (mp4state.m_aac_buffer)
        free(mp4state.m_aac_buffer);

    while (target)
    {
        struct seek_list *tmp = target;
        target = target->next;
        if (tmp) free(tmp);
    }
	if (mp4state.hDecoder)
		NeAACDecClose(mp4state.hDecoder);
    if (mp4state.filetype)
        ;//fclose(mp4state.aacfile);
    else {
        mp4ff_close(mp4state.mp4file);
    }

	while (!m_bKillThread && m_pOutput->IsPlaying())
	{
		Sleep(10);
	}

	m_state = PS_STOPPED;

	m_pOutput->stop();
	m_pOutput->Release();
	m_pOutput = NULL;

	m_pInput->Release();
	m_pInput = NULL;

	m_pMemAllocator->Release();
	m_pMemAllocator = NULL;

	m_pPlayer->notifyEod(this, ERR_OK);
	m_pPlayer->Release();
	m_pPlayer = NULL;
}

#ifdef _WIN32_WCE

IMediaDecoder *MPlayerGetMDMP4()
{
	return new CMDMP4;
}

#else

//
// Query all Plugins and its description in a DLL
// Query will start with nIndex = 0, till !ERR_OK returned.
//
// nIndex = 0, return its interface type, and description, and lpInterface.
// lpInterface can be NULL.
//
extern "C" __declspec(dllexport) ResultCode DHPlayerQueryPluginIF(
	int index,
	MPInterfaceType *interfaceType,
	const char **description,
	LPVOID *interfacePtr
	)
{
	if (index == 0)
	{
		*interfaceType = MPIT_DECODE;
		if (description)
			description = SZ_DECODER_DESC;
		if (interfacePtr) {
			IMediaDecoder	*pDecoder = new CMDMP4;
			pDecoder->AddRef();
			*interfacePtr = pDecoder;
		}

		return ERR_OK;
	}
	else
		return ERR_FALSE;
}

#endif // _WIN32_WCE
