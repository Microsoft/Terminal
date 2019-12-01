// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*
Module Name:
- ITermDispatch.hpp

Abstract:
- This is the interface for all output state machine callbacks. When actions
    occur, they will be dispatched to the methods on this interface which must
    be implemented by a child class and passed into the state machine on
    creation.
*/
#pragma once
#include "DispatchTypes.hpp"

namespace Microsoft::Console::VirtualTerminal
{
    class ITermDispatch;
};

class Microsoft::Console::VirtualTerminal::ITermDispatch
{
public:
#pragma warning(push)
#pragma warning(disable : 26432) // suppress rule of 5 violation on interface because tampering with this is fraught with peril
    virtual ~ITermDispatch() = 0;

    virtual void Execute(const wchar_t wchControl) = 0;
    virtual void Print(const wchar_t wchPrintable) = 0;
    virtual void PrintString(const std::wstring_view string) = 0;

    virtual bool CursorUp(const size_t distance) = 0; // CUU
    virtual bool CursorDown(const size_t distance) = 0; // CUD
    virtual bool CursorForward(const size_t distance) = 0; // CUF
    virtual bool CursorBackward(const size_t distance) = 0; // CUB
    virtual bool CursorNextLine(const size_t distance) = 0; // CNL
    virtual bool CursorPrevLine(const size_t distance) = 0; // CPL
    virtual bool CursorHorizontalPositionAbsolute(const size_t column) = 0; // CHA
    virtual bool VerticalLinePositionAbsolute(const size_t line) = 0; // VPA
    virtual bool CursorPosition(const size_t line, const size_t column) = 0; // CUP
    virtual bool CursorSaveState() = 0; // DECSC
    virtual bool CursorRestoreState() = 0; // DECRC
    virtual bool CursorVisibility(const bool isVisible) = 0; // DECTCEM
    virtual bool InsertCharacter(const size_t count) = 0; // ICH
    virtual bool DeleteCharacter(const size_t count) = 0; // DCH
    virtual bool ScrollUp(const size_t distance) = 0; // SU
    virtual bool ScrollDown(const size_t distance) = 0; // SD
    virtual bool InsertLine(const size_t distance) = 0; // IL
    virtual bool DeleteLine(const size_t distance) = 0; // DL
    virtual bool SetColumns(const size_t columns) = 0; // DECSCPP, DECCOLM
    virtual bool SetCursorKeysMode(const bool applicationMode) = 0; // DECCKM
    virtual bool SetKeypadMode(const bool applicationMode) = 0; // DECKPAM, DECKPNM
    virtual bool EnableCursorBlinking(const bool enable) = 0; // ATT610
    virtual bool SetScreenMode(const bool reverseMode) = 0; //DECSCNM
    virtual bool SetOriginMode(const bool relativeMode) = 0; // DECOM
    virtual bool SetTopBottomScrollingMargins(const size_t topMargin, const size_t bottomMargin) = 0; // DECSTBM
    virtual bool ReverseLineFeed() = 0; // RI
    virtual bool SetWindowTitle(std::wstring_view title) = 0; // OscWindowTitle
    virtual bool UseAlternateScreenBuffer() = 0; // ASBSET
    virtual bool UseMainScreenBuffer() = 0; // ASBRST
    virtual bool HorizontalTabSet() = 0; // HTS
    virtual bool ForwardTab(const size_t numTabs) = 0; // CHT
    virtual bool BackwardsTab(const size_t numTabs) = 0; // CBT
    virtual bool TabClear(const size_t clearType) = 0; // TBC
    virtual bool EnableDECCOLMSupport(const bool enabled) = 0; // ?40
    virtual bool EnableVT200MouseMode(const bool enabled) = 0; // ?1000
    virtual bool EnableUTF8ExtendedMouseMode(const bool enabled) = 0; // ?1005
    virtual bool EnableSGRExtendedMouseMode(const bool enabled) = 0; // ?1006
    virtual bool EnableButtonEventMouseMode(const bool enabled) = 0; // ?1002
    virtual bool EnableAnyEventMouseMode(const bool enabled) = 0; // ?1003
    virtual bool EnableAlternateScroll(const bool enabled) = 0; // ?1007
    virtual bool SetColorTableEntry(const size_t tableIndex, const DWORD color) = 0; // OSCColorTable
    virtual bool SetDefaultForeground(const DWORD color) = 0; // OSCDefaultForeground
    virtual bool SetDefaultBackground(const DWORD color) = 0; // OSCDefaultBackground

    virtual bool EraseInDisplay(const DispatchTypes::EraseType eraseType) = 0; // ED
    virtual bool EraseInLine(const DispatchTypes::EraseType eraseType) = 0; // EL
    virtual bool EraseCharacters(const size_t numChars) = 0; // ECH

    virtual bool SetGraphicsRendition(const std::basic_string_view<DispatchTypes::GraphicsOptions> options) = 0; // SGR

    virtual bool SetPrivateModes(const std::basic_string_view<DispatchTypes::PrivateModeParams> params) = 0; // DECSET

    virtual bool ResetPrivateModes(const std::basic_string_view<DispatchTypes::PrivateModeParams> params) = 0; // DECRST

    virtual bool DeviceStatusReport(const DispatchTypes::AnsiStatusType statusType) = 0; // DSR
    virtual bool DeviceAttributes() = 0; // DA

    virtual bool DesignateCharset(const wchar_t wchCharset) = 0; // DesignateCharset

    virtual bool SoftReset() = 0; // DECSTR
    virtual bool HardReset() = 0; // RIS
    virtual bool ScreenAlignmentPattern() = 0; // DECALN

    virtual bool SetCursorStyle(const DispatchTypes::CursorStyle cursorStyle) = 0; // DECSCUSR
    virtual bool SetCursorColor(const COLORREF color) = 0; // OSCSetCursorColor, OSCResetCursorColor

    // DTTERM_WindowManipulation
    virtual bool WindowManipulation(const DispatchTypes::WindowManipulationType function,
                                    const std::basic_string_view<size_t> parameters) = 0;
};
inline Microsoft::Console::VirtualTerminal::ITermDispatch::~ITermDispatch() {}
#pragma warning(pop)
