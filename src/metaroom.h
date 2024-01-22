#ifndef METAROOM_H
#define METAROOM_H
#include "metaroomselection.h"
#include "metaroomdoors.h"
#include "metaroom_gl.h"
#include "metaroom_memory.h"
#include "quadtree.h"
#include "enums.hpp"
#include "controllerfsm.h"
#include "Spehleon/lib/Support/shared_array.hpp"
#include <glm/vec2.hpp>
#include <glm/gtc/type_precision.hpp>
#include <memory>
#include <vector>

#define HAVE_UUID 1

struct LinkVert
{
	int      face;
	int      vert;
	uint8_t  edge;
	uint8_t  type;
};

struct Link
{
	LinkVert begin;
	LinkVert end;
};

struct Room;

typedef struct FMOD_GUID FMOD_GUID;
class Document;

class Metaroom : public MetaroomMemory
{
public:
typedef std::array<SliceEdge, 2> SliceRoom;
typedef std::vector<SliceRoom> SliceArray;

	enum
	{
		VERSION_STILL_HAVE_SIZE=2,
		VERSION_ADDED_MUSIC=3,
		VERSION_ADDED_MVSF=4,
		VERSION_ADDED_SHADE=5,
		VERSION_ADDED_DEPTH=6,
		VERSION = 6
	};

	static int NextInEdge(int id) { return (id & 0xFFFFFFFC) + ((id+1) & 0x03); }
	static int GetOppositeEdge(int id) { return id ^ 0x02; }

	MetaroomGL gl{*this};

	Metaroom(Document *);
	~Metaroom();

	std::vector<std::pair<int, int> > GetSelectedDoors() const;
	std::vector<int>  GetExtrudableEdges() const;

	std::vector<uint32_t> Insert(std::vector<Room> const&);
	void Insert(std::vector<Room> const&, std::vector<uint32_t> const&);

	glm::vec4 GetDimensions() const { return _tree.GetDimensions(); };

	void AddFace(glm::ivec2 min, glm::ivec2 max);
	bool CanAddFace(glm::ivec2 * verts);
	std::vector<uint32_t> AddFaces(uint32_t no_faces = 1);

	void RemoveFace(int id);
	void RemoveFaces(const std::vector<uint32_t> &vec);

	std::vector<uint32_t> Duplicate(std::vector<uint32_t> const& faces);
	void PruneDegenerate();

	//int GetWallType() const;

	int GetPermeability() const;

	void SetDoorType(int);
	int GetDoorType();

	void Read(MainWindow * window, std::ifstream &, size_t offset);
	uint32_t Write(MainWindow * window, std::ofstream &);

	std::vector<uint32_t> Slice(SliceArray &slice);

	inline int GetFace(glm::ivec2 position) const { return _tree.GetFace(position); }
	int        GetSliceEdge(glm::ivec2 p) const;

	void Translate(glm::ivec2 translation, glm::ivec2 half_dimensions);
	void Rotate(glm::ivec2 center, glm::vec2 complex);
	void Scale(glm::ivec2 center, glm::vec2 scale);

	void ClickSelect(glm::ivec2 pos, Bitwise flags, bool alt, float zoom);
	void BoxSelect(glm::ivec2 tl, glm::ivec2 br, Bitwise flags);

	void CancelMove();
	void CommitMove(bool update_mvsf = true);
	void BeginMove();

	glm::ivec2 GetSelectionCenter() const;


	std::vector<glm::i16vec2> SaveVerts();
	void RestoreVerts(std::vector<glm::i16vec2> const& vec);

	void PruneLinks();
	void ConvertLinks(int a, int b);


	static bool IsColinear(glm::ivec2 p, glm::ivec2 q, glm::ivec2 r);

	Document *const   document;
	mutable QuadTree  _tree{this};

	void update_selections() {};


	void remove_face_links(int id);
	void RingSelectFace(int face, glm::ivec2 mouse, Bitwise flags);
	void RingSelectFaceInternal(int edge, glm::ivec2 position, Bitwise flags);

	void RingSelectEdge(int face, Bitwise flags);
	void RingSelectEdgeInternal(int face, Bitwise flags);

	void solve_constraints() {};

	std::string TestTreeSymmetry();
	std::string TestDoorSymmetry();

	std::vector<std::pair<glm::vec4, float>> GetPermTable() const;
	void SetPermTable(std::vector<std::pair<glm::vec4, float>> const&);


};


#endif // METAROOM_H
