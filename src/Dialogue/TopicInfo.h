#pragma once

#include "Conditions/Conditional.h"
#include "Conditions/RefMap.h"
#include "Util/FormLookup.h"
#include "Util/StringUtil.h"

namespace DDR
{
    struct Response
    {
        bool keep;
        std::string subtitle;
        std::string filePath;
    };

    class TopicInfo
    {
      public:
        TopicInfo(const YAML::Node& a_node, const Conditions::RefMap& a_refMap);
        ~TopicInfo() = default;

        _NODISCARD static std::string GenerateHash(RE::FormID a_id, const RE::BGSVoiceType* a_voiceType);
        _NODISCARD static std::string GenerateHash(RE::FormID a_id);
        _NODISCARD std::vector<std::string> GetHashes() const;

        _NODISCARD inline bool HasReplacement(int a_num) const { return a_num <= _responses.size() && !_responses[a_num - 1].keep; }
        _NODISCARD inline bool HasReplacementSubtitle(int a_num) const { return HasReplacement(a_num) && !_responses[a_num - 1].subtitle.empty(); }
        _NODISCARD inline bool HasReplacementVoiceFile(int a_num) const { return HasReplacement(a_num) && !_responses[a_num - 1].filePath.empty(); }

        _NODISCARD std::string GetVoiceFilePath(RE::TESTopic* a_topic, RE::TESTopicInfo* a_topicInfo, RE::BGSVoiceType* a_voiceType, int a_num) const;
        _NODISCARD inline std::string GetSubtitle(int a_num) const { return _responses[a_num - 1].subtitle; }
        _NODISCARD inline bool IsRandom() const { return _random; }
        _NODISCARD inline uint64_t GetPriority() const { return _priority; }
        _NODISCARD inline bool ShouldCut(int a_num) const { return _cut && a_num >= _responses.size(); }
        _NODISCARD inline bool ConditionsMet(RE::TESObjectREFR* a_subject, RE::TESObjectREFR* a_target) const { return _conditions.ConditionsMet(a_subject, a_target); }

      private:
        RE::FormID _topicInfoId;
        std::vector<Response> _responses;
        std::vector<RE::BGSVoiceType*> _voiceTypes{};
        Conditions::Conditional _conditions{};
        uint64_t _priority{ 0 };
        bool _random{ false };
        bool _cut{ true };
    };
}

namespace YAML
{
    template <>
    struct convert<DDR::Response>
    {
        static bool decode(const Node& node, DDR::Response& rhs)
        {
            if (node["subtitle"].IsDefined()) {
                rhs.subtitle = node["subtitle"].as<std::string>("");
            } else {
                rhs.subtitle = node["sub"].as<std::string>("");
            }
            rhs.filePath = node["path"].as<std::string>("");
            rhs.keep = node["keep"].as<std::string>("") == "true" || node["keep"].as<bool>(false);
            return true;
        }
    };
}
