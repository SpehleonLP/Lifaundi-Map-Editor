#ifndef FLUIDLINKS_H
#define FLUIDLINKS_H
#include <vector>

class Metaroom;

class FluidLinks
{
public:

	FluidLinks(Metaroom * metaroom);

	struct Pair
	{
		int m[2];
	};

	struct Hall_Link
	{
		int f0, f1, len;
	};

	std::vector<std::pair<int, int>> halls;
	std::vector<Hall_Link>           hall_links;
	std::vector<std::pair<int, int>> misc;

	std::pair<int, int> GetHall(int room) const;
};

#endif // FLUIDLINKS_H
