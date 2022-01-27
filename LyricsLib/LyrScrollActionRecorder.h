#pragma once

class CLyrScrollActionRecorder
{
public:
    struct ScrollTo
    {
        int            nTime;
        int            distance;        // The character offset of lyrics.
        double        speed;

        ScrollTo() { }
        ScrollTo(int d, int t) {
            nTime = t;
            distance = d;
            speed = 0.0;
        }
    };

    enum {
        MAX_TIME                    = 0xFFFFFF
    };

    class ListScrollTo : public vector<ScrollTo>
    {
    public:

    };

public:
    CLyrScrollActionRecorder();

    static bool isEventTag(cstr_t str);

    void initCreateAutoEvents(CLyricsLines &lyrLines);

    void close();

    bool hasActions() { return m_bHasActions; }

    string eventsDataToString();
    void eventsDataFromString(cstr_t szEventsData, CLyricsLines &lyrLines);

    void scrollToLine(CLyricsLines &lyrLines, int nLine, int nTime, bool bUpdateTimeTag = true);

    void updateTimeTagByEvents(CLyricsLines &lyrLines);

protected:
    void addScrollTo(CLyricsLines &lyrLines, int nPos, int nTime, bool bUpdateTimeTag = true);

protected:
    int getDistanceOfLine(CLyricsLines &lyrLines, int nLine);

    int getDistanceOfCurrentPos(CLyricsLines &lyrLines);

    int countLyrChar(CLyricsLines &lyrLines);

    friend class CTestCaseLyrScrollActionRecorder;

protected:
    ListScrollTo            m_listEvents;
    int                        m_nLyrCharCount;
    bool                    m_bHasActions;

};
