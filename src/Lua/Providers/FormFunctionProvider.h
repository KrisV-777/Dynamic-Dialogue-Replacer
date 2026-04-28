#pragma once

#include "../LuaFunctionProvider.h"

namespace DDR
{
	class FormFunctionProvider final : public ILuaFunctionProvider
	{
	public:
		void Register(sol::state& a_lua) override;

	private:
		static uint32_t GetFormId(uint32_t a_id, const std::string& a_esp);
	};
}
