/*++
Copyright (c) Microsoft Corporation
Licensed under the MIT license.

Module Name:
- Utils.h

Abstract:
- Helpers for the TerminalApp project
Author(s):
- Mike Griese - May 2019

--*/
#pragma once

std::wstring GetWstringFromJson(const Json::Value& json);

// Method Description:
// - Create a std::string from a string_view. We do this because we can't look
//   up a key in a Json::Value with a string_view directly, so instead we'll use
//   this helper. Should a string_view lookup ever be added to jsoncpp, we can
//   remove this entirely.
// Arguments:
// - key: the string_view to build a string from
// Return Value:
// - a std::string to use for looking up a value from a Json::Value
inline std::string JsonKey(const std::string_view key)
{
    return static_cast<std::string>(key);
}

winrt::Windows::UI::Xaml::Controls::IconElement GetColoredIcon(const winrt::hstring& path);

// Helper to establish an ordering on guids. This does _NOT_ work to check if
// two guids are the same. Use this with a std::set for determining if GUIDS are
// unique
struct GuidOrdering
{
    bool operator()(const GUID& lhs, const GUID& rhs) const
    {
        return memcmp(&lhs, &rhs, sizeof(rhs)) < 0;
    }
};

// Helper to establish if two GUIDs are equal.
struct GuidEquality
{
    bool operator()(const GUID& lhs, const GUID& rhs) const
    {
        return memcmp(&lhs, &rhs, sizeof(rhs)) == 0;
    }
};

namespace TerminalApp
{
    class JsonUtils;
};

class TerminalApp::JsonUtils final
{
public:
    static void GetOptionalColor(const Json::Value& json,
                                 std::string_view key,
                                 std::optional<uint32_t>& color);

    static void GetOptionalString(const Json::Value& json,
                                  std::string_view key,
                                  std::optional<std::wstring>& target);

    static void GetOptionalGuid(const Json::Value& json,
                                std::string_view key,
                                std::optional<GUID>& target);

    static void GetOptionalDouble(const Json::Value& json,
                                  std::string_view key,
                                  std::optional<double>& target);

    // Be careful, if you pass a lambda stright into this, it will explode in
    // the linker.
    template<typename T, typename F>
    static void GetOptionalValue(const Json::Value& json,
                                 std::string_view key,
                                 std::optional<T>& target,
                                 F&& conversion)
    {
        if (json.isMember(JsonKey(key)))
        {
            if (auto jsonVal{ json[JsonKey(key)] })
            {
                target = conversion(jsonVal);
            }
            else
            {
                target = std::nullopt;
            }
        }
    }
};
