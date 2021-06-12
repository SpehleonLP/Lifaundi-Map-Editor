#ifndef ENUMS_HPP
#define ENUMS_HPP

enum
{
	TILE_BYTES = 350000,
};

enum class Tool : int
{
	None = -1,
	Create,
	Duplicate,
	Select,
	Translate,
	Rotate,
	Scale,
	Slice,
	Extrude,
	Order,
	Finish
};

enum class State : int
{
	None = -1,
	MouseDownOnSelected,
	MouseDownOnUnselected,
	MouseDownOnNothing,

	CreateSet,
	BoxSelectSet,
	TranslateSet,
	RotateSet,
	ScaleSet,
	SliceSet,
	ExtrudeSet,

	CreateBegin,
	BoxSelectBegin,
	TranslateBegin,
	RotateBegin,
	ScaleBegin,
	SliceBegin,
	ExtrudeBegin,
	WeldBegin,

	Reorder,
};


enum class SelectionType : unsigned char
{
	VERTEX,
	EDGE,
	FACE,
};

enum class Tools
{
	CLICK_SELECT,
	BOX_SELECT,

	DELETE_FACES,

	TRANSLATE,
	ROTATE,
	SCALE,

	EXTRUDE,
	SLICE,
	KNIFE,
};

enum class Bitwise
{
	SET = 1,
	AND = 2,
	OR  = 4,
	XOR = 8,
	NOT = 16,
};


#endif // ENUMS_HPP
