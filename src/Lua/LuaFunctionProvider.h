#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <lua.hpp>
#include <sol/sol.hpp>

namespace DDR
{
	class ILuaFunctionProvider
	{
	public:
		virtual ~ILuaFunctionProvider() = default;
		virtual void Register(sol::state& a_lua) = 0;
	};
}
