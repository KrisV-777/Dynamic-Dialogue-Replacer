#include "DialogueManager.h"

#include "Conditions/RefMap.h"
#include "Util/Random.h"

namespace DDR
{
    void DialogueManager::Init()
    {
        logger::info("Initializing replacements");
        std::error_code ec{};
        if (!fs::exists(DIRECTORY_PATH, ec) || fs::is_empty(DIRECTORY_PATH, ec)) {
            logger::error("Error loading replacements in {}. Folder is empty or does not exist - {}", DIRECTORY_PATH, ec.message());
            return;
        }
        for (const auto& entry : fs::directory_iterator(DIRECTORY_PATH)) {
            if (entry.is_directory())
                continue;
            const auto ext = entry.path().extension();
            if (ext != ".yml" && ext != ".yaml") {
                continue;
            }
            const std::string fileName = entry.path().string();
            try {
                logger::info("Loading file {}", fileName);
                const auto file = YAML::LoadFile(fileName);
                const auto refs = file["refMap"].as<std::map<std::string, std::string>>(std::map<std::string, std::string>{});
                const Conditions::RefMap refMap{ refs };
                size_t responses = ParseResponses(file, refMap);
                size_t topics = ParseTopics(file, refMap);
                size_t scripts = ParseScripts(file);
                logger::info("Loaded {} response replacements, {} topic replacements and {} scripts from {}", responses, topics, scripts, fileName);
            } catch (std::exception& e) {
                logger::info("Failed to load {} - {}", fileName, e.what());
            }
        }
        auto sortByPriority = [](auto& map) {
            for (auto& [_, vec] : map) {
                std::ranges::sort(vec, [](const auto& a, const auto& b) { return a->GetPriority() > b->GetPriority(); });
            }
        };
        sortByPriority(_topicReplacements);
        sortByPriority(_topicReplacementOrphans);
        sortByPriority(_responseReplacements);
    }

    size_t DialogueManager::ParseResponses(const YAML::Node& a_node, const Conditions::RefMap& a_refMap)
    {
        const auto node = a_node["topicInfos"];
        if (!node.IsDefined() || !node.IsSequence()) {
            return 0;
        }
        logger::info("Loading TopicInfo replacements");
        size_t responses = 0;
        for (const auto&& it : node) {
            try {
                const auto repl = std::make_shared<TopicInfo>(it, a_refMap);
                for (const auto& hash : repl->GetHashes()) {
                    _responseReplacements[hash].emplace_back(repl);
                }
                responses++;
            } catch (std::exception& e) {
                logger::info("Line {}: Failed to load response replacement - {}", 1 + it.Mark().line, e.what());
            }
        }
        return responses;
    }

    size_t DialogueManager::ParseTopics(const YAML::Node& a_node, const Conditions::RefMap& a_refMap)
    {
        const auto node = a_node["topics"];
        if (!node.IsDefined() || !node.IsSequence()) {
            return 0;
        }
        logger::info("Loading Topic replacements");
        size_t topics = 0;
        for (const auto&& it : node) {
            try {
                const auto repl = std::make_shared<Topic>(it, a_refMap);
                if (auto id = repl->GetId(); id != 0) {
                    _topicReplacements[id].emplace_back(repl);
                } else {
                    _topicReplacementOrphans[repl->GetAffectedTopic()].emplace_back(repl);
                }
                topics++;
            } catch (std::exception& e) {
                logger::info("Line {}: Failed to load topic replacement - {}", 1 + it.Mark().line, e.what());
            }
        }
        return topics;
    }

    size_t DialogueManager::ParseScripts(const YAML::Node& a_node)
    {
        const auto node = a_node["scripts"];
        if (!node.IsDefined() || !node.IsSequence()) {
            return 0;
        }
        size_t scripts = 0;
        for (const auto&& it : node) {
            try {
                TextReplacement repl{ it };
                if (!_luaRuntime.InitializeEnvironment(repl)) {
                    logger::info("Line {}: Failed to initialize environment for script {}", 1 + it.Mark().line, repl.GetScript());
                } else {
                    scripts++;
                }
            } catch (std::exception& e) {
                logger::info("Line {}: Failed to load script - {}", 1 + it.Mark().line, e.what());
            }
        }
        return scripts;
    }


    RE::TESObjectREFR* DialogueManager::GetDialogueTarget(RE::Actor* a_speaker)
    {
        if (const auto& targetHandle = a_speaker->GetActorRuntimeData().dialogueItemTarget) {
            if (const auto& targetPtr = targetHandle.get()) {
                return targetPtr.get();
            }
        }
        return nullptr;
    }

