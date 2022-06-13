#ifndef HVH_WC_TOOLS_BITFIELD_H
#define HVH_WC_TOOLS_BITFIELD_H

#include <cstdint>
#include <cmath>

constexpr const int BITFIELD_DATA_SIZE = 32;

template <int BITS>
class Bitfield
{
public:
	Bitfield() { clear(); }
	Bitfield(const Bitfield&) = default;

	inline void clear()
		{ memset(this, 0, sizeof(*this)); }

	inline void set_bit(int which_bit)
	{
		if (which_bit >= BITS) return;
		storage.data[which_bit / BITFIELD_DATA_SIZE] |= (1 << (which_bit % BITFIELD_DATA_SIZE));
	}

	inline void flip_bit(int which_bit)
	{
		if (which_bit >= BITS) return;
		storage.data[which_bit / BITFIELD_DATA_SIZE] ^= (1 << (which_bit % BITFIELD_DATA_SIZE));
	}

	inline bool is_bit_set(int which_bit)
	{
		if (which_bit >= BITS) return false;
		return (storage.data[which_bit / BITFIELD_DATA_SIZE] & (1 << (which_bit % BITFIELD_DATA_SIZE))) != 0;
	}

private:

	template <int N>
	struct TightStorage
	{
		uint32_t data[(N / BITFIELD_DATA_SIZE)];
	};

	template <int N>
	struct LooseStorage
	{
		uint32_t data[(N / BITFIELD_DATA_SIZE) + 1];
	};

	typename std::conditional<(BITS % BITFIELD_DATA_SIZE) == 0, TightStorage<BITS>, LooseStorage<BITS>>::type storage;
};

#endif // HVH_WC_TOOLS_BITFIELD_H