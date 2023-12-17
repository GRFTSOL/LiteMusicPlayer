//
//  InputBuffer.hpp
//

#ifndef InputBuffer_hpp
#define InputBuffer_hpp

#include "IMPlayer.h"


class InputBuffer {
public:
    void bind(IMediaInput *input, size_t sizeBuf);
    bool fill() { return read(_buf.capacity()); }
    bool read(size_t size);
    void forward(int offset);
    void clear() { _pos = _end = 0; }

    StringView buf() const { return StringView(_buf.data() + _pos, int(_end - _pos)); }
    bool empty() const { return _pos >= _end; }
    uint8_t *data() const { return (uint8_t *)_buf.data() + _pos; }
    int size() const { return _end - _pos; }

    uint32_t filePosition(const uint8_t *p);

protected:
    std::vector<char>               _buf;
    int                             _pos = 0, _end = 0;
    IMediaInput                     *_input = nullptr;

};

#endif /* InputBuffer_hpp */
