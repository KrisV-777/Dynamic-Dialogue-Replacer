#pragma once

#include "RefMap.h"
#include "Util/FormLookup.h"
#include "Util/StringUtil.h"

// stolen from DAV (https://github.com/Exit-9B/DynamicArmorVariants)
namespace Conditions
{
    class ConditionParser
    {
      public:
        ConditionParser() = delete;

        static RE::TESConditionItem* Parse(std::string_view a_text, const RefMap& a_refMap);
        static std::shared_ptr<RE::TESCondition> ParseConditions(const std::vector<std::string>& a_rawConditions, const RefMap& a_refMap);

      private:
        union ConditionParam
        {
            char c;
            std::int32_t i;
            float f;
            RE::TESForm* form;
            RE::BSString* str;
        };

        static ConditionParam ParseParam(const std::string& a_text, RE::SCRIPT_PARAM_TYPE a_type, const RefMap& a_refMap);
    };
}
