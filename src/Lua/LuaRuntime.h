#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <lua.hpp>
#include <sol/sol.hpp>

#include <memory>

#include "LuaFunctionProvider.h"

#include "Dialogue/DialoguePaths.h"
#include "Dialogue/TextReplacement.h"

namespace DDR
{
    class LuaRuntime
    {
      public:
        LuaRuntime();
        ~LuaRuntime() { _lua.collect_garbage(); }

        bool InitializeEnvironment(TextReplacement a_replacement);
        void ApplyTextReplacements(std::string& a_text, RE::TESObjectREFR* a_speaker, RE::TESObjectREFR* a_target, ReplacementType a_type, uint32_t a_speakerId, uint32_t a_targetId);

      private:
        void RegisterFunctions();

      private:
        sol::state _lua{};
        std::vector<std::pair<TextReplacement, sol::environment>> _scripts{};
        std::vector<std::unique_ptr<ILuaFunctionProvider>> _functionProviders{};
        std::mutex _mutex{};
    };
}
