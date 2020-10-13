// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "AppLogic.h"
#include "AppCommandlineArgs.h"
#include <LibraryResources.h>

using namespace winrt::TerminalApp;
using namespace winrt::Microsoft::Terminal::Settings::Model;
using namespace TerminalApp;

// Either a ; at the start of a line, or a ; preceded by any non-\ char.
const std::wregex AppCommandlineArgs::_commandDelimiterRegex{ LR"(^;|[^\\];)" };

AppCommandlineArgs::AppCommandlineArgs()
{
    _buildParser();
    _resetStateToDefault();
}

// Method Description:
// - Attempt to parse a given command as a single commandline. If the command
//   doesn't have a subcommand, we'll try parsing the commandline again, as a
//   new-tab command.
// - Actions generated by this command are added to our _startupActions list.
// Arguments:
// - command: The individual commandline to parse as a command.
// Return Value:
// - 0 if the commandline was successfully parsed
// - nonzero return values are defined in CLI::ExitCodes
int AppCommandlineArgs::ParseCommand(const Commandline& command)
{
    const int argc = static_cast<int>(command.Argc());

    // Stash a pointer to the current Commandline instance we're parsing.
    // When we're trying to parse the commandline for a new-tab/split-pane
    // subcommand, we'll need to inspect the original Args from this
    // Commandline to find the entirety of the commandline args for the new
    // terminal instance. Discard the pointer when we leave this method. The
    // pointer will be safe for usage, since the parse callback will be
    // executed on the same thread, higher on the stack.
    _currentCommandline = &command;
    auto clearPointer = wil::scope_exit([this]() { _currentCommandline = nullptr; });
    try
    {
        // CLI11 needs a mutable vector<string>, so copy out the args here.
        // * When we're using the vector<string> parse(), it also expects that
        //   there isn't a leading executable name in the args, so slice that
        //   out.
        //   - In AppCommandlineArgs::BuildCommands, we'll make sure each
        //     subsequent command in a single commandline starts with a wt.exe.
        //     Our very first argument might not be "wt.exe", it could be `wt`,
        //     or `wtd.exe`, etc. Regardless, we want to ignore the first arg of
        //     every Commandline
        // * Not only that, but this particular overload of parse() wants the
        //   args _reversed_ here.
        std::vector<std::string> args{ command.Args().begin() + 1, command.Args().end() };
        std::reverse(args.begin(), args.end());

        // Revert our state to the initial state. As this function can be called
        // multiple times during the parsing of a single commandline (once for each
        // sub-command), we don't want the leftover state from previous calls to
        // pollute this run's state.
        _resetStateToDefault();

        // Manually check for the "/?" or "-?" flags, to manually trigger the help text.
        if (argc == 2 && (NixHelpFlag == til::at(command.Args(), 1) || WindowsHelpFlag == til::at(command.Args(), 1)))
        {
            throw CLI::CallForHelp();
        }
        // Clear the parser's internal state
        _app.clear();

        // attempt to parse the commandline
        _app.parse(args);

        // If we parsed the commandline, and _no_ subcommands were provided, try
        // parsing again as a "new-tab" command.

        if (_noCommandsProvided())
        {
            _newTabCommand.subcommand->clear();
            _newTabCommand.subcommand->parse(args);
        }
    }
    catch (const CLI::CallForHelp& e)
    {
        return _handleExit(_app, e);
    }
    catch (const CLI::ParseError& e)
    {
        // If we parsed the commandline, and _no_ subcommands were provided, try
        // parsing again as a "new-tab" command.
        if (_noCommandsProvided())
        {
            try
            {
                // CLI11 mutated the original vector the first time it tried to
                // parse the args. Reconstruct it the way CLI11 wants here.
                // "See above for why it's begin() + 1"
                std::vector<std::string> args{ command.Args().begin() + 1, command.Args().end() };
                std::reverse(args.begin(), args.end());
                _newTabCommand.subcommand->clear();
                _newTabCommand.subcommand->parse(args);
            }
            catch (const CLI::ParseError& e)
            {
                return _handleExit(*_newTabCommand.subcommand, e);
            }
        }
        else
        {
            return _handleExit(_app, e);
        }
    }
    return 0;
}

