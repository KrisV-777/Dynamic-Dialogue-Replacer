#include "TextReplacement.h"

#include "Util/FormLookup.h"
#include "Util/StringUtil.h"

namespace DDR::TextReplacement
{
    namespace
    {
        struct ReplacementTag
        {
            enum class Type
            {
                Actor,
                Relationship,
                Global,
            } type;
            std::string subtag;
            std::string argument;
            bool capitalize;
        };

        RE::TESObjectREFR* ResolveRef(std::string_view a_refName)
        {
            if (Util::IEquals(a_refName, std::string_view{ "Player" })) {
                return RE::PlayerCharacter::GetSingleton();
            }
            return Util::LookupForm<RE::TESObjectREFR>(a_refName);
        }

        std::string GetRefName(RE::TESObjectREFR* a_ref)
        {
            if (!a_ref) {
                return "[...]";
            }
            if (const auto* fullName = a_ref->GetDisplayFullName(); fullName && fullName[0] != '\0') {
                return fullName;
            }
            if (const auto* name = a_ref->GetName(); name && name[0] != '\0') {
                return name;
            }
            return "[...]";
        }

        std::string GetRefShortName(RE::TESObjectREFR* a_ref)
        {
            if (!a_ref) {
                return "[...]";
            }
            if (const auto* actor = a_ref->As<RE::Actor>()) {
                if (const auto* base = actor->GetActorBase(); base && !base->shortName.empty()) {
                    return base->shortName.c_str();
                }
            }
            return GetRefName(a_ref);
        }

        bool IsFemale(RE::TESObjectREFR* a_ref)
        {
            if (const auto* actor = a_ref ? a_ref->As<RE::Actor>() : nullptr) {
                if (const auto* base = actor->GetActorBase()) {
                    return base->GetSex() == RE::SEX::kFemale;
                }
            }
            return false;
        }

        std::string GetRefRace(RE::TESObjectREFR* a_ref)
        {
            if (!a_ref) {
                return "[...]";
            }
            const auto* actor = a_ref->As<RE::Actor>();
            const auto* race = actor ? actor->GetRace() : nullptr;
            if (!race) {
                return "[...]";
            }
            if (const auto* name = race->GetName(); name && name[0] != '\0') {
                return name;
            }
            return "[...]";
        }

        std::string GetRefPronoun(RE::TESObjectREFR* a_ref, std::string_view a_subtag)
        {
            if (!a_ref) {
                return "[...]";
            }

            const bool female = IsFemale(a_ref);
            const auto tag = Util::CastLower(std::string{ a_subtag });

            if (tag == "pronoun") {
                return female ? "she" : "he";
            }
            if (tag == "pronounobj") {
                return female ? "her" : "him";
            }
            if (tag == "pronounpos") {
                return female ? "hers" : "his";
            }
            if (tag == "pronounposobj") {
                return female ? "her" : "his";
            }
            if (tag == "pronounref" || tag == "pronounint") {
                return female ? "herself" : "himself";
            }
            return "[...]";
        }

        std::string GetRelationship(std::string_view a_leftRefArg, std::string_view a_rightRefArg)
        {
            const auto* leftRef = ResolveRef(a_leftRefArg);
            const auto* rightRef = ResolveRef(a_rightRefArg);
            const auto* leftActor = leftRef ? leftRef->As<RE::Actor>() : nullptr;
            const auto* rightActor = rightRef ? rightRef->As<RE::Actor>() : nullptr;
            const auto* leftNpc = leftActor ? leftActor->GetActorBase() : nullptr;
            const auto* rightNpc = rightActor ? rightActor->GetActorBase() : nullptr;
            if (!leftNpc || !rightNpc) {
                return "[...]";
            }

            const auto* rel = RE::BGSRelationship::GetRelationship(const_cast<RE::TESNPC*>(leftNpc), const_cast<RE::TESNPC*>(rightNpc));
            if (!rel || !rel->assocType) {
                return "[...]";
            }

            auto member = RE::BGSAssociationType::Members::kParent;
            if (rel->npc1 == rightNpc && rel->npc2 == leftNpc) {
                member = RE::BGSAssociationType::Members::kChild;
            }

            const auto sex = IsFemale(const_cast<RE::TESObjectREFR*>(leftRef)) ? RE::BGSAssociationType::Sexes::kFemale : RE::BGSAssociationType::Sexes::kMale;
            const auto& label = rel->assocType->associationLabels[member][sex];
            if (!label.empty()) {
                return label.c_str();
            }

            return "[...]";
        }

