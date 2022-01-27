// MP4Tag.cpp: implementation of the CMP4Tag class.
//
//////////////////////////////////////////////////////////////////////

#include "MP4Tag.h"

/* returns the name of the object type */
char *get_ot_string(int ot)
{
    switch (ot)
    {
    case 0:
        return "Main";
    case 1:
        return "LC";
    case 2:
        return "SSR";
    case 3:
        return "LTP";
    }
    return NULL;
}

int tag_add_field(mp4ff_metadata_t *tags, const char *item, const char *value, size_t v_len)
{
    void *backup = (void *)tags->tags;

    if (!item || (item && !*item) || !value) return 0;

    tags->tags = (mp4ff_tag_t *)realloc(tags->tags, (tags->count+1) * sizeof(mp4ff_tag_t));
    if (!tags->tags) {
        if (backup) free(backup);
        return 0;
    }
    else
    {
        size_t i_len = strlen(item);
        if (v_len == 0) v_len = strlen(value);

        tags->tags[tags->count].item = (char *)malloc(i_len+1);
        tags->tags[tags->count].value = (char *)malloc(v_len+1);

        if (!tags->tags[tags->count].item || !tags->tags[tags->count].value)
        {
            if (!tags->tags[tags->count].item) free (tags->tags[tags->count].item);
            if (!tags->tags[tags->count].value) free (tags->tags[tags->count].value);
            tags->tags[tags->count].item = NULL;
            tags->tags[tags->count].value = NULL;
            return 0;
        }

        memcpy(tags->tags[tags->count].item, item, i_len);
        memcpy(tags->tags[tags->count].value, value, v_len);
        tags->tags[tags->count].item[i_len] = '\0';
        tags->tags[tags->count].value[v_len] = '\0';
//        tags->tags[tags->count].len = v_len;

        tags->count++;
        return 1;
    }
}

int tag_set_field(mp4ff_metadata_t *tags, const char *item, const char *value, size_t v_len)
{
    unsigned int i;

    if (!item || (item && !*item) || !value) return 0;

    for (i = 0; i < tags->count; i++)
    {
        if (!stricmp(tags->tags[i].item, item))
        {
            void *backup = (void *)tags->tags[i].value;
            if (v_len == 0) v_len = strlen(value);

            tags->tags[i].value = (char *)realloc(tags->tags[i].value, v_len+1);
            if (!tags->tags[i].value)
            {
                if (backup) free(backup);
                return 0;
            }

            memcpy(tags->tags[i].value, value, v_len);
            tags->tags[i].value[v_len] = '\0';
//            tags->tags[i].len = v_len;

            return 1;
        }
    }

    return tag_add_field(tags, item, value, v_len);
}

int tag_delete(mp4ff_metadata_t *tags)
{
    unsigned int i;

    for (i = 0; i < tags->count; i++)
    {
        if (tags->tags[i].item) free(tags->tags[i].item);
        if (tags->tags[i].value) free(tags->tags[i].value);
    }

    if (tags->tags) free(tags->tags);

    tags->tags = NULL;
    tags->count = 0;

	return 0;
}

