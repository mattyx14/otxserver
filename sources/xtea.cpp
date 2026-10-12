////////////////////////////////////////////////////////////////////////
// XTEA encryption of outgoing packets (see xtea.h)
////////////////////////////////////////////////////////////////////////
// no otpch.h: this file can also be compiled on its own (e.g. to test it against xteaEncryptPlain)
#include "xtea.h"

#include <cstring>
#include <emmintrin.h>
#include <immintrin.h>

#if defined(_MSC_VER)
	#include <intrin.h>
	#define XTEA_TARGET_AVX2
#else
	#define XTEA_TARGET_AVX2 __attribute__((target("avx2")))
#endif

static const uint32_t delta = 0x61C88647;

// the per-round key/sum values are the same for every block
static void precacheControlSums(const uint32_t key[4], uint32_t sums[32][2])
{
	uint32_t sum = 0;
	for(int32_t i = 0; i < 32; ++i)
	{
		sums[i][0] = sum + key[sum & 3];
		sum -= delta;
		sums[i][1] = sum + key[(sum >> 11) & 3];
	}
}

static void encryptBlocks(uint8_t* buffer, size_t length, const uint32_t sums[32][2])
{
	for(size_t pos = 0; pos < length; pos += 8)
	{
		uint32_t v[2];
		memcpy(v, buffer + pos, 8);
		for(int32_t i = 0; i < 32; ++i)
		{
			v[0] += ((v[1] << 4 ^ v[1] >> 5) + v[1]) ^ sums[i][0];
			v[1] += ((v[0] << 4 ^ v[0] >> 5) + v[0]) ^ sums[i][1];
		}

		memcpy(buffer + pos, v, 8);
	}
}

void xteaEncryptPlain(uint8_t* buffer, size_t length, const uint32_t key[4])
{
	uint32_t sums[32][2];
	precacheControlSums(key, sums);
	encryptBlocks(buffer, length, sums);
}

// v += ((w << 4 ^ w >> 5) + w) ^ s, on 4 lanes
static inline __m128i xteaRoundSSE2(__m128i v, __m128i w, __m128i s)
{
	__m128i mixed = _mm_add_epi32(_mm_xor_si128(_mm_slli_epi32(w, 4), _mm_srli_epi32(w, 5)), w);
	return _mm_add_epi32(v, _mm_xor_si128(mixed, s));
}

void xteaEncryptSSE2(uint8_t* buffer, size_t length, const uint32_t key[4])
{
	uint32_t sums[32][2];
	precacheControlSums(key, sums);

	// 64 bytes (8 blocks) at a time: the first halves of 4 blocks in one register, the second halves in another
	size_t pos = 0;
	for(; pos + 64 <= length; pos += 64)
	{
		__m128i d0 = _mm_shuffle_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(buffer + pos)), _MM_SHUFFLE(3, 1, 2, 0));
		__m128i d1 = _mm_shuffle_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(buffer + pos + 16)), _MM_SHUFFLE(3, 1, 2, 0));
		__m128i d2 = _mm_shuffle_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(buffer + pos + 32)), _MM_SHUFFLE(3, 1, 2, 0));
		__m128i d3 = _mm_shuffle_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(buffer + pos + 48)), _MM_SHUFFLE(3, 1, 2, 0));

		__m128i a0 = _mm_unpacklo_epi64(d0, d1), b0 = _mm_unpackhi_epi64(d0, d1);
		__m128i a1 = _mm_unpacklo_epi64(d2, d3), b1 = _mm_unpackhi_epi64(d2, d3);
		for(int32_t i = 0; i < 32; ++i)
		{
			__m128i s0 = _mm_set1_epi32((int32_t)sums[i][0]), s1 = _mm_set1_epi32((int32_t)sums[i][1]);
			a0 = xteaRoundSSE2(a0, b0, s0);
			b0 = xteaRoundSSE2(b0, a0, s1);
			a1 = xteaRoundSSE2(a1, b1, s0);
			b1 = xteaRoundSSE2(b1, a1, s1);
		}

		_mm_storeu_si128(reinterpret_cast<__m128i*>(buffer + pos), _mm_unpacklo_epi32(a0, b0));
		_mm_storeu_si128(reinterpret_cast<__m128i*>(buffer + pos + 16), _mm_unpackhi_epi32(a0, b0));
		_mm_storeu_si128(reinterpret_cast<__m128i*>(buffer + pos + 32), _mm_unpacklo_epi32(a1, b1));
		_mm_storeu_si128(reinterpret_cast<__m128i*>(buffer + pos + 48), _mm_unpackhi_epi32(a1, b1));
	}

	encryptBlocks(buffer + pos, length - pos, sums);
}

