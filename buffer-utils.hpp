#ifndef _KEYBOX_BUFFER_UTILS_HEADER_
#define _KEYBOX_BUFFER_UTILS_HEADER_
#include <boost/asio.hpp>

/*
template <typename BufferSequence>
size_t copy2RawBuffer(const BufferSequence & seq, uint8_t * out, size_t outlen );
*/

template<typename BufferSequence>
size_t copy2RawBuffer(const BufferSequence & seq, uint8_t * out, size_t outlen )
{
    int l = 0;
    int maxlen;
    auto x = boost::asio::buffer_sequence_begin(seq);
    while( x < boost::asio::buffer_sequence_end(seq) && l < outlen) {
        maxlen = outlen - l;
        if( x->size() < maxlen) {
            maxlen = x->size();
        }
        memcpy(out+l, x->data(), maxlen);
        x++;
        l += maxlen;
    }
    return l;
}

#endif

