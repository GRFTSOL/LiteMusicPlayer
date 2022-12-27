#include "LyricsData.h"
#include "MediaTags.h"
#include "MidiTag.h"


using namespace MidiTag;


/* This file contains the classes for parsing and modifying
 * MIDI music files.

 http://www.sonicspot.com/guide/midifiles.html
 http://cs.fit.edu/~ryan/cse4051/projects/midi/midi.html
 http://www.lastrayofhope.com/2009/12/23/midi-delta-time-ticks-to-seconds/2/

 */

/* MIDI file format.
 *
 * The Midi File format is described below.  The description uses
 * the following abbreviations.
 *
 * u1     - One uint8_t
 * u2     - Two bytes (big endian)
 * u4     - Four bytes (big endian)
 * varlen - A variable length integer, that can be 1 to 4 bytes. The 
 *          integer ends when you encounter a uint8_t that doesn't have 
 *          the 8th bit set (a uint8_t less than 0x80).
 * len?   - The length of the data depends on some code
 *          
 *
 * The Midi files begins with the main Midi header
 * u4 = The four ascii characters 'MThd'
 * u4 = The length of the MThd header = 6 bytes
 * u2 = 0 if the file contains a single track
 *      1 if the file contains one or more simultaneous tracks
 *      2 if the file contains one or more independent tracks
 * u2 = number of tracks
 * u2 = if >  0, the number of pulses per quarter note
 *      if <= 0, then ???
 *
 * next come the individual Midi tracks.  The total number of Midi
 * tracks was given above, in the MThd header.  Each track starts
 * with a header:
 *
 * u4 = The four ascii characters 'MTrk'
 * u4 = Amount of track data, in bytes.
 * 
 * The track data consists of a series of Midi events.  Each Midi event
 * has the following format:
 *
 * varlen  - The time between the previous event and this event, measured
 *           in "pulses".  The number of pulses per quarter note is given
 *           in the MThd header.
 * u1      - The Event code, always betwee 0x80 and 0xFF
 * len?    - The event data.  The length of this data is determined by the
 *           event code.  The first uint8_t of the event data is always < 0x80.
 *
 * The event code is optional.  If the event code is missing, then it
 * defaults to the previous event code.  For example:
 *
 *   varlen, eventcode1, eventdata,
 *   varlen, eventcode2, eventdata,
 *   varlen, eventdata,  // eventcode is eventcode2
 *   varlen, eventdata,  // eventcode is eventcode2
 *   varlen, eventcode3, eventdata,
 *   ....
 *
 *   How do you know if the eventcode is there or missing? Well:
 *   - All event codes are between 0x80 and 0xFF
 *   - The first uint8_t of eventdata is always less than 0x80.
 *   So, after the varlen delta time, if the next uint8_t is between 0x80
 *   and 0xFF, its an event code.  Otherwise, its event data.
 *
 * The Event codes and event data for each event code are shown below.
 *
 * Code:  u1 - 0x80 thru 0x8F - Note Off event.
 *             0x80 is for channel 1, 0x8F is for channel 16.
 * Data:  u1 - The note number, 0-127.  Middle C is 60 (0x3C)
 *        u1 - The note velocity.  This should be 0
 * 
 * Code:  u1 - 0x90 thru 0x9F - Note On event.
 *             0x90 is for channel 1, 0x9F is for channel 16.
 * Data:  u1 - The note number, 0-127.  Middle C is 60 (0x3C)
 *        u1 - The note velocity, from 0 (no sound) to 127 (loud).
 *             A value of 0 is equivalent to a Note Off.
 *
 * Code:  u1 - 0xA0 thru 0xAF - Key Pressure
 * Data:  u1 - The note number, 0-127.
 *        u1 - The pressure.
 *
 * Code:  u1 - 0xB0 thru 0xBF - Control Change
 * Data:  u1 - The controller number
 *        u1 - The value
 *
 * Code:  u1 - 0xC0 thru 0xCF - Program Change
 * Data:  u1 - The program number.
 *
 * Code:  u1 - 0xD0 thru 0xDF - Channel Pressure
 *        u1 - The pressure.
 *
 * Code:  u1 - 0xE0 thru 0xEF - Pitch Bend
 * Data:  u2 - Some data
 *
 * Code:  u1     - 0xFF - Meta Event
 * Data:  u1     - Metacode
 *        varlen - length of meta event
 *        u1[varlen] - Meta event data.
 *
 *
 * The Meta Event codes are listed below:
 *
 * Metacode: u1         - 0x0  Sequence Number
 *           varlen     - 0 or 2
 *           u1[varlen] - Sequence number
 *
 * Metacode: u1         - 0x1  Text
 *           varlen     - length of text
 *           u1[varlen] - Text
 *
 * Metacode: u1         - 0x2  Copyright
 *           varlen     - length of text
 *           u1[varlen] - Text
 *
 * Metacode: u1         - 0x3  Track Name
 *           varlen     - length of name
 *           u1[varlen] - Track Name
 *
 * Metacode: u1         - 0x58  Time Signature
 *           varlen     - 4 
 *           u1         - numerator
 *           u1         - log2(denominator)
 *           u1         - clocks in metronome click
 *           u1         - 32nd notes in quarter note (usually 8)
 *
 * Metacode: u1         - 0x59  Key Signature
 *           varlen     - 2
 *           u1         - if >= 0, then number of sharps
 *                        if < 0, then number of flats * -1
 *           u1         - 0 if major key
 *                        1 if minor key
 *
 * Metacode: u1         - 0x51  Tempo
 *           varlen     - 3  
 *           u3         - quarter note length in microseconds
 */

