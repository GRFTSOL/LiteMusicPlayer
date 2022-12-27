#pragma once

#include "LyricsData.h"


namespace MidiTag {

#define MIDI_HEADER         "MThd"
#define TRACK_HEADER        "MTrk"

#define TRUNK_HEADER_LEN    8

    struct FileHeader {
        uint8_t                     trunkId[4];
        uint8_t                     size;

    };

    struct TrackTrunk {
        uint32_t                    sizeOfData;
        uint32_t                    offset;
    };

    typedef list<TrackTrunk> ListTrackTrunk;

    struct MidiEvent {
        int    DeltaTime;           /** The time between the previous event and this on */
        int    StartTime;           /** The absolute time this event occurs */
        bool   HasEventflag;        /** False if this is using the previous eventflag */
        uint8_t   EventFlag;        /** NoteOn, NoteOff, etc.  Full list is in class MidiFile */
        uint8_t   Channel;          /** The channel this event occurs on */
    };

    typedef list<MidiEvent> ListMidiEvent;


    // Midi Events
    enum MidiEventType {
        MET_NOTE_OFF                = 0x80,
        MET_NOTE_ON                 = 0x90,
        MET_KEY_PRESSURE            = 0xA0,
        MET_CONTROL_CHANGE          = 0xB0,
        MET_PROGRAM_CHANGE          = 0xC0,
        MET_CHANNEL_PRESSURE        = 0xD0,
        MET_PITCH_BEND              = 0xE0,
        MET_SYSEX1                  = 0xF0,
        MET_SYSEX2                  = 0xF7,
        MET_META                    = 0xFF,
    };

    // The Meta Events Type
    enum MetaEventType {
        MAT_SEQUENCE                = 0x0,
        MAT_TEXT                    = 0x1,
        MAT_COPYRIGHT               = 0x2,
        MAT_SEQUENCE_NAME           = 0x3,
        MAT_INSTRUMENT              = 0x4,
        MAT_LYRIC                   = 0x5,
        MAT_MARKER                  = 0x6,
        MAT_END_OF_TRACK            = 0x2F,
        MAT_TEMPO                   = 0x51,
        MAT_SMPTEOFFSET             = 0x54,
        MAT_TIMESIGNATURE           = 0x58,
        MAT_KEYSIGNATURE            = 0x59,
    };

};

class CMidiTag {
public:
    CMidiTag();
    ~CMidiTag();

    int open(cstr_t szFile);
    void close();

    int listLyrics(VecStrings &vLyrNames);
    int getLyrics(CLyricsLines &lyricsLines);

    bool hasLyrics();

protected:
    int readTrackTrunk(MidiTag::TrackTrunk &trunk);

    int deltaTickToMs(int tick);

    MidiTag::TrackTrunk *FindLyricsTrunk();

protected:
    CFileEx                     m_file;
    MidiTag::ListTrackTrunk     m_listTrunks;
    uint16_t                    m_timeDivision;     // Ticks Per Quarter Note
    uint8_t                     m_numberator, m_dominator;
    int                         m_tempo;            // Microseconds Per Quarter-Note

};
