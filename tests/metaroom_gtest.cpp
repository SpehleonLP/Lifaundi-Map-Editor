#include "src/metaroom.h"
#include "src/clipboard.h"
#include <glm/packing.hpp>
#include <gtest/gtest.h>
#include <climits>

// A Metaroom with no Document: geometry, the quad tree and the invariant
// checks never reach the document or the GL side.
namespace
{

// Same winding and defaults as ControllerFSM::AddFace, the create tool's path.
Room MakeRoom(glm::ivec2 min, glm::ivec2 max)
{
	Room room{};
	room.verts[0] = max;
	room.verts[1] = glm::ivec2(max.x, min.y);
	room.verts[2] = min;
	room.verts[3] = glm::ivec2(min.x, max.y);
	room.gravity     = glm::packHalf2x16(glm::vec2(0, 9.81));
	room.music_track = -1;
	room.depth       = glm::u16vec2(0, 4 * USHRT_MAX / 64);
	return room;
}

struct MetaroomTest : ::testing::Test
{
	Metaroom mta{nullptr};
};

}

TEST(MetaroomEdges, IndexHelpersStayWithinFace)
{
	EXPECT_EQ(Metaroom::NextInEdge(4), 5);
	EXPECT_EQ(Metaroom::NextInEdge(7), 4);
	EXPECT_EQ(Metaroom::PrevInEdge(4), 7);
	EXPECT_EQ(Metaroom::PrevInEdge(6), 5);
	EXPECT_EQ(Metaroom::GetOppositeEdge(8), 10);
	EXPECT_EQ(Metaroom::GetOppositeEdge(9), 11);
}

TEST_F(MetaroomTest, InsertedRoomIsFoundAtItsInterior)
{
	auto faces = mta.Insert({MakeRoom({0, 0}, {100, 100})});

	ASSERT_EQ(faces.size(), 1u);
	EXPECT_EQ(mta.noFaces(), 1u);
	EXPECT_EQ(mta.GetFace({50, 50}), (int)faces[0]);
	EXPECT_EQ(mta.GetFace({150, 50}), -1);
}

TEST_F(MetaroomTest, CanAddFaceRejectsOverlapButNotAdjacency)
{
	mta.Insert({MakeRoom({0, 0}, {100, 100})});

	auto overlapping = MakeRoom({50, 50}, {150, 150});
	auto adjacent    = MakeRoom({100, 0}, {200, 100});

	EXPECT_FALSE(mta.CanAddFace(overlapping.verts.data()));
	EXPECT_TRUE(mta.CanAddFace(adjacent.verts.data()));
}

TEST_F(MetaroomTest, AdjacentRoomsAreSymmetric)
{
	mta.Insert({
		MakeRoom({0, 0}, {100, 100}),
		MakeRoom({100, 0}, {200, 100}),
		MakeRoom({0, 100}, {100, 200}),
	});

	ASSERT_EQ(mta.noFaces(), 3u);
	EXPECT_EQ(mta.TestTreeSymmetry(), "");
	EXPECT_EQ(mta.TestDoorSymmetry(), "");
}

TEST_F(MetaroomTest, RemovedRoomIsNoLongerFound)
{
	auto faces = mta.Insert({
		MakeRoom({0, 0}, {100, 100}),
		MakeRoom({100, 0}, {200, 100}),
	});
	ASSERT_EQ(mta.GetFace({150, 50}), (int)faces[1]);

	mta.RemoveFace(faces[1]);

	EXPECT_EQ(mta.noFaces(), 1u);
	EXPECT_EQ(mta.GetFace({150, 50}), -1);
	EXPECT_EQ(mta.GetFace({50, 50}), (int)faces[0]);
}
