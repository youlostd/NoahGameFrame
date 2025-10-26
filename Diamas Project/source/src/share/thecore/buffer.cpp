/*
 *    Filename: buffer.c
 * Description: Buffer Processing-Modul
 *
 *      Author: Gimhanju (aka. Biyeop, Cronan)
 */
#define __LIBTHECORE__
#include "stdafx.h"
#include <algorithm>
#include <cstring>
#include "spdlog/spdlog.h"


// internal function forward
void buffer_realloc(LPBUFFER& buffer, int length);

LPBUFFER buffer_new(int size)
{
	if (size < 0)
		return NULL;

	buffer* buf = new buffer;

	buf->mem_size = size;
	buf->mem_data = new char[size];

	buffer_reset(buf);
	return buf;
}

void buffer_delete(LPBUFFER buf)
{
	if (buf == NULL)
		return;

	delete[] buf->mem_data;
	delete buf;
}

uint32_t buffer_size(LPBUFFER buf)
{
	return buf->length;
}

void buffer_reset(LPBUFFER buf)
{
	buf->read_point = buf->mem_data;
	buf->write_point = buf->mem_data;
	buf->write_point_pos = 0;
	buf->length = 0;
}

void buffer_write(LPBUFFER& buf, const void* src, int length)
{
	if (!buf)
		return;

	if (buf->write_point_pos + length >= buf->mem_size)
		buffer_realloc(buf,
		               buf->mem_size + length + std::max<int>(1024, length));

	memcpy(buf->write_point, src, length);
	buffer_write_proceed(buf, length);
}

void buffer_read(LPBUFFER buf, void* data, int bytes)
{
	memcpy(data, buf->read_point, bytes);
	buffer_read_proceed(buf, bytes);
}

uint8_t buffer_byte(LPBUFFER buf)
{
	uint8_t val = *(uint8_t*)buf->read_point;
	buffer_read_proceed(buf, sizeof(uint8_t));
	return val;
}

uint16_t buffer_word(LPBUFFER buf)
{
	uint16_t val = *(uint16_t*)buf->read_point;
	buffer_read_proceed(buf, sizeof(uint16_t));
	return val;
}

uint32_t buffer_dword(LPBUFFER buf)
{
	uint32_t val = *(uint32_t*)buf->read_point;
	buffer_read_proceed(buf, sizeof(uint32_t));
	return val;
}

const void* buffer_read_peek(LPBUFFER buf)
{
	return (const void*)buf->read_point;
}

void buffer_read_proceed(LPBUFFER buf, int length)
{
	if (length == 0)
		return;

	if (length < 0) {
		spdlog::error(
		          "buffer_proceed: length argument lower than zero (length: "
		          "{0})",
		          length);

		return;
	} else if (length > buf->length) {
		spdlog::error(
		          "buffer_proceed: length argument bigger than buffer (length: "
		          "{0}, buffer: {1})",
		          length, buf->length);

		length = buf->length;
	}

	// 처리할 길이가 버퍼 길이보다 작다면, 버퍼를 남겨두어야 한다.
	if (length < buf->length) {
		// write_point 와 pos 는 그대로 두고 read_point 만 증가 시킨다.
		STORM_ASSERT(buf->read_point + length - buf->mem_data <= buf->mem_size,
		             "Buffer overflow");

		buf->read_point += length;
		buf->length -= length;
	} else {
		buffer_reset(buf);
	}
}

void* buffer_write_peek(LPBUFFER buf)
{
	return buf->write_point;
}

void buffer_write_proceed(LPBUFFER buf, int length)
{
	buf->length += length;
	buf->write_point += length;
	buf->write_point_pos += length;
}

int buffer_has_space(LPBUFFER buf)
{
	return buf->mem_size - buf->write_point_pos;
}

void buffer_adjust_size(LPBUFFER& buf, int add_size)
{
	if (buf->mem_size >= buf->write_point_pos + add_size)
		return;

	SPDLOG_INFO( "buffer_adjust {0} current {1}/{2}", add_size, buf->length,
	           buf->mem_size);

	buffer_realloc(buf, buf->mem_size + add_size);
}

void buffer_realloc(LPBUFFER& buf, int length)
{
	int i, read_point_pos;
	LPBUFFER temp;

	STORM_ASSERT(length >= 0, "buffer_realloc: length is lower than zero");

	if (buf->mem_size >= length)
		return;

	// i 는 새로 할당된 크기와 이전크기의 차, 실제로 새로 생긴
	// 메모리의 크기를 뜻한다.
	i = length - buf->mem_size;

	if (i <= 0)
		return;

	temp = buffer_new(length);

	SPDLOG_INFO( "reallocating buffer to {0}, current {1}", temp->mem_size,
	          buf->mem_size);

	memmove(temp->mem_data, buf->mem_data, buf->mem_size);

	read_point_pos = buf->read_point - buf->mem_data;

	// write_point 와 read_point 를 재 연결 시킨다.
	temp->write_point = temp->mem_data + buf->write_point_pos;
	temp->write_point_pos = buf->write_point_pos;
	temp->read_point = temp->mem_data + read_point_pos;
	temp->length = buf->length;

	buffer_delete(buf);
	buf = temp;
}