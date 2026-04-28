#include "ActorFunctionProvider.h"

namespace DDR
{
	void ActorFunctionProvider::Register(sol::state& a_lua)
	{
		a_lua.set_function("has_keyword", &ActorFunctionProvider::HasKeyword);
		a_lua.set_function("is_in_faction", &ActorFunctionProvider::IsInFaction);
        a_lua.set_function("get_faction_rank", &ActorFunctionProvider::GetFactionRank);
		a_lua.set_function("has_magic_effect", &ActorFunctionProvider::HasMagicEffect);
		a_lua.set_function("get_relationship_rank", &ActorFunctionProvider::GetRelationshipRank);
		a_lua.set_function("get_sex", &ActorFunctionProvider::GetSex);
		a_lua.set_function("get_name", &ActorFunctionProvider::GetName);
	}

	int ActorFunctionProvider::HasKeyword(uint32_t a_id, const std::string& a_kwd, bool a_partialMatch)
	{
		auto form = RE::TESForm::LookupByID<RE::BGSKeywordForm>(a_id);
		if (!form) {
			return -1;
		}
		return a_partialMatch ? form->ContainsKeywordString(a_kwd) : form->HasKeywordString(a_kwd);
	}

	int ActorFunctionProvider::IsInFaction(uint32_t a_id, uint32_t a_faction)
	{
		auto form = RE::TESForm::LookupByID<RE::Actor>(a_id);
		auto fac = RE::TESForm::LookupByID<RE::TESFaction>(a_faction);
		if (!form || !fac) {
			return -1;
		}
		return form->IsInFaction(fac);
	}

    int ActorFunctionProvider::GetFactionRank(uint32_t a_id, uint32_t a_faction)
    {
        auto form = RE::TESForm::LookupByID<RE::Actor>(a_id);
        auto fac = RE::TESForm::LookupByID<RE::TESFaction>(a_faction);
        if (!form || !fac) {
            return -1;
        }
        return form->GetFactionRank(fac, form->IsPlayerRef());
    }

	int ActorFunctionProvider::HasMagicEffect(uint32_t a_id, uint32_t a_magicEffect)
	{
		auto form = RE::TESForm::LookupByID<RE::Actor>(a_id);
		auto mgEff = RE::TESForm::LookupByID<RE::EffectSetting>(a_magicEffect);
		if (!form || !mgEff) {
			return -1;
		}
		return form->AsMagicTarget()->HasMagicEffect(mgEff);
	}

	std::string ActorFunctionProvider::GetRelationshipRank(uint32_t a_id, uint32_t a_target)
	{
		auto form = RE::TESForm::LookupByID<RE::Actor>(a_id);
		auto target = RE::TESForm::LookupByID<RE::Actor>(a_target);
		if (!form || !target) {
			return "";
		}
		auto formBase = form->GetActorBase();
		auto targetBase = target->GetActorBase();
		if (!formBase || !targetBase || !formBase->relationships) {
			return "";
		}
		for (auto&& it : *formBase->relationships) {
			if (it->npc1 == targetBase || it->npc2 == targetBase) {
				auto lv = it->level.get();
				std::string ret{ magic_enum::enum_name(lv) };
				return ret;
			}
		}
		return "";
	}

	int ActorFunctionProvider::GetSex(uint32_t a_id)
	{
		auto form = RE::TESForm::LookupByID(a_id);
		if (!form) {
			return -1;
		} else if (auto act = form->As<RE::Actor>()) {
			auto base = act->GetActorBase();
			return base ? base->GetSex() : -1;
		} else if (auto npc = form->As<RE::TESNPC>()) {
			return npc->GetSex();
		}
		return -1;
	}

	std::string ActorFunctionProvider::GetName(uint32_t a_id)
	{
		auto form = RE::TESForm::LookupByID(a_id);
		if (!form) {
			return "";
		}
		std::string ret{ form->GetName() };
		if (ret.empty()) {
			if (auto act = form->As<RE::Actor>()) {
				const auto base = act->GetActorBase();
				return base ? base->GetName() : ret;
			}
		}
		return ret;
	}
}
