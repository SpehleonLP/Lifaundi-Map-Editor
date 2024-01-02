#ifndef METAROOMSELECTION_H
#define METAROOMSELECTION_H
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
	size_t size() const { return m_alloced; };

	void erase(size_t i);
	void erase(std::vector<uint32_t> const& vec);
	void clear();

	void begin_and();
	void end_and();

	uint8_t operator[](size_t i) const { return m_array[i]; }
	uint8_t & operator[](size_t i) { return m_array[i]; }

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

	bool IsVertSelected(int id) const { return m_array[id] != 0; }
	bool IsFaceSelected(int id) const { return IsVertSelected(id*4+0) & IsVertSelected(id*4+1) & IsVertSelected(id*4+2) & IsVertSelected(id*4+3); }
	bool IsEdgeSelected(int id) const { return IsVertSelected(id) & IsVertSelected((id & 0xFFFFFFFC) + (id+1) % 4); }
	bool IsSelected(int id) const { return IsVertSelected(id*4+0) | IsVertSelected(id*4+1) | IsVertSelected(id*4+2) | IsVertSelected(id*4+3); }

private:
	std::unique_ptr<uint8_t[]>    m_array;
	uint32_t                      m_alloced{};

	bool                          m_selectionChanged{true};
	uint32_t                      m_vbo[2]{};
	uint32_t                      m_selectedFaces{};
	uint32_t					  m_selectedEdges{};
	uint32_t                      m_glalloc{};
};

#endif // METAROOMSELECTION_H