/* KAR trunk fields

Origin: The Company Tune 1000 
A file kar (kar) is in fact, a midi file, but whose words are standardized in events meta of the type TEXT. 
The texts starting with @ are additional indications compared to the words. 
Example:

@L specifies the language of the words 
@I any additional information 
@T information of title 
@KMIDI KARAOKE SPINS information of copyright and type of file

Several lines of titles and information can be present. KaraWin extracts information from titles to post them. 
The text even has a very simple format to him: 
\ indicates a page break, 
/a return indicates has the line

*/


/** read a variable-length integer (1 to 4 bytes). The integer ends
 * when you encounter a uint8_t that doesn't have the 8th bit set
 * (a uint8_t less than 0x80).
 */
int readVarlen(CFileEx &file, int &len) {
    uint8_t b;
    len = 0;

    for (int i = 0; i < 4; i++) {
        int nRet = file.readByte(b);
        if (nRet != ERR_OK) {
            return nRet;
        }

        len = (len << 7) | (b & 0x7f);
        if ((b & 0x80) == 0) {
            break;
        }
    }

    return ERR_OK;
}

CMidiTag::CMidiTag() {
    m_tempo = 0;
    m_dominator = 4;
    m_numberator = 4;
}

CMidiTag::~CMidiTag() {
    close();
}

int CMidiTag::open(cstr_t szFile) {
    close();

    int nRet = m_file.open(szFile, "rb");
    if (nRet != ERR_OK) {
        return nRet;
    }

    char header[4];
    nRet = m_file.readCount(header, CountOf(header));
    if (nRet != ERR_OK) {
        return nRet;
    }
    if (memcmp(header, MIDI_HEADER, CountOf(header)) != 0) {
        return ERR_NOT_SUPPORT_FILE_FORMAT;
    }

    uint32_t trunkLen;
    nRet = m_file.readUInt32BE(trunkLen);
    if (nRet != ERR_OK) {
        return nRet;
    }
    if (trunkLen != 6) {
        return ERR_NOT_SUPPORT_FILE_FORMAT;
    }

    uint16_t formatType, numbTracks;
    nRet = m_file.readUInt16BE(formatType);
    if (nRet != ERR_OK) {
        return nRet;
    }
    nRet = m_file.readUInt16BE(numbTracks);
    if (nRet != ERR_OK) {
        return nRet;
    }
    nRet = m_file.readUInt16BE(m_timeDivision);
    if (nRet != ERR_OK) {
        return nRet;
    }

    while (!m_file.isEOF()) {
        TrackTrunk trunk;
        trunk.offset = (uint32_t)m_file.getPos();

        nRet = m_file.readCount(header, CountOf(header));
        if (nRet != ERR_OK) {
            if (m_file.isEOF()) {
                break;
            }
            return nRet;
        }
        if (memcmp(header, TRACK_HEADER, CountOf(header)) != 0) {
            return ERR_NOT_SUPPORT_FILE_FORMAT;
        }

        nRet = m_file.readUInt32BE(trunk.sizeOfData);
        if (nRet != ERR_OK) {
            return nRet;
        }

        nRet = readTrackTrunk(trunk);
        if (nRet != ERR_OK) {
            return nRet;
        }

        m_listTrunks.push_back(trunk);

        m_file.seek(trunk.sizeOfData + TRUNK_HEADER_LEN + trunk.offset, SEEK_SET);
    }

    if (m_tempo == 0) {
        m_tempo = 500000; // 120 beats/minute
    }

    return ERR_OK;
}

void CMidiTag::close() {
    m_listTrunks.clear();
    m_file.close();
    m_tempo = 0;
    m_dominator = 4;
    m_numberator = 4;
}

int CMidiTag::listLyrics(VecStrings &vLyrNames) {
    if (hasLyrics()) {
        vLyrNames.push_back(SZ_SONG_KAR_LYRICS);
        return ERR_OK;
    }
    return ERR_NOT_FOUND;
}