// Method Description:
// - Calls App::exit() for the provided command, and collects it's output into
//   our _exitMessage buffer.
// Arguments:
// - command: Either the root App object, or a subcommand for which to call exit() on.
// - e: the CLI::Error to process as the exit reason for parsing.
// Return Value:
// - 0 if the command exited successfully
// - nonzero return values are defined in CLI::ExitCodes
int AppCommandlineArgs::_handleExit(const CLI::App& command, const CLI::Error& e)
{
    // Create some streams to collect the output that would otherwise go to stdout.
    std::ostringstream out;
    std::ostringstream err;
    const auto result = command.exit(e, out, err);
    // I believe only CallForHelp will return 0
    if (result == 0)
    {
        _exitMessage = out.str();
    }
    else
    {
        _exitMessage = err.str();
    }

    // We're displaying an error message - we should always exit instead of
    // actually starting the Terminal.
    _shouldExitEarly = true;

    return result;
}

// Method Description:
// - Add each subcommand and options to the commandline parser.
// Arguments:
// - <none>
// Return Value:
// - <none>
void AppCommandlineArgs::_buildParser()
{
    // -v,--version: Displays version info
    auto versionCallback = [this](int64_t /*count*/) {
        // Set our message to display the application name and the current version.
        _exitMessage = fmt::format("{0}\n{1}",
                                   til::u16u8(CascadiaSettings::ApplicationDisplayName()),
                                   til::u16u8(CascadiaSettings::ApplicationVersion()));
        // Theoretically, we don't need to exit now, since this isn't really
        // an error case. However, in practice, it feels weird to have `wt
        // -v` open a new tab, and makes enough sense that `wt -v ;
        // split-pane` (or whatever) just displays the version and exits.
        _shouldExitEarly = true;
    };
    _app.add_flag_function("-v,--version", versionCallback, RS_A(L"CmdVersionDesc"));

    // Launch mode related flags
    //   -M,--maximized: Maximizes the window on launch
    //   -F,--fullscreen: Fullscreens the window on launch
    //   -f,--focus: Sets the terminal into the Focus mode
    // While fullscreen ecludes both maximized and focus mode, the user can combine betwen the maximized and focused (-fM)
    auto maximizedCallback = [this](int64_t /*count*/) {
        _launchMode = (_launchMode.has_value() && _launchMode.value() == LaunchMode::FocusMode) ?
                          LaunchMode::MaximizedFocusMode :
                          LaunchMode::MaximizedMode;
    };
    auto fullscreenCallback = [this](int64_t /*count*/) {
        _launchMode = LaunchMode::FullscreenMode;
    };
    auto focusCallback = [this](int64_t /*count*/) {
        _launchMode = (_launchMode.has_value() && _launchMode.value() == LaunchMode::MaximizedMode) ?
                          LaunchMode::MaximizedFocusMode :
                          LaunchMode::FocusMode;
    };

    auto maximized = _app.add_flag_function("-M,--maximized", maximizedCallback, RS_A(L"CmdMaximizedDesc"));
    auto fullscreen = _app.add_flag_function("-F,--fullscreen", fullscreenCallback, RS_A(L"CmdFullscreenDesc"));
    auto focus = _app.add_flag_function("-f,--focus", focusCallback, RS_A(L"CmdFocusDesc"));
    maximized->excludes(fullscreen);
    focus->excludes(fullscreen);

    // Subcommands
    _buildNewTabParser();
    _buildSplitPaneParser();
    _buildFocusTabParser();
}

// Method Description:
// - Adds the `new-tab` subcommand and related options to the commandline parser.
// - Additionally adds the `nt` subcommand, which is just a shortened version of `new-tab`
// Arguments:
// - <none>
// Return Value:
// - <none>
void AppCommandlineArgs::_buildNewTabParser()
{
    _newTabCommand.subcommand = _app.add_subcommand("new-tab", RS_A(L"CmdNewTabDesc"));
    _newTabShort.subcommand = _app.add_subcommand("nt", RS_A(L"CmdNTDesc"));

    auto setupSubcommand = [this](auto& subcommand) {
        _addNewTerminalArgs(subcommand);

        // When ParseCommand is called, if this subcommand was provided, this
        // callback function will be triggered on the same thread. We can be sure
        // that `this` will still be safe - this function just lets us know this
        // command was parsed.
        subcommand.subcommand->callback([&, this]() {
            // Build the NewTab action from the values we've parsed on the commandline.
            ActionAndArgs newTabAction{};
            newTabAction.Action(ShortcutAction::NewTab);
            // _getNewTerminalArgs MUST be called before parsing any other options,
            // as it might clear those options while finding the commandline
            NewTabArgs args{ _getNewTerminalArgs(subcommand) };
            newTabAction.Args(args);
            _startupActions.push_back(newTabAction);
        });
    };

    setupSubcommand(_newTabCommand);
    setupSubcommand(_newTabShort);
}

