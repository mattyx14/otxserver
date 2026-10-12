////////////////////////////////////////////////////////////////////////
// XTEA encryption of outgoing packets, with SSE2 and AVX2 versions.
// The fastest version the CPU supports is picked at runtime, so the binary
// does not need AVX2. All versions give the same bytes as xteaEncryptPlain.
////////////////////////////////////////////////////////////////////////

#ifndef __XTEA__
#define __XTEA__

#include <cstdint>
#include <cstddef>

// length must be a multiple of 8
void xteaEncryptPlain(uint8_t* buffer, size_t length, const uint32_t key[4]);
void xteaEncryptSSE2(uint8_t* buffer, size_t length, const uint32_t key[4]);
void xteaEncryptAVX2(uint8_t* buffer, size_t length, const uint32_t key[4]);

enum XteaImplementation
{
	XTEA_PLAIN,
	XTEA_SSE2,
	XTEA_AVX2
};

XteaImplementation getXteaImplementation(); // detected once
const char* getXteaImplementationName();
void xteaEncrypt(uint8_t* buffer, size_t length, const uint32_t key[4]);

#endif
