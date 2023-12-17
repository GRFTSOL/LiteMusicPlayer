#pragma once


#include <thread>
#include <mutex>
#include <condition_variable>


class MediaScanner {
public:
    MediaScanner();
    virtual ~MediaScanner();

    void scanMedia(const MediaPtr &media);

    void quit();

protected:
    void threadRun();

protected:
    std::mutex                  _mutex;
    std::condition_variable     _cv;
    std::thread                 _thread;
    VecMediaPtrs                _medias;
    volatile bool               _quit = false;

};

extern MediaScanner g_mediaScanner;
