#include "TextReplacement.h"

#include "Util/FormLookup.h"

namespace DDR
{
    TextReplacement::TextReplacement(const YAML::Node& a_node) :
      _script(a_node["script"].as<std::string>()),
      _speakerId(Util::FormFromString(a_node["speaker"].as<std::string>(""))),
      _targetId(Util::FormFromString(a_node["target"].as<std::string>(""))),
      _type(magic_enum::enum_cast<ReplacementType>(a_node["type"].as<int>()).or_else([]() -> std::optional<ReplacementType> {
                                                                                throw std::runtime_error("Property 'type' is missing or invalid");
                                                                            })
                .value())
    {
        if (_script.empty()) {
            throw std::runtime_error("Failed to load script");
        }
    }

    bool TextReplacement::CanApplyReplacement(RE::TESObjectREFR* a_speaker, RE::TESObjectREFR* a_target, ReplacementType a_type) const
    {
        if (_type != ReplacementType::Any && _type != a_type)
            return false;
        if (_speakerId != 0) {
            if (!a_speaker || a_speaker->GetFormID() != _speakerId) {
                return false;
            }
        }
        if (_targetId != 0) {
            if (!a_target || a_target->GetFormID() != _targetId) {
                return false;
            }
        }
        return true;
    }

}  // namespace DDR
