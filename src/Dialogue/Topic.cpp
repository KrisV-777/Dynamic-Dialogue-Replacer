#include "Topic.h"

namespace DDR
{
    Topic::Topic(const YAML::Node& a_node, const Conditions::RefMap& a_refMap) :
      _id(a_refMap.LookupId(a_node["id"].as<std::string>(""))),
      _affectedTopic(a_refMap.LookupId(a_node["affects"].as<std::string>(""))),
      _replaceWith(a_refMap.LookupId(a_node["replace"].IsDefined() ? a_node["replace"].as<std::string>("") : a_node["with"].as<std::string>(""))),
      _text(a_node["text"].as<std::string>("")),
      _inject(a_node["inject"].as<std::vector<std::string>>(std::vector<std::string>{}) |
              std::ranges::views::transform([&](const auto& str) { return a_refMap.Lookup<RE::TESTopic>(str); }) |
              std::ranges::views::filter([](const auto it) { return it != nullptr; }) |
              std::ranges::to<std::vector>()),
      _conditions(Conditions::Conditional{ a_node["conditions"].as<std::vector<std::string>>(std::vector<std::string>{}), a_refMap }),
      _priority(a_node["priority"].as<uint64_t>(0)),
      _proceed(a_node["proceed"].as<std::string>("true") == "true" || a_node["proceed"].as<bool>(true)),
      _check(a_node["check"].as<std::string>("") == "true" || a_node["check"].as<bool>(false)),
      _hide(a_node["hide"].as<std::string>("") == "true" || a_node["hide"].as<bool>(false))
    {
        if (_id != 0 && !RE::TESForm::LookupByID<RE::TESTopic>(_id)) {
            throw std::runtime_error("Invalid topic id");
        }
        if (_affectedTopic && !RE::TESForm::LookupByID<RE::TESTopic>(_affectedTopic)) {
            throw std::runtime_error("Invalid affected topic id");
        }
        if (_replaceWith) {
            if (!RE::TESForm::LookupByID<RE::TESTopic>(_replaceWith)) {
                throw std::runtime_error("Failed to obtain replacement topic");
            } else if (!_affectedTopic) {
                throw std::runtime_error("Missing affected topic. Replacement must specify a topic to replace");
            }
        }
        if (_hide && _replaceWith) {
            throw std::runtime_error("Replacement and hide cannot be used together");
        }
        if (_hide && !_affectedTopic) {
            throw std::runtime_error("Hide must specify a topic to hide");
        }
        if (!_inject.empty() && _id == 0) {
            throw std::runtime_error("Injection topics must specify a topic id");
        }
        if (_text.empty() && !_replaceWith && _inject.empty() && !_hide) {
            throw std::runtime_error("Invalid topic: no text, replacement, injection, or hide flag specified. This topic does nothing.");
        }
    }

    Topic::Topic(RE::FormID a_id, std::string a_text) :
      _id(a_id), _text(a_text)
    {
        if (_id == 0 || !RE::TESForm::LookupByID<RE::TESTopic>(_id)) {
            throw std::runtime_error("Failed to obtain topic");
        }
    }
}  // namespace DDR
