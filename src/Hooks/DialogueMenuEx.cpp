#include "DialogueMenuEx.h"

#include "Dialogue/DialogueManager.h"
#include "Dialogue/TextReplacement.h"

namespace DDR
{
    RE::UI_MESSAGE_RESULTS DialogueMenuEx::ProcessMessageEx(RE::UIMessage& a_message)
    {
        static std::map<RE::FormID, std::string> cache{};
        static RE::FormID _activeRootId{ 0 };
        const auto menu = RE::MenuTopicManager::GetSingleton();
        const auto manager = DialogueManager::GetSingleton();
        const auto rootId = menu->rootTopicInfo ? menu->rootTopicInfo->GetFormID() : 0xFFFFFFFF;
        switch (*a_message.type) {
        case RE::UI_MESSAGE_TYPE::kShow:
        case RE::UI_MESSAGE_TYPE::kUpdate:
            _activeRootId = 0;
            __fallthrough;
        default:
            if (_activeRootId != rootId) {
                _activeRootId = rootId;
                cache.clear();
            }
            if (const auto dialogue = menu->dialogueList) {
#pragma warning(suppress : 4834)
                for (auto it = dialogue->begin(); it != dialogue->end(); it++) {
                    const auto activeTopic = *it;
                    if (!activeTopic) {
                        continue;
                    }
                    const auto formId = activeTopic->parentTopic->GetFormID();
                    auto where = cache.find(formId);
                    if (where != cache.end()) {
                        activeTopic->topicText = where->second;
                        continue;
                    }
                    const auto speaker = menu->speaker.get().get();
                    auto topics = manager->FindReplacementTopic(formId, 0, speaker, false);
                    std::string text{ activeTopic->topicText.c_str() };
                    for (auto&& topic : topics) {
                        if (!topic->GetText().empty()) {
                            text = topic->GetText();
                            break;
                        }
                    }
                    manager->ApplyLuaScripts(text, speaker, LuaScript::Type::Topic);
                    TextReplacement::ApplyTextReplacement(text);
                    activeTopic->topicText = text;
                    cache[formId] = text;
                }
            }
            break;
        case RE::UI_MESSAGE_TYPE::kForceHide:
        case RE::UI_MESSAGE_TYPE::kHide:
            _activeRootId = 0;
            cache.clear();
            break;
        }
        return _ProcessMessageFn(this, a_message);
    }

}