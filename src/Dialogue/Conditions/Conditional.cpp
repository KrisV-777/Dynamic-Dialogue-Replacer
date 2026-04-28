#include "Conditional.h"

#include "Util/Script.h"
#include "Util/StringUtil.h"

namespace Conditions
{
    bool Conditional::ConditionsMet(RE::TESObjectREFR* a_subject, RE::TESObjectREFR* a_target) const
    {
        if (!_conditions) {
            return true;
        }
        RE::ConditionCheckParams params{ a_subject, a_target };
        auto ptr = _conditions->head;
        while (ptr) {
            bool result;
            if (ptr->data.flags.isOR) {
                result = ProgressOr(ptr, params);
            } else {
                result = IsTrue(ptr, params);
                ptr = ptr->next;
            }
            if (!result) {
                return false;
            }
        }
        return true;
    }

    bool Conditional::ProgressOr(RE::TESConditionItem*& a_item, RE::ConditionCheckParams& a_params)
    {
        bool res = false;
        bool inOR = true;
        while (a_item && inOR) {
            res = res || IsTrue(a_item, a_params);
            inOR = a_item->data.flags.isOR;
            a_item = a_item->next;
        }
        return res;
    }

    bool Conditional::IsTrue(RE::TESConditionItem* a_item, RE::ConditionCheckParams& a_params)
    {
        // depending on type use custom logic instead
        const auto type = a_item->data.functionData.function.get();
        if (type != RE::FUNCTION_DATA::FunctionID::kGetVMQuestVariable) {
            return a_item->IsTrue(a_params);
        }
        const auto quest = std::bit_cast<RE::TESQuest*>(a_item->data.functionData.params[0]);
        const auto scriptVar = std::bit_cast<RE::BSString*>(a_item->data.functionData.params[1]);
        auto splits = Util::StringSplitToOwned(scriptVar->c_str(), "::");
        if (splits.size() < 2) {
            logger::error("GetVMQuestVariable: Invalid script variable format: {}. Expected format: <script>::<variable>", scriptVar->c_str());
            return false;
        }
        const auto& script = splits[0];
        auto& variable = splits[1];
        variable = variable.ends_with("_var") ? variable.substr(0, variable.size() - 4) : variable;
        auto questObj = Script::GetScriptObject(quest, script.c_str());
        if (!questObj) {
            logger::error("GetVMQuestVariable: Failed to get script object: {} from quest: {}", script, quest->GetFormID());
            return false;
        }
        const auto value = Script::GetTrivialPropertySave<float>(questObj, variable.c_str());
        if (!value) {
            logger::error("GetVMQuestVariable: Failed to get property: {} from script object: {} from quest: {}", variable, script, quest->GetFormID());
            return false;
        }
        const auto comparand = a_item->data.flags.global ? a_item->data.comparisonValue.g->value : a_item->data.comparisonValue.f;
        switch (a_item->data.flags.opCode) {
        case RE::CONDITION_ITEM_DATA::OpCode::kEqualTo:
            return *value == comparand;
        case RE::CONDITION_ITEM_DATA::OpCode::kNotEqualTo:
            return *value != comparand;
        case RE::CONDITION_ITEM_DATA::OpCode::kGreaterThan:
            return *value > comparand;
        case RE::CONDITION_ITEM_DATA::OpCode::kGreaterThanOrEqualTo:
            return *value >= comparand;
        case RE::CONDITION_ITEM_DATA::OpCode::kLessThan:
            return *value < comparand;
        case RE::CONDITION_ITEM_DATA::OpCode::kLessThanOrEqualTo:
            return *value <= comparand;
        default:
            return false;
        }
    }

}  // namespace Condition
