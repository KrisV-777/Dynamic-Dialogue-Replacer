#include "ConditionParser.h"
#include "EnumLookup.h"
#include "Util/StringUtil.h"

using namespace Conditions;

// stolen from DAV (https://github.com/Exit-9B/DynamicArmorVariants)

RE::TESConditionItem* ConditionParser::Parse(std::string_view a_text, const RefMap& a_refMap)
{
    const auto splits = Util::StringSplitToOwned(std::string{ a_text }, "<>"sv);
    const std::string text{ splits.size() == 2 ? splits[1] : splits[0] };
    const std::string refStr{ splits.size() == 2 ? splits[0] : "" };

    static const std::regex re{
        R"((\w+)\s+(([\w|.]+)(\s+([\w|.:]+))?\s*)?(==|!=|>|>=|<|<=)\s*(\w+)(\s+(AND|OR))?)"
    };

    std::smatch m;
    if (!std::regex_match(text, m, re)) {
        logger::error("Could not parse condition: {}"sv, a_text);
        return nullptr;
    }

    RE::CONDITION_ITEM_DATA data;
    auto& mFunction = m[1];
    auto& mParam1 = m[3];
    auto& mParam2 = m[5];
    auto& mOperator = m[6];
    auto& mComparand = m[7];
    auto& mConnective = m[9];
    logger::debug("Matching {}. Results: Func: {}, Param1: {}, Param2: {}, Operator: {}, Comparand: {}, Connective: {}",
        text, mFunction.str(), mParam1.str(), mParam2.str(), mOperator.str(), mComparand.str(), mConnective.str());

    auto function = RE::SCRIPT_FUNCTION::LocateScriptCommand(mFunction.str().data());
    if (!function || !function->conditionFunction) {
        logger::error("Did not find condition function: {}"sv, mFunction.str());
        return nullptr;
    }

    auto functionIndex = std::to_underlying(function->output) - 0x1000;
    data.functionData.function = static_cast<RE::FUNCTION_DATA::FunctionID>(functionIndex);

    if (mParam1.matched) {
        if (function->numParams >= 1) {
            data.functionData.params[0] = std::bit_cast<void*>(
                ParseParam(mParam1.str(), function->params[0].paramType.get(), a_refMap));
        } else {
            logger::warn("Condition function {} ignoring parameter: {}", function->functionName, mParam1.str());
        }
    }

    if (mParam2.matched) {
        if (function->numParams >= 2) {
            data.functionData.params[1] = std::bit_cast<void*>(
                ParseParam(mParam2.str(), function->params[1].paramType.get(), a_refMap));
        } else {
            logger::warn("Condition function {} ignoring parameter: {}", function->functionName, mParam2.str());
        }
    }

    if (mOperator.matched) {
        auto op = mOperator.str();
        if (op == "=="s) {
            data.flags.opCode = RE::CONDITION_ITEM_DATA::OpCode::kEqualTo;
        } else if (op == "!="s) {
            data.flags.opCode = RE::CONDITION_ITEM_DATA::OpCode::kNotEqualTo;
        } else if (op == ">"s) {
            data.flags.opCode = RE::CONDITION_ITEM_DATA::OpCode::kGreaterThan;
        } else if (op == ">="s) {
            data.flags.opCode = RE::CONDITION_ITEM_DATA::OpCode::kGreaterThanOrEqualTo;
        } else if (op == "<"s) {
            data.flags.opCode = RE::CONDITION_ITEM_DATA::OpCode::kLessThan;
        } else if (op == "<="s) {
            data.flags.opCode = RE::CONDITION_ITEM_DATA::OpCode::kLessThanOrEqualTo;
        }
    } else {
        data.flags.opCode = RE::CONDITION_ITEM_DATA::OpCode::kNotEqualTo;
    }

    if (mComparand.matched) {
        auto comparand = mComparand.str();
        if (auto global = a_refMap.Lookup<RE::TESGlobal>(comparand)) {
            data.comparisonValue.g = global;
            data.flags.global = true;
        } else {
            data.comparisonValue.f = std::stof(comparand);
        }
    } else {
        data.comparisonValue.f = 0.f;
    }

    if (mConnective.matched) {
        auto connective = mConnective.str();
        if (connective == "OR"s) {
            data.flags.isOR = true;
        }
    }

    if (!refStr.empty()) {
        if (const auto ref = a_refMap.Lookup<RE::TESObjectREFR>(refStr)) {
            data.runOnRef = ref->CreateRefHandle();
            data.object = RE::CONDITIONITEMOBJECT::kRef;
        } else {
            throw std::runtime_error(std::format("Failed to parse subject form: {}", refStr));
        }
    }

    auto conditionItem = new RE::TESConditionItem();
    conditionItem->data = data;
    return conditionItem;
}

std::shared_ptr<RE::TESCondition> ConditionParser::ParseConditions(const std::vector<std::string>& a_rawConditions, const RefMap& a_refMap)
{
    auto condition = std::make_shared<RE::TESCondition>();
    RE::TESConditionItem** head = std::addressof(condition->head);
    int numConditions = 0;
    for (auto& text : a_rawConditions) {
        if (text.empty())
            continue;
        if (auto conditionItem = ConditionParser::Parse(text, a_refMap)) {
            *head = conditionItem;
            head = std::addressof(conditionItem->next);
            numConditions += 1;
        } else {
            throw std::runtime_error("Failed to parse condition: " + std::string(text));
        }
    }
    return numConditions ? condition : nullptr;
}

ConditionParser::ConditionParam ConditionParser::ParseParam(const std::string& a_text, RE::SCRIPT_PARAM_TYPE a_type, const RefMap& a_refMap)
{
    ConditionParam param{};
    switch (a_type) {
    case RE::SCRIPT_PARAM_TYPE::kChar:
    case RE::SCRIPT_PARAM_TYPE::kInt:
    case RE::SCRIPT_PARAM_TYPE::kStage:
    case RE::SCRIPT_PARAM_TYPE::kRelationshipRank:
        param.i = std::stoi(a_text);
        break;
    case RE::SCRIPT_PARAM_TYPE::kFloat:
        param.f = std::stof(a_text);
        break;
    case RE::SCRIPT_PARAM_TYPE::kActorValue:
        param.i = std::to_underlying(EnumLookup::LookupActorValue(a_text));
        break;
    case RE::SCRIPT_PARAM_TYPE::kAxis:
        param.i = EnumLookup::LookupAxis(a_text);
        break;
    case RE::SCRIPT_PARAM_TYPE::kSex:
        param.i = EnumLookup::LookupSex(a_text);
        break;
    case RE::SCRIPT_PARAM_TYPE::kCastingSource:
        param.i = std::to_underlying(EnumLookup::LookupCastingSource(a_text));
        break;
    case RE::SCRIPT_PARAM_TYPE::kVMScriptVar:
        param.str = new RE::BSString(a_text.c_str());
        break;
    default:
        param.form = a_refMap.Lookup(a_text);
        break;
    }
    return param;
}
