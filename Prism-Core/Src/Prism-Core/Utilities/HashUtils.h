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

template<HashSize Size>
using HashStorageType = std::conditional_t<
	Size == HashSize::Bit32,
	uint32_t, std::conditional_t<
		Size == HashSize::Bit64,
		uint64_t, HashSize128Storage>>;

template<HashSize Size>
struct Hash
{
	using Canonical = std::array<uint8_t, sizeof(HashStorageType<Size>)>;

	Hash() = default;
	Hash(const void* data, size_t size);
	explicit Hash(HashStorageType<Size> hash);
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
	HashStorageType<Size> hashValue = {};
};

template Hash<HashSize::Bit32>;
template Hash<HashSize::Bit64>;
template Hash<HashSize::Bit128>;
}