    std::shared_ptr<TopicInfo> DialogueManager::FindReplacementResponse(RE::Character* a_speaker, RE::TESTopicInfo* a_topicInfo, RE::TESTopicInfo::TESResponse*)
    {
        if (!a_topicInfo || !a_speaker) {
            return nullptr;
        }
        // regular convo between actors
        const auto base = a_speaker->GetActorBase();
        const auto voiceType = base ? base->GetVoiceType() : nullptr;
        if (!voiceType) {
            return nullptr;
        }
        RE::TESObjectREFR* target = GetDialogueTarget(a_speaker);
        // try stored overrides next
        const auto key = TopicInfo::GenerateHash(a_topicInfo->GetFormID(), voiceType);
        auto iter = _responseReplacements.find(key);
        if (iter == _responseReplacements.end()) {
            const auto allKey = TopicInfo::GenerateHash(a_topicInfo->GetFormID());
            iter = _responseReplacements.find(allKey);
        }
        if (iter != _responseReplacements.end()) {
            const auto& replacements = iter->second;
            std::vector<std::shared_ptr<TopicInfo>> candidates;
            for (const auto& repl : replacements) {
                const auto rand = repl->IsRandom();
                if ((candidates.empty() || (rand && repl->GetPriority() >= candidates[0]->GetPriority())) && repl->ConditionsMet(a_speaker, target)) {
                    if (rand) {
                        candidates.push_back(repl);
                    } else {
                        return repl;
                    }
                }
            }
            if (!candidates.empty()) {
                return candidates[Random::draw<size_t>(0, candidates.size() - 1)];
            }
        }
        return nullptr;
    }

    std::vector<std::shared_ptr<Topic>> DialogueManager::FindReplacementTopic(RE::FormID a_parentId, RE::FormID a_topicId, RE::TESObjectREFR* a_target, bool a_preprocessing)
    {
        std::vector<std::shared_ptr<Topic>> ret{};
        if (_tempTopicMutex.try_lock()) {
            if (_tempTopicKeys.contains(a_parentId)) {
                ret.push_back(_tempTopicReplacements[a_parentId]);
            }
            _tempTopicMutex.unlock();
        }
        const auto player = RE::PlayerCharacter::GetSingleton();
        const auto append = [&](const auto& topicMap, const auto findId) {
            auto iter = topicMap.find(findId);
            if (iter == topicMap.end())
                return;
            const auto& replacements = iter->second;
            for (const auto& repl : replacements) {
                if (a_preprocessing && !repl->HasPreProcessingAction())
                    continue;
                if (repl->ConditionsMet(a_target, player)) {
                    ret.push_back(repl);
                }
            }
        };
        append(_topicReplacements, a_parentId);
        append(_topicReplacementOrphans, a_topicId);
        return ret;
    }

    std::string DialogueManager::AddReplacementTopic(RE::FormID a_topicId, std::string a_text)
    {
        std::unique_lock lock{ _tempTopicMutex };
        std::string key{ Random::generateUUID() };
        if (_tempTopicKeys.count(a_topicId)) {
            logger::info("overwrite detected on {} - previous key = {}", a_topicId, _tempTopicKeys.count(a_topicId));
        }
        _tempTopicKeys[a_topicId] = key;
        _tempTopicReplacements[a_topicId] = std::make_shared<Topic>(a_topicId, a_text);
        return key;
    }

    void DialogueManager::RemoveReplacementTopic(RE::FormID a_topicId, std::string a_key)
    {
        std::unique_lock lock{ _tempTopicMutex };

        if (_tempTopicKeys.count(a_topicId) && _tempTopicKeys[a_topicId] != a_key) {
            return;
        }

        _tempTopicKeys.erase(a_topicId);
        _tempTopicReplacements.erase(a_topicId);
    }

    void DialogueManager::ApplyTextReplacements(std::string& a_text, RE::TESObjectREFR* a_speaker, ReplacementType a_type)
    {
        if (a_text.empty()) {
            return;
        }
        const auto actor = a_speaker ? a_speaker->As<RE::Actor>() : nullptr;
        const auto target = actor ? GetDialogueTarget(actor) : nullptr;
        const uint32_t speakerId = actor ? actor->GetFormID() : 0;
        const uint32_t targetId = target ? target->GetFormID() : 0;
        _luaRuntime.ApplyTextReplacements(a_text, a_speaker, target, a_type, speakerId, targetId);
    }

}  // namespace DDR
