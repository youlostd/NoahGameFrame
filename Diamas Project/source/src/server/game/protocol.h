#ifndef METIN2_SERVER_GAME_PROTOCOL_H
#define METIN2_SERVER_GAME_PROTOCOL_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


inline uint8_t decode_byte(const void * a)
{
	return (*(uint8_t *) a);
}

inline uint16_t decode_2bytes(const void * a)
{
	return (*((uint16_t *) a));
}

inline uint32_t decode_4bytes(const void *a)
{
	return (*((uint32_t *) a));
}

inline uint64_t decode_8bytes(const void *a)
{
	return (*((uint64_t *)a));
}


#endif /* METIN2_SERVER_GAME_PROTOCOL_H */