int ReadMP4Tag(mp4ff_t *file, mp4ff_metadata_t *tags)
{
    unsigned __int8 *pValue;
    char *pName;
    unsigned int i = 0;

    do {
        pName = 0;
        pValue = 0;


        if (mp4ff_meta_get_by_index(file, i, (char **)&pName, (char**)&pValue))
        {
            char *val = (char *)strdup((char*)pValue);
            if (!val) return 0;

            if (pName[0] == '\xA9')
            {
                if (memcmp(pName, "©nam", 4) == 0)
                {
                    tag_add_field(tags, "title", val, strlen(val));
                } else if (memcmp(pName, "©ART", 4) == 0) {
                    tag_add_field(tags, "artist", val, strlen(val));
                } else if (memcmp(pName, "©wrt", 4) == 0) {
                    tag_add_field(tags, "writer", val, strlen(val));
                } else if (memcmp(pName, "©alb", 4) == 0) {
                    tag_add_field(tags, "album", val, strlen(val));
                } else if (memcmp(pName, "©day", 4) == 0) {
                    tag_add_field(tags, "date", val, strlen(val));
                } else if (memcmp(pName, "©too", 4) == 0) {
                    tag_add_field(tags, "tool", val, strlen(val));
                } else if (memcmp(pName, "©cmt", 4) == 0) {
                    tag_add_field(tags, "comment", val, strlen(val));
                } else if (memcmp(pName, "©gen", 4) == 0) {
                    tag_add_field(tags, "genre", val, strlen(val));
                } else {
                    tag_add_field(tags, pName, val, strlen(val));
                }
            } else if (memcmp(pName, "covr", 4) == 0) {
                tag_add_field(tags, "cover", val, strlen(val));
            } else if (memcmp(pName, "gnre", 4) == 0) {
                tag_add_field(tags, "genre", val, strlen(val));
            } else if (memcmp(pName, "trkn", 4) == 0) {
                tag_add_field(tags, "tracknumber", val, strlen(val));
            } else if (memcmp(pName, "disk", 4) == 0) {
                tag_add_field(tags, "disc", val, strlen(val));
            } else if (memcmp(pName, "cpil", 4) == 0) {
                tag_add_field(tags, "compilation", val, strlen(val));
            } else if (memcmp(pName, "tmpo", 4) == 0) {
                tag_add_field(tags, "tempo", val, strlen(val));
            } else if (memcmp(pName, "NDFL", 4) == 0) {
                /* Removed */
            } else {
                tag_add_field(tags, pName, val, strlen(val));
            }

            free(val);
        }

        i++;
    } while (pValue != NULL);

    return 1;
}


#ifdef DEBUG_OUTPUT
void in_mp4_DebugOutput(char *message)
{
    char s[1024];

    sprintf(s, "in_mp4: %s", message);
    OutputDebugString(s);
}
#endif

static void mp4fileinfo(mp4ff_t *mp4, char *info, size_t len)
{
    char *ot[6] = { "NULL", "MAIN AAC", "LC AAC", "SSR AAC", "LTP AAC", "HE AAC" };
    long samples;
    float f = 1024.0;
    float seconds;
    int track;

    NeAACDecHandle hDecoder;
    NeAACDecFrameInfo frameInfo;
    mp4AudioSpecificConfig mp4ASC = {0};
    unsigned char *buffer = NULL;
    unsigned int buffer_size = 0;
    unsigned long sr = 0;
    unsigned char ch = 0;

    if ((track = GetAACTrack(mp4)) < 0)
    {
        info[0] = '\0';
        return;
    }

    hDecoder = NeAACDecOpen();

    samples = mp4ff_num_samples(mp4, track);

    mp4ff_get_decoder_config(mp4, track, &buffer, &buffer_size);
    if (buffer)
    {
        if (NeAACDecAudioSpecificConfig(buffer, buffer_size, &mp4ASC) >= 0)
        {
            if (mp4ASC.frameLengthFlag == 1) f = 960.0;
            if (mp4ASC.sbr_present_flag == 1) f *= 2;
        }

        if(NeAACDecInit2(hDecoder, buffer, buffer_size, &sr, &ch) < 0)
        {
            /* If some error initializing occured, skip the file */
            free(buffer);
            return;
        }

        free(buffer);
        buffer = NULL;
    }

    if (mp4ff_read_sample(mp4, track, 0, &buffer,  &buffer_size) == 0)
    {
        return;
    }
    NeAACDecDecode(hDecoder, &frameInfo, buffer, buffer_size);

    if (buffer) free(buffer);

    seconds = (float)samples*(float)(f-1.0)/(float)mp4ASC.samplingFrequency;

    wsprintf(info, "MPEG-4 %s, %d.%d secs, %d ch, %d Hz\nSBR: %s\nParametric stereo: %s",
        ot[(mp4ASC.objectTypeIndex > 5)?0:mp4ASC.objectTypeIndex],
        (int)(seconds),
        (int)(seconds*1000.0 + 0.5) % 1000,
        mp4ASC.channelsConfiguration,
        mp4ASC.samplingFrequency,
        /* SBR: 0: off, 1: on; upsample, 2: on; downsampled, 3: off; upsampled */
        (frameInfo.sbr == 0) ? "off" : ((frameInfo.sbr == 1) ? "on, normal" : ((frameInfo.sbr == 2) ? "on, downsampled" : "off, upsampled")),
        (frameInfo.ps == 0) ? "no" : "yes");

    NeAACDecClose(hDecoder);
}

