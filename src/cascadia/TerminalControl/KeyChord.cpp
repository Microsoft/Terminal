// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "KeyChord.h"

#include "KeyChord.g.cpp"

namespace winrt::Microsoft::Terminal::Control::implementation
{
    static Control::KeyModifiers modifiersFromBooleans(bool ctrl, bool alt, bool shift, bool win)
    {
        Control::KeyModifiers modifiers = Control::KeyModifiers::None;
        WI_SetFlagIf(modifiers, Control::KeyModifiers::Ctrl, ctrl);
        WI_SetFlagIf(modifiers, Control::KeyModifiers::Alt, alt);
        WI_SetFlagIf(modifiers, Control::KeyModifiers::Shift, shift);
        WI_SetFlagIf(modifiers, Control::KeyModifiers::Windows, win);
        return modifiers;
    }

    KeyChord::KeyChord(bool ctrl, bool alt, bool shift, bool win, int32_t vkey, int32_t scanCode) noexcept :
        _modifiers{ modifiersFromBooleans(ctrl, alt, shift, win) },
        _vkey{ vkey },
        _scanCode{ scanCode }
    {
    }

    KeyChord::KeyChord(const Control::KeyModifiers& modifiers, int32_t vkey, int32_t scanCode) noexcept :
        _modifiers{ modifiers },
        _vkey{ vkey },
        _scanCode{ scanCode }
    {
    }

    Control::KeyModifiers KeyChord::Modifiers() noexcept
    {
        return _modifiers;
    }

    void KeyChord::Modifiers(Control::KeyModifiers const& value) noexcept
    {
        _modifiers = value;
    }

    int32_t KeyChord::Vkey() noexcept
    {
        return _vkey;
    }

    void KeyChord::Vkey(int32_t value) noexcept
    {
        _vkey = value;
    }

    int32_t KeyChord::ScanCode() noexcept {
        return _scanCode;
    }

    void KeyChord::ScanCode(int32_t value) noexcept {
        _scanCode = value;
    }
}
