#ifndef LF_MATH_H
#define LF_MATH_H
#include <glm/vec2.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace math
{
	template<typename T, glm::qualifier Q=glm::highp>
	inline T length2(glm::vec<2, T, Q> const &p)
	{
		return p.x*p.x + p.y*p.y;
	}

	template<typename T, glm::qualifier Q=glm::highp>
	inline float length(glm::vec<2, T, Q> const &p)
	{
		return sqrt(length2(p));
	}

	template<typename T, glm::qualifier Q=glm::highp>
	inline T cross(glm::vec<2, T, Q> const &p, glm::vec<2, T, Q> const &q)
	{
		return p.x*q.y - q.x*p.y;
	}

	template<typename T, glm::qualifier Q=glm::highp>
	inline T dot(glm::vec<2, T, Q> const &p, glm::vec<2, T, Q> const &q)
	{
		return p.x*q.x + q.y*p.y;
	}

	template<typename T, glm::qualifier Q=glm::highp>
	inline T IsColinear(glm::vec<2, T, Q> p, glm::vec<2, T, Q> q, glm::vec<2, T, Q> r)
	{
		int value = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
		return value == 0;
	}

	template<typename T, glm::qualifier Q=glm::highp>
	inline T IsColinear(glm::vec<2, T, Q> a0, glm::vec<2, T, Q> a1, glm::vec<2, T, Q> b0, glm::vec<2, T, Q> b1)
	{
		return IsColinear(a0, a1, b0) && IsColinear(a0, a1, b1);
	}

	template<typename T, glm::qualifier Q=glm::highp>
	inline T IsOpposite(glm::vec<2, T, Q> p, glm::vec<2, T, Q> q)
	{
		return p.x*q.x <= 0 && p.y*q.y <= 0;
	}

	template<typename T, glm::qualifier Q=glm::highp>
	bool GetOverlap(glm::vec<2, T, Q> a0, glm::vec<2, T, Q> vec_a, glm::vec<2, T, Q> b0, glm::vec<2, T, Q> b1, std::pair<float, int> * begin, std::pair<float, int> * end)
	{
		std::pair<float, int> t[4];

		t[0] = std::make_pair(0.f, 0);
		t[1] = std::make_pair(1.f, 1);

		if(std::fabs(vec_a.x) < std::fabs(vec_a.y))
		{
			t[2] = std::make_pair((b0.y - a0.y) / (float) vec_a.y, 2);
			t[3] = std::make_pair((b1.y - a0.y) / (float) vec_a.y, 3);
		}
		else
		{
			t[2] = std::make_pair((b0.x  - a0.x) / (float) vec_a.x, 2);
			t[3] = std::make_pair((b1.x  - a0.x) / (float) vec_a.x, 3);
		}

		//check that we really overlap...
		if((t[2].first >= 1.f && t[3].first >= 1.f)
		|| (t[2].first <= 0.f && t[3].first <= 0.f))
			return false;

		std::sort(&t[0], &t[4], [](std::pair<float, int> const& b0, std::pair<float, int> const& b1) { return b0.first < b1.first; } );

		*begin = t[1];
		*end   = t[2];

		return true;
	}

	template<typename T, glm::qualifier Q=glm::highp>
	bool DoesOverlap(glm::vec<2, T, Q> a0, glm::vec<2, T, Q> vec_a, glm::vec<2, T, Q> b0, glm::vec<2, T, Q> b1)
	{
		float t[2];

		if(std::fabs(vec_a.x) < std::fabs(vec_a.y))
		{
			t[0] = (b0.y - a0.y) / (float) vec_a.y;
			t[1] = (b1.y - a0.y) / (float) vec_a.y;
		}
		else
		{
			t[0] = (b0.x  - a0.x) / (float) vec_a.x;
			t[1] = (b1.x  - a0.x) / (float) vec_a.x;
		}

		//check that we really overlap...
		return (t[0] < 1.f || t[1] < 1.f) && (t[0] > 0.f || t[1] > 0.f);
	}

	template<typename U, typename V, glm::qualifier Q=glm::highp>
	inline bool boxIntersects(glm::vec<2, U, Q> v0, glm::vec<2, U, Q> v1, glm::vec<2, V, Q> u0, glm::vec<2, V, Q> u1)
	{
		return v0.x <= u1.x && v0.y <= u1.y
			&& u0.x <= v1.x && u0.y <= v1.y;
	}

	template<typename U, typename V, glm::qualifier Q=glm::highp>
	inline bool boxContains(glm::vec<2, U, Q> u, glm::vec<2, V, Q> v0, glm::vec<2, V, Q> v1)
	{
		return v0.x < u.x && u.x < v1.x
			&& v0.y < u.y && u.y < v1.y;
	}

	template<typename T, glm::qualifier Q=glm::highp>
	float LineDistanceSquared(glm::vec<2, T, Q> v0, glm::vec<2, T, Q> v1, glm::vec<2, T, Q> pos)
	{
		glm::vec<2, T, Q> distance = v1 -  v0;
		const float l2 = math::length2(distance);
		if(l2 == 0) return math::length2(pos - v0);
		const float t  = math::dot(pos - v0, distance) / l2;
		const glm::vec2 projection = glm::vec2(v0) + t * glm::vec2(distance);

		return math::length2(glm::vec2(pos) - projection);
	}

	template<typename T, glm::qualifier Q=glm::highp>
	float SemgentDistanceSquared(glm::vec<2, T, Q> v0, glm::vec<2, T, Q> v1, glm::vec<2, T, Q> pos)
	{
		glm::vec<2, T, Q> distance = v1 -  v0;
		const float l2 = math::length2(distance);
		if(l2 == 0) return math::length2(pos - v0);
		float t  = math::dot(pos - v0, distance) / l2;
		t = std::max(0.f, std::min(t, 1.f));
		const glm::vec2 projection = glm::vec2(v0) + t * glm::vec2(distance);

		return math::length2(glm::vec2(pos) - projection);
	}

	inline uint32_t GetZOrder(uint32_t x, uint32_t y)
	{
		x = (x | (x << 8)) & 0x00FF00FF;
		x = (x | (x << 4)) & 0x0F0F0F0F;
		x = (x | (x << 2)) & 0x33333333;
		x = (x | (x << 1)) & 0x55555555;

		y = (y | (y << 8)) & 0x00FF00FF;
		y = (y | (y << 4)) & 0x0F0F0F0F;
		y = (y | (y << 2)) & 0x33333333;
		y = (y | (y << 1)) & 0x55555555;

		return x | (y << 1);
	}

	template<typename T, glm::qualifier Q=glm::highp>
	inline uint32_t GetZOrder(glm::vec<2, T, Q> v)
	{
		return GetZOrder(v.x, v.y);
	}

	template<typename T, glm::qualifier Q=glm::highp>
	bool IsPointContained(glm::vec<2, T, Q> const * verts, glm::vec<2, T, Q> pos)
	{
		return
			(math::cross(verts[1] - verts[0], pos - verts[0]) <= 0)
		&&  (math::cross(verts[2] - verts[1], pos - verts[1]) <= 0)
		&&  (math::cross(verts[3] - verts[2], pos - verts[2]) <= 0)
		&&  (math::cross(verts[0] - verts[3], pos - verts[3]) <= 0);
	}

	template<typename T, glm::qualifier Q=glm::highp>
	int orientation(glm::vec<2, T, Q> p, glm::vec<2, T, Q> q, glm::vec<2, T, Q> r)
	{
		// See https://www.geeksforgeeks.org/orientation-3-ordered-points/
		// for details of below formula.
		int val = (q.y - p.y) * (r.x - q.x) -
		(q.x - p.x) * (r.y - q.y);

		if (val == 0) return 0;  // colinear

		return (val > 0)? 1: 2; // clock or counterclock wise
	}

	template<typename T, glm::qualifier Q=glm::highp>
	bool DoLinesIntersect(glm::vec<2, T, Q> p1, glm::vec<2, T, Q> p2, glm::vec<2, T, Q> q1, glm::vec<2, T, Q> q2)
	{
		int i = math::cross(q1-p1, p2-p1) * math::cross(q2-p1, p2-p1);
		int j = math::cross(p1-q1, q2-q1) * math::cross(p2-q1, q2-q1);

		if(i < 0 && j < 0)
			return true;
		if( i > 0 && j > 0)
			return true;

		return false;
	}

};

#endif // LF_MATH_H
