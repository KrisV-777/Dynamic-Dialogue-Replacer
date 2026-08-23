#pragma once

#include "Conditions/RefMap.h"
#include "DialoguePaths.h"
#include "Lua/LuaRuntime.h"
#include "Lua/LuaScript.h"
#include "Topic.h"
#include "TopicInfo.h"
#include "Util/Singleton.h"

namespace DDR
{
    class DialogueManager :
      public Singleton<DialogueManager>
    {
      public:
        static RE::TESObjectREFR* GetDialogueTarget(RE::Actor* a_speaker);

      public:
        void Init();
        std::shared_ptr<TopicInfo> FindReplacementResponse(RE::Character* a_speaker, RE::TESTopicInfo* a_topicInfo, RE::TESTopicInfo::TESResponse* a_responseData);
        std::vector<std::shared_ptr<Topic>> FindReplacementTopic(RE::FormID a_parentId, RE::FormID a_topicId, RE::TESObjectREFR* a_target, bool a_preprocessing);

        std::string AddReplacementTopic(RE::FormID a_topicId, std::string a_text);
        void RemoveReplacementTopic(RE::FormID a_topicId, std::string a_key);
        void ApplyLuaScripts(std::string& a_text, RE::TESObjectREFR* a_speaker, LuaScript::Type a_type, RE::TESForm* a_source = nullptr);

      private:
        size_t ParseResponses(const YAML::Node& a_node, const Conditions::RefMap& a_refMap);
        size_t ParseTopics(const YAML::Node& a_node, const Conditions::RefMap& a_refMap);
        size_t ParseScripts(const YAML::Node& a_node);

      private:
        LuaRuntime _luaRuntime{};
        std::map<std::string, std::vector<std::shared_ptr<TopicInfo>>> _responseReplacements;
        std::unordered_map<RE::FormID, std::vector<std::shared_ptr<Topic>>> _topicReplacements;
        std::unordered_map<RE::FormID, std::vector<std::shared_ptr<Topic>>> _topicReplacementOrphans;  // Replacements without a parent topic

        std::unordered_map<RE::FormID, std::string> _tempTopicKeys;
        std::unordered_map<RE::FormID, std::shared_ptr<Topic>> _tempTopicReplacements;
        std::mutex _tempTopicMutex{};
    };
}  // namespace DDR
