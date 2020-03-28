#ifndef RESEATALGORITHM_H
#define RESEATALGORITHM_H
#include <vector>

struct ReseatData
{
	int prev{-1};
	int next{-1};

	float best_prev{0};
	float best_next{0};
};

std::vector<int> GetReseatingArray(const std::vector<int> & selection, std::vector<ReseatData> && );

#endif // RESEATALGORITHM_H
