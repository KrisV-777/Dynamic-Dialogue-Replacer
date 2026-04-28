#pragma once

#include "Conditions/Conditional.h"
#include "Conditions/RefMap.h"
#include "Util/FormLookup.h"

namespace DDR
{
    class Topic
    {
      public:
        Topic(const YAML::Node& a_node, const Conditions::RefMap& a_refMap);
        Topic(RE::FormID a_id, std::string a_text);
        ~Topic() = default;

        /// @brief FormID of the affected topic
        _NODISCARD RE::FormID GetId() { return _id; }
        _NODISCARD RE::FormID GetAffectedTopic() { return _affectedTopic; }
        /// @brief Check if the topic is affected by the replacement
        _NODISCARD bool AffectsInfoTopic(RE::TESTopic* a_topic) { return a_topic->formID == _affectedTopic; }
        /// @brief Player response to replace the topic with
        _NODISCARD std::string GetText() { return _text; }
        /// @brief If the topic should be hidden (no responses available)
        _NODISCARD bool IsHidden() { return _hide; }
        /// @brief Is this topic relevant when pre-processing the affected dialogue topic
        _NODISCARD bool HasPreProcessingAction() { return _replaceWith || _hide || !_inject.empty(); }
        /// @brief If default processing should continue after edits have been applied (implied true on replacement and false on hide)
        _NODISCARD bool ShouldProceed() { return _proceed; }
        /// @brief The topic to replace the affected topic with
        _NODISCARD RE::TESTopic* GetReplacingTopic() { return RE::TESForm::LookupByID<RE::TESTopic>(_replaceWith); }
        /// @brief Additional response topics to inject into the dialogue
        _NODISCARD const std::vector<RE::TESTopic*>& GetInjections() { return _inject; }
        /// @brief Verify existing conditions met before applying any overrides
        _NODISCARD bool VerifyExistingConditions() { return _check; }
        /// @brief Check if the conditions are met
        _NODISCARD bool ConditionsMet(RE::TESObjectREFR* a_speaker, RE::TESObjectREFR* a_target) { return _conditions.ConditionsMet(a_speaker, a_target); }
        /// @brief ¯\_(ツ)_/¯
        _NODISCARD uint64_t GetPriority() { return _priority; }

      private:
        RE::FormID _id;
        RE::FormID _affectedTopic{ 0 };
        RE::FormID _replaceWith{ 0 };
        std::string _text{ "" };
        std::vector<RE::TESTopic*> _inject{};
        Conditions::Conditional _conditions{};
        uint64_t _priority{ 0 };
        bool _proceed{ true };
        bool _check{ false };
        bool _hide{ false };
    };
}  // namespace DDR
