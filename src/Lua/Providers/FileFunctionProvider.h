#pragma once

#include "../LuaFunctionProvider.h"

namespace DDR
{
    class FileFunctionProvider final : public ILuaFunctionProvider
    {
      public:
        void Register(sol::state& a_lua) override;

      private:
        static std::string ReadTextFile(const std::string& a_relativePath);
        static sol::object ReadJsonFile(sol::this_state a_state, const std::string& a_relativePath);
    };
}
