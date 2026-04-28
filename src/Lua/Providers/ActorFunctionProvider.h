#pragma once

#include "../LuaFunctionProvider.h"

namespace DDR
{
	class ActorFunctionProvider final : public ILuaFunctionProvider
	{
	public:
		void Register(sol::state& a_lua) override;

	private:
		static int HasKeyword(uint32_t a_id, const std::string& a_kwd, bool a_partialMatch);
		static int IsInFaction(uint32_t a_id, uint32_t a_faction);
        static int GetFactionRank(uint32_t a_id, uint32_t a_faction);
		static int HasMagicEffect(uint32_t a_id, uint32_t a_magicEffect);
		static std::string GetRelationshipRank(uint32_t a_id, uint32_t a_target);
		static int GetSex(uint32_t a_id);
		static std::string GetName(uint32_t a_id);
	};
}
