// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "precomp.h"
#include "inc/utils.hpp"
#include "til/static_map.h"

using namespace std::string_view_literals;
using namespace Microsoft::Console;

static constexpr std::array<til::color, 16> campbellColorTable{
    til::color{ 0x0C, 0x0C, 0x0C },
    til::color{ 0xC5, 0x0F, 0x1F },
    til::color{ 0x13, 0xA1, 0x0E },
    til::color{ 0xC1, 0x9C, 0x00 },
    til::color{ 0x00, 0x37, 0xDA },
    til::color{ 0x88, 0x17, 0x98 },
    til::color{ 0x3A, 0x96, 0xDD },
    til::color{ 0xCC, 0xCC, 0xCC },
    til::color{ 0x76, 0x76, 0x76 },
    til::color{ 0xE7, 0x48, 0x56 },
    til::color{ 0x16, 0xC6, 0x0C },
    til::color{ 0xF9, 0xF1, 0xA5 },
    til::color{ 0x3B, 0x78, 0xFF },
    til::color{ 0xB4, 0x00, 0x9E },
    til::color{ 0x61, 0xD6, 0xD6 },
    til::color{ 0xF2, 0xF2, 0xF2 },
};

static constexpr std::array<til::color, 256> standardXterm256ColorTable{
    til::color{ 0x00, 0x00, 0x00 },
    til::color{ 0x80, 0x00, 0x00 },
    til::color{ 0x00, 0x80, 0x00 },
    til::color{ 0x80, 0x80, 0x00 },
    til::color{ 0x00, 0x00, 0x80 },
    til::color{ 0x80, 0x00, 0x80 },
    til::color{ 0x00, 0x80, 0x80 },
    til::color{ 0xC0, 0xC0, 0xC0 },
    til::color{ 0x80, 0x80, 0x80 },
    til::color{ 0xFF, 0x00, 0x00 },
    til::color{ 0x00, 0xFF, 0x00 },
    til::color{ 0xFF, 0xFF, 0x00 },
    til::color{ 0x00, 0x00, 0xFF },
    til::color{ 0xFF, 0x00, 0xFF },
    til::color{ 0x00, 0xFF, 0xFF },
    til::color{ 0xFF, 0xFF, 0xFF },
    til::color{ 0x00, 0x00, 0x00 },
    til::color{ 0x00, 0x00, 0x5F },
    til::color{ 0x00, 0x00, 0x87 },
    til::color{ 0x00, 0x00, 0xAF },
    til::color{ 0x00, 0x00, 0xD7 },
    til::color{ 0x00, 0x00, 0xFF },
    til::color{ 0x00, 0x5F, 0x00 },
    til::color{ 0x00, 0x5F, 0x5F },
    til::color{ 0x00, 0x5F, 0x87 },
    til::color{ 0x00, 0x5F, 0xAF },
    til::color{ 0x00, 0x5F, 0xD7 },
    til::color{ 0x00, 0x5F, 0xFF },
    til::color{ 0x00, 0x87, 0x00 },
    til::color{ 0x00, 0x87, 0x5F },
    til::color{ 0x00, 0x87, 0x87 },
    til::color{ 0x00, 0x87, 0xAF },
    til::color{ 0x00, 0x87, 0xD7 },
    til::color{ 0x00, 0x87, 0xFF },
    til::color{ 0x00, 0xAF, 0x00 },
    til::color{ 0x00, 0xAF, 0x5F },
    til::color{ 0x00, 0xAF, 0x87 },
    til::color{ 0x00, 0xAF, 0xAF },
    til::color{ 0x00, 0xAF, 0xD7 },
    til::color{ 0x00, 0xAF, 0xFF },
    til::color{ 0x00, 0xD7, 0x00 },
    til::color{ 0x00, 0xD7, 0x5F },
    til::color{ 0x00, 0xD7, 0x87 },
    til::color{ 0x00, 0xD7, 0xAF },
    til::color{ 0x00, 0xD7, 0xD7 },
    til::color{ 0x00, 0xD7, 0xFF },
    til::color{ 0x00, 0xFF, 0x00 },
    til::color{ 0x00, 0xFF, 0x5F },
    til::color{ 0x00, 0xFF, 0x87 },
    til::color{ 0x00, 0xFF, 0xAF },
    til::color{ 0x00, 0xFF, 0xD7 },
    til::color{ 0x00, 0xFF, 0xFF },
    til::color{ 0x5F, 0x00, 0x00 },
    til::color{ 0x5F, 0x00, 0x5F },
    til::color{ 0x5F, 0x00, 0x87 },
    til::color{ 0x5F, 0x00, 0xAF },
    til::color{ 0x5F, 0x00, 0xD7 },
    til::color{ 0x5F, 0x00, 0xFF },
    til::color{ 0x5F, 0x5F, 0x00 },
    til::color{ 0x5F, 0x5F, 0x5F },
    til::color{ 0x5F, 0x5F, 0x87 },
    til::color{ 0x5F, 0x5F, 0xAF },
    til::color{ 0x5F, 0x5F, 0xD7 },
    til::color{ 0x5F, 0x5F, 0xFF },
    til::color{ 0x5F, 0x87, 0x00 },
    til::color{ 0x5F, 0x87, 0x5F },
    til::color{ 0x5F, 0x87, 0x87 },
    til::color{ 0x5F, 0x87, 0xAF },
    til::color{ 0x5F, 0x87, 0xD7 },
    til::color{ 0x5F, 0x87, 0xFF },
    til::color{ 0x5F, 0xAF, 0x00 },
    til::color{ 0x5F, 0xAF, 0x5F },
    til::color{ 0x5F, 0xAF, 0x87 },
    til::color{ 0x5F, 0xAF, 0xAF },
    til::color{ 0x5F, 0xAF, 0xD7 },
    til::color{ 0x5F, 0xAF, 0xFF },
    til::color{ 0x5F, 0xD7, 0x00 },
    til::color{ 0x5F, 0xD7, 0x5F },
    til::color{ 0x5F, 0xD7, 0x87 },
    til::color{ 0x5F, 0xD7, 0xAF },
    til::color{ 0x5F, 0xD7, 0xD7 },
    til::color{ 0x5F, 0xD7, 0xFF },
    til::color{ 0x5F, 0xFF, 0x00 },
    til::color{ 0x5F, 0xFF, 0x5F },
    til::color{ 0x5F, 0xFF, 0x87 },
    til::color{ 0x5F, 0xFF, 0xAF },
    til::color{ 0x5F, 0xFF, 0xD7 },
    til::color{ 0x5F, 0xFF, 0xFF },
    til::color{ 0x87, 0x00, 0x00 },
    til::color{ 0x87, 0x00, 0x5F },
    til::color{ 0x87, 0x00, 0x87 },
    til::color{ 0x87, 0x00, 0xAF },
    til::color{ 0x87, 0x00, 0xD7 },
    til::color{ 0x87, 0x00, 0xFF },
    til::color{ 0x87, 0x5F, 0x00 },
    til::color{ 0x87, 0x5F, 0x5F },
    til::color{ 0x87, 0x5F, 0x87 },
    til::color{ 0x87, 0x5F, 0xAF },
    til::color{ 0x87, 0x5F, 0xD7 },
    til::color{ 0x87, 0x5F, 0xFF },
    til::color{ 0x87, 0x87, 0x00 },
    til::color{ 0x87, 0x87, 0x5F },
    til::color{ 0x87, 0x87, 0x87 },
    til::color{ 0x87, 0x87, 0xAF },
    til::color{ 0x87, 0x87, 0xD7 },
    til::color{ 0x87, 0x87, 0xFF },
    til::color{ 0x87, 0xAF, 0x00 },
    til::color{ 0x87, 0xAF, 0x5F },
    til::color{ 0x87, 0xAF, 0x87 },
    til::color{ 0x87, 0xAF, 0xAF },
    til::color{ 0x87, 0xAF, 0xD7 },
    til::color{ 0x87, 0xAF, 0xFF },
    til::color{ 0x87, 0xD7, 0x00 },
    til::color{ 0x87, 0xD7, 0x5F },
    til::color{ 0x87, 0xD7, 0x87 },
    til::color{ 0x87, 0xD7, 0xAF },
    til::color{ 0x87, 0xD7, 0xD7 },
    til::color{ 0x87, 0xD7, 0xFF },
    til::color{ 0x87, 0xFF, 0x00 },
    til::color{ 0x87, 0xFF, 0x5F },
    til::color{ 0x87, 0xFF, 0x87 },
    til::color{ 0x87, 0xFF, 0xAF },
    til::color{ 0x87, 0xFF, 0xD7 },
    til::color{ 0x87, 0xFF, 0xFF },
    til::color{ 0xAF, 0x00, 0x00 },
    til::color{ 0xAF, 0x00, 0x5F },
    til::color{ 0xAF, 0x00, 0x87 },
    til::color{ 0xAF, 0x00, 0xAF },
    til::color{ 0xAF, 0x00, 0xD7 },
    til::color{ 0xAF, 0x00, 0xFF },
    til::color{ 0xAF, 0x5F, 0x00 },
    til::color{ 0xAF, 0x5F, 0x5F },
    til::color{ 0xAF, 0x5F, 0x87 },
    til::color{ 0xAF, 0x5F, 0xAF },
    til::color{ 0xAF, 0x5F, 0xD7 },
    til::color{ 0xAF, 0x5F, 0xFF },
    til::color{ 0xAF, 0x87, 0x00 },
    til::color{ 0xAF, 0x87, 0x5F },
    til::color{ 0xAF, 0x87, 0x87 },
    til::color{ 0xAF, 0x87, 0xAF },
    til::color{ 0xAF, 0x87, 0xD7 },
    til::color{ 0xAF, 0x87, 0xFF },
    til::color{ 0xAF, 0xAF, 0x00 },
    til::color{ 0xAF, 0xAF, 0x5F },
    til::color{ 0xAF, 0xAF, 0x87 },
    til::color{ 0xAF, 0xAF, 0xAF },
    til::color{ 0xAF, 0xAF, 0xD7 },
    til::color{ 0xAF, 0xAF, 0xFF },
    til::color{ 0xAF, 0xD7, 0x00 },
    til::color{ 0xAF, 0xD7, 0x5F },
    til::color{ 0xAF, 0xD7, 0x87 },
    til::color{ 0xAF, 0xD7, 0xAF },
    til::color{ 0xAF, 0xD7, 0xD7 },
    til::color{ 0xAF, 0xD7, 0xFF },
    til::color{ 0xAF, 0xFF, 0x00 },
    til::color{ 0xAF, 0xFF, 0x5F },
    til::color{ 0xAF, 0xFF, 0x87 },
    til::color{ 0xAF, 0xFF, 0xAF },
    til::color{ 0xAF, 0xFF, 0xD7 },
    til::color{ 0xAF, 0xFF, 0xFF },
    til::color{ 0xD7, 0x00, 0x00 },
    til::color{ 0xD7, 0x00, 0x5F },
    til::color{ 0xD7, 0x00, 0x87 },
    til::color{ 0xD7, 0x00, 0xAF },
    til::color{ 0xD7, 0x00, 0xD7 },
    til::color{ 0xD7, 0x00, 0xFF },
    til::color{ 0xD7, 0x5F, 0x00 },
    til::color{ 0xD7, 0x5F, 0x5F },
    til::color{ 0xD7, 0x5F, 0x87 },
    til::color{ 0xD7, 0x5F, 0xAF },
    til::color{ 0xD7, 0x5F, 0xD7 },
    til::color{ 0xD7, 0x5F, 0xFF },
    til::color{ 0xD7, 0x87, 0x00 },
    til::color{ 0xD7, 0x87, 0x5F },
    til::color{ 0xD7, 0x87, 0x87 },
    til::color{ 0xD7, 0x87, 0xAF },
    til::color{ 0xD7, 0x87, 0xD7 },
    til::color{ 0xD7, 0x87, 0xFF },
    til::color{ 0xD7, 0xAF, 0x00 },
    til::color{ 0xD7, 0xAF, 0x5F },
    til::color{ 0xD7, 0xAF, 0x87 },
    til::color{ 0xD7, 0xAF, 0xAF },
    til::color{ 0xD7, 0xAF, 0xD7 },
    til::color{ 0xD7, 0xAF, 0xFF },
    til::color{ 0xD7, 0xD7, 0x00 },
    til::color{ 0xD7, 0xD7, 0x5F },
    til::color{ 0xD7, 0xD7, 0x87 },
    til::color{ 0xD7, 0xD7, 0xAF },
    til::color{ 0xD7, 0xD7, 0xD7 },
    til::color{ 0xD7, 0xD7, 0xFF },
    til::color{ 0xD7, 0xFF, 0x00 },
    til::color{ 0xD7, 0xFF, 0x5F },
    til::color{ 0xD7, 0xFF, 0x87 },
    til::color{ 0xD7, 0xFF, 0xAF },
    til::color{ 0xD7, 0xFF, 0xD7 },
    til::color{ 0xD7, 0xFF, 0xFF },
    til::color{ 0xFF, 0x00, 0x00 },
    til::color{ 0xFF, 0x00, 0x5F },
    til::color{ 0xFF, 0x00, 0x87 },
    til::color{ 0xFF, 0x00, 0xAF },
    til::color{ 0xFF, 0x00, 0xD7 },
    til::color{ 0xFF, 0x00, 0xFF },
    til::color{ 0xFF, 0x5F, 0x00 },
    til::color{ 0xFF, 0x5F, 0x5F },
    til::color{ 0xFF, 0x5F, 0x87 },
    til::color{ 0xFF, 0x5F, 0xAF },
    til::color{ 0xFF, 0x5F, 0xD7 },
    til::color{ 0xFF, 0x5F, 0xFF },
    til::color{ 0xFF, 0x87, 0x00 },
    til::color{ 0xFF, 0x87, 0x5F },
    til::color{ 0xFF, 0x87, 0x87 },
    til::color{ 0xFF, 0x87, 0xAF },
    til::color{ 0xFF, 0x87, 0xD7 },
    til::color{ 0xFF, 0x87, 0xFF },
    til::color{ 0xFF, 0xAF, 0x00 },
    til::color{ 0xFF, 0xAF, 0x5F },
    til::color{ 0xFF, 0xAF, 0x87 },
    til::color{ 0xFF, 0xAF, 0xAF },
    til::color{ 0xFF, 0xAF, 0xD7 },
    til::color{ 0xFF, 0xAF, 0xFF },
    til::color{ 0xFF, 0xD7, 0x00 },
    til::color{ 0xFF, 0xD7, 0x5F },
    til::color{ 0xFF, 0xD7, 0x87 },
    til::color{ 0xFF, 0xD7, 0xAF },
    til::color{ 0xFF, 0xD7, 0xD7 },
    til::color{ 0xFF, 0xD7, 0xFF },
    til::color{ 0xFF, 0xFF, 0x00 },
    til::color{ 0xFF, 0xFF, 0x5F },
    til::color{ 0xFF, 0xFF, 0x87 },
    til::color{ 0xFF, 0xFF, 0xAF },
    til::color{ 0xFF, 0xFF, 0xD7 },
    til::color{ 0xFF, 0xFF, 0xFF },
    til::color{ 0x08, 0x08, 0x08 },
    til::color{ 0x12, 0x12, 0x12 },
    til::color{ 0x1C, 0x1C, 0x1C },
    til::color{ 0x26, 0x26, 0x26 },
    til::color{ 0x30, 0x30, 0x30 },
    til::color{ 0x3A, 0x3A, 0x3A },
    til::color{ 0x44, 0x44, 0x44 },
    til::color{ 0x4E, 0x4E, 0x4E },
    til::color{ 0x58, 0x58, 0x58 },
    til::color{ 0x62, 0x62, 0x62 },
    til::color{ 0x6C, 0x6C, 0x6C },
    til::color{ 0x76, 0x76, 0x76 },
    til::color{ 0x80, 0x80, 0x80 },
    til::color{ 0x8A, 0x8A, 0x8A },
    til::color{ 0x94, 0x94, 0x94 },
    til::color{ 0x9E, 0x9E, 0x9E },
    til::color{ 0xA8, 0xA8, 0xA8 },
    til::color{ 0xB2, 0xB2, 0xB2 },
    til::color{ 0xBC, 0xBC, 0xBC },
    til::color{ 0xC6, 0xC6, 0xC6 },
    til::color{ 0xD0, 0xD0, 0xD0 },
    til::color{ 0xDA, 0xDA, 0xDA },
    til::color{ 0xE4, 0xE4, 0xE4 },
    til::color{ 0xEE, 0xEE, 0xEE },
};