// Method Description:
// - Adds the `split-pane` subcommand and related options to the commandline parser.
// - Additionally adds the `sp` subcommand, which is just a shortened version of `split-pane`
// Arguments:
// - <none>
// Return Value:
// - <none>
void AppCommandlineArgs::_buildSplitPaneParser()
{
    _newPaneCommand.subcommand = _app.add_subcommand("split-pane", RS_A(L"CmdSplitPaneDesc"));
    _newPaneShort.subcommand = _app.add_subcommand("sp", RS_A(L"CmdSPDesc"));

    auto setupSubcommand = [this](auto& subcommand) {
        _addNewTerminalArgs(subcommand);
        subcommand._horizontalOption = subcommand.subcommand->add_flag("-H,--horizontal",
                                                                       _splitHorizontal,
                                                                       RS_A(L"CmdSplitPaneHorizontalArgDesc"));
        subcommand._verticalOption = subcommand.subcommand->add_flag("-V,--vertical",
                                                                     _splitVertical,
                                                                     RS_A(L"CmdSplitPaneVerticalArgDesc"));
        subcommand._verticalOption->excludes(subcommand._horizontalOption);

        // When ParseCommand is called, if this subcommand was provided, this
        // callback function will be triggered on the same thread. We can be sure
        // that `this` will still be safe - this function just lets us know this
        // command was parsed.
        subcommand.subcommand->callback([&, this]() {
            // Build the SplitPane action from the values we've parsed on the commandline.
            ActionAndArgs splitPaneActionAndArgs{};
            splitPaneActionAndArgs.Action(ShortcutAction::SplitPane);

            // _getNewTerminalArgs MUST be called before parsing any other options,
            // as it might clear those options while finding the commandline
            auto terminalArgs{ _getNewTerminalArgs(subcommand) };
            auto style{ SplitState::Automatic };
            // Make sure to use the `Option`s here to check if they were set -
            // _getNewTerminalArgs might reset them while parsing a commandline
            if ((*subcommand._horizontalOption || *subcommand._verticalOption))
            {
                if (_splitHorizontal)
                {
                    style = SplitState::Horizontal;
                }
                else if (_splitVertical)
                {
                    style = SplitState::Vertical;
                }
            }
            SplitPaneArgs args{ style, terminalArgs };
            splitPaneActionAndArgs.Args(args);
            _startupActions.push_back(splitPaneActionAndArgs);
        });
    };

    setupSubcommand(_newPaneCommand);
    setupSubcommand(_newPaneShort);
}

// Method Description:
// - Adds the `focus-tab` subcommand and related options to the commandline parser.
// - Additionally adds the `ft` subcommand, which is just a shortened version of `focus-tab`
// Arguments:
// - <none>
// Return Value:
// - <none>
void AppCommandlineArgs::_buildFocusTabParser()
{
    _focusTabCommand = _app.add_subcommand("focus-tab", RS_A(L"CmdFocusTabDesc"));
    _focusTabShort = _app.add_subcommand("ft", RS_A(L"CmdFTDesc"));

    auto setupSubcommand = [this](auto* subcommand) {
        auto* indexOpt = subcommand->add_option("-t,--target",
                                                _focusTabIndex,
                                                RS_A(L"CmdFocusTabTargetArgDesc"));
        auto* nextOpt = subcommand->add_flag("-n,--next",
                                             _focusNextTab,
                                             RS_A(L"CmdFocusTabNextArgDesc"));
        auto* prevOpt = subcommand->add_flag("-p,--previous",
                                             _focusPrevTab,
                                             RS_A(L"CmdFocusTabPrevArgDesc"));
        nextOpt->excludes(prevOpt);
        indexOpt->excludes(prevOpt);
        indexOpt->excludes(nextOpt);

        // When ParseCommand is called, if this subcommand was provided, this
        // callback function will be triggered on the same thread. We can be sure
        // that `this` will still be safe - this function just lets us know this
        // command was parsed.
        subcommand->callback([&, this]() {
            // Build the action from the values we've parsed on the commandline.
            ActionAndArgs focusTabAction{};

            if (_focusTabIndex >= 0)
            {
                focusTabAction.Action(ShortcutAction::SwitchToTab);
                SwitchToTabArgs args{ static_cast<unsigned int>(_focusTabIndex) };
                focusTabAction.Args(args);
                _startupActions.push_back(focusTabAction);
            }
            else if (_focusNextTab || _focusPrevTab)
            {
                focusTabAction.Action(_focusNextTab ? ShortcutAction::NextTab : ShortcutAction::PrevTab);
                _startupActions.push_back(std::move(focusTabAction));
            }
        });
    };

    setupSubcommand(_focusTabCommand);
    setupSubcommand(_focusTabShort);
}

