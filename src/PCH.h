#pragma once

#pragma warning(push)
#pragma warning(disable : 4200)
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#ifdef SKSE_SUPPORT_XBYAK
#include <xbyak/xbyak.h>
#endif
#pragma warning(pop)

#include <atomic>
#include <unordered_map>
#include <unordered_set>

#pragma warning(push)
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#pragma warning(pop)

namespace logger = SKSE::log;
namespace fs = std::filesystem;
using namespace std::literals;

#include <detours.h>
#include <magic_enum.hpp>
#include <yaml-cpp/yaml.h>
static_assert(magic_enum::is_magic_enum_supported, "magic_enum is not supported");

namespace stl
{
    using namespace SKSE::stl;

    template <class T>
    void write_thunk_call(std::uintptr_t a_src)
    {
        SKSE::AllocTrampoline(14);
        auto& trampoline = SKSE::GetTrampoline();
        T::func = trampoline.write_call<5>(a_src, T::thunk);
    }

    template <class T>
    void write_thunk_call_6(std::uintptr_t a_src)
    {
        SKSE::AllocTrampoline(14);
        auto& trampoline = SKSE::GetTrampoline();
        T::func = *(uintptr_t*)trampoline.write_call<6>(a_src, T::thunk);
    }

    template <class F, size_t index, class T>
    void write_vfunc()
    {
        REL::Relocation<std::uintptr_t> vtbl{ F::VTABLE[index] };
        T::func = vtbl.write_vfunc(T::size, T::thunk);
    }

    template <std::size_t idx, class T>
    void write_vfunc(REL::VariantID id)
    {
        REL::Relocation<std::uintptr_t> vtbl{ id };
        T::func = vtbl.write_vfunc(idx, T::thunk);
    }

    template <class T>
    void write_thunk_jmp(std::uintptr_t a_src)
    {
        SKSE::AllocTrampoline(14);
        auto& trampoline = SKSE::GetTrampoline();
        T::func = trampoline.write_branch<5>(a_src, T::thunk);
    }

    template <class F, class T>
    void write_vfunc()
    {
        write_vfunc<F, 0, T>();
    }
}

#define DLLEXPORT __declspec(dllexport)