static til::static_map xorgAppColorTable{
    std::pair{ L"snow"sv, til::color{ 255, 250, 250 } },
    std::pair{ L"ghostwhite"sv, til::color{ 248, 248, 255 } },
    std::pair{ L"whitesmoke"sv, til::color{ 245, 245, 245 } },
    std::pair{ L"gainsboro"sv, til::color{ 220, 220, 220 } },
    std::pair{ L"floralwhite"sv, til::color{ 255, 250, 240 } },
    std::pair{ L"oldlace"sv, til::color{ 253, 245, 230 } },
    std::pair{ L"linen"sv, til::color{ 250, 240, 230 } },
    std::pair{ L"antiquewhite"sv, til::color{ 250, 235, 215 } },
    std::pair{ L"papayawhip"sv, til::color{ 255, 239, 213 } },
    std::pair{ L"blanchedalmond"sv, til::color{ 255, 235, 205 } },
    std::pair{ L"bisque"sv, til::color{ 255, 228, 196 } },
    std::pair{ L"peachpuff"sv, til::color{ 255, 218, 185 } },
    std::pair{ L"navajowhite"sv, til::color{ 255, 222, 173 } },
    std::pair{ L"moccasin"sv, til::color{ 255, 228, 181 } },
    std::pair{ L"cornsilk"sv, til::color{ 255, 248, 220 } },
    std::pair{ L"ivory"sv, til::color{ 255, 255, 240 } },
    std::pair{ L"lemonchiffon"sv, til::color{ 255, 250, 205 } },
    std::pair{ L"seashell"sv, til::color{ 255, 245, 238 } },
    std::pair{ L"honeydew"sv, til::color{ 240, 255, 240 } },
    std::pair{ L"mintcream"sv, til::color{ 245, 255, 250 } },
    std::pair{ L"azure"sv, til::color{ 240, 255, 255 } },
    std::pair{ L"aliceblue"sv, til::color{ 240, 248, 255 } },
    std::pair{ L"lavender"sv, til::color{ 230, 230, 250 } },
    std::pair{ L"lavenderblush"sv, til::color{ 255, 240, 245 } },
    std::pair{ L"mistyrose"sv, til::color{ 255, 228, 225 } },
    std::pair{ L"white"sv, til::color{ 255, 255, 255 } },
    std::pair{ L"black"sv, til::color{ 0, 0, 0 } },
    std::pair{ L"darkslategray"sv, til::color{ 47, 79, 79 } },
    std::pair{ L"darkslategrey"sv, til::color{ 47, 79, 79 } },
    std::pair{ L"dimgray"sv, til::color{ 105, 105, 105 } },
    std::pair{ L"dimgrey"sv, til::color{ 105, 105, 105 } },
    std::pair{ L"slategray"sv, til::color{ 112, 128, 144 } },
    std::pair{ L"slategrey"sv, til::color{ 112, 128, 144 } },
    std::pair{ L"lightslategray"sv, til::color{ 119, 136, 153 } },
    std::pair{ L"lightslategrey"sv, til::color{ 119, 136, 153 } },
    std::pair{ L"gray"sv, til::color{ 190, 190, 190 } },
    std::pair{ L"grey"sv, til::color{ 190, 190, 190 } },
    std::pair{ L"x11gray"sv, til::color{ 190, 190, 190 } },
    std::pair{ L"x11grey"sv, til::color{ 190, 190, 190 } },
    std::pair{ L"webgray"sv, til::color{ 128, 128, 128 } },
    std::pair{ L"webgrey"sv, til::color{ 128, 128, 128 } },
    std::pair{ L"lightgrey"sv, til::color{ 211, 211, 211 } },
    std::pair{ L"lightgray"sv, til::color{ 211, 211, 211 } },
    std::pair{ L"midnightblue"sv, til::color{ 25, 25, 112 } },
    std::pair{ L"navy"sv, til::color{ 0, 0, 128 } },
    std::pair{ L"navyblue"sv, til::color{ 0, 0, 128 } },
    std::pair{ L"cornflowerblue"sv, til::color{ 100, 149, 237 } },
    std::pair{ L"darkslateblue"sv, til::color{ 72, 61, 139 } },
    std::pair{ L"slateblue"sv, til::color{ 106, 90, 205 } },
    std::pair{ L"mediumslateblue"sv, til::color{ 123, 104, 238 } },
    std::pair{ L"lightslateblue"sv, til::color{ 132, 112, 255 } },
    std::pair{ L"mediumblue"sv, til::color{ 0, 0, 205 } },
    std::pair{ L"royalblue"sv, til::color{ 65, 105, 225 } },
    std::pair{ L"blue"sv, til::color{ 0, 0, 255 } },
    std::pair{ L"dodgerblue"sv, til::color{ 30, 144, 255 } },
    std::pair{ L"deepskyblue"sv, til::color{ 0, 191, 255 } },
    std::pair{ L"skyblue"sv, til::color{ 135, 206, 235 } },
    std::pair{ L"lightskyblue"sv, til::color{ 135, 206, 250 } },
    std::pair{ L"steelblue"sv, til::color{ 70, 130, 180 } },
    std::pair{ L"lightsteelblue"sv, til::color{ 176, 196, 222 } },
    std::pair{ L"lightblue"sv, til::color{ 173, 216, 230 } },
    std::pair{ L"powderblue"sv, til::color{ 176, 224, 230 } },
    std::pair{ L"paleturquoise"sv, til::color{ 175, 238, 238 } },
    std::pair{ L"darkturquoise"sv, til::color{ 0, 206, 209 } },
    std::pair{ L"mediumturquoise"sv, til::color{ 72, 209, 204 } },
    std::pair{ L"turquoise"sv, til::color{ 64, 224, 208 } },
    std::pair{ L"cyan"sv, til::color{ 0, 255, 255 } },
    std::pair{ L"aqua"sv, til::color{ 0, 255, 255 } },
    std::pair{ L"lightcyan"sv, til::color{ 224, 255, 255 } },
    std::pair{ L"cadetblue"sv, til::color{ 95, 158, 160 } },
    std::pair{ L"mediumaquamarine"sv, til::color{ 102, 205, 170 } },
    std::pair{ L"aquamarine"sv, til::color{ 127, 255, 212 } },
    std::pair{ L"darkgreen"sv, til::color{ 0, 100, 0 } },
    std::pair{ L"darkolivegreen"sv, til::color{ 85, 107, 47 } },
    std::pair{ L"darkseagreen"sv, til::color{ 143, 188, 143 } },
    std::pair{ L"seagreen"sv, til::color{ 46, 139, 87 } },
    std::pair{ L"mediumseagreen"sv, til::color{ 60, 179, 113 } },
    std::pair{ L"lightseagreen"sv, til::color{ 32, 178, 170 } },
    std::pair{ L"palegreen"sv, til::color{ 152, 251, 152 } },
    std::pair{ L"springgreen"sv, til::color{ 0, 255, 127 } },
    std::pair{ L"lawngreen"sv, til::color{ 124, 252, 0 } },
    std::pair{ L"green"sv, til::color{ 0, 255, 0 } },
    std::pair{ L"lime"sv, til::color{ 0, 255, 0 } },
    std::pair{ L"x11green"sv, til::color{ 0, 255, 0 } },
    std::pair{ L"webgreen"sv, til::color{ 0, 128, 0 } },
    std::pair{ L"chartreuse"sv, til::color{ 127, 255, 0 } },
    std::pair{ L"mediumspringgreen"sv, til::color{ 0, 250, 154 } },
    std::pair{ L"greenyellow"sv, til::color{ 173, 255, 47 } },
    std::pair{ L"limegreen"sv, til::color{ 50, 205, 50 } },
    std::pair{ L"yellowgreen"sv, til::color{ 154, 205, 50 } },
    std::pair{ L"forestgreen"sv, til::color{ 34, 139, 34 } },
    std::pair{ L"olivedrab"sv, til::color{ 107, 142, 35 } },
    std::pair{ L"darkkhaki"sv, til::color{ 189, 183, 107 } },
    std::pair{ L"khaki"sv, til::color{ 240, 230, 140 } },
    std::pair{ L"palegoldenrod"sv, til::color{ 238, 232, 170 } },
    std::pair{ L"lightgoldenrodyellow"sv, til::color{ 250, 250, 210 } },
    std::pair{ L"lightyellow"sv, til::color{ 255, 255, 224 } },
    std::pair{ L"yellow"sv, til::color{ 255, 255, 0 } },
    std::pair{ L"gold"sv, til::color{ 255, 215, 0 } },
    std::pair{ L"lightgoldenrod"sv, til::color{ 238, 221, 130 } },
    std::pair{ L"goldenrod"sv, til::color{ 218, 165, 32 } },
    std::pair{ L"darkgoldenrod"sv, til::color{ 184, 134, 11 } },
    std::pair{ L"rosybrown"sv, til::color{ 188, 143, 143 } },
    std::pair{ L"indianred"sv, til::color{ 205, 92, 92 } },
    std::pair{ L"saddlebrown"sv, til::color{ 139, 69, 19 } },
    std::pair{ L"sienna"sv, til::color{ 160, 82, 45 } },
    std::pair{ L"peru"sv, til::color{ 205, 133, 63 } },
    std::pair{ L"burlywood"sv, til::color{ 222, 184, 135 } },
    std::pair{ L"beige"sv, til::color{ 245, 245, 220 } },
    std::pair{ L"wheat"sv, til::color{ 245, 222, 179 } },
    std::pair{ L"sandybrown"sv, til::color{ 244, 164, 96 } },
    std::pair{ L"tan"sv, til::color{ 210, 180, 140 } },
    std::pair{ L"chocolate"sv, til::color{ 210, 105, 30 } },
    std::pair{ L"firebrick"sv, til::color{ 178, 34, 34 } },
    std::pair{ L"brown"sv, til::color{ 165, 42, 42 } },
    std::pair{ L"darksalmon"sv, til::color{ 233, 150, 122 } },
    std::pair{ L"salmon"sv, til::color{ 250, 128, 114 } },
    std::pair{ L"lightsalmon"sv, til::color{ 255, 160, 122 } },
    std::pair{ L"orange"sv, til::color{ 255, 165, 0 } },
    std::pair{ L"darkorange"sv, til::color{ 255, 140, 0 } },
    std::pair{ L"coral"sv, til::color{ 255, 127, 80 } },
    std::pair{ L"lightcoral"sv, til::color{ 240, 128, 128 } },
    std::pair{ L"tomato"sv, til::color{ 255, 99, 71 } },
    std::pair{ L"orangered"sv, til::color{ 255, 69, 0 } },
    std::pair{ L"red"sv, til::color{ 255, 0, 0 } },
    std::pair{ L"hotpink"sv, til::color{ 255, 105, 180 } },
    std::pair{ L"deeppink"sv, til::color{ 255, 20, 147 } },
    std::pair{ L"pink"sv, til::color{ 255, 192, 203 } },
    std::pair{ L"lightpink"sv, til::color{ 255, 182, 193 } },
    std::pair{ L"palevioletred"sv, til::color{ 219, 112, 147 } },
    std::pair{ L"maroon"sv, til::color{ 176, 48, 96 } },
    std::pair{ L"x11maroon"sv, til::color{ 176, 48, 96 } },
    std::pair{ L"webmaroon"sv, til::color{ 128, 0, 0 } },
    std::pair{ L"mediumvioletred"sv, til::color{ 199, 21, 133 } },
    std::pair{ L"violetred"sv, til::color{ 208, 32, 144 } },
    std::pair{ L"magenta"sv, til::color{ 255, 0, 255 } },
    std::pair{ L"fuchsia"sv, til::color{ 255, 0, 255 } },
    std::pair{ L"violet"sv, til::color{ 238, 130, 238 } },
    std::pair{ L"plum"sv, til::color{ 221, 160, 221 } },
    std::pair{ L"orchid"sv, til::color{ 218, 112, 214 } },
    std::pair{ L"mediumorchid"sv, til::color{ 186, 85, 211 } },
    std::pair{ L"darkorchid"sv, til::color{ 153, 50, 204 } },
    std::pair{ L"darkviolet"sv, til::color{ 148, 0, 211 } },
    std::pair{ L"blueviolet"sv, til::color{ 138, 43, 226 } },
    std::pair{ L"purple"sv, til::color{ 160, 32, 240 } },
    std::pair{ L"x11purple"sv, til::color{ 160, 32, 240 } },
    std::pair{ L"webpurple"sv, til::color{ 128, 0, 128 } },
    std::pair{ L"mediumpurple"sv, til::color{ 147, 112, 219 } },
    std::pair{ L"thistle"sv, til::color{ 216, 191, 216 } },
    std::pair{ L"snow1"sv, til::color{ 255, 250, 250 } },
    std::pair{ L"snow2"sv, til::color{ 238, 233, 233 } },
    std::pair{ L"snow3"sv, til::color{ 205, 201, 201 } },
    std::pair{ L"snow4"sv, til::color{ 139, 137, 137 } },
    std::pair{ L"seashell1"sv, til::color{ 255, 245, 238 } },
    std::pair{ L"seashell2"sv, til::color{ 238, 229, 222 } },
    std::pair{ L"seashell3"sv, til::color{ 205, 197, 191 } },
    std::pair{ L"seashell4"sv, til::color{ 139, 134, 130 } },
    std::pair{ L"antiquewhite1"sv, til::color{ 255, 239, 219 } },
    std::pair{ L"antiquewhite2"sv, til::color{ 238, 223, 204 } },
    std::pair{ L"antiquewhite3"sv, til::color{ 205, 192, 176 } },
    std::pair{ L"antiquewhite4"sv, til::color{ 139, 131, 120 } },
    std::pair{ L"bisque1"sv, til::color{ 255, 228, 196 } },
    std::pair{ L"bisque2"sv, til::color{ 238, 213, 183 } },
    std::pair{ L"bisque3"sv, til::color{ 205, 183, 158 } },
    std::pair{ L"bisque4"sv, til::color{ 139, 125, 107 } },
    std::pair{ L"peachpuff1"sv, til::color{ 255, 218, 185 } },
    std::pair{ L"peachpuff2"sv, til::color{ 238, 203, 173 } },
    std::pair{ L"peachpuff3"sv, til::color{ 205, 175, 149 } },
    std::pair{ L"peachpuff4"sv, til::color{ 139, 119, 101 } },
    std::pair{ L"navajowhite1"sv, til::color{ 255, 222, 173 } },
    std::pair{ L"navajowhite2"sv, til::color{ 238, 207, 161 } },
    std::pair{ L"navajowhite3"sv, til::color{ 205, 179, 139 } },
    std::pair{ L"navajowhite4"sv, til::color{ 139, 121, 94 } },
    std::pair{ L"lemonchiffon1"sv, til::color{ 255, 250, 205 } },
    std::pair{ L"lemonchiffon2"sv, til::color{ 238, 233, 191 } },
    std::pair{ L"lemonchiffon3"sv, til::color{ 205, 201, 165 } },
    std::pair{ L"lemonchiffon4"sv, til::color{ 139, 137, 112 } },
    std::pair{ L"cornsilk1"sv, til::color{ 255, 248, 220 } },
    std::pair{ L"cornsilk2"sv, til::color{ 238, 232, 205 } },
    std::pair{ L"cornsilk3"sv, til::color{ 205, 200, 177 } },
    std::pair{ L"cornsilk4"sv, til::color{ 139, 136, 120 } },
    std::pair{ L"ivory1"sv, til::color{ 255, 255, 240 } },
    std::pair{ L"ivory2"sv, til::color{ 238, 238, 224 } },
    std::pair{ L"ivory3"sv, til::color{ 205, 205, 193 } },
    std::pair{ L"ivory4"sv, til::color{ 139, 139, 131 } },
    std::pair{ L"honeydew1"sv, til::color{ 240, 255, 240 } },
    std::pair{ L"honeydew2"sv, til::color{ 224, 238, 224 } },
    std::pair{ L"honeydew3"sv, til::color{ 193, 205, 193 } },
    std::pair{ L"honeydew4"sv, til::color{ 131, 139, 131 } },
    std::pair{ L"lavenderblush1"sv, til::color{ 255, 240, 245 } },
    std::pair{ L"lavenderblush2"sv, til::color{ 238, 224, 229 } },
    std::pair{ L"lavenderblush3"sv, til::color{ 205, 193, 197 } },
    std::pair{ L"lavenderblush4"sv, til::color{ 139, 131, 134 } },
    std::pair{ L"mistyrose1"sv, til::color{ 255, 228, 225 } },
    std::pair{ L"mistyrose2"sv, til::color{ 238, 213, 210 } },
    std::pair{ L"mistyrose3"sv, til::color{ 205, 183, 181 } },
    std::pair{ L"mistyrose4"sv, til::color{ 139, 125, 123 } },
    std::pair{ L"azure1"sv, til::color{ 240, 255, 255 } },
    std::pair{ L"azure2"sv, til::color{ 224, 238, 238 } },
    std::pair{ L"azure3"sv, til::color{ 193, 205, 205 } },
    std::pair{ L"azure4"sv, til::color{ 131, 139, 139 } },
    std::pair{ L"slateblue1"sv, til::color{ 131, 111, 255 } },
    std::pair{ L"slateblue2"sv, til::color{ 122, 103, 238 } },
    std::pair{ L"slateblue3"sv, til::color{ 105, 89, 205 } },
    std::pair{ L"slateblue4"sv, til::color{ 71, 60, 139 } },
    std::pair{ L"royalblue1"sv, til::color{ 72, 118, 255 } },
    std::pair{ L"royalblue2"sv, til::color{ 67, 110, 238 } },
    std::pair{ L"royalblue3"sv, til::color{ 58, 95, 205 } },
    std::pair{ L"royalblue4"sv, til::color{ 39, 64, 139 } },
    std::pair{ L"blue1"sv, til::color{ 0, 0, 255 } },
    std::pair{ L"blue2"sv, til::color{ 0, 0, 238 } },
    std::pair{ L"blue3"sv, til::color{ 0, 0, 205 } },
    std::pair{ L"blue4"sv, til::color{ 0, 0, 139 } },
    std::pair{ L"dodgerblue1"sv, til::color{ 30, 144, 255 } },
    std::pair{ L"dodgerblue2"sv, til::color{ 28, 134, 238 } },
    std::pair{ L"dodgerblue3"sv, til::color{ 24, 116, 205 } },
    std::pair{ L"dodgerblue4"sv, til::color{ 16, 78, 139 } },
    std::pair{ L"steelblue1"sv, til::color{ 99, 184, 255 } },
    std::pair{ L"steelblue2"sv, til::color{ 92, 172, 238 } },
    std::pair{ L"steelblue3"sv, til::color{ 79, 148, 205 } },
    std::pair{ L"steelblue4"sv, til::color{ 54, 100, 139 } },
    std::pair{ L"deepskyblue1"sv, til::color{ 0, 191, 255 } },
    std::pair{ L"deepskyblue2"sv, til::color{ 0, 178, 238 } },
    std::pair{ L"deepskyblue3"sv, til::color{ 0, 154, 205 } },
    std::pair{ L"deepskyblue4"sv, til::color{ 0, 104, 139 } },
    std::pair{ L"skyblue1"sv, til::color{ 135, 206, 255 } },
    std::pair{ L"skyblue2"sv, til::color{ 126, 192, 238 } },
    std::pair{ L"skyblue3"sv, til::color{ 108, 166, 205 } },
    std::pair{ L"skyblue4"sv, til::color{ 74, 112, 139 } },
    std::pair{ L"lightskyblue1"sv, til::color{ 176, 226, 255 } },
    std::pair{ L"lightskyblue2"sv, til::color{ 164, 211, 238 } },
    std::pair{ L"lightskyblue3"sv, til::color{ 141, 182, 205 } },
    std::pair{ L"lightskyblue4"sv, til::color{ 96, 123, 139 } },
    std::pair{ L"slategray1"sv, til::color{ 198, 226, 255 } },
    std::pair{ L"slategray2"sv, til::color{ 185, 211, 238 } },
    std::pair{ L"slategray3"sv, til::color{ 159, 182, 205 } },
    std::pair{ L"slategray4"sv, til::color{ 108, 123, 139 } },
    std::pair{ L"lightsteelblue1"sv, til::color{ 202, 225, 255 } },
    std::pair{ L"lightsteelblue2"sv, til::color{ 188, 210, 238 } },
    std::pair{ L"lightsteelblue3"sv, til::color{ 162, 181, 205 } },
    std::pair{ L"lightsteelblue4"sv, til::color{ 110, 123, 139 } },
    std::pair{ L"lightblue1"sv, til::color{ 191, 239, 255 } },
    std::pair{ L"lightblue2"sv, til::color{ 178, 223, 238 } },
    std::pair{ L"lightblue3"sv, til::color{ 154, 192, 205 } },
    std::pair{ L"lightblue4"sv, til::color{ 104, 131, 139 } },
    std::pair{ L"lightcyan1"sv, til::color{ 224, 255, 255 } },
    std::pair{ L"lightcyan2"sv, til::color{ 209, 238, 238 } },
    std::pair{ L"lightcyan3"sv, til::color{ 180, 205, 205 } },
    std::pair{ L"lightcyan4"sv, til::color{ 122, 139, 139 } },
    std::pair{ L"paleturquoise1"sv, til::color{ 187, 255, 255 } },
    std::pair{ L"paleturquoise2"sv, til::color{ 174, 238, 238 } },
    std::pair{ L"paleturquoise3"sv, til::color{ 150, 205, 205 } },
    std::pair{ L"paleturquoise4"sv, til::color{ 102, 139, 139 } },
    std::pair{ L"cadetblue1"sv, til::color{ 152, 245, 255 } },
    std::pair{ L"cadetblue2"sv, til::color{ 142, 229, 238 } },
    std::pair{ L"cadetblue3"sv, til::color{ 122, 197, 205 } },
    std::pair{ L"cadetblue4"sv, til::color{ 83, 134, 139 } },
    std::pair{ L"turquoise1"sv, til::color{ 0, 245, 255 } },
    std::pair{ L"turquoise2"sv, til::color{ 0, 229, 238 } },
    std::pair{ L"turquoise3"sv, til::color{ 0, 197, 205 } },
    std::pair{ L"turquoise4"sv, til::color{ 0, 134, 139 } },
    std::pair{ L"cyan1"sv, til::color{ 0, 255, 255 } },
    std::pair{ L"cyan2"sv, til::color{ 0, 238, 238 } },
    std::pair{ L"cyan3"sv, til::color{ 0, 205, 205 } },
    std::pair{ L"cyan4"sv, til::color{ 0, 139, 139 } },
    std::pair{ L"darkslategray1"sv, til::color{ 151, 255, 255 } },
    std::pair{ L"darkslategray2"sv, til::color{ 141, 238, 238 } },
    std::pair{ L"darkslategray3"sv, til::color{ 121, 205, 205 } },
    std::pair{ L"darkslategray4"sv, til::color{ 82, 139, 139 } },
    std::pair{ L"aquamarine1"sv, til::color{ 127, 255, 212 } },
    std::pair{ L"aquamarine2"sv, til::color{ 118, 238, 198 } },
    std::pair{ L"aquamarine3"sv, til::color{ 102, 205, 170 } },
    std::pair{ L"aquamarine4"sv, til::color{ 69, 139, 116 } },
    std::pair{ L"darkseagreen1"sv, til::color{ 193, 255, 193 } },
    std::pair{ L"darkseagreen2"sv, til::color{ 180, 238, 180 } },
    std::pair{ L"darkseagreen3"sv, til::color{ 155, 205, 155 } },
    std::pair{ L"darkseagreen4"sv, til::color{ 105, 139, 105 } },
    std::pair{ L"seagreen1"sv, til::color{ 84, 255, 159 } },
    std::pair{ L"seagreen2"sv, til::color{ 78, 238, 148 } },
    std::pair{ L"seagreen3"sv, til::color{ 67, 205, 128 } },
    std::pair{ L"seagreen4"sv, til::color{ 46, 139, 87 } },
    std::pair{ L"palegreen1"sv, til::color{ 154, 255, 154 } },
    std::pair{ L"palegreen2"sv, til::color{ 144, 238, 144 } },
    std::pair{ L"palegreen3"sv, til::color{ 124, 205, 124 } },
    std::pair{ L"palegreen4"sv, til::color{ 84, 139, 84 } },
    std::pair{ L"springgreen1"sv, til::color{ 0, 255, 127 } },
    std::pair{ L"springgreen2"sv, til::color{ 0, 238, 118 } },
    std::pair{ L"springgreen3"sv, til::color{ 0, 205, 102 } },
    std::pair{ L"springgreen4"sv, til::color{ 0, 139, 69 } },
    std::pair{ L"green1"sv, til::color{ 0, 255, 0 } },
    std::pair{ L"green2"sv, til::color{ 0, 238, 0 } },
    std::pair{ L"green3"sv, til::color{ 0, 205, 0 } },
    std::pair{ L"green4"sv, til::color{ 0, 139, 0 } },
    std::pair{ L"chartreuse1"sv, til::color{ 127, 255, 0 } },
    std::pair{ L"chartreuse2"sv, til::color{ 118, 238, 0 } },
    std::pair{ L"chartreuse3"sv, til::color{ 102, 205, 0 } },
    std::pair{ L"chartreuse4"sv, til::color{ 69, 139, 0 } },
    std::pair{ L"olivedrab1"sv, til::color{ 192, 255, 62 } },
    std::pair{ L"olivedrab2"sv, til::color{ 179, 238, 58 } },
    std::pair{ L"olivedrab3"sv, til::color{ 154, 205, 50 } },
    std::pair{ L"olivedrab4"sv, til::color{ 105, 139, 34 } },
    std::pair{ L"darkolivegreen1"sv, til::color{ 202, 255, 112 } },
    std::pair{ L"darkolivegreen2"sv, til::color{ 188, 238, 104 } },
    std::pair{ L"darkolivegreen3"sv, til::color{ 162, 205, 90 } },
    std::pair{ L"darkolivegreen4"sv, til::color{ 110, 139, 61 } },
    std::pair{ L"khaki1"sv, til::color{ 255, 246, 143 } },
    std::pair{ L"khaki2"sv, til::color{ 238, 230, 133 } },
    std::pair{ L"khaki3"sv, til::color{ 205, 198, 115 } },
    std::pair{ L"khaki4"sv, til::color{ 139, 134, 78 } },
    std::pair{ L"lightgoldenrod1"sv, til::color{ 255, 236, 139 } },
    std::pair{ L"lightgoldenrod2"sv, til::color{ 238, 220, 130 } },
    std::pair{ L"lightgoldenrod3"sv, til::color{ 205, 190, 112 } },
    std::pair{ L"lightgoldenrod4"sv, til::color{ 139, 129, 76 } },
    std::pair{ L"lightyellow1"sv, til::color{ 255, 255, 224 } },
    std::pair{ L"lightyellow2"sv, til::color{ 238, 238, 209 } },
    std::pair{ L"lightyellow3"sv, til::color{ 205, 205, 180 } },
    std::pair{ L"lightyellow4"sv, til::color{ 139, 139, 122 } },
    std::pair{ L"yellow1"sv, til::color{ 255, 255, 0 } },
    std::pair{ L"yellow2"sv, til::color{ 238, 238, 0 } },
    std::pair{ L"yellow3"sv, til::color{ 205, 205, 0 } },
    std::pair{ L"yellow4"sv, til::color{ 139, 139, 0 } },
    std::pair{ L"gold1"sv, til::color{ 255, 215, 0 } },
    std::pair{ L"gold2"sv, til::color{ 238, 201, 0 } },
    std::pair{ L"gold3"sv, til::color{ 205, 173, 0 } },
    std::pair{ L"gold4"sv, til::color{ 139, 117, 0 } },
    std::pair{ L"goldenrod1"sv, til::color{ 255, 193, 37 } },
    std::pair{ L"goldenrod2"sv, til::color{ 238, 180, 34 } },
    std::pair{ L"goldenrod3"sv, til::color{ 205, 155, 29 } },
    std::pair{ L"goldenrod4"sv, til::color{ 139, 105, 20 } },
    std::pair{ L"darkgoldenrod1"sv, til::color{ 255, 185, 15 } },
    std::pair{ L"darkgoldenrod2"sv, til::color{ 238, 173, 14 } },
    std::pair{ L"darkgoldenrod3"sv, til::color{ 205, 149, 12 } },
    std::pair{ L"darkgoldenrod4"sv, til::color{ 139, 101, 8 } },
    std::pair{ L"rosybrown1"sv, til::color{ 255, 193, 193 } },
    std::pair{ L"rosybrown2"sv, til::color{ 238, 180, 180 } },
    std::pair{ L"rosybrown3"sv, til::color{ 205, 155, 155 } },
    std::pair{ L"rosybrown4"sv, til::color{ 139, 105, 105 } },
    std::pair{ L"indianred1"sv, til::color{ 255, 106, 106 } },
    std::pair{ L"indianred2"sv, til::color{ 238, 99, 99 } },
    std::pair{ L"indianred3"sv, til::color{ 205, 85, 85 } },
    std::pair{ L"indianred4"sv, til::color{ 139, 58, 58 } },
    std::pair{ L"sienna1"sv, til::color{ 255, 130, 71 } },
    std::pair{ L"sienna2"sv, til::color{ 238, 121, 66 } },
    std::pair{ L"sienna3"sv, til::color{ 205, 104, 57 } },
    std::pair{ L"sienna4"sv, til::color{ 139, 71, 38 } },
    std::pair{ L"burlywood1"sv, til::color{ 255, 211, 155 } },
    std::pair{ L"burlywood2"sv, til::color{ 238, 197, 145 } },
    std::pair{ L"burlywood3"sv, til::color{ 205, 170, 125 } },
    std::pair{ L"burlywood4"sv, til::color{ 139, 115, 85 } },
    std::pair{ L"wheat1"sv, til::color{ 255, 231, 186 } },
    std::pair{ L"wheat2"sv, til::color{ 238, 216, 174 } },
    std::pair{ L"wheat3"sv, til::color{ 205, 186, 150 } },
    std::pair{ L"wheat4"sv, til::color{ 139, 126, 102 } },
    std::pair{ L"tan1"sv, til::color{ 255, 165, 79 } },
    std::pair{ L"tan2"sv, til::color{ 238, 154, 73 } },
    std::pair{ L"tan3"sv, til::color{ 205, 133, 63 } },
    std::pair{ L"tan4"sv, til::color{ 139, 90, 43 } },
    std::pair{ L"chocolate1"sv, til::color{ 255, 127, 36 } },
    std::pair{ L"chocolate2"sv, til::color{ 238, 118, 33 } },
    std::pair{ L"chocolate3"sv, til::color{ 205, 102, 29 } },
    std::pair{ L"chocolate4"sv, til::color{ 139, 69, 19 } },
    std::pair{ L"firebrick1"sv, til::color{ 255, 48, 48 } },
    std::pair{ L"firebrick2"sv, til::color{ 238, 44, 44 } },
    std::pair{ L"firebrick3"sv, til::color{ 205, 38, 38 } },
    std::pair{ L"firebrick4"sv, til::color{ 139, 26, 26 } },
    std::pair{ L"brown1"sv, til::color{ 255, 64, 64 } },
    std::pair{ L"brown2"sv, til::color{ 238, 59, 59 } },
    std::pair{ L"brown3"sv, til::color{ 205, 51, 51 } },
    std::pair{ L"brown4"sv, til::color{ 139, 35, 35 } },
    std::pair{ L"salmon1"sv, til::color{ 255, 140, 105 } },
    std::pair{ L"salmon2"sv, til::color{ 238, 130, 98 } },
    std::pair{ L"salmon3"sv, til::color{ 205, 112, 84 } },
    std::pair{ L"salmon4"sv, til::color{ 139, 76, 57 } },
    std::pair{ L"lightsalmon1"sv, til::color{ 255, 160, 122 } },
    std::pair{ L"lightsalmon2"sv, til::color{ 238, 149, 114 } },
    std::pair{ L"lightsalmon3"sv, til::color{ 205, 129, 98 } },
    std::pair{ L"lightsalmon4"sv, til::color{ 139, 87, 66 } },
    std::pair{ L"orange1"sv, til::color{ 255, 165, 0 } },
    std::pair{ L"orange2"sv, til::color{ 238, 154, 0 } },
    std::pair{ L"orange3"sv, til::color{ 205, 133, 0 } },
    std::pair{ L"orange4"sv, til::color{ 139, 90, 0 } },
    std::pair{ L"darkorange1"sv, til::color{ 255, 127, 0 } },
    std::pair{ L"darkorange2"sv, til::color{ 238, 118, 0 } },
    std::pair{ L"darkorange3"sv, til::color{ 205, 102, 0 } },
    std::pair{ L"darkorange4"sv, til::color{ 139, 69, 0 } },
    std::pair{ L"coral1"sv, til::color{ 255, 114, 86 } },
    std::pair{ L"coral2"sv, til::color{ 238, 106, 80 } },
    std::pair{ L"coral3"sv, til::color{ 205, 91, 69 } },
    std::pair{ L"coral4"sv, til::color{ 139, 62, 47 } },
    std::pair{ L"tomato1"sv, til::color{ 255, 99, 71 } },
    std::pair{ L"tomato2"sv, til::color{ 238, 92, 66 } },
    std::pair{ L"tomato3"sv, til::color{ 205, 79, 57 } },
    std::pair{ L"tomato4"sv, til::color{ 139, 54, 38 } },
    std::pair{ L"orangered1"sv, til::color{ 255, 69, 0 } },
    std::pair{ L"orangered2"sv, til::color{ 238, 64, 0 } },
    std::pair{ L"orangered3"sv, til::color{ 205, 55, 0 } },
    std::pair{ L"orangered4"sv, til::color{ 139, 37, 0 } },
    std::pair{ L"red1"sv, til::color{ 255, 0, 0 } },
    std::pair{ L"red2"sv, til::color{ 238, 0, 0 } },
    std::pair{ L"red3"sv, til::color{ 205, 0, 0 } },
    std::pair{ L"red4"sv, til::color{ 139, 0, 0 } },
    std::pair{ L"deeppink1"sv, til::color{ 255, 20, 147 } },
    std::pair{ L"deeppink2"sv, til::color{ 238, 18, 137 } },
    std::pair{ L"deeppink3"sv, til::color{ 205, 16, 118 } },
    std::pair{ L"deeppink4"sv, til::color{ 139, 10, 80 } },
    std::pair{ L"hotpink1"sv, til::color{ 255, 110, 180 } },
    std::pair{ L"hotpink2"sv, til::color{ 238, 106, 167 } },
    std::pair{ L"hotpink3"sv, til::color{ 205, 96, 144 } },
    std::pair{ L"hotpink4"sv, til::color{ 139, 58, 98 } },
    std::pair{ L"pink1"sv, til::color{ 255, 181, 197 } },
    std::pair{ L"pink2"sv, til::color{ 238, 169, 184 } },
    std::pair{ L"pink3"sv, til::color{ 205, 145, 158 } },
    std::pair{ L"pink4"sv, til::color{ 139, 99, 108 } },
    std::pair{ L"lightpink1"sv, til::color{ 255, 174, 185 } },
    std::pair{ L"lightpink2"sv, til::color{ 238, 162, 173 } },
    std::pair{ L"lightpink3"sv, til::color{ 205, 140, 149 } },
    std::pair{ L"lightpink4"sv, til::color{ 139, 95, 101 } },
    std::pair{ L"palevioletred1"sv, til::color{ 255, 130, 171 } },
    std::pair{ L"palevioletred2"sv, til::color{ 238, 121, 159 } },
    std::pair{ L"palevioletred3"sv, til::color{ 205, 104, 137 } },
    std::pair{ L"palevioletred4"sv, til::color{ 139, 71, 93 } },
    std::pair{ L"maroon1"sv, til::color{ 255, 52, 179 } },
    std::pair{ L"maroon2"sv, til::color{ 238, 48, 167 } },
    std::pair{ L"maroon3"sv, til::color{ 205, 41, 144 } },
    std::pair{ L"maroon4"sv, til::color{ 139, 28, 98 } },
    std::pair{ L"violetred1"sv, til::color{ 255, 62, 150 } },
    std::pair{ L"violetred2"sv, til::color{ 238, 58, 140 } },
    std::pair{ L"violetred3"sv, til::color{ 205, 50, 120 } },
    std::pair{ L"violetred4"sv, til::color{ 139, 34, 82 } },
    std::pair{ L"magenta1"sv, til::color{ 255, 0, 255 } },
    std::pair{ L"magenta2"sv, til::color{ 238, 0, 238 } },
    std::pair{ L"magenta3"sv, til::color{ 205, 0, 205 } },
    std::pair{ L"magenta4"sv, til::color{ 139, 0, 139 } },
    std::pair{ L"orchid1"sv, til::color{ 255, 131, 250 } },
    std::pair{ L"orchid2"sv, til::color{ 238, 122, 233 } },
    std::pair{ L"orchid3"sv, til::color{ 205, 105, 201 } },
    std::pair{ L"orchid4"sv, til::color{ 139, 71, 137 } },
    std::pair{ L"plum1"sv, til::color{ 255, 187, 255 } },
    std::pair{ L"plum2"sv, til::color{ 238, 174, 238 } },
    std::pair{ L"plum3"sv, til::color{ 205, 150, 205 } },
    std::pair{ L"plum4"sv, til::color{ 139, 102, 139 } },
    std::pair{ L"mediumorchid1"sv, til::color{ 224, 102, 255 } },
    std::pair{ L"mediumorchid2"sv, til::color{ 209, 95, 238 } },
    std::pair{ L"mediumorchid3"sv, til::color{ 180, 82, 205 } },
    std::pair{ L"mediumorchid4"sv, til::color{ 122, 55, 139 } },
    std::pair{ L"darkorchid1"sv, til::color{ 191, 62, 255 } },
    std::pair{ L"darkorchid2"sv, til::color{ 178, 58, 238 } },
    std::pair{ L"darkorchid3"sv, til::color{ 154, 50, 205 } },
    std::pair{ L"darkorchid4"sv, til::color{ 104, 34, 139 } },
    std::pair{ L"purple1"sv, til::color{ 155, 48, 255 } },
    std::pair{ L"purple2"sv, til::color{ 145, 44, 238 } },
    std::pair{ L"purple3"sv, til::color{ 125, 38, 205 } },
    std::pair{ L"purple4"sv, til::color{ 85, 26, 139 } },
    std::pair{ L"mediumpurple1"sv, til::color{ 171, 130, 255 } },
    std::pair{ L"mediumpurple2"sv, til::color{ 159, 121, 238 } },
    std::pair{ L"mediumpurple3"sv, til::color{ 137, 104, 205 } },
    std::pair{ L"mediumpurple4"sv, til::color{ 93, 71, 139 } },
    std::pair{ L"thistle1"sv, til::color{ 255, 225, 255 } },
    std::pair{ L"thistle2"sv, til::color{ 238, 210, 238 } },
    std::pair{ L"thistle3"sv, til::color{ 205, 181, 205 } },
    std::pair{ L"thistle4"sv, til::color{ 139, 123, 139 } },
    std::pair{ L"gray0"sv, til::color{ 0, 0, 0 } },
    std::pair{ L"grey0"sv, til::color{ 0, 0, 0 } },
    std::pair{ L"gray1"sv, til::color{ 3, 3, 3 } },
    std::pair{ L"grey1"sv, til::color{ 3, 3, 3 } },
    std::pair{ L"gray2"sv, til::color{ 5, 5, 5 } },
    std::pair{ L"grey2"sv, til::color{ 5, 5, 5 } },
    std::pair{ L"gray3"sv, til::color{ 8, 8, 8 } },
    std::pair{ L"grey3"sv, til::color{ 8, 8, 8 } },
    std::pair{ L"gray4"sv, til::color{ 10, 10, 10 } },
    std::pair{ L"grey4"sv, til::color{ 10, 10, 10 } },
    std::pair{ L"gray5"sv, til::color{ 13, 13, 13 } },
    std::pair{ L"grey5"sv, til::color{ 13, 13, 13 } },
    std::pair{ L"gray6"sv, til::color{ 15, 15, 15 } },
    std::pair{ L"grey6"sv, til::color{ 15, 15, 15 } },
    std::pair{ L"gray7"sv, til::color{ 18, 18, 18 } },
    std::pair{ L"grey7"sv, til::color{ 18, 18, 18 } },
    std::pair{ L"gray8"sv, til::color{ 20, 20, 20 } },
    std::pair{ L"grey8"sv, til::color{ 20, 20, 20 } },
    std::pair{ L"gray9"sv, til::color{ 23, 23, 23 } },
    std::pair{ L"grey9"sv, til::color{ 23, 23, 23 } },
    std::pair{ L"gray10"sv, til::color{ 26, 26, 26 } },
    std::pair{ L"grey10"sv, til::color{ 26, 26, 26 } },
    std::pair{ L"gray11"sv, til::color{ 28, 28, 28 } },
    std::pair{ L"grey11"sv, til::color{ 28, 28, 28 } },
    std::pair{ L"gray12"sv, til::color{ 31, 31, 31 } },
    std::pair{ L"grey12"sv, til::color{ 31, 31, 31 } },
    std::pair{ L"gray13"sv, til::color{ 33, 33, 33 } },
    std::pair{ L"grey13"sv, til::color{ 33, 33, 33 } },
    std::pair{ L"gray14"sv, til::color{ 36, 36, 36 } },
    std::pair{ L"grey14"sv, til::color{ 36, 36, 36 } },
    std::pair{ L"gray15"sv, til::color{ 38, 38, 38 } },
    std::pair{ L"grey15"sv, til::color{ 38, 38, 38 } },
    std::pair{ L"gray16"sv, til::color{ 41, 41, 41 } },
    std::pair{ L"grey16"sv, til::color{ 41, 41, 41 } },
    std::pair{ L"gray17"sv, til::color{ 43, 43, 43 } },
    std::pair{ L"grey17"sv, til::color{ 43, 43, 43 } },
    std::pair{ L"gray18"sv, til::color{ 46, 46, 46 } },
    std::pair{ L"grey18"sv, til::color{ 46, 46, 46 } },
    std::pair{ L"gray19"sv, til::color{ 48, 48, 48 } },
    std::pair{ L"grey19"sv, til::color{ 48, 48, 48 } },
    std::pair{ L"gray20"sv, til::color{ 51, 51, 51 } },
    std::pair{ L"grey20"sv, til::color{ 51, 51, 51 } },
    std::pair{ L"gray21"sv, til::color{ 54, 54, 54 } },
    std::pair{ L"grey21"sv, til::color{ 54, 54, 54 } },
    std::pair{ L"gray22"sv, til::color{ 56, 56, 56 } },
    std::pair{ L"grey22"sv, til::color{ 56, 56, 56 } },
    std::pair{ L"gray23"sv, til::color{ 59, 59, 59 } },
    std::pair{ L"grey23"sv, til::color{ 59, 59, 59 } },
    std::pair{ L"gray24"sv, til::color{ 61, 61, 61 } },
    std::pair{ L"grey24"sv, til::color{ 61, 61, 61 } },
    std::pair{ L"gray25"sv, til::color{ 64, 64, 64 } },
    std::pair{ L"grey25"sv, til::color{ 64, 64, 64 } },
    std::pair{ L"gray26"sv, til::color{ 66, 66, 66 } },
    std::pair{ L"grey26"sv, til::color{ 66, 66, 66 } },
    std::pair{ L"gray27"sv, til::color{ 69, 69, 69 } },
    std::pair{ L"grey27"sv, til::color{ 69, 69, 69 } },
    std::pair{ L"gray28"sv, til::color{ 71, 71, 71 } },
    std::pair{ L"grey28"sv, til::color{ 71, 71, 71 } },
    std::pair{ L"gray29"sv, til::color{ 74, 74, 74 } },
    std::pair{ L"grey29"sv, til::color{ 74, 74, 74 } },
    std::pair{ L"gray30"sv, til::color{ 77, 77, 77 } },
    std::pair{ L"grey30"sv, til::color{ 77, 77, 77 } },
    std::pair{ L"gray31"sv, til::color{ 79, 79, 79 } },
    std::pair{ L"grey31"sv, til::color{ 79, 79, 79 } },
    std::pair{ L"gray32"sv, til::color{ 82, 82, 82 } },
    std::pair{ L"grey32"sv, til::color{ 82, 82, 82 } },
    std::pair{ L"gray33"sv, til::color{ 84, 84, 84 } },
    std::pair{ L"grey33"sv, til::color{ 84, 84, 84 } },
    std::pair{ L"gray34"sv, til::color{ 87, 87, 87 } },
    std::pair{ L"grey34"sv, til::color{ 87, 87, 87 } },
    std::pair{ L"gray35"sv, til::color{ 89, 89, 89 } },
    std::pair{ L"grey35"sv, til::color{ 89, 89, 89 } },
    std::pair{ L"gray36"sv, til::color{ 92, 92, 92 } },
    std::pair{ L"grey36"sv, til::color{ 92, 92, 92 } },
    std::pair{ L"gray37"sv, til::color{ 94, 94, 94 } },
    std::pair{ L"grey37"sv, til::color{ 94, 94, 94 } },
    std::pair{ L"gray38"sv, til::color{ 97, 97, 97 } },
    std::pair{ L"grey38"sv, til::color{ 97, 97, 97 } },
    std::pair{ L"gray39"sv, til::color{ 99, 99, 99 } },
    std::pair{ L"grey39"sv, til::color{ 99, 99, 99 } },
    std::pair{ L"gray40"sv, til::color{ 102, 102, 102 } },
    std::pair{ L"grey40"sv, til::color{ 102, 102, 102 } },
    std::pair{ L"gray41"sv, til::color{ 105, 105, 105 } },
    std::pair{ L"grey41"sv, til::color{ 105, 105, 105 } },
    std::pair{ L"gray42"sv, til::color{ 107, 107, 107 } },
    std::pair{ L"grey42"sv, til::color{ 107, 107, 107 } },
    std::pair{ L"gray43"sv, til::color{ 110, 110, 110 } },
    std::pair{ L"grey43"sv, til::color{ 110, 110, 110 } },
    std::pair{ L"gray44"sv, til::color{ 112, 112, 112 } },
    std::pair{ L"grey44"sv, til::color{ 112, 112, 112 } },
    std::pair{ L"gray45"sv, til::color{ 115, 115, 115 } },
    std::pair{ L"grey45"sv, til::color{ 115, 115, 115 } },
    std::pair{ L"gray46"sv, til::color{ 117, 117, 117 } },
    std::pair{ L"grey46"sv, til::color{ 117, 117, 117 } },
    std::pair{ L"gray47"sv, til::color{ 120, 120, 120 } },
    std::pair{ L"grey47"sv, til::color{ 120, 120, 120 } },
    std::pair{ L"gray48"sv, til::color{ 122, 122, 122 } },
    std::pair{ L"grey48"sv, til::color{ 122, 122, 122 } },
    std::pair{ L"gray49"sv, til::color{ 125, 125, 125 } },
    std::pair{ L"grey49"sv, til::color{ 125, 125, 125 } },
    std::pair{ L"gray50"sv, til::color{ 127, 127, 127 } },
    std::pair{ L"grey50"sv, til::color{ 127, 127, 127 } },
    std::pair{ L"gray51"sv, til::color{ 130, 130, 130 } },
    std::pair{ L"grey51"sv, til::color{ 130, 130, 130 } },
    std::pair{ L"gray52"sv, til::color{ 133, 133, 133 } },
    std::pair{ L"grey52"sv, til::color{ 133, 133, 133 } },
    std::pair{ L"gray53"sv, til::color{ 135, 135, 135 } },
    std::pair{ L"grey53"sv, til::color{ 135, 135, 135 } },
    std::pair{ L"gray54"sv, til::color{ 138, 138, 138 } },
    std::pair{ L"grey54"sv, til::color{ 138, 138, 138 } },
    std::pair{ L"gray55"sv, til::color{ 140, 140, 140 } },
    std::pair{ L"grey55"sv, til::color{ 140, 140, 140 } },
    std::pair{ L"gray56"sv, til::color{ 143, 143, 143 } },
    std::pair{ L"grey56"sv, til::color{ 143, 143, 143 } },
    std::pair{ L"gray57"sv, til::color{ 145, 145, 145 } },
    std::pair{ L"grey57"sv, til::color{ 145, 145, 145 } },
    std::pair{ L"gray58"sv, til::color{ 148, 148, 148 } },
    std::pair{ L"grey58"sv, til::color{ 148, 148, 148 } },
    std::pair{ L"gray59"sv, til::color{ 150, 150, 150 } },
    std::pair{ L"grey59"sv, til::color{ 150, 150, 150 } },
    std::pair{ L"gray60"sv, til::color{ 153, 153, 153 } },
    std::pair{ L"grey60"sv, til::color{ 153, 153, 153 } },
    std::pair{ L"gray61"sv, til::color{ 156, 156, 156 } },
    std::pair{ L"grey61"sv, til::color{ 156, 156, 156 } },
    std::pair{ L"gray62"sv, til::color{ 158, 158, 158 } },
    std::pair{ L"grey62"sv, til::color{ 158, 158, 158 } },
    std::pair{ L"gray63"sv, til::color{ 161, 161, 161 } },
    std::pair{ L"grey63"sv, til::color{ 161, 161, 161 } },
    std::pair{ L"gray64"sv, til::color{ 163, 163, 163 } },
    std::pair{ L"grey64"sv, til::color{ 163, 163, 163 } },
    std::pair{ L"gray65"sv, til::color{ 166, 166, 166 } },
    std::pair{ L"grey65"sv, til::color{ 166, 166, 166 } },
    std::pair{ L"gray66"sv, til::color{ 168, 168, 168 } },
    std::pair{ L"grey66"sv, til::color{ 168, 168, 168 } },
    std::pair{ L"gray67"sv, til::color{ 171, 171, 171 } },
    std::pair{ L"grey67"sv, til::color{ 171, 171, 171 } },
    std::pair{ L"gray68"sv, til::color{ 173, 173, 173 } },
    std::pair{ L"grey68"sv, til::color{ 173, 173, 173 } },
    std::pair{ L"gray69"sv, til::color{ 176, 176, 176 } },
    std::pair{ L"grey69"sv, til::color{ 176, 176, 176 } },
    std::pair{ L"gray70"sv, til::color{ 179, 179, 179 } },
    std::pair{ L"grey70"sv, til::color{ 179, 179, 179 } },
    std::pair{ L"gray71"sv, til::color{ 181, 181, 181 } },
    std::pair{ L"grey71"sv, til::color{ 181, 181, 181 } },
    std::pair{ L"gray72"sv, til::color{ 184, 184, 184 } },
    std::pair{ L"grey72"sv, til::color{ 184, 184, 184 } },
    std::pair{ L"gray73"sv, til::color{ 186, 186, 186 } },
    std::pair{ L"grey73"sv, til::color{ 186, 186, 186 } },
    std::pair{ L"gray74"sv, til::color{ 189, 189, 189 } },
    std::pair{ L"grey74"sv, til::color{ 189, 189, 189 } },
    std::pair{ L"gray75"sv, til::color{ 191, 191, 191 } },
    std::pair{ L"grey75"sv, til::color{ 191, 191, 191 } },
    std::pair{ L"gray76"sv, til::color{ 194, 194, 194 } },
    std::pair{ L"grey76"sv, til::color{ 194, 194, 194 } },
    std::pair{ L"gray77"sv, til::color{ 196, 196, 196 } },
    std::pair{ L"grey77"sv, til::color{ 196, 196, 196 } },
    std::pair{ L"gray78"sv, til::color{ 199, 199, 199 } },
    std::pair{ L"grey78"sv, til::color{ 199, 199, 199 } },
    std::pair{ L"gray79"sv, til::color{ 201, 201, 201 } },
    std::pair{ L"grey79"sv, til::color{ 201, 201, 201 } },
    std::pair{ L"gray80"sv, til::color{ 204, 204, 204 } },
    std::pair{ L"grey80"sv, til::color{ 204, 204, 204 } },
    std::pair{ L"gray81"sv, til::color{ 207, 207, 207 } },
    std::pair{ L"grey81"sv, til::color{ 207, 207, 207 } },
    std::pair{ L"gray82"sv, til::color{ 209, 209, 209 } },
    std::pair{ L"grey82"sv, til::color{ 209, 209, 209 } },
    std::pair{ L"gray83"sv, til::color{ 212, 212, 212 } },
    std::pair{ L"grey83"sv, til::color{ 212, 212, 212 } },
    std::pair{ L"gray84"sv, til::color{ 214, 214, 214 } },
    std::pair{ L"grey84"sv, til::color{ 214, 214, 214 } },
    std::pair{ L"gray85"sv, til::color{ 217, 217, 217 } },
    std::pair{ L"grey85"sv, til::color{ 217, 217, 217 } },
    std::pair{ L"gray86"sv, til::color{ 219, 219, 219 } },
    std::pair{ L"grey86"sv, til::color{ 219, 219, 219 } },
    std::pair{ L"gray87"sv, til::color{ 222, 222, 222 } },
    std::pair{ L"grey87"sv, til::color{ 222, 222, 222 } },
    std::pair{ L"gray88"sv, til::color{ 224, 224, 224 } },
    std::pair{ L"grey88"sv, til::color{ 224, 224, 224 } },
    std::pair{ L"gray89"sv, til::color{ 227, 227, 227 } },
    std::pair{ L"grey89"sv, til::color{ 227, 227, 227 } },
    std::pair{ L"gray90"sv, til::color{ 229, 229, 229 } },
    std::pair{ L"grey90"sv, til::color{ 229, 229, 229 } },
    std::pair{ L"gray91"sv, til::color{ 232, 232, 232 } },
    std::pair{ L"grey91"sv, til::color{ 232, 232, 232 } },
    std::pair{ L"gray92"sv, til::color{ 235, 235, 235 } },
    std::pair{ L"grey92"sv, til::color{ 235, 235, 235 } },
    std::pair{ L"gray93"sv, til::color{ 237, 237, 237 } },
    std::pair{ L"grey93"sv, til::color{ 237, 237, 237 } },
    std::pair{ L"gray94"sv, til::color{ 240, 240, 240 } },
    std::pair{ L"grey94"sv, til::color{ 240, 240, 240 } },
    std::pair{ L"gray95"sv, til::color{ 242, 242, 242 } },
    std::pair{ L"grey95"sv, til::color{ 242, 242, 242 } },
    std::pair{ L"gray96"sv, til::color{ 245, 245, 245 } },
    std::pair{ L"grey96"sv, til::color{ 245, 245, 245 } },
    std::pair{ L"gray97"sv, til::color{ 247, 247, 247 } },
    std::pair{ L"grey97"sv, til::color{ 247, 247, 247 } },
    std::pair{ L"gray98"sv, til::color{ 250, 250, 250 } },
    std::pair{ L"grey98"sv, til::color{ 250, 250, 250 } },
    std::pair{ L"gray99"sv, til::color{ 252, 252, 252 } },
    std::pair{ L"grey99"sv, til::color{ 252, 252, 252 } },
    std::pair{ L"gray100"sv, til::color{ 255, 255, 255 } },
    std::pair{ L"grey100"sv, til::color{ 255, 255, 255 } },
    std::pair{ L"darkgrey"sv, til::color{ 169, 169, 169 } },
    std::pair{ L"darkgray"sv, til::color{ 169, 169, 169 } },
    std::pair{ L"darkblue"sv, til::color{ 0, 0, 139 } },
    std::pair{ L"darkcyan"sv, til::color{ 0, 139, 139 } },
    std::pair{ L"darkmagenta"sv, til::color{ 139, 0, 139 } },
    std::pair{ L"darkred"sv, til::color{ 139, 0, 0 } },
    std::pair{ L"lightgreen"sv, til::color{ 144, 238, 144 } },
    std::pair{ L"crimson"sv, til::color{ 220, 20, 60 } },
    std::pair{ L"indigo"sv, til::color{ 75, 0, 130 } },
    std::pair{ L"olive"sv, til::color{ 128, 128, 0 } },
    std::pair{ L"rebeccapurple"sv, til::color{ 102, 51, 153 } },
    std::pair{ L"silver"sv, til::color{ 192, 192, 192 } },
    std::pair{ L"teal"sv, til::color{ 0, 128, 128 } }
};

