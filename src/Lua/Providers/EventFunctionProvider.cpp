#include "EventFunctionProvider.h"

namespace DDR
{
    void EventFunctionProvider::Register(sol::state& a_lua)
    {
        a_lua.set_function("send_mod_event", &EventFunctionProvider::SendModEvent);
    }

    void EventFunctionProvider::SendModEvent(const std::string& a_event, const std::string& a_argStr, float a_argNum, uint32_t a_argForm)
    {
        SKSE::ModCallbackEvent modEvent{
            a_event,
            a_argStr,
            a_argNum,
            a_argForm ? RE::TESForm::LookupByID(a_argForm) : nullptr
        };
        SKSE::GetModCallbackEventSource()->SendEvent(&modEvent);
    }
}