        std::string FormatGlobalDefault(const RE::TESGlobal* a_global)
        {
            if (!a_global) {
                return "[...]";
            }

            const float value = a_global->value;
            if (a_global->type == RE::TESGlobal::Type::kFloat) {
                return std::format("{:.2f}", value);
            }
            return std::to_string(static_cast<int>(std::round(value)));
        }

        std::string FormatGameDaysSubtag(float a_days, std::string_view a_subtag)
        {
            auto monthName = [](int a_monthIndex) -> std::string {
                switch (a_monthIndex) {
                case 0:
                    return "Morning Star";
                case 1:
                    return "Sun's Dawn";
                case 2:
                    return "First Seed";
                case 3:
                    return "Rain's Hand";
                case 4:
                    return "Second Seed";
                case 5:
                    return "Midyear";
                case 6:
                    return "Sun's Height";
                case 7:
                    return "Last Seed";
                case 8:
                    return "Hearthfire";
                case 9:
                    return "Frostfall";
                case 10:
                    return "Sun's Dusk";
                case 11:
                    return "Evening Star";
                default:
                    return "[...]";
                }
            };

            auto weekDayName = [](int a_weekDayIndex) -> std::string {
                switch (a_weekDayIndex) {
                case 0:
                    return "Morndas";
                case 1:
                    return "Tirdas";
                case 2:
                    return "Middas";
                case 3:
                    return "Turdas";
                case 4:
                    return "Fredas";
                case 5:
                    return "Loredas";
                case 6:
                    return "Sundas";
                default:
                    return "[...]";
                }
            };

            float clampedDays = a_days;
            if (clampedDays < 0.0f) {
                clampedDays = 0.0f;
            }

            const int wholeDays = static_cast<int>(std::floor(clampedDays));
            const float dayFraction = clampedDays - static_cast<float>(wholeDays);
            const int hour24 = static_cast<int>(std::floor(dayFraction * 24.0f)) % 24;
            const int minuteValue = static_cast<int>(std::floor(dayFraction * 24.0f * 60.0f)) % 60;

            int dayOfYear = wholeDays % 365;
            int monthIndex = 0;
            int monthDay = dayOfYear + 1;
            for (int i = 0; i < static_cast<int>(RE::Calendar::Months::kTotal); ++i) {
                const int daysInMonth = RE::Calendar::DAYS_IN_MONTH[i];
                if (dayOfYear < daysInMonth) {
                    monthIndex = i;
                    monthDay = dayOfYear + 1;
                    break;
                }
                dayOfYear -= daysInMonth;
            }

            const auto tag = Util::CastLower(std::string{ a_subtag });
            if (tag == "hour12") {
                int hour12 = hour24 % 12;
                if (hour12 == 0) {
                    hour12 = 12;
                }
                return std::to_string(hour12);
            }
            if (tag == "minutes") {
                return std::to_string(minuteValue);
            }
            if (tag == "month") {
                return std::to_string(monthIndex + 1);
            }
            if (tag == "monthword") {
                return monthName(monthIndex);
            }
            if (tag == "day") {
                return std::to_string(monthDay);
            }
            if (tag == "weekday") {
                return weekDayName(wholeDays % 7);
            }
            if (tag == "year") {
                return std::to_string(1 + wholeDays / 365);
            }
            if (tag == "timespan") {
                if (hour24 < 6) {
                    return "Night";
                }
                if (hour24 < 12) {
                    return "Morning";
                }
                if (hour24 < 18) {
                    return "Afternoon";
                }
                return "Evening";
            }
            if (tag == "meridiem") {
                if (hour24 < 12) {
                    return "AM";
                }
                return "PM";
            }
            if (tag == "time") {
                if (a_days >= 1.0f) {
                    const auto hours = static_cast<int>(std::round(a_days * 24.0f));
                    return std::format("{} Hours", hours);
                }
                const auto mins = static_cast<int>(std::round(a_days * 24.0f * 60.0f));
                return std::format("{} Minutes", mins);
            }

            return "[...]";
        }

        std::string ResolveGlobal(std::string_view a_name, std::string_view a_subtag)
        {
            const auto* global = Util::LookupForm<RE::TESGlobal>(a_name);
            if (!global) {
                return "[...]";
            }

            if (a_subtag.empty()) {
                return FormatGlobalDefault(global);
            }

            return FormatGameDaysSubtag(global->value, a_subtag);
        }

