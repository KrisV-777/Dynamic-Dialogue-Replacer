#pragma once

namespace DDR
{
    enum class ReplacementType
    {
        Any = 0,
        Topic = 1,
        Response = 2,

        Total
    };

    struct TextReplacement
    {
        TextReplacement(const YAML::Node& a_node);
        ~TextReplacement() = default;

        _NODISCARD std::string_view GetScript() const { return _script; }
        _NODISCARD bool CanApplyReplacement(RE::TESObjectREFR* a_speaker, RE::TESObjectREFR* a_target, ReplacementType a_type) const;

      private:
        std::string _script;
        RE::FormID _speakerId;
        RE::FormID _targetId;
        ReplacementType _type;

      public:
        bool operator<(const TextReplacement& a_rhs) const noexcept { return _script < a_rhs._script; };
    };

}  // namespace DDR