int CMidiTag::getLyrics(CLyricsLines &lyricsLines) {
    TrackTrunk *pTrunk = FindLyricsTrunk();
    if (pTrunk == nullptr) {
        return ERR_NOT_FOUND;
    }

    m_file.seek(pTrunk->offset + TRUNK_HEADER_LEN, SEEK_SET);

    // DBG_LOG2("Trunk, offset: %d, len: %d", pTrunk->offset, pTrunk->sizeOfData);

    LyricsLine *line = nullptr;
    float startTime = 0;
    uint8_t eventCode = 0;
    while (m_file.getPos() < long(pTrunk->offset + pTrunk->sizeOfData)) {
        int deltaTime;
        int nRet = readVarlen(m_file, deltaTime);
        if (nRet != ERR_OK) {
            return nRet;
        }

        uint8_t newEventCode;
        nRet = m_file.readByte(newEventCode);
        if (nRet != ERR_OK) {
            return nRet;
        }
        if (newEventCode >= MET_NOTE_OFF) {
            eventCode = newEventCode;
        } else {
            m_file.seek(-1, SEEK_CUR);
        }

        if (eventCode >= MET_NOTE_OFF && eventCode <= MET_CONTROL_CHANGE) {
            // DBG_LOG1("  MidiEvent, code: 0x%x", eventCode);
            m_file.seek(2, SEEK_CUR);
        } else if (eventCode >= MET_PROGRAM_CHANGE && eventCode <= MET_PITCH_BEND) {
            if (eventCode == MET_PROGRAM_CHANGE) {
                startTime += (float)deltaTime * m_tempo / 1000 / m_numberator;
            }

            // DBG_LOG1("  MidiEvent, code: 0x%x", eventCode);
            m_file.seek(1, SEEK_CUR);
        } else if (eventCode >= MET_SYSEX1 && eventCode <= MET_SYSEX2) {
            // DBG_LOG1("  MidiEvent, code: 0x%x", eventCode);
            int len;
            nRet = readVarlen(m_file, len);
            if (nRet != ERR_OK) {
                return nRet;
            }
            m_file.seek(len, SEEK_CUR);
        } else if (eventCode == MET_META) {
            uint8_t metaEvent;
            nRet = m_file.readByte(metaEvent);
            if (nRet != ERR_OK) {
                return nRet;
            }

            int len;
            nRet = readVarlen(m_file, len);
            if (nRet != ERR_OK) {
                return nRet;
            }

            string buf;
            buf.resize(len);
            nRet = m_file.readCount((char *)buf.data(), len);
            if (nRet != ERR_OK) {
                return nRet;
            }

            if (metaEvent == MAT_END_OF_TRACK) {
                break;
            }

            string str;
            bool bNewLine = (buf[0] == '/' || buf[0] == '\\' || buf[0] == '\n');
            if (bNewLine) {
                str.assign(buf.c_str() + 1, buf.size() - 1);
            } else {
                str = buf;
            }

            if (metaEvent == MAT_TEMPO) {
                m_tempo = (uint8_t)buf[2] | ((uint8_t)buf[1] << 8) | ((uint8_t)buf[0] << 16);
                DBG_LOG1("  Tempo: %d", m_tempo);
            } else if (metaEvent == MAT_SEQUENCE_NAME) {
                // assert(strcmp(str.c_str(), "Words") == 0);
            } else if (metaEvent == MAT_TEXT) {
                startTime += (float)deltaTime * m_tempo / 1000 / m_timeDivision;
                if (buf[0] == '@') {
                    continue;
                }
                if (bNewLine || line == nullptr) {
                    line = newLyricsLine((int)startTime, TEMP_TIME);
                    lyricsLines.push_back(line);
                }
                line->appendPiece((int)startTime, TEMP_TIME, str.c_str(), str.size(), false, true);

                // DBG_LOG0(stringPrintf("Time division: %d, delta: %d, time: %d, %s", m_timeDivision, deltaTime, (int)startTime, str.c_str()).c_str());
            } else if (metaEvent == MAT_LYRIC) {
                if (bNewLine || line == nullptr) {
                    line = newLyricsLine((int)startTime, TEMP_TIME);
                    lyricsLines.push_back(line);
                }
                line->appendPiece((int)startTime, TEMP_TIME, str.c_str(), str.size(), false, true);
                // DBG_LOG0(stringPrintf("Time division: %d, delta: %d, time: %d, %s", m_timeDivision, deltaTime, (int)startTime, str.c_str()).c_str());
                startTime += (float)deltaTime * m_tempo / 1000 / m_numberator;
            }
        }
    }

    return ERR_OK;
}

