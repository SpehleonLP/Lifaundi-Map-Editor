#ifndef LF_MATH_H
#define LF_MATH_H
#include <glm/mat3x3.hpp>
#include <glm/vec2.hpp>
#include <algorithm>
#include <cmath>
#include "glm_iostream.h"
#include <iostream>

namespace math
{
	template<typename T, glm::qualifier Q=glm::highp>
	inline auto ortho(glm::vec<2, T, Q> const &p)
	{
		return glm::vec<2, T, Q>(p.y, -p.x);
	}

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

	template<typename T, typename S, glm::qualifier Q=glm::highp>
	inline auto cross(glm::vec<2, T, Q> const &p, glm::vec<2, S, Q> const &q)
	{
		return p.x*q.y - p.y*q.x;
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
	inline float LineDistanceSquared(glm::vec<2, T, Q> v0, glm::vec<2, T, Q> v1, glm::vec<2, T, Q> pos)
	{
		glm::vec<2, T, Q> distance = v1 -  v0;
		const float l2 = math::length2(distance);
		if(l2 == 0) return math::length2(pos - v0);
		const float t  = math::dot(pos - v0, distance) / l2;
		const glm::vec2 projection = glm::vec2(v0) + t * glm::vec2(distance);

		return math::length2(glm::vec2(pos) - projection);
	}

	template<typename T, glm::qualifier Q=glm::highp>
	inline float SemgentDistanceSquared(glm::vec<2, T, Q> v0, glm::vec<2, T, Q> v1, glm::vec<2, T, Q> pos)
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
	inline bool IsPointContained(glm::vec<2, T, Q> const * verts, glm::vec<2, T, Q> pos)
	{
		return
			(math::cross(verts[1] - verts[0], pos - verts[0]) <= 0)
		&&  (math::cross(verts[2] - verts[1], pos - verts[1]) <= 0)
		&&  (math::cross(verts[3] - verts[2], pos - verts[2]) <= 0)
		&&  (math::cross(verts[0] - verts[3], pos - verts[3]) <= 0);
	}

	template<typename T, glm::qualifier Q=glm::highp>
	inline int orientation(glm::vec<2, T, Q> p, glm::vec<2, T, Q> q, glm::vec<2, T, Q> r)
	{
		// See https://www.geeksforgeeks.org/orientation-3-ordered-points/
		// for details of below formula.
		int val = (q.y - p.y) * (r.x - q.x) -
		(q.x - p.x) * (r.y - q.y);

		if (val == 0) return 0;  // colinear

		return (val > 0)? 1: 2; // clock or counterclock wise
	}

	template<typename T, glm::qualifier Q=glm::highp>
	inline bool DoLinesIntersect(glm::vec<2, T, Q> s1, glm::vec<2, T, Q> s_p, glm::vec<2, T, Q> r_p, glm::vec<2, T, Q> r1)
	{
		auto s_d = s1 - s_p;
		auto r_d = r1 - r_p;

		float T1, T2;
		T2 = math::cross(r_d, s_d);
//runs parallel
		if(T2 == 0.f) return false;
		T2 = math::cross(s_p - r_p, r_d) / T2;

		if(!(0.f < T2 && T2 < 1.f)) return false;

		if(std::abs(r_d.x) > std::abs(r_d.y))
			T1 = (s_p.x + s_d.x*T2 - r_p.x)/r_d.x;
		else
			T1 = (s_p.y + s_d.y*T2 - r_p.y)/r_d.y;

		return (0.f < T1 && T1 < 1.f);
	}

	inline glm::mat3 GetRotation(glm::vec2 const& a, glm::vec2 const& b)
	{
		auto _sin = (math::cross(a, b));
		auto _cos = glm::dot(a, b);

		return glm::mat3(
			_cos, -_sin, 0,
			_sin,  _cos, 0,
			0, 0, 1);
	}


	inline glm::vec2 OrthoNormal(glm::ivec2 const& a)
	{
		return glm::normalize(glm::vec2(math::ortho(a)));
	}

};

#endif // LF_MATH_H
