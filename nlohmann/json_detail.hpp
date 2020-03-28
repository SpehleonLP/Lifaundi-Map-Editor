#ifndef JSON_DETAIL_HPP
#define JSON_DETAIL_HPP
#include "nlohmann/json.hpp"

namespace detail
{
	constexpr std::array<float, 2> NegVec2{-1, -1};
	constexpr std::array<float, 4> NegVec4{-1, -1, -1, -1};

	constexpr std::array<float, 2> NullVec2{0, 0};
	constexpr std::array<float, 3> NullVec3{0, 0, 0};
	constexpr std::array<float, 3> ZVec3{0, 0, 1};
	constexpr std::array<float, 4> NullVec4{0, 0, 0, 0};
	constexpr std::array<float, 2> IdentityVec2{1, 1};
	constexpr std::array<float, 3> IdentityVec3{1, 1, 1};
	constexpr std::array<float, 4> IdentityVec4{1, 1, 1, 1};
	constexpr std::array<float, 4> IdentityRotation{0, 0, 0, 1};


	#if defined(FX_GLTF_HAS_CPP_17)
	template <typename TTarget>
	inline void ReadRequiredField(std::string_view key, nlohmann::json const & json, TTarget & target)
	#else
	template <typename TKey, typename TTarget>
	inline void ReadRequiredField(TKey && key, nlohmann::json const & json, TTarget & target)
	#endif
	{
		const nlohmann::json::const_iterator iter = json.find(key);
		if (iter == json.end())
		{
			throw std::runtime_error("Required field not found: " + std::string(key));
		}

		target = iter->get<TTarget>();
	}

	#if defined(FX_GLTF_HAS_CPP_17)
	template <typename TTarget>
	inline void ReadOptionalField(std::string_view key, nlohmann::json const & json, TTarget & target)
	#else
	template <typename TKey, typename TTarget>
	inline void ReadOptionalField(TKey && key, nlohmann::json const & json, TTarget & target)
	#endif
	{
		const nlohmann::json::const_iterator iter = json.find(key);
		if (iter != json.end())
		{
			target = iter->get<TTarget>();
		}
	}

	template <typename TKey>
	inline bool ReadBitField(TKey && key, nlohmann::json const & json)
	{
		const nlohmann::json::const_iterator iter = json.find(key);
		if (iter != json.end())
		{
			return iter->get<bool>();
		}

		return false;
	}

		template <typename TValue>
	inline void WriteField(std::string const & key, nlohmann::json & json, TValue const & value)
	{
		if (!value.empty())
		{
			json[key] = value;
		}
	}

	template <typename TValue>
	inline void WriteField(std::string const & key, nlohmann::json & json, TValue const & value, TValue const & defaultValue)
	{
		if (value != defaultValue)
		{
			json[key] = value;
		}
	}

	inline void WriteBitField(std::string const & key, nlohmann::json & json, bool const & value)
	{
		if (value != false)
		{
			json[key] = value;
		}
	}
}


#endif // JSON_DETAIL_HPP
