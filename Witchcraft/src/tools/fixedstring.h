#ifndef HVH_WC_TOOLS_FIXEDSTRING_H
#define HVH_WC_TOOLS_FIXEDSTRING_H

#include <cstdint>
#include <functional>
#include <ostream>

template <size_t strsize>
struct FixedString
{
	static_assert(strsize % 8 == 0, "Size of a FixedString must be a multiple of 8.");
	static_assert(strsize > 0, "Size of a FixedString must be greater than 0.");
	constexpr const static size_t datasize = strsize / 8;

	/* Default constructors so we can have real constructors. */
	FixedString() = default;
	FixedString(const FixedString<strsize>&) = default;
	FixedString(FixedString<strsize>&&) = default;

	/* Actual constructor so we can initialize the data. */
	FixedString(const char* arg)
	{
		strncpy(c_str, arg, strsize - 1);
		c_str[strsize - 1] = '\0';
	}

	FixedString& operator = (const FixedString<strsize>&) = default;
	FixedString& operator = (FixedString<strsize>&&) = default;

	/* FixedStrings are exactly 32 bytes (not neccesarily characters) long. */
	/* We use a union so we can access the raw memory and compare it as if it were 64-bit integers, */
	/* which makes comparing two fixed strings take a maximum of 4 comparisons, instead of 32. */
	union
	{
		char c_str[strsize];
		uint64_t raw[datasize];
	};

	/* Returns whether or not two fixed-strings are exactly equal to each other. */
	const bool operator == (const FixedString<strsize>& rhs) const
	{
		for (size_t i = 0; i < datasize; ++i)
		{
			if (raw[i] != rhs.raw[i])
				return false;
		}
		return true;
	}

	/* Returns whether one fixed-string is 'less than' the other.  This is not an exact lexicographic ordering! */
	const bool operator < (const FixedString<strsize>& rhs) const
	{
		for (size_t i = 0; i < datasize; ++i)
		{
			if (raw[i] < rhs.raw[i]) return true;
			else if (raw[i] > rhs.raw[i]) return false;
			// else if (raw[i] == rhs.raw[i]) continue;
		}

		// If we've reached this point, the two strings are exactly equal.
		return false;
	}
};

/* Here we define the hash function for our FixedString class, so it can be used as a key in unordered containers. */
namespace std {
	template<size_t strsize>
	struct hash<FixedString<strsize>>
	{
		size_t operator()(const FixedString<strsize>& x) const
		{
			uint64_t result = 0;
			for (size_t i = 0; i < (strsize / 8); ++i)
				{ result += x.raw[i]; }
			return (size_t)result;
		}
	};
}

template <size_t strsize>
std::ostream& operator<<(std::ostream& os, const FixedString<strsize>& rhs)
	{ return os << rhs.c_str; }

#endif