bool CMidiTag::hasLyrics() {
    TrackTrunk *pTrunk = FindLyricsTrunk();
    return pTrunk != nullptr;
}

int CMidiTag::readTrackTrunk(TrackTrunk &trunk) {
    m_file.seek(trunk.offset + TRUNK_HEADER_LEN, SEEK_SET);

    // DBG_LOG2("Trunk, offset: %d, len: %d", trunk.offset, trunk.sizeOfData);

    string buf;
    uint8_t eventCode = 0;
    while (m_file.getPos() < long(trunk.offset + trunk.sizeOfData)) {
        int deltaTime;
        int nRet = readVarlen(m_file, deltaTime);
        if (nRet != ERR_OK) {
            return nRet;
        }

        uint8_t newEventCode;
        nRet = m_file.readByte(newEventCode);
        if (nRet != ERR_OK) {
            return nRet;
        }
        if (newEventCode >= MET_NOTE_OFF) {
            eventCode = newEventCode;
        } else {
            m_file.seek(-1, SEEK_CUR);
        }

        if (eventCode >= MET_NOTE_OFF && eventCode <= MET_CONTROL_CHANGE) {
            // DBG_LOG2("  MidiEvent, code: 0x%x, delta: %d", eventCode, deltaTime);
            m_file.seek(2, SEEK_CUR);
        } else if (eventCode >= MET_PROGRAM_CHANGE && eventCode <= MET_PITCH_BEND) {
            // DBG_LOG2("  MidiEvent, code: 0x%x, delta: %d", eventCode, deltaTime);
            m_file.seek(1, SEEK_CUR);
        } else if (eventCode >= MET_SYSEX1 && eventCode <= MET_SYSEX2) {
            // DBG_LOG2("  MidiEvent, code: 0x%x, delta: %d", eventCode, deltaTime);
            int len;
            nRet = readVarlen(m_file, len);
            if (nRet != ERR_OK) {
                return nRet;
            }
            m_file.seek(len, SEEK_CUR);
        } else if (eventCode == MET_META) {
            uint8_t metaEvent;
            nRet = m_file.readByte(metaEvent);
            if (nRet != ERR_OK) {
                return nRet;
            }

            int len;
            nRet = readVarlen(m_file, len);
            if (nRet != ERR_OK) {
                return nRet;
            }
            // m_file.seek(len, SEEK_CUR);
            buf.resize(len);
            nRet = m_file.readCount((uint8_t *)buf.data(), len);
            if (nRet != ERR_OK) {
                return nRet;
            }

            if (metaEvent == MAT_END_OF_TRACK) {
                break;
            }

            if (metaEvent == MAT_TIMESIGNATURE) {
                m_numberator = buf[0];
                m_dominator = buf[1];
                DBG_LOG2("  Time Signature: %d, %d", m_numberator, m_dominator);
            } else if (metaEvent == MAT_TEMPO && m_tempo == 0) {
                m_tempo = buf[2] | (buf[1] << 8) | (buf[0] << 16);
                DBG_LOG1("  Tempo: %d", m_tempo);
            }

            // DBG_LOG0(stringPrintf("  Meta, metaType: 0x%x, deltatime: %d, len: %d, %s", metaEvent, deltaTime, len, buf.c_str()).c_str());
        }
    }

    return ERR_OK;
}

TrackTrunk *CMidiTag::FindLyricsTrunk() {
    for (ListTrackTrunk::iterator it = m_listTrunks.begin(); it != m_listTrunks.end(); ++it) {
        TrackTrunk &trunk = *it;

        m_file.seek(trunk.offset + TRUNK_HEADER_LEN, SEEK_SET);

        for (int i = 0; i < 20 && m_file.getPos() < long(trunk.offset + trunk.sizeOfData); i++) {
            int deltaTime;
            int nRet = readVarlen(m_file, deltaTime);
            if (nRet != ERR_OK) {
                break;
            }
            if (deltaTime != 0) {
                continue;
            }

            uint8_t eventCode = 0;
            nRet = m_file.readByte(eventCode);
            if (nRet != ERR_OK) {
                break;
            }
            if (eventCode != MET_META) {
                continue;
            }

            uint8_t metaEvent;
            nRet = m_file.readByte(metaEvent);
            if (nRet != ERR_OK) {
                return nullptr;
            }

            if (metaEvent == MAT_LYRIC) {
                return &(*it);
            }

            int len;
            nRet = readVarlen(m_file, len);
            if (nRet != ERR_OK) {
                return nullptr;
            }

            string buf;
            buf.resize(len);
            nRet = m_file.readCount((uint8_t *)buf.data(), len);
            if (nRet != ERR_OK) {
                return nullptr;
            }

            if (strcmp(buf.c_str(), "Words") == 0) {
                return &(*it);
            }
        }
    }

    return nullptr;
}
