#include "pcpch.h"
#include "HashUtils.h"
#include "xxhash.h"

namespace Prism
{
template<HashSize Size>
Hash<Size>::Hash(const void* data, size_t size)
{
	if constexpr (Size == HashSize::Bit32)
	{
		hashValue = XXH32(data, size, 0);
	}
	else if constexpr (Size == HashSize::Bit64)
	{
		hashValue = XXH3_64bits(data, size);
	}
	else if constexpr (Size == HashSize::Bit128)
	{
		auto [low, high] = XXH3_128bits(data, size);
		hashValue.low = low;
		hashValue.high = high;
	}
	else
	{
		static_assert(true);
	}
}

template<HashSize Size>
Hash<Size>::Hash(typename HashInternal::HashStorage<Size>::Type hash)
	: hashValue(hash)
{
}

template<HashSize Size>
Hash<Size>::Hash(Canonical canonicalHash)
{
	if constexpr (Size == HashSize::Bit32)
	{
		hashValue = XXH32_hashFromCanonical(reinterpret_cast<XXH32_canonical_t*>(canonicalHash.data()));
	}
	else if constexpr (Size == HashSize::Bit64)
	{
		hashValue = XXH64_hashFromCanonical(reinterpret_cast<XXH64_canonical_t*>(canonicalHash.data()));
	}
	else if constexpr (Size == HashSize::Bit128)
	{
		auto [low, high] = XXH128_hashFromCanonical(reinterpret_cast<XXH128_canonical_t*>(canonicalHash.data()));
		hashValue.low = low;
		hashValue.high = high;
	}
	else
	{
		static_assert(true);
	}
}

template<HashSize Size>
typename Hash<Size>::Canonical Hash<Size>::GetCanonicalHash() const
{
	if constexpr (Size == HashSize::Bit32)
	{
		Hash<HashSize::Bit32>::Canonical canonical;
		XXH32_canonicalFromHash(reinterpret_cast<XXH32_canonical_t*>(canonical.data()), hashValue);

		return canonical;
	}
	else if constexpr (Size == HashSize::Bit64)
	{
		Hash<HashSize::Bit64>::Canonical canonical;
		XXH64_canonicalFromHash(reinterpret_cast<XXH64_canonical_t*>(canonical.data()), hashValue);

		return canonical;
	}
	else if constexpr (Size == HashSize::Bit128)
	{
		Hash<HashSize::Bit128>::Canonical canonical;
		XXH128_canonicalFromHash(reinterpret_cast<XXH128_canonical_t*>(canonical.data()), { .low64 = hashValue.low, .high64 = hashValue.high });

		return canonical;
	}
	else
	{
		static_assert(true);
		return {};
	}
}

template<HashSize Size>
bool Hash<Size>::operator==(const Hash& other) const
{
	return hashValue == other.hashValue;
}

bool HashSize128Storage::operator==(const HashSize128Storage& other) const
{
	return low == other.low && high == other.high;
}
}
