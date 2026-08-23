#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <lua.hpp>
#include <sol/sol.hpp>

#include "LuaFunctionProvider.h"
#include "LuaScript.h"

#include "Dialogue/DialoguePaths.h"

namespace DDR
{
    class LuaRuntime
    {
      public:
        LuaRuntime();
        ~LuaRuntime() { _lua.collect_garbage(); }

        bool InitializeEnvironment(LuaScript a_replacement);
        void ApplyScripts(
          std::string& a_text,
          RE::TESObjectREFR* a_speaker,
          RE::TESObjectREFR* a_target,
          LuaScript::Type a_type,
          uint32_t a_speakerId,
          uint32_t a_targetId,
          uint32_t a_sourceId,
          const std::string& a_sourcePlugin);

      private:
        void RegisterFunctions();

      private:
        sol::state _lua{};
        std::vector<std::pair<LuaScript, sol::environment>> _scripts{};
        std::vector<std::unique_ptr<ILuaFunctionProvider>> _functionProviders{};
        std::mutex _mutex{};
    };
}
