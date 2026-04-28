#include "LuaRuntime.h"

#include "Providers/ActorFunctionProvider.h"
#include "Providers/EventFunctionProvider.h"
#include "Providers/FileFunctionProvider.h"
#include "Providers/FormFunctionProvider.h"

namespace DDR
{
	LuaRuntime::LuaRuntime()
	{
		_lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::table, sol::lib::math);
		_functionProviders.emplace_back(std::make_unique<FormFunctionProvider>());
		_functionProviders.emplace_back(std::make_unique<ActorFunctionProvider>());
		_functionProviders.emplace_back(std::make_unique<EventFunctionProvider>());
		RegisterFunctions();
	}

	void LuaRuntime::RegisterFunctions()
	{
		for (auto& provider : _functionProviders) {
			provider->Register(_lua);
		}
	}

	bool LuaRuntime::InitializeEnvironment(TextReplacement a_replacement)
	{
		sol::environment env{ _lua, sol::create, _lua.globals() };
		if (!env.valid()) {
			logger::error("Failed to create environment");
			return false;
		}
		const auto scriptName = a_replacement.GetScript();
		const auto scriptPath = std::format("{}/{}", SCRIPT_PATH, scriptName);
		if (!fs::exists(scriptPath)) {
			logger::error("Failed to load script. Invalid path - {}", scriptPath);
			return false;
		}
		_lua.script_file(scriptPath, env);
		if (!env.valid()) {
			logger::error("Failed to load script - {}", scriptPath);
			return false;
		} else if (!env["replace"].valid()) {
			logger::error("Failed to find replace function");
			return false;
		}
		std::string scriptNameStr{ scriptName };
		env.set_function("log_info", [=](const std::string& message) {
			logger::info("Lua - {} - {}", scriptNameStr, message);
		});
		env.set_function("log_error", [=](const std::string& message) {
			logger::error("Lua - {} - {}", scriptNameStr, message);
		});
		if (!env.valid()) {
			logger::error("Failed to set functions");
			return false;
		}
		_scripts.emplace_back(a_replacement, env);
		return true;
	}

	void LuaRuntime::ApplyTextReplacements(std::string& a_text, RE::TESObjectREFR* a_speaker, RE::TESObjectREFR* a_target, ReplacementType a_type, uint32_t a_speakerId, uint32_t a_targetId)
	{
		for (auto& [replacement, environment] : _scripts) {
			if (!replacement.CanApplyReplacement(a_speaker, a_target, a_type)) {
				continue;
			}
			try {
				std::unique_lock lock{ _mutex };
				environment["context"] = std::to_underlying(a_type);
				environment["speaker_id"] = a_speakerId;
				environment["target_id"] = a_targetId;
				sol::protected_function_result result = environment["replace"](a_text);
				if (!result.valid()) {
					sol::error err = result;
					logger::error("Failed to apply replacement - {}", err.what());
				} else if (result.get_type() != sol::type::string) {
					const auto type = magic_enum::enum_name(result.get_type());
					logger::error("Failed to apply replacement - expected string, got {}", type);
				} else {
					a_text = result;
				}
			} catch (const std::exception& e) {
				logger::error("Failed to apply replacement - {}", e.what());
			}
		}
	}
}