int getlength()
{
    if (!mp4state.filetype)
    {
        int track;
        long msDuration;
        long length;

        if ((track = GetAACTrack(mp4state.mp4file)) < 0)
        {
            return -1;
        }

        length = mp4ff_get_track_duration(mp4state.mp4file, track);

        msDuration = (long)(length*1000.0 / (float)mp4ff_time_scale(mp4state.mp4file, track) + 0.5);

        return msDuration;
    } else {
        return mp4state.m_length;
    }
    return 0;
}

/* new Media Library interface */

int mp4_get_metadata(mp4ff_t *file, const char *item, char *dest, int dlen)
{
    char *pVal = NULL, dummy1[4096];

    if (dlen < 1) return 0;

    if (!stricmp(item, "track") || !stricmp(item, "tracknumber"))
    {
        if (mp4ff_meta_get_track(file, &pVal))
        {
            wsprintf(dummy1, "%s", pVal);
            strncpy(dest, dummy1, dlen-1);
            dest[dlen-1] = '\0';
            return 1;
        }
    }
    else if (!stricmp(item, "disc") || !stricmp(item, "disknumber"))
    {
        if (mp4ff_meta_get_disc(file, &pVal))
        {
            wsprintf(dummy1, "%s", pVal);
            strncpy(dest, dummy1, dlen-1);
            dest[dlen-1] = '\0';
            return 1;
        }
    }
    else if (!stricmp(item, "compilation"))
    {
        uint8_t cpil = 0;
        if (mp4ff_meta_get_compilation(file, &pVal))
        {
            wsprintf(dummy1, "%s", pVal);
            strncpy(dest, dummy1, dlen-1);
            dest[dlen-1] = '\0';
            return 1;
        }
    }
    else if (!stricmp(item, "tempo"))
    {
        if (mp4ff_meta_get_tempo(file, &pVal))
        {
            wsprintf(dummy1, "%s", pVal);
            strncpy(dest, dummy1, dlen-1);
            dest[dlen-1] = '\0';
            return 1;
        }
    }
    else if (!stricmp(item, "artist"))
    {
        if (mp4ff_meta_get_artist(file, &pVal))
        {
            strncpy(dest, pVal, dlen-1);
            dest[dlen-1] = '\0';
            return 1;
        }
    }
    else if (!stricmp(item, "writer"))
    {
        if (mp4ff_meta_get_writer(file, &pVal))
        {
            strncpy(dest, pVal, dlen-1);
            dest[dlen-1] = '\0';
            return 1;
        }
    }
    else if (!stricmp(item, "title"))
    {
        if (mp4ff_meta_get_title(file, &pVal))
        {
            strncpy(dest, pVal, dlen-1);
            dest[dlen-1] = '\0';
            return 1;
        }
    }
    else if (!stricmp(item, "album"))
    {
        if (mp4ff_meta_get_album(file, &pVal))
        {
            strncpy(dest, pVal, dlen-1);
            dest[dlen-1] = '\0';
            return 1;
        }
    }
    else if (!stricmp(item, "date") || !stricmp(item, "year"))
    {
        if (mp4ff_meta_get_date(file, &pVal))
        {
            strncpy(dest, pVal, dlen-1);
            dest[dlen-1] = '\0';
            return 1;
        }
    }
    else if (!stricmp(item, "comment"))
    {
        if (mp4ff_meta_get_comment(file, &pVal))
        {
            strncpy(dest, pVal, dlen-1);
            dest[dlen-1] = '\0';
            return 1;
        }
    }
    else if (!stricmp(item, "genre"))
    {
        if (mp4ff_meta_get_genre(file, &pVal))
        {
            strncpy(dest, pVal, dlen-1);
            dest[dlen-1] = '\0';
            return 1;
        }
    }
    else if (!stricmp(item, "tool"))
    {
        if (mp4ff_meta_get_tool(file, &pVal))
        {
            strncpy(dest, pVal, dlen-1);
            dest[dlen-1] = '\0';
            return 1;
        }
    }
#if 0
    else
    {
        uint32_t valueSize = 0;
        uint8_t *pValue = NULL;

        if (MP4GetMetadataFreeForm(file, (char *)item, &pValue, &valueSize))
        {
            unsigned int len = (valueSize < (unsigned int)(dlen-1)) ? valueSize : (unsigned int)(dlen-1);
            memcpy(dest, pValue, len);
            dest[len] = '\0';
            return 1;
        }
    }
#endif

    return 0;
}

