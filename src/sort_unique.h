#ifndef SORT_UNIQUE_H
#define SORT_UNIQUE_H
#include <vector>

template<typename T, typename Cmp>
void Uniq(std::vector<T> & vec, Cmp cmp)
{
	size_t write = 0;

	for(size_t read = 0; read < vec.size(); )
	{
		auto const& t = vec[read];

		for(; read < vec.size() && cmp(vec[read], t); ++read) {}

		if(read-1 != write)
			vec[write++] = std::move(t);
	}

	vec.resize(write);
}


template<typename T, typename Cmp>
T * GetMode(std::vector<T> & vec, Cmp cmp)
{
	T * t = nullptr;
	int run_length = -1;

	for(size_t i = 0; i < vec.size();)
	{
		auto j = i;

		for(; i < vec.size() && cmp(vec[i], vec[j]); ++i) {}

		if((int)(i-j) > run_length)
		{
			run_length = (i-j);
			t          = &vec[j];
		}
	}

	return t;
}

template<typename T>
void Uniq(std::vector<T> & vec)
{
	struct Comparator
	{
		bool operator()(T const& a, T const& b) { return a == b; }
	};

	return Uniq<T, Comparator>(vec, Comparator{});
}

template<typename T>
T * GetMode(std::vector<T> & vec)
{
	struct Comparator
	{
		bool operator()(T const& a, T const& b) { return a == b; }
	};

	return GetMode<T, Comparator>(vec, Comparator{});
}


#endif // SORT_UNIQUE_H
