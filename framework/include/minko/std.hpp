/*
Copyright (c) 2014 Aerys

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "minko/Common.hpp"

namespace std
{
    template <class T>
    inline
    void
    hash_combine(std::size_t & seed, const T & v)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    template<typename T, typename V>
    struct pair_hash
    {
        inline
        size_t
        operator()(const std::pair<T, V>& key) const
        {
            size_t seed = 0;

            hash_combine(seed, key.first);
            hash_combine(seed, key.second);

            return seed;
        }
    };

    template<typename T, typename V>
    struct pair_eq
    {
        inline
        bool
        operator()(const std::pair<T, V>& lhs, const std::pair<T, V>& rhs) const
        {
            return lhs.first == rhs.first && lhs.second == rhs.second;
        }
    };

    template<>
	struct hash<minko::math::mat4>
	{
		inline
		size_t
		operator()(const minko::math::mat4& matrix) const
		{
			return (size_t)minko::math::value_ptr(matrix);
		}
	};

    /*
	inline
	std::string
	to_string(const minko::math::mat4& matrix)
	{
		std::string str = "mat4(";
		auto ptr = minko::math::value_ptr(matrix);
		for (auto i = 0; i < 15; ++i)
			str += to_string(ptr[i]) + ", ";
		str += to_string(ptr[15]) + ")";

		return str;
	}
    */

    template<typename T, minko::math::precision P>
    inline
    std::string
    to_string(const minko::math::detail::tvec1<T, P>& v)
    {
        return minko::math::to_string(v);
    }

    template<typename T, minko::math::precision P>
    inline
    std::string
    to_string(const minko::math::detail::tvec2<T, P>& v)
    {
        return minko::math::to_string(v);
    }

    template<typename T, minko::math::precision P>
    inline
    std::string
    to_string(const minko::math::detail::tvec3<T, P>& v)
    {
        return minko::math::to_string(v);
    }

    template<typename T, minko::math::precision P>
    inline
    std::string
    to_string(const minko::math::detail::tvec4<T, P>& v)
    {
        return minko::math::to_string(v);
    }

    template<typename T>
    inline
    std::string
    to_string(const std::list<T>& list, const std::string& separator)
    {
        std::string s = "";

        for (const T& e : list)
        {
            if (s.size())
                s += separator;
            s += std::to_string(e);
        }

        return s;
    }

    template<>
    inline
    std::string
    to_string<std::string>(const std::list<std::string>& list, const std::string& separator)
    {
        std::string s = "";

        for (const std::string& e : list)
        {
            if (s.size())
                s += separator;
            s += e;
        }

        return s;
    }

    template<typename T>
    inline
    std::string
    to_string(const std::vector<T>& list, const std::string& separator = ", ")
    {
        std::string s = "";

        for (const T& e : list)
        {
            if (s.size())
                s += separator;
            s += std::to_string(e);
        }

        return s;
    }

    template<>
    inline
    std::string
    to_string<std::string>(const std::vector<std::string>& list, const std::string& separator)
    {
        std::string s = "";

        for (const std::string& e : list)
        {
            if (s.size())
                s += separator;
            s += e;
        }

        return s;
    }

#ifdef __ANDROID__
	template <typename T>
	inline
	string
	to_string(T v)
	{
		ostringstream oss;
		oss << v;
		return oss.str();
	}
#endif

    template<>
    struct hash<minko::render::TextureFormat>
    {
        inline
        size_t
        operator()(const minko::render::TextureFormat& x) const
        {
            return std::hash<unsigned int>()(static_cast<unsigned int>(x));
        }
    };

    inline
    std::string
    replaceAll(std::string str, const std::string& from, const std::string& to)
    {
        size_t start_pos = 0;

        while ((start_pos = str.find(from, start_pos)) != std::string::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
        }

        return str;
    }

    inline
    std::vector<std::string>
    stringSplit(std::string str, std::string token)
    {
        std::vector<std::string> result;

        auto i = str.find_first_of(token);

        while (i != -1)
        {
            result.push_back(str.substr(0, i));

            str = str.substr(i + 1);

            i = str.find_first_of(token);
        }

        result.push_back(str);

        return result;
    }


    inline
    std::string
    stringJoin(std::vector<std::string> strings, std::string delimiter)
    {
        std::string result;

        for (size_t i = 0; i < strings.size(); ++i)
        {
            if (i != 0)
                result += delimiter;

            result += strings[i];
        }

        return result;
    }
}