// Method Description:
// - Add the `NewTerminalArgs` parameters to the given subcommand. This enables
//   that subcommand to support all the properties in a NewTerminalArgs.
// Arguments:
// - subcommand: the command to add the args to.
// Return Value:
// - <none>
void AppCommandlineArgs::_addNewTerminalArgs(AppCommandlineArgs::NewTerminalSubcommand& subcommand)
{
    subcommand.profileNameOption = subcommand.subcommand->add_option("-p,--profile",
                                                                     _profileName,
                                                                     RS_A(L"CmdProfileArgDesc"));
    subcommand.startingDirectoryOption = subcommand.subcommand->add_option("-d,--startingDirectory",
                                                                           _startingDirectory,
                                                                           RS_A(L"CmdStartingDirArgDesc"));
    subcommand.titleOption = subcommand.subcommand->add_option("--title",
                                                               _startingTitle,
                                                               RS_A(L"CmdTitleArgDesc"));

    // Using positionals_at_end allows us to support "wt new-tab -d wsl -d Ubuntu"
    // without CLI11 thinking that we've specified -d twice.
    // There's an alternate construction where we make all subcommands "prefix commands",
    // which lets us get all remaining non-option args provided at the end, but that
    // doesn't support "wt new-tab -- wsl -d Ubuntu -- sleep 10" because the first
    // -- breaks out of the subcommand (instead of the subcommand options).
    // See https://github.com/CLIUtils/CLI11/issues/417 for more info.
    subcommand.commandlineOption = subcommand.subcommand->add_option("command", _commandline, RS_A(L"CmdCommandArgDesc"));
    subcommand.subcommand->positionals_at_end(true);
}

// Method Description:
// - Build a NewTerminalArgs instance from the data we've parsed
// Arguments:
// - <none>
// Return Value:
// - A fully initialized NewTerminalArgs corresponding to values we've currently parsed.
NewTerminalArgs AppCommandlineArgs::_getNewTerminalArgs(AppCommandlineArgs::NewTerminalSubcommand& subcommand)
{
    NewTerminalArgs args{};

    if (!_commandline.empty())
    {
        std::ostringstream cmdlineBuffer;

        for (const auto& arg : _commandline)
        {
            if (cmdlineBuffer.tellp() != 0)
            {
                // If there's already something in here, prepend a space
                cmdlineBuffer << ' ';
            }

            if (arg.find(" ") != std::string::npos)
            {
                cmdlineBuffer << '"' << arg << '"';
            }
            else
            {
                cmdlineBuffer << arg;
            }
        }

        args.Commandline(winrt::to_hstring(cmdlineBuffer.str()));
    }

    if (*subcommand.profileNameOption)
    {
        args.Profile(winrt::to_hstring(_profileName));
    }

    if (*subcommand.startingDirectoryOption)
    {
        args.StartingDirectory(winrt::to_hstring(_startingDirectory));
    }

    if (*subcommand.titleOption)
    {
        args.TabTitle(winrt::to_hstring(_startingTitle));
    }

    return args;
}

// Method Description:
// - This function should return true if _no_ subcommands were parsed from the
//   given commandline. In that case, we'll fall back to trying the commandline
//   as a new tab command.
// Arguments:
// - <none>
// Return Value:
// - true if no sub commands were parsed.
bool AppCommandlineArgs::_noCommandsProvided()
{
    return !(*_newTabCommand.subcommand ||
             *_newTabShort.subcommand ||
             *_focusTabCommand ||
             *_focusTabShort ||
             *_newPaneShort.subcommand ||
             *_newPaneCommand.subcommand);
}

// Method Description:
// - Reset any state we might have accumulated back to its default values. Since
//   we'll be re-using these members across the parsing of many commandlines, we
//   need to make sure the state from one run doesn't pollute the following one.
// Arguments:
// - <none>
// Return Value:
// - <none>
void AppCommandlineArgs::_resetStateToDefault()
{
    _profileName.clear();
    _startingDirectory.clear();
    _startingTitle.clear();
    _commandline.clear();

    _splitVertical = false;
    _splitHorizontal = false;

    _focusTabIndex = -1;
    _focusNextTab = false;
    _focusPrevTab = false;

    // DON'T clear _launchMode here! This will get called once for every
    // subcommand, so we don't want `wt -F new-tab ; split-pane` clearing out
    // the "global" fullscreen flag (-F).
}

