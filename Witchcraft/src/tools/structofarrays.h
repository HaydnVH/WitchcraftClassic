#ifndef HVH_WC_TOOLS_STRUCTOFARRAYS_H
#define HVH_WC_TOOLS_STRUCTOFARRAYS_H

#include <vector>
#include <functional>
#include <algorithm>


template <typename... Ts>
class StructOfArrays
{
public:

	StructOfArrays<>& operator = (const StructOfArrays<>& rhs) { return *this; }

	void push_back() {}
	void emplace_back() {}
	void insert(size_t) {}
	void erase_swap(size_t) {}
	void erase_shift(size_t) {}
	void clear() {}
	void resize(size_t) {}
	void pack_buffer(std::vector<char>&) {}
};

template <typename FirstType, typename... RestTypes>
class StructOfArrays<FirstType, RestTypes...>: StructOfArrays<RestTypes...>
{
	template <size_t, typename> struct elem_type_holder;

	template <typename T, typename... Ts>
	struct elem_type_holder<0, StructOfArrays<T, Ts...>>
	{
		typedef T type;
	};

	template <size_t K, typename T, typename... Ts>
	struct elem_type_holder<K, StructOfArrays<T, Ts...>>
	{
		typedef typename elem_type_holder<K-1, StructOfArrays<Ts...>>::type type;
	};


public:
	StructOfArrays()
	{}

	StructOfArrays<FirstType, RestTypes...>& operator = (const StructOfArrays<FirstType, RestTypes...>& rhs)
	{
		data = rhs.data;
		StructOfArrays<RestTypes...>& base_lhs = *this;
		const StructOfArrays<RestTypes...>& base_rhs = rhs;
		base_lhs = base_rhs;
		return *this;
	}

	template <size_t K>
	typename std::enable_if<K == 0, const std::vector<FirstType>&>::type
		get() const
	{
		return data;
	}

	template <size_t K>
	typename std::enable_if<K != 0, const std::vector<typename elem_type_holder<K, StructOfArrays<FirstType, RestTypes...>>::type>&>::type
		get() const
	{
		StructOfArrays<RestTypes...>& base = *this;
		return base.get<K - 1>();
	}

	template <size_t K>
	typename std::enable_if<K == 0, FirstType&>::type
		get(size_t index)
	{
		return data[index];
	}

	template <size_t K>
	typename std::enable_if<K != 0, typename elem_type_holder<K, StructOfArrays<FirstType, RestTypes...>>::type&>::type
		get(size_t index)
	{
		StructOfArrays<RestTypes...>& base = *this;
		return base.get<K - 1>(index);
	}

	template <size_t K>
	typename std::enable_if<K == 0, FirstType*>::type
		rawdata()
	{
		return data.data();
	}

	template <size_t K>
	typename std::enable_if<K != 0, typename elem_type_holder<K, StructOfArrays<FirstType, RestTypes...>>::type*>::type
		rawdata()
	{
		StructOfArrays<RestTypes...>& base = *this;
		return base.rawdata<K - 1>();
	}

	template <size_t K>
	typename std::enable_if<K == 0, FirstType&>::type
		back()
	{
		return data.back();
	}

	template <size_t K>
	typename std::enable_if<K != 0, typename elem_type_holder<K, StructOfArrays<FirstType, RestTypes...>>::type&>::type
		back()
	{
		StructOfArrays<RestTypes...>& base = *this;
		return base.back<K - 1>();
	}

	// Finds the specified element by scanning each entry and checking for equality, returning the first entry found. O(N).
	template <size_t K, typename T>
	size_t find(const T& value, std::function<bool(const T&, const T&)> compare_equals = [](const T& lhs, const T& rhs) const { return (lhs == rhs); }) const
	{
		const std::vector<T>& arr = get<K>();
		for (size_t i = 0; i < arr.size(); ++i)
		{
			if (compare_equals(value, arr[i]))
				return i;
		}

		return arr.size();
	}

	// Finds the specified element using std::lower_bound (typically a binary search).  O(logN).
	template <size_t K, typename T>
	size_t find_sorted(const T& value, std::function<bool(const T&, const T&)> compare_lessthan = [](const T& lhs, const T& rhs) { return (lhs < rhs); })
	{
		const std::vector<T>& arr = get<K>();
		auto it = std::lower_bound(arr.begin(), arr.end(), value, compare_lessthan);
		if (it == arr.end())
		{
			return arr.size();
		}
		else
		{
			return it - arr.begin();
		}
	}

	void push_back(const FirstType& firstarg, const RestTypes&... restargs)
	{
		data.push_back(firstarg);

		StructOfArrays<RestTypes...>& base = *this;
		base.push_back(restargs...);
	}

	template <typename T, typename... Ts>
	void emplace_back(T& firstarg, Ts&... restargs)
	{
		data.emplace_back(firstarg);

		StructOfArrays<RestTypes...>& base = *this;
		base.emplace_back(restargs...);
	}

	void insert(size_t index, const FirstType& firstarg, const RestTypes&... restargs)
	{
		if (index >= data.size())
			data.push_back(firstarg);
		else
			data.insert(data.begin() + index, firstarg);

		StructOfArrays<RestTypes...>& base = *this;
		base.insert(index, restargs...);
	}

	// Swaps the entry at the specified index with the entry at the rear of the list, then pops the rear of the list. O(1).
	void erase_swap(size_t index)
	{
		data[index] = data.back();
		data.pop_back();

		StructOfArrays<RestTypes...>& base = *this;
		base.erase_swap(index);
	}

	// Erases the entry at the specified index, then shifts everything after that index one element to the left. O(N).
	void erase_shift(size_t index)
	{
		data.erase(data.begin() + index);

		StructOfArrays<RestTypes...>& base = *this;
		base.erase_shift(index);
	}

	void clear()
	{
		data.clear();

		StructOfArrays<RestTypes...>& base = *this;
		base.clear();
	}

	void resize(size_t new_size)
	{
		data.resize(new_size);

		StructOfArrays<RestTypes...>& base = *this;
		base.resize(new_size);
	}

	// Pack the entire structure of arrays into one big contiguous memory buffer.
	// Used primarily for the creation of geometry data.
	// TODO: Rewrite this whole friggin' class so the data is naturally stored in a single contiguous buffer.
	void pack_buffer(std::vector<char>& buffer)
	{
		size_t old_buffer_size = buffer.size();
		size_t new_buffer_size = sizeof(FirstType) * data.size();
		buffer.resize(old_buffer_size + new_buffer_size);
		memcpy(&buffer[old_buffer_size], &data[0], new_buffer_size);

		StructOfArrays<RestTypes...>& base = *this;
		base.pack_buffer(buffer);
	}

	// Returns the number of entries.
	size_t size() const
	{
		return data.size();
	}
	
protected:

	std::vector<FirstType> data;
};


#endif // HVH_WC_TOOLS_STRUCTOFARRAYS_H