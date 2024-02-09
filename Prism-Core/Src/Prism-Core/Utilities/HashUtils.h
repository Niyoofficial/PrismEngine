#pragma once

namespace Prism
{
enum class HashSize
{
	Bit32,
	Bit64,
	Bit128
};

struct HashSize128Storage
{
	uint64_t low = 0;
	uint64_t high = 0;

	bool operator==(const HashSize128Storage& other) const;
};

	namespace HashInternal
	{
	template<HashSize>
	struct HashStorage
	{
	};

	template<>
	struct HashStorage<HashSize::Bit32>
	{
		using Type = uint32_t;
	};

	template<>
	struct HashStorage<HashSize::Bit64>
	{
		using Type = uint64_t;
	};

	template<>
	struct HashStorage<HashSize::Bit128>
	{
		using Type = HashSize128Storage;
	};
	}

template<HashSize Size>
struct Hash
{
	using Canonical = std::array<uint8_t, sizeof(typename HashInternal::HashStorage<Size>::Type)>;

	Hash() = default;
	Hash(const void* data, size_t size);
	explicit Hash(typename HashInternal::HashStorage<Size>::Type hash);
	explicit Hash(Canonical canonicalHash);
	template<typename T>
	explicit Hash(const T& object)
		: Hash(&object, sizeof(T)) {}

	// Returns a big endian hash representation.
	// When writing hash values to storage, sending them over a network, or printing
	// them, it's highly recommended to use the canonical representation to ensure
	// portability across a wider range of systems, present and future.
	Canonical GetCanonicalHash() const;

	bool operator==(const Hash& other) const;

public:
	typename HashInternal::HashStorage<Size>::Type hashValue = {};
};

template Hash<HashSize::Bit32>;
template Hash<HashSize::Bit64>;
template Hash<HashSize::Bit128>;
}
