#pragma once

#include "../LuaFunctionProvider.h"

namespace DDR
{
	class EventFunctionProvider final : public ILuaFunctionProvider
	{
	public:
		void Register(sol::state& a_lua) override;

	private:
		static void SendModEvent(const std::string& a_event, const std::string& a_argStr, float a_argNum, uint32_t a_argForm);
	};
}
