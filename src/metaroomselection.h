#ifndef METAROOMSELECTION_H
#define METAROOMSELECTION_H
#include "Support/shared_array.hpp"
#include "entitysystem.h"
#include "enums.hpp"
#include <memory>
#include <vector>
#include <cstdint>

class GLViewWidget;
class Metaroom;

class MetaroomSelection
{
public:
	typedef EntitySystem::Range Range;
	MetaroomSelection() = default;
	MetaroomSelection(MetaroomSelection const& it) = delete;
	MetaroomSelection(MetaroomSelection &&) = delete;
	MetaroomSelection(uint32_t size) { resize(size); }
	~MetaroomSelection();

    void Release(GLViewWidget *gl);

	bool ToggleSelectAll(Range faces, Bitwise flags);

	bool Changed() const { return m_selectionChanged; }

	void MergeSelection(MetaroomSelection const&, Bitwise flags);
    void Prepare(GLViewWidget *gl);
	uint32_t GetBuffer() const { return m_vbo[0]; }
	uint32_t GetIndices() const { return m_vbo[1]; }
	uint32_t NoSelectedFaces() const { return m_selectedFaces; }
	uint32_t NoSelectedEdges() const { return m_selectedEdges; }

	void resize(size_t);
	size_t size() const { return _selection.size(); };

	void erase(size_t i);
	void erase(std::vector<uint32_t> const& vec);
	void clear();

	void begin_and();
	void end_and();

//	uint8_t operator[](size_t i) const { return m_array[i]; }
//	uint8_t & operator[](size_t i) { return m_array[i]; }

	std::vector<uint32_t> GetVertSelection() const;
	std::vector<uint32_t> GetFaceSelection() const;
	std::vector<uint32_t> GetEdgeSelection() const;
	std::vector<uint32_t> GetSelection() const;
	void SetSelection(std::vector<uint32_t> const&);
	void SetVertexSelection(const std::vector<uint32_t> &);

	void select_vertex(int id, Bitwise flags);
	void select_face(int id, Bitwise flags);
	void select_edge(int id, Bitwise flags);

	void SetSelectionType(SelectionType type);

	bool MarkFace(int id);
	void ClearMarks();

	__always_inline bool IsVertSelected(int id) const { return _selection[id>>2][id&0x3] != 0; }
	__always_inline bool IsVertSelected(int face, int vert) const { return _selection[face][vert] != 0; }
	__always_inline bool IsFaceSelected(int id) const { return IsVertSelected(id, 0) && IsVertSelected(id, 1) && IsVertSelected(id, 2) && IsVertSelected(id, 3); }
	__always_inline bool IsEdgeSelected(int edge) const { return IsVertSelected(edge) && IsVertSelected(edge >> 2, (edge+1) % 4); }
	__always_inline bool IsAnyCornerSelected(int face) const { return IsVertSelected(face, 0) || IsVertSelected(face, 1) || IsVertSelected(face, 2) || IsVertSelected(face, 3); }

	__always_inline uint8_t * data() const { return (uint8_t*)_selection.data(); };

	__always_inline uint8_t & vertex(int i) { return ((uint8_t*)_selection.data())[i]; };
	__always_inline std::array<uint8_t, 4> & face(int i) { return _selection[i]; };

	__always_inline uint8_t vertex(int i) const { return ((uint8_t*)_selection.data())[i]; };
	__always_inline std::array<uint8_t, 4> face(int i) const { return _selection[i]; };

private:
	shared_array<std::array<uint8_t, 4>>  _selection;

	bool                          m_selectionChanged{true};
	uint32_t                      m_vbo[2]{};
	uint32_t                      m_selectedFaces{};
	uint32_t					  m_selectedEdges{};
	uint32_t                      m_glalloc{};
};

#endif // METAROOMSELECTION_H
