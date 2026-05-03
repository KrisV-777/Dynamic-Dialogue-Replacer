#pragma once

namespace DDR
{
    struct LuaScript
    {
        enum class Type
        {
            Any = 0,
            Topic = 1,
            Response = 2,

            Total
        };

        public:
        LuaScript(const YAML::Node& a_node);
        ~LuaScript() = default;

        _NODISCARD std::string_view GetScript() const { return _script; }
        _NODISCARD bool CanApplyReplacement(RE::TESObjectREFR* a_speaker, RE::TESObjectREFR* a_target, Type a_type) const;

      private:
        std::string _script;
        RE::FormID _speakerId;
        RE::FormID _targetId;
        Type _type;

      public:
        bool operator<(const LuaScript& a_rhs) const noexcept { return _script < a_rhs._script; };
    };

}  // namespace DDR
