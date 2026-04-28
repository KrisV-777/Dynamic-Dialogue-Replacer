#include "TopicInfo.h"

namespace DDR
{
    TopicInfo::TopicInfo(const YAML::Node& a_node, const Conditions::RefMap& a_refMap) :
      _topicInfoId(a_refMap.LookupId(a_node["id"].as<std::string>())),
      _responses(a_node["responses"].as<std::vector<Response>>(std::vector<Response>{})),
      _priority(a_node["priority"].as<uint64_t>(0)),
      _conditions(Conditions::Conditional{ a_node["conditions"].as<std::vector<std::string>>(std::vector<std::string>{}), a_refMap }),
      _voiceTypes(a_node["voices"].as<std::vector<std::string>>(std::vector<std::string>{}) |
                  std::ranges::views::transform([](const auto& str) { return RE::TESForm::LookupByEditorID<RE::BGSVoiceType>(str); }) |
                  std::ranges::views::filter([](const auto it) { return it != nullptr; }) |
                  std::ranges::to<std::vector>()),
      _random(a_node["random"].as<std::string>("") == "true" || a_node["random"].as<bool>(false)),
      _cut(a_node["cut"].as<std::string>("true") == "true" || a_node["cut"].as<bool>(true))
    {
        if (_topicInfoId == 0) {
            throw std::runtime_error("Invalid topic info id");
        }
        if (_responses.empty()) {
            const auto err = std::format("Failed to find responses in replacement {}", _topicInfoId);
            throw std::runtime_error(err.c_str());
        }
    }

    std::string TopicInfo::GenerateHash(RE::FormID a_id, const RE::BGSVoiceType* a_voiceType)
    {
        if (!a_voiceType) {
            return "";
        }
        return std::format("{}|{}", a_id, a_voiceType->GetFormEditorID());
    }

    std::string TopicInfo::GenerateHash(RE::FormID a_id)
    {
        return std::format("{}|all", a_id);
    }

    std::vector<std::string> TopicInfo::GetHashes() const
    {
        if (_voiceTypes.empty()) {
            return std::vector<std::string>{ GenerateHash(_topicInfoId) };
        }
        return _voiceTypes | std::ranges::views::transform([this](RE::BGSVoiceType* a_voiceType) {
            return GenerateHash(_topicInfoId, a_voiceType);
        }) | std::ranges::to<std::vector>();
    }

    std::string TopicInfo::GetVoiceFilePath(RE::TESTopic* a_topic, RE::TESTopicInfo* a_topicInfo, RE::BGSVoiceType* a_voiceType, int a_num) const
    {
        const auto path{ _responses[a_num - 1].filePath };
        if (path[0] != '$')
            return path;
        auto delim = path.contains('\\') ? "\\"sv : "/"sv;
        auto sections = Util::StringSplitToOwned(path, delim);
        for (auto& section : sections) {
            if (section == "[VOICE_TYPE]") {
                section = a_voiceType->GetFormEditorID();
            } else if (section == "[TOPIC_MOD_FILE]") {
                section = a_topic->GetFile()->GetFilename();
            } else if (section == "[TOPIC_INFO_MOD_FILE]") {
                section = a_topicInfo->GetFile()->GetFilename();
            } else if (section == "[VOICE_MOD_FILE]") {
                section = a_voiceType->GetDescriptionOwnerFile()->GetFilename();
            }
        }
        return Util::StringJoin(sections, "\\"sv);
    }

}  // namespace DDR