XTEA_TARGET_AVX2 static inline __m256i xteaRoundAVX2(__m256i v, __m256i w, __m256i s)
{
	__m256i mixed = _mm256_add_epi32(_mm256_xor_si256(_mm256_slli_epi32(w, 4), _mm256_srli_epi32(w, 5)), w);
	return _mm256_add_epi32(v, _mm256_xor_si256(mixed, s));
}

XTEA_TARGET_AVX2 void xteaEncryptAVX2(uint8_t* buffer, size_t length, const uint32_t key[4])
{
	uint32_t sums[32][2];
	precacheControlSums(key, sums);

	// 128 bytes (16 blocks) at a time; the shuffles/unpacks work per 128-bit lane, like the SSE2 version
	size_t pos = 0;
	for(; pos + 128 <= length; pos += 128)
	{
		__m256i d0 = _mm256_shuffle_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(buffer + pos)), _MM_SHUFFLE(3, 1, 2, 0));
		__m256i d1 = _mm256_shuffle_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(buffer + pos + 32)), _MM_SHUFFLE(3, 1, 2, 0));
		__m256i d2 = _mm256_shuffle_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(buffer + pos + 64)), _MM_SHUFFLE(3, 1, 2, 0));
		__m256i d3 = _mm256_shuffle_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(buffer + pos + 96)), _MM_SHUFFLE(3, 1, 2, 0));

		__m256i a0 = _mm256_unpacklo_epi64(d0, d1), b0 = _mm256_unpackhi_epi64(d0, d1);
		__m256i a1 = _mm256_unpacklo_epi64(d2, d3), b1 = _mm256_unpackhi_epi64(d2, d3);
		for(int32_t i = 0; i < 32; ++i)
		{
			__m256i s0 = _mm256_set1_epi32((int32_t)sums[i][0]), s1 = _mm256_set1_epi32((int32_t)sums[i][1]);
			a0 = xteaRoundAVX2(a0, b0, s0);
			b0 = xteaRoundAVX2(b0, a0, s1);
			a1 = xteaRoundAVX2(a1, b1, s0);
			b1 = xteaRoundAVX2(b1, a1, s1);
		}

		_mm256_storeu_si256(reinterpret_cast<__m256i*>(buffer + pos), _mm256_unpacklo_epi32(a0, b0));
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(buffer + pos + 32), _mm256_unpackhi_epi32(a0, b0));
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(buffer + pos + 64), _mm256_unpacklo_epi32(a1, b1));
		_mm256_storeu_si256(reinterpret_cast<__m256i*>(buffer + pos + 96), _mm256_unpackhi_epi32(a1, b1));
	}

	_mm256_zeroupper(); // avoid AVX/SSE transition stalls in the code that follows
	encryptBlocks(buffer + pos, length - pos, sums);
}

static bool cpuHasAVX2()
{
#if defined(_MSC_VER)
	int32_t info[4];
	__cpuid(info, 0);
	if(info[0] < 7)
		return false;

	__cpuid(info, 1);
	bool osxsave = (info[2] & (1 << 27)) != 0, avx = (info[2] & (1 << 28)) != 0;
	if(!osxsave || !avx || (_xgetbv(0) & 6) != 6) // the OS must save the AVX registers
		return false;

	__cpuidex(info, 7, 0);
	return (info[1] & (1 << 5)) != 0;
#else
	__builtin_cpu_init();
	return __builtin_cpu_supports("avx2");
#endif
}

XteaImplementation getXteaImplementation()
{
	// x86-64 always has SSE2
	static const XteaImplementation implementation = cpuHasAVX2() ? XTEA_AVX2 : XTEA_SSE2;
	return implementation;
}

const char* getXteaImplementationName()
{
	switch(getXteaImplementation())
	{
		case XTEA_AVX2:
			return "AVX2";
		case XTEA_SSE2:
			return "SSE2";
		default:
			break;
	}

	return "plain";
}

void xteaEncrypt(uint8_t* buffer, size_t length, const uint32_t key[4])
{
	switch(getXteaImplementation())
	{
		case XTEA_AVX2:
			xteaEncryptAVX2(buffer, length, key);
			break;
		case XTEA_SSE2:
			xteaEncryptSSE2(buffer, length, key);
			break;
		default:
			xteaEncryptPlain(buffer, length, key);
			break;
	}
}
