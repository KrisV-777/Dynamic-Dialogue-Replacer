#pragma once

namespace DDR
{
    class DialogueMenuEx : public RE::DialogueMenu
    {
      public:
        static inline void Install()
        {
            REL::Relocation<uintptr_t> vtbl(RE::VTABLE_DialogueMenu[0]);
            _ProcessMessageFn = vtbl.write_vfunc(0x4, &ProcessMessageEx);
        }

        RE::UI_MESSAGE_RESULTS ProcessMessageEx(RE::UIMessage& a_message);

      private:
        using ProcessMessageFn = decltype(&RE::DialogueMenu::ProcessMessage);
        static inline REL::Relocation<ProcessMessageFn> _ProcessMessageFn;
    };
}