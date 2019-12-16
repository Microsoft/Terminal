// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "AppCommandlineArgs.h"
#include "ActionArgs.h"

using namespace winrt::TerminalApp;
using namespace TerminalApp;

AppCommandlineArgs::AppCommandlineArgs()
{
    _BuildParser();
    _ResetStateToDefault();
}

void AppCommandlineArgs::_ResetStateToDefault()
{
    _profileName = "";
    _startingDirectory = "";
    _commandline.clear();
}

int AppCommandlineArgs::ParseCommand(const Commandline& command)
{
    int localArgc = static_cast<int>(command.Argc());
    auto localArgv = command.Argv();

    // std::cout << "######################### starting command #########################\n";
    // for (int i = 0; i < localArgc; i++)
    // {
    //     char* arg = localArgv[i];
    //     std::cout << "arg[" << i << "]=\"";
    //     printf("%s\"\n", arg);
    // }
    _ResetStateToDefault();

    try
    {
        if (localArgc == 2 && (std::string("/?") == localArgv[1] || std::string("-?") == localArgv[1]))
        {
            throw CLI::CallForHelp();
        }
        _app.clear();
        _app.parse(localArgc, localArgv);

        if (_NoCommandsProvided())
        {
            // std::cout << "Didn't find _any_ commands, using newTab to parse\n";
            _newTabCommand->clear();
            _newTabCommand->parse(localArgc, localArgv);
        }
    }
    catch (const CLI::CallForHelp& e)
    {
        return _app.exit(e);
    }
    catch (const CLI::ParseError& e)
    {
        if (_NoCommandsProvided())
        {
            // std::cout << "EXCEPTIONALLY Didn't find _any_ commands, using newTab to parse\n";
            try
            {
                _newTabCommand->clear();
                _newTabCommand->parse(localArgc, localArgv);
            }
            catch (const CLI::ParseError& e)
            {
                return _newTabCommand->exit(e);
            }
        }
        else
        {
            return _app.exit(e);
        }
    }
    return 0;
}

void AppCommandlineArgs::_BuildParser()
{
    _BuildNewTabParser();
    _BuildSplitPaneParser();

    ////////////////////////////////////////////////////////////////////////////
    _listProfilesCommand = _app.add_subcommand("list-profiles", "List all the available profiles");
    ////////////////////////////////////////////////////////////////////////////
}

void AppCommandlineArgs::_BuildNewTabParser()
{
    _newTabCommand = _app.add_subcommand("new-tab", "Create a new tab");
    _AddNewTerminalArgs(_newTabCommand);
    _newTabCommand->callback([&, this]() {
        // std::cout << "######################### new-tab #########################\n";

        auto newTabAction = winrt::make_self<implementation::ActionAndArgs>();
        newTabAction->Action(ShortcutAction::NewTab);
        auto args = winrt::make_self<implementation::NewTabArgs>();
        args->TerminalArgs(_GetNewTerminalArgs());
        newTabAction->Args(*args);
        _startupActions.push_back(*newTabAction);
    });
}

void AppCommandlineArgs::_BuildSplitPaneParser()
{
    _newPaneCommand = _app.add_subcommand("split-pane", "Create a new pane");
    _AddNewTerminalArgs(_newPaneCommand);
    auto* horizontalOpt = _newPaneCommand->add_flag("-H,--horizontal", _splitHorizontal, "TODO");
    auto* verticalOpt = _newPaneCommand->add_flag("-V,--vertical", _splitVertical, "TODO");
    verticalOpt->excludes(horizontalOpt);

    _newPaneCommand->callback([&, this]() {
        // std::cout << "######################### new-pane #########################\n";
        auto newPaneAction = winrt::make_self<implementation::ActionAndArgs>();
        newPaneAction->Action(ShortcutAction::SplitPane);
        auto args = winrt::make_self<implementation::SplitPaneArgs>();
        args->TerminalArgs(_GetNewTerminalArgs());

        if (_splitHorizontal)
        {
            args->SplitStyle(SplitState::Horizontal);
        }
        else
        {
            args->SplitStyle(SplitState::Vertical);
        }

        newPaneAction->Args(*args);
        _startupActions.push_back(*newPaneAction);
    });
}

void AppCommandlineArgs::_AddNewTerminalArgs(CLI::App* subcommand)
{
    subcommand->add_option("-p,--profile", _profileName, "Open with the give profile");
    subcommand->add_option("-d,--startingDirectory", _startingDirectory, "Open in the given directory instead of the profile's set startingDirectory");
    subcommand->add_option("cmdline", _commandline, "Commandline to run in the given profile");
}

NewTerminalArgs AppCommandlineArgs::_GetNewTerminalArgs()
{
    auto args = winrt::make_self<implementation::NewTerminalArgs>();
    if (!_profileName.empty())
    {
        args->Profile(winrt::to_hstring(_profileName));
    }

    if (!_startingDirectory.empty())
    {
        args->StartingDirectory(winrt::to_hstring(_startingDirectory));
    }

    if (!_commandline.empty())
    {
        std::string buffer;
        auto i = 0;
        for (auto arg : _commandline)
        {
            if (arg.find(" ") != std::string::npos)
            {
                buffer += "\"";
                buffer += arg;
                buffer += "\"";
            }
            else
            {
                buffer += arg;
            }
            if (i + 1 < _commandline.size())
            {
                buffer += " ";
            }
            i++;
        }
        args->Commandline(winrt::to_hstring(buffer));
    }

    return *args;
}

bool AppCommandlineArgs::_NoCommandsProvided()
{
    return !(*_listProfilesCommand ||
             *_newTabCommand ||
             *_newPaneCommand);
}

std::vector<Commandline> AppCommandlineArgs::BuildCommands(const int w_argc, const wchar_t* w_argv[])
{
    std::wstring cmdSeperator = L";";
    std::vector<Commandline> commands;
    commands.emplace_back(Commandline{});

    // Either a ; at the start of a line, or a ; preceeded by any non-\ char.
    // We need \\\\ here, to have an escaped backslash in the actual regex itself.
    std::wregex reg{ L"^;|[^\\\\];" };

    // For each arg in argv:
    // Check the string for a delimiter.
    // * If there isn't a delimiter, add the arg to the current commandline.
    // * If there is a delimiter, split the string at that delimiter. Add the
    //   first part of the string to the current command, ansd start a new
    //   command with the second bit.
    for (auto i = 0; i < w_argc; i++)
    {
        auto remaining = std::wstring{ w_argv[i] };
        std::wsmatch match;
        // Keep looking for matches until we've found no unescaped delimiters,
        // or we've hit the end of the string.
        std::regex_search(remaining, match, reg);
        do
        {
            if (match.size() == 0)
            {
                // Easy case: no delimiter. Add it to the current command.
                commands.rbegin()->AddArg(remaining);
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
                auto delimiterPosition = matchedFirstChar ? match.position(0) : match.position(0) + 1;
                auto nextArg = remaining.substr(0, delimiterPosition);

                if (nextArg != L"")
                {
                    commands.rbegin()->AddArg(nextArg);
                }

                // Create a new commandline
                commands.emplace_back(Commandline{});
                commands.rbegin()->AddArg(std::wstring{ L"wt.exe" });

                remaining = match.suffix().str();
                std::regex_search(remaining, match, reg);
            }
        } while (remaining.size() > 0);
    }

    return commands;
}