__declspec(dllexport) int winampGetExtendedFileInfo(const char *fn, const char *data, char *dest, int destlen)
{
    if (!fn || (fn && !*fn) || !destlen) return 0;

    dest[0] = '\0';

    if (!stricmp(data, "length"))
    {
        char temp[32];
        int len = getsonglength(fn);
        itoa(len, temp, 10);
        strncpy(dest, temp, destlen-1);
        dest[destlen-1] = '\0';
    }
    else
    {
        char temp[2048], temp2[2048];
        FILE *mp4File;
        mp4ff_callback_t mp4cb = {0};
        mp4ff_t *file;

        mp4File = fopen(fn, "rb");
        mp4cb.read = read_callback;
        mp4cb.seek = seek_callback;
        mp4cb.write = write_callback;
        mp4cb.truncate = truncate_callback;
        mp4cb.user_data = mp4File;


        file = mp4ff_open_read(&mp4cb);
        if (file == NULL) return 0;

        if (mp4_get_metadata(file, data, temp, sizeof(temp)))
        {
            int len = ConvertUTF8ToANSI(temp, temp2);
            if (len > destlen-1) len = destlen-1;
            memcpy(dest, temp2, len);
            dest[len] = '\0';
        }

        mp4ff_close(file);
        fclose(mp4File);
    }

    return 1;
}

static mp4ff_metadata_t mltags = {0, 0};
static BOOL medialib_init = FALSE;
static char medialib_lastfn[2048] = "";

__declspec(dllexport) int winampSetExtendedFileInfo(const char *fn, const char *data, char *val)
{
    int len, ret = 0;
    char *temp;

    if (!medialib_init || (medialib_init && stricmp(fn, medialib_lastfn))) {
        FILE *mp4File;
        mp4ff_callback_t mp4cb = {0};
        mp4ff_t *file;
        strcpy(medialib_lastfn, fn);

        if (medialib_init) tag_delete(&mltags);

        mp4File = fopen(medialib_lastfn, "rb");
        mp4cb.read = read_callback;
        mp4cb.seek = seek_callback;
        mp4cb.user_data = mp4File;


        file = mp4ff_open_read(&mp4cb);
        if (file == NULL) return 0;

        ReadMP4Tag(file, &mltags);

        mp4ff_close(file);
        fclose(mp4File);

        medialib_init = TRUE;
    }

    len = strlen(val);
    temp = (char *)malloc((len+1)*4);
    if (!temp) return 0;

    if (ConvertANSIToUTF8(val, temp))
    {
        ret = 1;
        tag_set_field(&mltags, data, temp, len);
    }

    free(temp);

    return ret;
}    

__declspec(dllexport) int winampWriteExtendedFileInfo()
{
    if (medialib_init)
    {
        FILE *mp4File;
        mp4ff_callback_t mp4cb = {0};

        mp4File = fopen(medialib_lastfn, "rb+");
        mp4cb.read = read_callback;
        mp4cb.seek = seek_callback;
        mp4cb.write = write_callback;
        mp4cb.truncate = truncate_callback;
        mp4cb.user_data = mp4File;

        mp4ff_meta_update(&mp4cb, &mltags);

        fclose(mp4File);

        return 1;
    }
    else
    {
        return 0;
    }
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMP4Tag::CMP4Tag()
{

}

CMP4Tag::~CMP4Tag()
{

}
