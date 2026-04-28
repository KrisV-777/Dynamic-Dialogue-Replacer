#include "FormFunctionProvider.h"

namespace DDR
{
    void FormFunctionProvider::Register(sol::state& a_lua)
    {
        a_lua.set_function("get_formid", &FormFunctionProvider::GetFormId);
    }

    uint32_t FormFunctionProvider::GetFormId(uint32_t a_id, const std::string& a_esp)
    {
        return RE::TESDataHandler::GetSingleton()->LookupFormID(a_id, a_esp);
    }
}