        std::string ResolveRefTag(const ReplacementTag& a_tag)
        {
            const auto* ref = ResolveRef(a_tag.argument);
            if (a_tag.subtag.empty()) {
                return GetRefName(const_cast<RE::TESObjectREFR*>(ref));
            }

            const auto subtag = Util::CastLower(a_tag.subtag);
            if (subtag == "shortname") {
                return GetRefShortName(const_cast<RE::TESObjectREFR*>(ref));
            }
            if (subtag == "race") {
                return GetRefRace(const_cast<RE::TESObjectREFR*>(ref));
            }
            if (subtag.starts_with("pronoun")) {
                return GetRefPronoun(const_cast<RE::TESObjectREFR*>(ref), subtag);
            }

            return "[...]";
        }

        std::optional<ReplacementTag> ParseTag(std::string_view a_tag)
        {
            const auto equalsPos = a_tag.find('=');
            if (equalsPos == std::string_view::npos || equalsPos == 0 || equalsPos + 1 >= a_tag.size()) {
                return std::nullopt;
            }

            auto lhs = Util::TrimCopy(a_tag.substr(0, equalsPos));
            const auto rhs = Util::TrimCopy(a_tag.substr(equalsPos + 1));
            if (lhs.empty() || rhs.empty()) {
                return std::nullopt;
            }

            bool capitalize = false;
            if (lhs.size() >= 3 && Util::IEquals(std::string_view{ lhs }.substr(lhs.size() - 3), std::string_view{ "Cap" })) {
                lhs.resize(lhs.size() - 3);
                capitalize = true;
            }

            const auto dotPos = lhs.find('.');
            const std::string root = dotPos == std::string::npos ? lhs : lhs.substr(0, dotPos);
            const std::string subtag = dotPos == std::string::npos ? "" : lhs.substr(dotPos + 1);

            ReplacementTag::Type type;
            if (Util::IEquals(root, std::string_view{ "Ref" })) {
                type = ReplacementTag::Type::Actor;
            } else if (Util::IEquals(root, std::string_view{ "Relationship" })) {
                type = ReplacementTag::Type::Relationship;
            } else if (Util::IEquals(root, std::string_view{ "Global" })) {
                type = ReplacementTag::Type::Global;
            } else {
                return std::nullopt;
            }

            return ReplacementTag{ type, subtag, rhs, capitalize };
        }
    }

    void ApplyTextReplacement(std::string& a_text)
    {
        if (a_text.empty()) {
            return;
        }

        std::string out{};
        out.reserve(a_text.size());

        size_t pos = 0;
        while (pos < a_text.size()) {
            const auto openPos = a_text.find('<', pos);
            if (openPos == std::string::npos) {
                out.append(a_text.substr(pos));
                break;
            }

            out.append(a_text.substr(pos, openPos - pos));
            const auto closePos = a_text.find('>', openPos + 1);
            if (closePos == std::string::npos) {
                out.append(a_text.substr(openPos));
                break;
            }

            const auto inner = std::string_view{ a_text }.substr(openPos + 1, closePos - openPos - 1);
            std::string replacement{};
            bool replaced = false;

            if (const auto parsed = ParseTag(inner)) {
                switch (parsed->type) {
                case ReplacementTag::Type::Actor:
                    replacement = ResolveRefTag(*parsed);
                    replaced = true;
                    break;
                case ReplacementTag::Type::Relationship:
                    {
                        const auto lhsRefArg = parsed->subtag;
                        const auto rhsRefArg = parsed->argument;
                        if (!lhsRefArg.empty() && !rhsRefArg.empty()) {
                            replacement = GetRelationship(lhsRefArg, rhsRefArg);
                            replaced = true;
                        }
                        break;
                    }
                case ReplacementTag::Type::Global:
                    replacement = ResolveGlobal(parsed->argument, parsed->subtag);
                    replaced = true;
                    break;
                }

                if (replaced && parsed->capitalize) {
                    replacement = Util::CapitalizeFirst(std::move(replacement));
                }
            }

            if (replaced) {
                out.append(replacement);
            } else {
                out.push_back('<');
                out.append(inner);
                out.push_back('>');
            }

            pos = closePos + 1;
        }

        a_text = std::move(out);
    }

}  // namespace DDR::TextReplacement
