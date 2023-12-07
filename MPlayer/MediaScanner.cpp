#include "MPlayerApp.h"
#include "Player.h"
#include "MediaScanner.h"


MediaScanner g_mediaScanner;

MediaScanner::MediaScanner() {
}

MediaScanner::~MediaScanner() {
    quit();
}

void MediaScanner::scanMedia(const MediaPtr &media) {
    {
        std::lock_guard autolock(_mutex);
        _medias.push_back(media);
    }

    if (!_thread.joinable()) {
        _thread = std::thread(&MediaScanner::threadRun, this);
    }

    _cv.notify_one();
}

void MediaScanner::quit() {
    _quit = true;
    _cv.notify_one();
    if (_thread.joinable()) {
        _thread.join();
    }
}

void MediaScanner::threadRun() {
    while (!_quit) {
        VecMediaPtrs medias;

        {
            std::lock_guard autolock(_mutex);
            medias = _medias;
            _medias.clear();
        }

        if (medias.empty()) {
            std::unique_lock lock(_mutex);
            _cv.wait(lock);
            medias = _medias;
            _medias.clear();
        }

        for (auto &media : medias) {
            if (_quit) {
                break;
            }
            g_player.updateMediaInfo(media.get());
        }

        // Wait for some time
        Sleep(50);
    }
}

