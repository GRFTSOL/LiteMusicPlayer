//
//  FBuffer.cpp
//

#include "FBuffer.hpp"


void FBuffer::set(int bps, int channels, int sampleRate) {
    _bps = bps;
    _channels = channels;
    _sampleRate = sampleRate;
}
