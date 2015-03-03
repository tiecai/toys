/*
 * ringbuff.h
 * desc: 
 */

#ifndef _RINGBUFF_H
#define _RINGBUFF_H

#include <stdlib.h>

typedef  unsigned char   uchar;
typedef  unsigned short  ushort;
typedef  unsigned int    uint;

// 
template <typename T, uint BUFFER_SIZE = 1024>
class RingBuffer {
public:
	RingBuffer() : m_size(BUFFER_SIZE), m_num(0), m_head(0), m_tail(0) { memset(m_buff, 0, m_size * sizeof(T)); }
	~RingBuffer() {}

public:
	uint push(const T* buf, uint len) {
		if(!buf) { return 0; }
		if((len <= 0) || (len > (m_size - m_num))) { return 0; }
		for(uint i = 0; i < len; i++) {
			if(m_num < m_size) {
				m_buff[m_tail] = buf[i];
				if((++m_tail) >= m_size) { m_tail = 0; }
				m_num++;
			} else if(m_num == m_size) { return i; } else { return 0; }
		}
		return len;
	}

	uint pop(T* buf, uint len = 0) {
		if(!buf) { return 0; }
		if(len > m_num) { return 0; }
		if(len <= 0) { len = m_num; }
		for(uint i = 0; i < len; i++) {
			if(m_num > 0) {
				buf[i] = m_buff[m_head];
				if((++m_head) >= m_size) { m_head = 0; }
				m_num--;
			} else if(m_num == 0) { return i; } else { return 0; }
		}
		return len;
	}

	uint length() const { return m_num; }

private:
	T m_buff[BUFFER_SIZE];
	uint m_num;
	uint m_head;
	uint m_tail;
	const uint m_size;

private:
	RingBuffer(const RingBuffer&);
	RingBuffer& operator=(const RingBuffer&);
};

#endif // #ifndef _RINGBUFF_H