// Function Description:
// - Builds a list of Commandline objects for the given argc,argv. Each
//   Commandline represents a single command to parse. These commands can be
//   separated by ";", which indicates the start of the next commandline. If the
//   user would like to provide ';' in the text of the commandline, they can
//   escape it as "\;".
// Arguments:
// - args: an array of arguments to parse into Commandlines
// Return Value:
// - a list of Commandline objects, where each one represents a single
//   commandline to parse.
std::vector<Commandline> AppCommandlineArgs::BuildCommands(winrt::array_view<const winrt::hstring>& args)
{
    std::vector<Commandline> commands;
    commands.emplace_back(Commandline{});

    // For each arg in argv:
    // Check the string for a delimiter.
    // * If there isn't a delimiter, add the arg to the current commandline.
    // * If there is a delimiter, split the string at that delimiter. Add the
    //   first part of the string to the current command, and start a new
    //   command with the second bit.
    for (const auto& arg : args)
    {
        _addCommandsForArg(commands, { arg });
    }

    return commands;
}

// Function Description:
// - Builds a list of Commandline objects for the given argc,argv. Each
//   Commandline represents a single command to parse. These commands can be
//   separated by ";", which indicates the start of the next commandline. If the
//   user would like to provide ';' in the text of the commandline, they can
//   escape it as "\;".
// Arguments:
// - argc: the number of arguments provided in argv
// - argv: a c-style array of wchar_t strings. These strings can include spaces in them.
// Return Value:
// - a list of Commandline objects, where each one represents a single
//   commandline to parse.
std::vector<Commandline> AppCommandlineArgs::BuildCommands(const std::vector<const wchar_t*>& args)
{
    std::vector<Commandline> commands;
    // Initialize a first Commandline without a leading `wt.exe` argument. When
    // we're run from the commandline, `wt.exe` (or whatever the exe's name is)
    // will be the first argument passed to us
    commands.resize(1);

    // For each arg in argv:
    // Check the string for a delimiter.
    // * If there isn't a delimiter, add the arg to the current commandline.
    // * If there is a delimiter, split the string at that delimiter. Add the
    //   first part of the string to the current command, ansd start a new
    //   command with the second bit.
    for (const auto& arg : args)
    {
        _addCommandsForArg(commands, { arg });
    }

    return commands;
}

// Function Description:
// - Update and append Commandline objects for the given arg to the given list
//   of commands. Each Commandline represents a single command to parse. These
//   commands can be separated by ";", which indicates the start of the next
//   commandline. If the user would like to provide ';' in the text of the
//   commandline, they can escape it as "\;".
// - As we parse arg, if it doesn't contain a delimiter in it, we'll add it to
//   the last command in commands. Otherwise, we'll generate a new Commandline
//   object for each command in arg.
// Arguments:
// - commands: a list of Commandline objects to modify and append to
// - arg: a single argument that should be parsed into args to append to the
//   current command, or create more Commandlines
// Return Value:
// <none>
void AppCommandlineArgs::_addCommandsForArg(std::vector<Commandline>& commands, std::wstring_view arg)
{
    std::wstring remaining{ arg };
    std::wsmatch match;
    // Keep looking for matches until we've found no unescaped delimiters,
    // or we've hit the end of the string.
    std::regex_search(remaining, match, AppCommandlineArgs::_commandDelimiterRegex);
    do
    {
        if (match.empty())
        {
            // Easy case: no delimiter. Add it to the current command.
            commands.back().AddArg(remaining);
            break;
        }
        else
        {
            // Harder case: There was a match.
            const bool matchedFirstChar = match.position(0) == 0;
            // If the match was at the beginning of the string, then the
            // next arg should be "", since there was no content before the
            // delimiter. Otherwise, add one, since the regex will include
            // the last character of the string before the delimiter.
            const auto delimiterPosition = matchedFirstChar ? match.position(0) : match.position(0) + 1;
            const auto nextArg = remaining.substr(0, delimiterPosition);

            if (!nextArg.empty())
            {
                commands.back().AddArg(nextArg);
            }

            // Create a new commandline
            commands.emplace_back(Commandline{});
            // Initialize it with "wt.exe" as the first arg, as if that command
            // was passed individually by the user on the commandline.
            commands.back().AddArg(std::wstring{ AppCommandlineArgs::PlaceholderExeName });

            // Look for the next match in the string, but updating our
            // remaining to be the text after the match.
            remaining = match.suffix().str();
            std::regex_search(remaining, match, AppCommandlineArgs::_commandDelimiterRegex);
        }
    } while (!remaining.empty());
}

