#ifndef METAROOM_H
#define METAROOM_H
#include "metaroomselection.h"
#include "metaroomdoors.h"
#include "quadtree.h"
#include "enums.hpp"
#include "indexbuffers.h"
#include "controllerfsm.h"
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

class Metaroom
{
public:
	static int NextInEdge(int id) { return (id & 0xFFFFFFFC) + ((id+1) & 0x03); }
	static int GetOppositeEdge(int id) { return id ^ 0x02; }

	Metaroom(Document *);
	~Metaroom();

	std::vector<std::pair<int, int> > GetSelectedDoors() const;
	std::vector<int>  GetExtrudableEdges() const;

    void Release(GLViewWidget *gl);

	int Insert(std::vector<Room> const&);
	void SetRoom(int index, Room const&);
	void Insert(std::vector<Room> const&, std::vector<int> const&);
	void CopyRoom(int dst, int src);
	uint64_t GetDoorKey(uint32_t a, uint32_t b) const;


	glm::vec2 GetGravity(int room) const;
	glm::vec2 GetCenter(int room) const;

	uint32_t GetGravity() const;
	int GetMusicTrack() const;
	int GetRoomType() const;
	int GetWallType() const;

	int GetPermeability() const;
	int GetPermeability(int a, int b) const;

	void SetDoorType(int);
	int GetDoorType();

	size_t size() const { return faces; }

	void Read(MainWindow * window, std::ifstream &, size_t offset);
	uint32_t Write(MainWindow * window, std::ofstream &);

    void Prepare(GLViewWidget *gl);
    void Render(GLViewWidget* gl, int selected_door_type);

	int Slice(std::vector<SliceInfo> & slice);
	int Duplicate(std::vector<int> const& faces);
	void Expand(std::vector<int> const& faces);

	int        GetSliceEdge(glm::ivec2 p) const;
	glm::ivec2 GetPointOnEdge(int v0, float p) const;
	float      GetPercentOfEdge(int v0, glm::ivec2) const;
	float      ProjectOntoEdge(int v0, glm::ivec2) const;

	void AddFace(glm::ivec2 min, glm::ivec2 max);

	int AddFaces(int no_faces = 1);
	int GetFace(glm::ivec2 position);

	void RemoveFace(int id);
	void RemoveFaces(std::vector<int> const& vec);

	void Translate(glm::ivec2 translation, glm::ivec2 half_dimensions);
	void Rotate(glm::ivec2 center, glm::vec2 complex);
	void Scale(glm::ivec2 center, glm::vec2 scale);

	void ClickSelect(glm::ivec2 pos, Bitwise flags);
	void BoxSelect(glm::ivec2 tl, glm::ivec2 br, Bitwise flags);

	void CancelMove();
	void CommitMove(bool update_mvsf = true);
	void BeginMove();

	glm::ivec2 GetSelectionCenter() const;

	inline glm::ivec2 GetNextVertex(int i) const { return m_verts[(i & 0xFFFFFFFC) + ((i+1)&0x03)]; }
	inline glm::ivec2 GetVertex(int i) const { return m_verts[i]; }
	inline glm::ivec2 GetEdge(int i) const { return m_verts[(i & 0xFFFFFFFC) + ((i+1)&0x03)] - m_verts[i]; }

	inline glm::ivec2 GetVertex(int i, int j) const { return m_verts[i*4 + j%4]; }
	inline glm::ivec2 GetEdge(int i, int j) const { return GetVertex(i, j+1) - GetVertex(i, j); }

	inline int GetDoorType(int i, int j) const { return m_doorType[i*4 + j%4]; }

	std::vector<glm::i16vec2> SaveVerts();
	void RestoreVerts(std::vector<glm::i16vec2> const& vec);

	void GetFaceAABB(int id, glm::i16vec2 & tl, glm::i16vec2 & br) const;

	glm::i16vec4 GetBoundingBox() const;

	void PruneLinks();
	void ConvertLinks(int a, int b);

	bool IsPointContained(int face, glm::ivec2 point);

	static bool IsColinear(glm::ivec2 p, glm::ivec2 q, glm::ivec2 r);

	Document *const   document;
    mutable QuadTree  m_tree{this};
	MetaroomSelection m_selection;
	IndexBuffers      m_indices;

	void update_selections() {};


	bool m_dirty{true};

	void remove_face_links(int id);


	void solve_constraints() {};

	std::vector<std::pair<uint64_t, uint8_t> > m_permeabilities;

	std::unique_ptr<glm::ivec2[]> m_verts;
	std::unique_ptr<glm::ivec2[]> m_prev_verts;
	std::unique_ptr<glm::ivec2[]> m_scratch_verts;

//room properties
	std::unique_ptr<uint32_t[]>  m_gravity;  // 2 half floats
	std::unique_ptr<int8_t[]>    m_music;
	std::unique_ptr<uint8_t[]>   m_roomType;
	std::unique_ptr<uint8_t[]>   m_doorType;
	std::unique_ptr<std::pair<int, int>[]> m_hall;
#if HAVE_UUID
	std::unique_ptr<uint32_t[]>  m_uuid;
	uint32_t                      m_uuid_counter{};
#endif

	uint32_t allocated{};
	uint32_t faces{};

	uint32_t gl_vert_texture{};
	uint32_t gl_vert_vbo{};

	uint32_t m_vao[3]{};
	uint32_t m_buffers[3]{};

	uint32_t m_glAlloced{};
	uint32_t m_hallAlloc{};
	uint32_t m_noHalls{};
};


#endif // METAROOM_H