// Function Description:
// - Creates a String representation of a guid, in the format
//      "{12345678-ABCD-EF12-3456-7890ABCDEF12}"
// Arguments:
// - guid: the GUID to create the string for
// Return Value:
// - a string representation of the GUID. On failure, throws E_INVALIDARG.
std::wstring Utils::GuidToString(const GUID guid)
{
    return wil::str_printf<std::wstring>(L"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}", guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}

// Method Description:
// - Parses a GUID from a string representation of the GUID. Throws an exception
//      if it fails to parse the GUID. See documentation of IIDFromString for
//      details.
// Arguments:
// - wstr: a string representation of the GUID to parse
// Return Value:
// - A GUID if the string could successfully be parsed. On failure, throws the
//      failing HRESULT.
GUID Utils::GuidFromString(const std::wstring wstr)
{
    GUID result{};
    THROW_IF_FAILED(IIDFromString(wstr.c_str(), &result));
    return result;
}

// Method Description:
// - Creates a GUID, but not via an out parameter.
// Return Value:
// - A GUID if there's enough randomness; otherwise, an exception.
GUID Utils::CreateGuid()
{
    GUID result{};
    THROW_IF_FAILED(::CoCreateGuid(&result));
    return result;
}

// Function Description:
// - Creates a String representation of a color, in the format "#RRGGBB"
// Arguments:
// - color: the COLORREF to create the string for
// Return Value:
// - a string representation of the color
std::string Utils::ColorToHexString(const til::color color)
{
    std::stringstream ss;
    ss << "#" << std::uppercase << std::setfill('0') << std::hex;
    // Force the compiler to promote from byte to int. Without it, the
    // stringstream will try to write the components as chars
    ss << std::setw(2) << static_cast<int>(color.r);
    ss << std::setw(2) << static_cast<int>(color.g);
    ss << std::setw(2) << static_cast<int>(color.b);
    return ss.str();
}

// Function Description:
// - Parses a color from a string. The string should be in the format "#RRGGBB" or "#RGB"
// Arguments:
// - str: a string representation of the COLORREF to parse
// Return Value:
// - A COLORREF if the string could successfully be parsed. If the string is not
//      the correct format, throws E_INVALIDARG
til::color Utils::ColorFromHexString(const std::string_view str)
{
    THROW_HR_IF(E_INVALIDARG, str.size() != 7 && str.size() != 4);
    THROW_HR_IF(E_INVALIDARG, str.at(0) != '#');

    std::string rStr;
    std::string gStr;
    std::string bStr;

    if (str.size() == 4)
    {
        rStr = std::string(2, str.at(1));
        gStr = std::string(2, str.at(2));
        bStr = std::string(2, str.at(3));
    }
    else
    {
        rStr = std::string(&str.at(1), 2);
        gStr = std::string(&str.at(3), 2);
        bStr = std::string(&str.at(5), 2);
    }

    const BYTE r = gsl::narrow_cast<BYTE>(std::stoul(rStr, nullptr, 16));
    const BYTE g = gsl::narrow_cast<BYTE>(std::stoul(gStr, nullptr, 16));
    const BYTE b = gsl::narrow_cast<BYTE>(std::stoul(bStr, nullptr, 16));

    return til::color{ r, g, b };
}

// Function Description:
// - Parses a color from a string based on the XOrg app color name table.
// Arguments:
// - str: a string representation of the color name to parse
// - color: a color to write the result to 
// Return Value:
// - True if the string is successfully parsed. False otherwise.
bool Utils::ColorFromXOrgAppColorName(const std::wstring_view wstr, til::color& color)
{
    std::wstring key(wstr);
    std::transform(key.begin(), key.end(), key.begin(), std::towlower);
    key.erase(std::remove_if(key.begin(), key.end(), std::iswspace), key.end());
    const auto iter = xorgAppColorTable.find(key);
    if (iter == xorgAppColorTable.end())
    {
        return false;
    }

    color = iter->second;
    return true;
}

// Routine Description:
// - Shorthand check if a handle value is null or invalid.
// Arguments:
// - Handle
// Return Value:
// - True if non zero and not set to invalid magic value. False otherwise.
bool Utils::IsValidHandle(const HANDLE handle) noexcept
{
    return handle != nullptr && handle != INVALID_HANDLE_VALUE;
}

// Function Description:
// - Fill the first 16 entries of a given color table with the Campbell color
//   scheme, in the ANSI/VT RGB order.
// Arguments:
// - table: a color table with at least 16 entries
// Return Value:
// - <none>, throws if the table has less that 16 entries
void Utils::InitializeCampbellColorTable(const gsl::span<COLORREF> table)
{
    THROW_HR_IF(E_INVALIDARG, table.size() < 16);

    std::copy(campbellColorTable.begin(), campbellColorTable.end(), table.begin());
}

// Function Description:
// - Fill the first 16 entries of a given color table with the Campbell color
//   scheme, in the Windows BGR order.
// Arguments:
// - table: a color table with at least 16 entries
// Return Value:
// - <none>, throws if the table has less that 16 entries
void Utils::InitializeCampbellColorTableForConhost(const gsl::span<COLORREF> table)
{
    THROW_HR_IF(E_INVALIDARG, table.size() < 16);
    InitializeCampbellColorTable(table);
    SwapANSIColorOrderForConhost(table);
}

// Function Description:
// - modifies in-place the given color table from ANSI (RGB) order to Console order (BRG).
// Arguments:
// - table: a color table with at least 16 entries
// Return Value:
// - <none>, throws if the table has less that 16 entries
void Utils::SwapANSIColorOrderForConhost(const gsl::span<COLORREF> table)
{
    THROW_HR_IF(E_INVALIDARG, table.size() < 16);
    std::swap(til::at(table, 1), til::at(table, 4));
    std::swap(til::at(table, 3), til::at(table, 6));
    std::swap(til::at(table, 9), til::at(table, 12));
    std::swap(til::at(table, 11), til::at(table, 14));
}

// Function Description:
// - Fill the first 255 entries of a given color table with the default values
//      of a full 256-color table
// Arguments:
// - table: a color table with at least 256 entries
// Return Value:
// - <none>, throws if the table has less that 256 entries
void Utils::Initialize256ColorTable(const gsl::span<COLORREF> table)
{
    THROW_HR_IF(E_INVALIDARG, table.size() < 256);

    std::copy(standardXterm256ColorTable.begin(), standardXterm256ColorTable.end(), table.begin());
}

// Function Description:
// - Generate a Version 5 UUID (specified in RFC4122 4.3)
//   v5 UUIDs are stable given the same namespace and "name".
// Arguments:
// - namespaceGuid: The GUID of the v5 UUID namespace, which provides both
//                  a seed and a tacit agreement that all UUIDs generated
//                  with it will follow the same data format.
// - name: Bytes comprising the name (in a namespace-specific format)
// Return Value:
// - a new stable v5 UUID
GUID Utils::CreateV5Uuid(const GUID& namespaceGuid, const gsl::span<const gsl::byte> name)
{
    // v5 uuid generation happens over values in network byte order, so let's enforce that
    auto correctEndianNamespaceGuid{ EndianSwap(namespaceGuid) };

    wil::unique_bcrypt_hash hash;
    THROW_IF_NTSTATUS_FAILED(BCryptCreateHash(BCRYPT_SHA1_ALG_HANDLE, &hash, nullptr, 0, nullptr, 0, 0));

    // According to N4713 8.2.1.11 [basic.lval], accessing the bytes underlying an object
    // through unsigned char or char pointer *is defined*.
    THROW_IF_NTSTATUS_FAILED(BCryptHashData(hash.get(), reinterpret_cast<PUCHAR>(&correctEndianNamespaceGuid), sizeof(GUID), 0));
    // BCryptHashData is ill-specified in that it leaves off "const" qualification for pbInput
    THROW_IF_NTSTATUS_FAILED(BCryptHashData(hash.get(), reinterpret_cast<PUCHAR>(const_cast<gsl::byte*>(name.data())), gsl::narrow<ULONG>(name.size()), 0));

    std::array<uint8_t, 20> buffer;
    THROW_IF_NTSTATUS_FAILED(BCryptFinishHash(hash.get(), buffer.data(), gsl::narrow<ULONG>(buffer.size()), 0));

    buffer.at(6) = (buffer.at(6) & 0x0F) | 0x50; // set the uuid version to 5
    buffer.at(8) = (buffer.at(8) & 0x3F) | 0x80; // set the variant to 2 (RFC4122)

    // We're using memcpy here pursuant to N4713 6.7.2/3 [basic.types],
    // "...the underlying bytes making up the object can be copied into an array
    // of char or unsigned char...array is copied back into the object..."
    // std::copy may compile down to ::memcpy for these types, but using it might
    // contravene the standard and nobody's got time for that.
    GUID newGuid{ 0 };
    ::memcpy_s(&newGuid, sizeof(GUID), buffer.data(), sizeof(GUID));
    return EndianSwap(newGuid);
}