// Method Description:
// - Returns the deque of actions we've buffered as a result of parsing commands.
// Arguments:
// - <none>
// Return Value:
// - the deque of actions we've buffered as a result of parsing commands.
std::vector<ActionAndArgs>& AppCommandlineArgs::GetStartupActions()
{
    return _startupActions;
}

// Method Description:
// - Get the string of text that should be displayed to the user on exit. This
//   is usually helpful for cases where the user entered some sort of invalid
//   commandline. It's additionally also used when the user has requested the
//   help text.
// Arguments:
// - <none>
// Return Value:
// - The help text, or an error message, generated from parsing the input
//   provided by the user.
const std::string& AppCommandlineArgs::GetExitMessage()
{
    return _exitMessage;
}

// Method Description:
// - Returns true if we should exit the application before even starting the
//   window. We might want to do this if we're displaying an error message or
//   the version string, or if we want to open the settings file.
// Arguments:
// - <none>
// Return Value:
// - true iff we should exit the application before even starting the window
bool AppCommandlineArgs::ShouldExitEarly() const noexcept
{
    return _shouldExitEarly;
}

// Method Description:
// - Ensure that the first command in our list of actions is a NewTab action.
//   This makes sure that if the user passes a commandline like "wt split-pane
//   -H", we _first_ create a new tab, so there's always at least one tab.
// - If the first command in our queue of actions is a NewTab action, this does
//   nothing.
// - This should only be called once - if the first NewTab action is popped from
//   our _startupActions, calling this again will add another.
// Arguments:
// - <none>
// Return Value:
// - <none>
void AppCommandlineArgs::ValidateStartupCommands()
{
    // If we parsed no commands, or the first command we've parsed is not a new
    // tab action, prepend a new-tab command to the front of the list.
    if (_startupActions.empty() ||
        _startupActions.front().Action() != ShortcutAction::NewTab)
    {
        // Build the NewTab action from the values we've parsed on the commandline.
        NewTerminalArgs newTerminalArgs{};
        NewTabArgs args{ newTerminalArgs };
        ActionAndArgs newTabAction{ ShortcutAction::NewTab, args };
        // push the arg onto the front
        _startupActions.insert(_startupActions.begin(), 1, newTabAction);
    }
}

std::optional<winrt::Microsoft::Terminal::Settings::Model::LaunchMode> AppCommandlineArgs::GetLaunchMode() const noexcept
{
    return _launchMode;
}

// Method Description:
// - Attempts to parse an array of commandline args into a list of
//   commands to execute, and then parses these commands. As commands are
//   successfully parsed, they will generate ShortcutActions for us to be
//   able to execute. If we fail to parse any commands, we'll return the
//   error code from the failure to parse that command, and stop processing
//   additional commands.
// - The first arg in args should be the program name "wt" (or some variant). It
//   will be ignored during parsing.
// Arguments:
// - args: an array of strings to process as a commandline. These args can contain spaces
// Return Value:
// - 0 if the commandline was successfully parsed
int AppCommandlineArgs::ParseArgs(winrt::array_view<const winrt::hstring>& args)
{
    auto commands = ::TerminalApp::AppCommandlineArgs::BuildCommands(args);

    for (auto& cmdBlob : commands)
    {
        // On one hand, it seems like we should be able to have one
        // AppCommandlineArgs for parsing all of them, and collect the
        // results one at a time.
        //
        // On the other hand, re-using a CLI::App seems to leave state from
        // previous parsings around, so we could get mysterious behavior
        // where one command affects the values of the next.
        //
        // From https://cliutils.github.io/CLI11/book/chapters/options.html:
        // > If that option is not given, CLI11 will not touch the initial
        // > value. This allows you to set up defaults by simply setting
        // > your value beforehand.
        //
        // So we pretty much need the to either manually reset the state
        // each command, or build new ones.
        const auto result = ParseCommand(cmdBlob);

        // If this succeeded, result will be 0. Otherwise, the caller should
        // exit(result), to exit the program.
        if (result != 0)
        {
            return result;
        }
    }

    // If all the args were successfully parsed, we'll have some commands
    // built in _appArgs, which we'll use when the application starts up.
    return 0;
}
