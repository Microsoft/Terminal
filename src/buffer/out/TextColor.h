/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- TextColor.h

Abstract:
- contains data for a single color of the text. Text Attributes are composed of
  two of these - one for the foreground and one for the background.
  The color can be in one of three states:
    * Default Colors - The terminal should use the terminal's notion of whatever
      the default color should be for this component.
      It's up to the terminal that's consuming this buffer to control the
      behavior of default attributes.
      Terminals typically have a pair of Default colors that are separate from
      their color table. This component should use that value.
      Consoles also can have a legacy table index as their default colors.
    * Indexed Color - The terminal should use our value as an index into the
      color table to retrieve the real value of the color.
      This is the type of color that "legacy" 16-color attributes have.
    * RGB color - We'll store a real color value in this attribute

Author(s):
- Mike Griese (migrie) Nov 2018

Revision History:
- From components of output.h/.c
  by Therese Stowell (ThereseS) 1990-1991
- Pulled into its own file from textBuffer.hpp/cpp (AustDi, 2017)
- Moved the colors into their own separate abstraction. (migrie Nov 2018)
--*/

#pragma once

#ifdef UNIT_TESTING
#include "WexTestClass.h"
#endif

enum class ColorType : BYTE
{
    IsIndex256 = 0x0,
    IsIndex16 = 0x1,
    IsDefault = 0x2,
    IsRgb = 0x3
};

struct TextColor
{
public:
    constexpr TextColor() noexcept :
        _meta{ ColorType::IsDefault },
        _red{ 0 },
        _green{ 0 },
        _blue{ 0 }
    {
    }

    constexpr TextColor(const BYTE index, const bool isIndex256) noexcept :
        _meta{ isIndex256 ? ColorType::IsIndex256 : ColorType::IsIndex16 },
        _index{ index },
        _green{ 0 },
        _blue{ 0 }
    {
    }

    constexpr TextColor(const COLORREF rgb) noexcept :
        _meta{ ColorType::IsRgb },
        _red{ GetRValue(rgb) },
        _green{ GetGValue(rgb) },
        _blue{ GetBValue(rgb) }
    {
    }

    bool operator==(const TextColor& other) const noexcept
    {
        return memcmp(this, &other, sizeof(TextColor)) == 0;
    }

    bool operator!=(const TextColor& other) const noexcept
    {
        return memcmp(this, &other, sizeof(TextColor)) != 0;
    }

    bool CanBeBrightened() const noexcept;
    bool IsLegacy() const noexcept;
    bool IsIndex16() const noexcept;
    bool IsIndex256() const noexcept;
    bool IsDefault() const noexcept;
    bool IsRgb() const noexcept;

    void SetColor(const COLORREF rgbColor) noexcept;
    void SetIndex(const BYTE index, const bool isIndex256) noexcept;
    void SetDefault() noexcept;

    COLORREF GetColor(gsl::span<const COLORREF> colorTable,
                      const COLORREF defaultColor,
                      const bool brighten = false) const noexcept;

    BYTE GetLegacyIndex(const BYTE defaultIndex) const noexcept;

    constexpr BYTE GetIndex() const noexcept
    {
        return _index;
    }

    COLORREF GetRGB() const noexcept;

private:
    union
    {
        BYTE _red, _index;
    };
    BYTE _green;
    BYTE _blue;
    ColorType _meta;

#ifdef UNIT_TESTING
    friend class TextBufferTests;
    template<typename TextColor>
    friend class WEX::TestExecution::VerifyOutputTraits;
#endif
};

#ifdef UNIT_TESTING

namespace WEX
{
    namespace TestExecution
    {
        template<>
        class VerifyOutputTraits<TextColor>
        {
        public:
            static WEX::Common::NoThrowString ToString(const TextColor& color)
            {
                if (color.IsDefault())
                {
                    return L"{default}";
                }
                else if (color.IsRgb())
                {
                    return WEX::Common::NoThrowString().Format(L"{RGB:0x%06x}", color.GetRGB());
                }
                else
                {
                    return WEX::Common::NoThrowString().Format(L"{index:0x%04x}", color._red);
                }
            }
        };
    }
}
#endif
