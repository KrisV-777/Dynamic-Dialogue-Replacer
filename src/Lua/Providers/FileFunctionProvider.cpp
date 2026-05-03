#include "FileFunctionProvider.h"

#include "Dialogue/DialoguePaths.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>

namespace DDR
{
    namespace
    {
        fs::path ResolveDataFilePath(const std::string& a_relativePath)
        {
            if (a_relativePath.empty()) {
                throw std::runtime_error("Path cannot be empty");
            }

            const fs::path relativePath{ a_relativePath };
            if (relativePath.is_absolute() || relativePath.has_root_directory() || relativePath.has_root_name()) {
                throw std::runtime_error("Absolute paths are not allowed");
            }

            const fs::path root = fs::weakly_canonical(fs::path{ DIRECTORY_PATH });
            const fs::path candidate = fs::weakly_canonical(root / relativePath);
            const fs::path relativeToRoot = candidate.lexically_relative(root);

            if (relativeToRoot.empty() || *relativeToRoot.begin() == "..") {
                throw std::runtime_error("Path must stay inside Data\\SKSE\\DynamicDialogueReplacer");
            }
            if (!fs::exists(candidate)) {
                throw std::runtime_error(std::format("File does not exist: {}", candidate.string()));
            }
            if (fs::is_directory(candidate)) {
                throw std::runtime_error(std::format("Path points to a directory: {}", candidate.string()));
            }

            return candidate;
        }

        std::string ReadTextFileFromPath(const fs::path& a_path)
        {
            std::ifstream input{ a_path, std::ios::binary };
            if (!input.is_open()) {
                throw std::runtime_error(std::format("Failed to open file: {}", a_path.string()));
            }

            std::ostringstream buffer{};
            buffer << input.rdbuf();
            return buffer.str();
        }

        sol::object JsonToLua(sol::this_state a_state, const nlohmann::json& a_value)
        {
            sol::state_view lua{ a_state };
            if (a_value.is_null()) {
                return sol::make_object(lua, sol::lua_nil);
            }
            if (a_value.is_boolean()) {
                return sol::make_object(lua, a_value.get<bool>());
            }
            if (a_value.is_number_integer()) {
                return sol::make_object(lua, a_value.get<std::int64_t>());
            }
            if (a_value.is_number_unsigned()) {
                return sol::make_object(lua, a_value.get<std::uint64_t>());
            }
            if (a_value.is_number_float()) {
                return sol::make_object(lua, a_value.get<double>());
            }
            if (a_value.is_string()) {
                return sol::make_object(lua, a_value.get<std::string>());
            }

            sol::table table{};
            if (a_value.is_array()) {
                table = lua.create_table(static_cast<int>(a_value.size()), 0);
                int index = 1;
                for (const auto& item : a_value) {
                    table[index++] = JsonToLua(a_state, item);
                }
                return sol::make_object(lua, table);
            }

            table = lua.create_table(0, static_cast<int>(a_value.size()));
            for (const auto& [key, value] : a_value.items()) {
                table[key] = JsonToLua(a_state, value);
            }
            return sol::make_object(lua, table);
        }
    }

    void FileFunctionProvider::Register(sol::state& a_lua)
    {
        a_lua.set_function("read_text_file", &FileFunctionProvider::ReadTextFile);
        a_lua.set_function("read_json_file", &FileFunctionProvider::ReadJsonFile);
    }

    std::string FileFunctionProvider::ReadTextFile(const std::string& a_relativePath)
    {
        try {
            const auto path = ResolveDataFilePath(a_relativePath);
            return ReadTextFileFromPath(path);
        } catch (const std::exception& ex) {
            logger::error("Error reading text file '{}': {}", a_relativePath, ex.what());
            return {};
        }
    }

    sol::object FileFunctionProvider::ReadJsonFile(sol::this_state a_state, const std::string& a_relativePath)
    {
        try {
            const auto path = ResolveDataFilePath(a_relativePath);
            const auto content = ReadTextFileFromPath(path);
            const auto parsed = nlohmann::json::parse(content);
            return JsonToLua(a_state, parsed);
        } catch (const std::exception& ex) {
            logger::error("Error reading JSON file '{}': {}", a_relativePath, ex.what());
            return sol::make_object(sol::state_view{ a_state }, sol::lua_nil);
        }
    }
}
