//
//  FBuffer.hpp
//

#ifndef FBuffer_hpp
#define FBuffer_hpp

#include "IMPlayer.h"


class FBuffer : public IFBuffer {
public:
    void set(int bps, int channels, int sampleRate) override;

    uint8_t *data() override { return _buf.data(); }
    uint32_t size() override { return (uint32_t)_buf.size(); }
    uint32_t capacity() override { return (uint32_t)_buf.capacity(); }
    void resize(uint32_t size) override { return _buf.resize(size); }
    ResultCode reserve(uint32_t capacity) override { _buf.reserve(capacity); return ERR_OK; }

    int bps() override { return _bps; }
    int channels() override { return _channels; }
    int sampleRate() override { return _sampleRate; }

protected:
    int                     _bps = 0, _channels = 0, _sampleRate = 0;
    // uint8_t             *_buf = nullptr;
    std::vector<uint8_t>    _buf;

};

#endif /* FBuffer_hpp */
