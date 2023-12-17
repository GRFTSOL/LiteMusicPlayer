//
//  InputBuffer.cpp
//

#include "InputBuffer.hpp"


void InputBuffer::bind(IMediaInput *input, size_t sizeBuf) {
    _input = input;
    _buf.resize(sizeBuf);
}

bool InputBuffer::read(size_t size) {
    assert(_input);

    int toRead = int(size) - (_end - _pos);
    if (toRead > 0) {
        if (_buf.capacity() < size) {
            _buf.resize(size);
        }

        if (_pos + size > _buf.capacity()) {
            _end = _end - _pos;
            if (_end > 0) {
                memmove(_buf.data(), _buf.data() + _pos, _end);
            }
            _pos = 0;
        }

        int read = _input->read(_buf.data() + _end, toRead);
        _end += read;
        assert(_end <= _buf.size());

        return read > 0;
    }

    return true;
}

void InputBuffer::forward(int offset) {
    _pos += offset;
    if (_pos > _end) {
        _input->seek(_pos - _end, SEEK_CUR);
        _pos = _end = 0;
    }
}

uint32_t InputBuffer::filePosition(const uint8_t *p) {
    auto pos = uint32_t((cstr_t)p - _buf.data());

    assert(_input);
    assert(pos >= _pos && pos <= _end);

    return uint32_t(_input->getPos() - (_end - pos));
}
