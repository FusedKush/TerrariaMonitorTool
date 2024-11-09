/*
* UserInterface.cpp
*
* Source File defining the UserInterface class, which encapsulates all of the functionality
* used to provide a fluid and consistent User Interface for the program.
*/


#include "UserInterface.h"
#include <filesystem>
#include <fstream>
#include <regex>
#include <ShlObj.h>


namespace PROGRAM_NAMESPACE {

    /* TextSizingStruct */
    // Structure Constructors

    UserInterface::TextSizingStruct::TextSizingStruct () : TextSizingStruct(6U, 12U, 12U, 20U, 12U, 2U) {}
    UserInterface::TextSizingStruct::TextSizingStruct (
        text_sizing_t iLeftColSize,
        text_sizing_t iDisplayIdColSize,
        text_sizing_t iMonitorNameColSize,
        text_sizing_t iResolutionColSize,
        text_sizing_t iCommentsColSize,
        text_sizing_t iExtraColPadding
    ) :
        leftColSize(iLeftColSize),
        displayIdColSize(iDisplayIdColSize),
        monitorNameColSize(iMonitorNameColSize),
        resolutionColSize(iResolutionColSize),
        commentsColSize(iCommentsColSize),
        extraColPadding(iExtraColPadding),
        consoleBoxWidth(
              ( iLeftColSize + iDisplayIdColSize + iMonitorNameColSize + iResolutionColSize + iCommentsColSize )
            + (iExtraColPadding * 4U) + 4U  // Whitespace Padding
            + 2U                            // Box Border
        ),
        boxBorder( L"+" + std::wstring(this->consoleBoxWidth + 2U, L'-') + L"+" ),
        boxSpace( L"|" + std::wstring(this->consoleBoxWidth + 2U, L' ') + L"|" )
    {}

    // Structure Methods

    UserInterface::TextSizingStruct UserInterface::TextSizingStruct::withNewMonitorNameColSize ( text_sizing_t newMonitorNameColSize ) const {
    
        return TextSizingStruct(
            this->leftColSize, this->displayIdColSize, newMonitorNameColSize,
            this->resolutionColSize, this->commentsColSize, this->extraColPadding
        );
    
    }


    /* UserInterface::ConfigurationPathHistory */
    // Class Constants

    const std::wstring UserInterface::ConfigurationPathHistory::PATH_HISTORY_FILE_NAME = L"path_history";
    const std::filesystem::path UserInterface::ConfigurationPathHistory::PATH_HISTORY_FILE_PATH = { PROGRAM_DATA_PATH / PATH_HISTORY_FILE_NAME };

    // Serialization & Persistence to File

    UserInterface::ConfigurationPathHistory UserInterface::ConfigurationPathHistory::fetchFromFile () {

        // The new `ConfigurationPathHistory` object being returned.
        ConfigurationPathHistory pathHistory = {};

        if ( !programSettings.statelessMode ) {
            std::wifstream fileStream(PATH_HISTORY_FILE_PATH);  // The File Stream used to read the Configuration Path History File.
            std::wstring currentLine = {};                      // Contains the Current Line from the Configuration Path History File.

            while ( fileStream.good() ) {
                std::getline(fileStream, currentLine);

                if ( !currentLine.empty() ) {
                    try {
                        pathHistory.emplace_back(currentLine);
                    }
                    catch (...) {
                        // Ignore invalid lines
                    }
                }
            }
        }

        // Return the `pathHistory`, regardless of if it was populated with
        // any Configuration File Paths or not.
        return pathHistory;
    
    }

    bool UserInterface::ConfigurationPathHistory::saveToFile () {

        // Don't modify the Configuration Path History File in Stateless Mode.
        if (programSettings.statelessMode)
            return true;

        // The path to the Temporary File used to write the contents of the
        // Configuration Path History to file. 
        std::optional<std::wstring> tempFilePath = UTILS_NAMESPACE::createTempFile();
    
        if (tempFilePath) {
            // The File Stream used to write to the Temporary File.
            std::wofstream fileStream(*tempFilePath);

            if ( fileStream.good() ) {
                if ( !this->empty() ) {
                    // A reference to the first path in the Configuration Path History.
                    const std::filesystem::path& firstPath = this->front();

                    for ( const std::filesystem::path& path: *this ) {
                        if (path != firstPath)
                            fileStream << L'\n';

                        // We need to explicitly call a method like `c_str()` to avoid passing the 
                        // `path` directly to `<<`, which will use `std::quoted()` in the output.
                        fileStream << path.c_str();

                        if ( !fileStream.good() )
                            break;
                    }

                    // Once we have finished writing the updated contents of the
                    // to the Temporary File, we can replace the contents of the 
                    // Configuration Path History File with the Temporary File.
                    if ( fileStream.good() ) {
                        fileStream.close();
                        
                        if ( ensureProgramDataDirectoryExists(nullptr) ) {
                            std::filesystem::rename(*tempFilePath, PATH_HISTORY_FILE_PATH);
                            return true;
                        }
                    }
                }
            }
        }


        return false;
    
    }

    bool UserInterface::ConfigurationPathHistory::deleteSavedData () {
    
        // Don't modify the Configuration Path History File in Stateless Mode.
        if ( programSettings.statelessMode || !std::filesystem::exists(PATH_HISTORY_FILE_PATH) )
            return true;

        return std::filesystem::remove(PATH_HISTORY_FILE_PATH);
    
    }


    /* UserInterface */
    // Class Constructors

	UserInterface::UserInterface ( const Console::console_ptr_t& iConsolePtr ) : console(iConsolePtr) {

        // Use the setter method to ensure the `programTitle` is properly updated.
        this->changeTextSizing({});

    }

    // Getter & Setter Instance Methods

    const UserInterface::TextSizing& UserInterface::getTextSizing () const {
        
        return this->textSizing;
    
    }
    UserInterface::TextSizing UserInterface::changeTextSizing ( const TextSizing& newTextSizing ) {
    
        std::wstring greenTermSeq = Console::getVirtualTerminalSequence(L"[92m");
        TextSizing oldTextSizing = std::move(this->textSizing);

        this->textSizing = newTextSizing;

        // Generate the `programTitle` according to the new `TextSizing` structure.
        this->programTitle = std::regex_replace(
            std::regex_replace(
                std::format(
                    L"| {:s}{:^{}s}{:s} |",
                    greenTermSeq,
                    PROGRAM_TITLE,
                    newTextSizing.consoleBoxWidth,
                    Console::getVirtualTerminalSequence(L"[39m")
                ), 
                std::wregex(L"by (.+) \\("), 
                std::format(
                    L"by {:s}$1{:s} (",
                    Console::getVirtualTerminalSequence(L"[94m"),
                    greenTermSeq
                )
            ),
            std::wregex(L"v[0-9a-zA-z.]+"),
            std::format(
                L"{:s}$&{:s}",
                Console::getVirtualTerminalSequence(L"[96m"),
                greenTermSeq
            )
        );

        return std::move(oldTextSizing);
    
    }

    // User Interface Procedures

    void UserInterface::printUsageMessage ( int argc, LPCWSTR argv[] ) const {

        // Defines the minimum width of the column containing the Command-Line Flags
        // and Switches for the Program Help/Usage Message. 
        const unsigned short MAIN_USAGE_FLAG_WIDTH = 40U;

        // Defines the Command-Line Flags and their associated quick summaries,
        // to be printed for the Program Help/Usage Message.
        const std::vector< std::pair<std::wstring, std::wstring> > FLAG_SUMMARY_MAP = {
            { L"/?, --help, --usage",                   L"Get help and usage information" },
            { L"-v, --version",                         L"Display Version Information" },
            { L"-d, --dry-run",                         L"Don't write changes to the Configuration File" },
            { L"-s, --stateless",                       L"Skips reading from or writing to any program files" },
            { L"-y, --yes",                             L"Automatically answer \"yes\" to all Confirmation Prompts" },
            { L"-b, --disable-custom-buffer-behavior",  L"Disable custom behavior for Console Output Buffers" },
            { L"    --clear-program-data",              L"Clear existing Program Data before launch" },
            { L"    --debug",                           L"Enable functionality useful for debugging" }
        };

        // Print the Help/Usage Message for the Specified Command-Line Argument, Flag, or Switch (if possible).
        if ( argc > 2 ) {
            std::wstring arg(argv[2]);                                      // The value of the current Command-Line Argument being processed.
            std::wstring lcArg = UTILS_NAMESPACE::stringToLowercase(arg);   // The lowercase value of the current Command-Line Argument being processed.
            
            if ( arg == L"-d" || lcArg == L"--dry-run" ) {
                this->printArgUsageMessage(
                    L"Dry Run Mode",
                    L"[ -d | --dry-run ]",

                    L"Writes the contents of the modified Terraria Configuration File to",
                    L"the Console instead of saving them to the Configuration File."
                );
                return;
            }
            else if ( arg == L"-s" || lcArg == L"--stateless" ) {
                this->printArgUsageMessage(
                    L"Stateless Mode",
                    L"[ -s | --stateless ]",

                    L"Skips any optional reading from or writing to any program files.",
                    L"",
                    L"As a result, no changes to any existing program data will be made",
                    L"by the program, even if changes are made to the Program Settings.",
                    L"",
                    L"This option does *not* effect whether or not changes are made to",
                    L"the Terraria Configuration File or any Backup Files (see --dry-run instead)."
                );
                return;
            }
            else if ( arg == L"-y" || lcArg == L"--yes" ) {
                this->printArgUsageMessage(
                    L"Automatically Confirm Prompts",
                    L"[ -y | --yes ]",

                    L"Automatically confirms or answers \"yes\" to any Confirmation Prompts.",
                    L"",
                    L"This flag should be used with caution, as it permits for potentially dangerous",
                    L"actions to proceed without any further confirmation.",
                    L"",
                    L"This flag applies to Confirmation Prompts triggered in response",
                    L"to both Comamnd-Line Arguments, as well as Interactive Menu Selections."
                );
                return;
            }
            else if ( arg == L"-b" || lcArg == L"--disable-custom-buffer-behavior" ) {
                this->printArgUsageMessage(
                    L"Custom Buffer Behavior",
                    L"[ -b | --disable-custom-buffer-behavior ]",

                    L"Disables custom behavior for the Console Output Buffers, including tweaking how",
                    L"the Console is Cleared and the way Alternate Output Buffers Work.",
                    L"",
                    L"If you are encountering issues with how the program is rendered when switching between",
                    L"screens or clearing the console, you can try running the program with this flag."
                );
                return;
            }
            else if ( lcArg == L"--clear-program-data" ) {
                this->printArgUsageMessage(
                    L"Clear Program Data Before Launch",
                    L"[ --clear-program-data [ -y | --yes ] ]",

                    L"Clear any existing Program Data before launching the program.",
                    L"",
                    L"Applies to files used internally by the program, as well as",
                    L"any existing Backup Configuration Files. This does *not* apply to",
                    L"any Terraria Configuration Files that are not being used as backups.",
                    L"",
                    L"This action requires confirmation before proceeding. To proceed automatically",
                    L"when launching the program, you can use the -y or --yes flag."
                );
                return;
            }
            else if ( arg == L"-v" || lcArg == L"--version" ) {
                this->printArgUsageMessage(
                    L"Version Details",
                    L"[ -v | --version ]",

                    L"Displays Version Information about the Program."
                );
                return;
            }
            else if ( lcArg == L"--debug" ) {
                this->printArgUsageMessage(
                    L"Debug-Friendly Mode",
                    L"[ --debug ]",

                    L"Enables functionality useful for debugging, including enabling additional logging",
                    L"and adding a delay at the start of the program for debuggers to be attached."
                );
                return;
            }
        }

        // Print the Help/Usage Message for the Main Program.
        this->printInterfaceHeader(
            L"A simple command-line tool used to change the",
            L"display monitor Terraria renders the game on."
        ).console
        ->println()
         .println(L"Usage:")
         .println(L"TerrariaMonitorTool [ /?|--help|--usage [<Option or Switch>] ] [ -v | --version ]")
         .println(L"                    [ -d|--dry-run ] [ -s|--stateless ] [ -y|--yes ]")
         .println(L"                    [ -b|--disable-custom-buffer-behavior ]")
         .println(L"                    [ --clear-program-data ] [ --debug ]")
         .println();

        for ( const auto& pair : FLAG_SUMMARY_MAP )
            console->printf(L"\n{:<{}s} {:s}", pair.first, MAIN_USAGE_FLAG_WIDTH, pair.second);
    
    }

    std::optional<std::wstring> UserInterface::promptForConfigFilePath () const {

        // The maximum permitted length for a Custom Configuration File Path,
        // *not* including the Filename and the Path Separator between the Path and Filename.
        size_t MAX_BASE_CUSTOM_PATH_LENGTH = MAX_PATH;
        // The maximum permitted length for a Custom Configuration File Path,
        // *including* the Filename and the Path Separator between the Path and Filename.
        size_t MAX_CUSTOM_PATH_LENGTH = (
            MAX_BASE_CUSTOM_PATH_LENGTH     // The maximum supported length in total is `MAX_BASE_CUSTOM_PATH_LENGTH`.
            - CONFIG_FILE_NAME.length()     // Account for the Filename of the Configuration File.
            - 1ULL                          // Account for the Path Separator between the Path and Filename.
        );

        // The Configuration Path History, if any exists.
        ConfigurationPathHistory pathHistory = ConfigurationPathHistory::fetchFromFile();

        std::optional<std::wstring> configFileDirPath = { L"" };    // The path to the directory containing the Terraria Configuration File.
        std::wstring configFilePath = {};                           // The full path to the Terraria Configuration File.
        bool isValidPath = false;                                   // Indicates whether the configFilePath exists or not.
        std::optional<size_t> selectionNum = {};                    // The current selection in the list of `menuOptions`.

        std::vector<Console::MenuOptionList::MenuOptionListAction>
        actionList(Console::MenuOptionList::DEFAULT_ACTIONS);       // The actions applied to the list of `menuOptions`.
        actionList[1].instructions[0] = L"Press ESC to exit the program.";

        size_t deletePathActionIndex = actionList.size();           // The index of the Action used to remove individual paths from the Configuration Path History.

        /**
         * A lambda helper function used to print the User Interface for the
         * Configuration File Path Screen.
         * 
         * Depends on this object.
         * 
         * @param console       The `Console` used to print the User Interface.
         * @param menuOptions   The `MenuOptionList` being printed.
         */
        auto printUserInterface = [this]( Console& console, Console::MenuOptionList& menuOptions ) -> void {

            this->printInterfaceHeader(L"Configuration File Path");
            console.printMenuOptions(menuOptions, true);

        };

        /**
         * A lambda helper function used to prompt the user for a Custom Configuration File Path.
         * 
         * Depends on and potentially modifies the `selectionNum` variable. Also depends on
         * the local `MAX_CUSTOM_PATH_LENGTH` constant.
         * 
         * @param console   The `Console` used to print the User Interface and prompt the user for input.
         */
        auto promptForCustomPath = [&selectionNum, MAX_CUSTOM_PATH_LENGTH]( const Console& console ) {

            console.print(L"Path to Configuration File: ");
            std::optional<std::wstring> customPath = console.waitForInputData(MAX_CUSTOM_PATH_LENGTH + 1ULL);

            if (customPath) {
                customPath = UTILS_NAMESPACE::trimString(*customPath);

                if (customPath->ends_with(L"\\"))
                    customPath->pop_back();
            }
            else {
                selectionNum.reset();
            }

            return customPath;

        };

        /**
         * A lambda helper function used to change the `configFileDirPath` and `configFilePath` variables
         * 
         * @param newConfigFileDirPath  The new value for the `configFileDirPath` variable.
         */
        auto changeConfigFilePaths = [&configFileDirPath, &configFilePath] ( std::optional<std::wstring>&& newConfigFileDirPath ) {

            configFileDirPath = std::move(newConfigFileDirPath);

            if ( configFileDirPath && !configFileDirPath->empty() )
                configFilePath = (*configFileDirPath + L"\\" + CONFIG_FILE_NAME);

        };

        /**
         * A lambda helper function used to get and add the Default Configuration File Path
         * to the list of `menuOptions`, if it can be detected for the current user.
         * 
         * Depends on this object and the `changeConfigFilePaths()` lambda function, as well as the
         * `configFileDirPath` and `configFilePath` variables, which this lambda function may modify.
         */
        auto addDefaultConfigPath = [this, &changeConfigFilePaths, &configFileDirPath, &configFilePath] (
            Console::MenuOptionList& menuOptions
        ) {

            // Wide-Character Null-Terminated String to the Current User's Documents Folder.
            // Has to be freed using `CoTaskMemFree` according to the Windows API.
            PWSTR documentsFolderPath;
            // The Folder ID being retrieved from the Windows API.
            // Has to be stored in a variable according to the Windows API.
            KNOWNFOLDERID folderId = FOLDERID_Documents;

            if ( SUCCEEDED( SHGetKnownFolderPath(folderId, 0, NULL, &documentsFolderPath) ) ) {
                changeConfigFilePaths( documentsFolderPath + std::wstring(L"\\My Games\\Terraria") );

                // Only display the Default Configuration File Path if it has been detected for the current user.
                if ( std::filesystem::exists(configFilePath) ) {
                    menuOptions.emplace(
                        menuOptions.cbegin(),
                        UTILS_NAMESPACE::truncatePathString(
                            *configFileDirPath + L" (Default)",
                            this->textSizing.consoleBoxWidth
                        )
                    );
                }
            }

            CoTaskMemFree(documentsFolderPath);

        };

        /**
         * A lambda helper function used to remove the Actions used to manage the
         * `pathHistory` from the list of `menuOptions`.
         * 
         * Depends on the `addDefaultConfigPath()` lambda function, as well as
         * the `deletePathActionIndex` and `pathHistory` variables, which this
         * lambda function may potentially modify.
         * 
         * @param menuOptions   The `MenuOptionList` being modified.
         */
        auto removePathActions = [&addDefaultConfigPath, &deletePathActionIndex, &pathHistory] ( Console::MenuOptionList& menuOptions ) {

            auto& actionList = menuOptions.getActions();    // A reference to the list of Actions for the list of `menuOptions`.
            auto actionListStartItr = actionList.begin();   // Iterator to the first element of the `actionList` being deleted.
            auto actionListEndItr = actionList.begin();     // Iterator to one position past the last element of the `actionList` being deleted.

            // Advance the iterators to the elements being deleted.
            std::advance(actionListStartItr, deletePathActionIndex);
            std::advance(actionListEndItr, deletePathActionIndex + 2);

            // Delete the Configuration Path History File since it is currently empty.
            pathHistory.deleteSavedData();
            // Remove the actions from the list of `menuOptions`.
            actionList.erase(actionListStartItr, actionListEndItr);
            // Try to add the Default Configuration File Path to the list of `menuOptions`.
            addDefaultConfigPath(menuOptions);

        };

        // Expose additional Actions for managing the Configuration Path History if applicable.
        if ( !pathHistory.empty() ) {
            // Remove individual paths from the Configuration Path History
            actionList.emplace_back(
                [this, &pathHistory, &printUserInterface, &removePathActions, deletePathActionIndex](
                    const Console::WinConsoleInputKey& key,
                    Console::MenuOptionList& menuOptions,
                    Console& console,
                    std::optional<size_t>& currentSelectionNum
                ) -> Console::MenuOptionList::MenuOptionListAction::InputProcessingResult {

                    // A reference to the list of Actions for the list of `menuOptions`.
                    auto& actionList = menuOptions.getActions();
                    // The result of the Action returned by the Action Function.
                    Console::MenuOptionList::MenuOptionListAction::InputProcessingResult result = {};

                    // Matches `DEL` (but explicitly *not* `SHIFT + DEL`)
                    if ( key.wVirtualKeyCode == VK_DELETE && !(key.dwControlKeyState & SHIFT_PRESSED) ) {
                        // Only trigger for `MenuOption`s that correspond to elements in the `pathHistory`.
                        if ( currentSelectionNum && *currentSelectionNum < (menuOptions.size() - 2ULL) ) {
                            auto pathHistoryItr = pathHistory.begin();  // Iterator to the path in the `pathHistory` being removed.
                            auto menuOptionsItr = menuOptions.begin();  // Iteraror to the `MenuOption` in the list of `menuOptions` being removed.

                            // Advance the iterators to the selected path.
                            std::advance(pathHistoryItr, *currentSelectionNum);
                            std::advance(menuOptionsItr, *currentSelectionNum);

                            // Confirm the destructive action with the user.
                            std::optional<bool> confirmation = this->promptForConfirmation(
                                L"Remove the following path from the Configuration Path History?",
                                UTILS_NAMESPACE::truncatePathString(*pathHistoryItr, this->textSizing.consoleBoxWidth)
                            );

                            // On confirmation, remove the path from the Configuration Path History.
                            // Otherwise, we will simply return the selection menu.
                            if (confirmation && *confirmation) {
                                menuOptions.setStatusMessage( std::wstring(L"Successfully removed path: ") + pathHistoryItr->c_str() );
                                menuOptions.erase(menuOptionsItr);
                                pathHistory.erase(pathHistoryItr);
                                pathHistory.saveToFile();

                                // If the Configuration Path History is empty, remove the additional
                                // Menu Option Actions used to manage it since they are no longer needed.
                                if ( pathHistory.empty() )
                                    removePathActions(menuOptions);

                                // Re-draw the User Interface to reflect the changes made to the Configuration Path History.
                                console.clear();
                                printUserInterface(console, menuOptions);
                            }

                            // Stop processing for the current key.
                            result = std::make_pair(true, false);
                        }
                    }

                    return result;

                },
                L"Press DEL to remove the selected path from the Config Path History."
            );

            // Clear the Configuration Path History
            actionList.emplace_back(
                [this, &pathHistory, &printUserInterface, &removePathActions, deletePathActionIndex](
                    const Console::WinConsoleInputKey& key,
                    Console::MenuOptionList& menuOptions,
                    Console& console,
                    std::optional<size_t>& currentSelectionNum
                ) -> Console::MenuOptionList::MenuOptionListAction::InputProcessingResult {

                        // A reference to the list of Actions for the list of `menuOptions`.
                        auto& actionList = menuOptions.getActions();
                        // The result of the Action returned by the Action Function.
                        Console::MenuOptionList::MenuOptionListAction::InputProcessingResult result = {};

                        // Matches `SHIFT + DEL`
                        if ( key.wVirtualKeyCode == VK_DELETE && (key.dwControlKeyState & SHIFT_PRESSED) ) {
                            // Confirm the destructive action with the user.
                            std::optional<bool> confirmation = this->promptForConfirmation(
                                L"Are you sure you want to completely clear the Configuration Path History?"
                            );

                            // On confirmation, remove the path from the Configuration Path History.
                            // Otherwise, we will simply return the selection menu.
                            if (confirmation && *confirmation) {
                                // Iterator to the first non-path `MenuOption` in the list of `menuOptions`.
                                auto menuOptionsItr = menuOptions.end();

                                std::advance(menuOptionsItr, -2);

                                // Erase all of the path `MenuOption`s in the list of `menuOptions`.
                                menuOptions.erase(menuOptions.begin(), menuOptionsItr);
                                // Clear the Configuration Path History.
                                pathHistory.clear();
                                pathHistory.deleteSavedData();
                                // Delete the Configuration Path History File and remove the Actions
                                // used to manage the Configuration Path History.
                                removePathActions(menuOptions);

                                // Re-draw the User Interface to reflect the changes made to the Configuration Path History.
                                menuOptions.setStatusMessage(L"Successfully cleared the Configuration Path History.");
                                console.clear();
                                printUserInterface(console, menuOptions);
                            }

                            // Stop processing for the current key.
                            result = std::make_pair(true, false);
                        }

                        return result;

                },
                L"Press SHIFT + DEL to clear the Config Path History."
            );
        }

        // The list of `MenuOption`s presented to the user.
        Console::MenuOptionList menuOptions = {
            actionList,
            L"| ",
            L" |",
            this->textSizing.boxBorder,
            this->textSizing.consoleBoxWidth,
            6
        };

        // Add each of the paths from the `pathHistory` to the list of `menuOptions`.
        if ( !pathHistory.empty() ) {
            for ( const std::filesystem::path& path : pathHistory )
                menuOptions.emplace_back(
                    UTILS_NAMESPACE::truncatePathString(path.wstring(), this->textSizing.consoleBoxWidth)
                );
        }
        // Get the default path to the Terraria Configuration File.
        else {
            addDefaultConfigPath(menuOptions);
        }

        // Add the non-Configuration Path `MenuOption`s.
        menuOptions.emplace_back(
            L"Enter Custom Path",
            L'c',
            false,
            Console::MenuOption::MenuOptionPadding(true)
        );
        menuOptions.emplace_back(L"Exit", L'.');

        // Print the User Interface.
        console->toggleCursorVisibility(false);
        printUserInterface(*console, menuOptions);

        // Wait for the user to make a selection in the Interactive Menu.
        selectionNum = console->waitForSelection(menuOptions);

        // Continue waiting for the user to make a selection in the Interactive Menu
        // until a valid Configuration File Path or other `MenuOption` is selected.
        while ( selectionNum && !isValidPath ) {
            // A reference to the user's selection in the list of `menuOptions`.
            auto& selection = menuOptions[*selectionNum];

            if (selection.option == L"Enter Custom Path") {
                console->createAltBuffer();

                // Prompt the user for a custom Configuration File Path.
                changeConfigFilePaths( promptForCustomPath(*console) );

                while ( configFileDirPath && (configFileDirPath->empty() || configFilePath.length() > MAX_CUSTOM_PATH_LENGTH || !(isValidPath = std::filesystem::exists(configFilePath)) ) ) {
                    console->clear(true);

                    // Only print an error if the specified path is not empty or blank.
                    if ( !configFileDirPath->empty() ) {
                        if ( configFilePath.length() > MAX_CUSTOM_PATH_LENGTH ) {
                            console->printfln(L"The Maximum Supported Length of the Configuration File Path is {:d} characters, including the Configuration File itself.", MAX_BASE_CUSTOM_PATH_LENGTH)
                                    .println(L"Please choose a different Terraria Configuration File or shorten the path and try again.");
                        }
                        else if ( !isValidPath ) {
                            console->printfln(L"The Terraria Configuration File ({:s}) could not be found in the specified location.", CONFIG_FILE_NAME)
                                    .println(L"Check the specified path to the Terraria Configuration File and try again.")
                                    .println();
                        }
                    }

                    // Continue prompting the user for a custom Configuration File Path
                    // until a valid path is specified.
                    changeConfigFilePaths( promptForCustomPath(*console) );
                }

                if (configFileDirPath) {
                    // When a custom Configuration File Path is specified, we can use `std::filesystem::canonical()`
                    // to get the case-normalized path for both storage and display.
                    changeConfigFilePaths( std::filesystem::canonical(*configFileDirPath) );
                    // Once we're done prompting for a custom Configuration File Path, we can
                    // close the Alternate Output Buffer and restore the previous User Interface.
                    console->restorePreviousBuffer();
                }
            }
            else if (selection.option != L"Exit") {
                // When a path from the Configuration Path History is selected,
                // `configFileDirPath` and `configFilePath` need to be updated to the selected path.
                if ( !selection.option.ends_with(L" (Default)") )
                    changeConfigFilePaths(selection.option);

                // Ensure the previously-used path still exists.
                if ( !(isValidPath = std::filesystem::exists(configFilePath)) ) {
                    console->printfln(L"The Terraria Configuration File ({:s}) could not be found in the specified location.", CONFIG_FILE_NAME)
                            .println(L"Check the specified path to the Terraria Configuration File or specify a different path and try again.")
                            .println();
                }
            }
            else {
                // Indicate that we are attempting to exit.
                selectionNum = std::optional<size_t>();
            }

            // Continue to wait for the user to make a selection in the Interactive Menu
            // until a valid Configuration File Path or other `MenuOption` is selected.
            if ( selectionNum && !isValidPath )
                selectionNum = console->waitForSelection(menuOptions);
        }

        // Once we are done selecting the Configuration File Path, we can remove
        // the Alternate Output Buffer and restore the previous User Interface.
        console->clear(true);

        // If a valid Configuration File Path is specified, add it to the
        // `pathHistory` or move it to the front if it already has been.
        if (isValidPath) {
            // Iterator to the end of the `pathHistory`.
            auto pathHistoryEndItr = pathHistory.end();
            // Iterator to the existing path in the `pathHistory` or `pathHistoryEndItr` if
            // the path does not exist in the `pathHistory` yet.
            auto existingPathItr = std::find( pathHistory.begin(), pathHistoryEndItr, *configFileDirPath );

            // If the path does not exist in the `pathHistory` yet, it is added to the front.
            if ( existingPathItr == pathHistoryEndItr ) {
                pathHistory.emplace_front( std::filesystem::path(*configFileDirPath).native() );
                pathHistory.saveToFile();
            }
            // If the path already exists in the `pathHistory`, it will be
            // moved to the front if it is not already.
            else if ( existingPathItr != pathHistory.begin() ) {
                pathHistory.splice(pathHistory.begin(), pathHistory, existingPathItr);
                pathHistory.saveToFile();
            }
        }

        return (
            isValidPath
                ? std::optional<std::wstring>(configFilePath)
                : std::optional<std::wstring>()
        );
	
	}
    
    std::optional<UserInterface::MainMenuSelection> UserInterface::mainMenu (
        const std::wstring& configFilePath,
        const DisplayMonitorList& displayMonitors,
        bool renderMenu,
        DisplayMonitor::display_number_t selectedMonitorNum
    ) const {
    
        const TextSizing& ts = this->textSizing;                // Constant reference to this object's `TextSizing` object.
        size_t displayMonitorCount = displayMonitors.size();    // The number of Connected Display Monitors.
        auto menuOptionActions = Console::MenuOptionList::DEFAULT_ACTIONS;
             menuOptionActions[1].instructions[0] = L"Press ESC to exit the program.";

        /**
         * The `MenuOptionList` used to render the Main Menu,
         * which is static to enable us to keep track of the
         * Main Menu's position in the console.
         */
        static Console::MenuOptionList menuOptions = {
            menuOptionActions,
            L"| ",
            L" |",
            ts.boxBorder,
            (unsigned short) ts.consoleBoxWidth,
            8
        };
        // Keeps track of the `selectedMonitorNum` between method calls.
        static DisplayMonitor::display_number_t previousSelectedMonitorNum;

        // Render the Main Menu for the first time.
        if (renderMenu) {
            // Ensure that any cached (static) information is properly reset
            // whenever we are rendering a new Main Menu. 
            if ( !menuOptions.empty() ) {
                menuOptions.clear();
            }
            
            previousSelectedMonitorNum = selectedMonitorNum;

            // Generate a `MenuOption` object for each Connected Display Monitor
            // and add it to our list of `menuOptions`.
            for (const DisplayMonitor& monitor : displayMonitors) {
                menuOptions.emplace_back(
                    std::format(
                        L"{:^{}s} {:{}s} {:{}s} {:{}s} {:{}s}",
                        (
                            (selectedMonitorNum == monitor.displayNum)
                                ? L"*"
                                : L""
                        ),                                          (1U + ts.extraColPadding),
                        monitor.displayId,                          (ts.displayIdColSize + ts.extraColPadding),
                        monitor.monitorName,                        (ts.monitorNameColSize + ts.extraColPadding),  
                        monitor.currentResolution.resolutionString, (ts.resolutionColSize + ts.extraColPadding),
                        monitor.comments,                           (ts.commentsColSize + ts.extraColPadding)
                    ),
                    std::optional<wchar_t>()
                );
                
                if (selectedMonitorNum == monitor.displayNum)
                    menuOptions.setSelectedOption(menuOptions.size() - 1ULL);
            }

            // Add the remaining `MenuOption`s to our list of `menuOptions`.
            menuOptions.emplace_back(
                L"Configuration File Backups", L'b', 
                true, 
                Console::MenuOption::MenuOptionPadding(true)
            );
            menuOptions.emplace_back(L"Settings", L's', true);
            menuOptions.emplace_back(L"Exit", L'.', false);

            // Render the Main Menu 
            this->console->toggleCursorVisibility(false);
            this->printInterfaceHeader(
                L"",
                L"Editing Configuration File:",
                UTILS_NAMESPACE::truncatePathString(configFilePath, ts.consoleBoxWidth - 4ULL)
            )
                 .console
                ->printfln(
                    L"| {:{}s} {:{}s} {:{}s} {:{}s} {:{}s} |",
                    L"",                     (ts.leftColSize + ts.extraColPadding),
                    L"Display:",             (ts.displayIdColSize + ts.extraColPadding),
                    L"Monitor Name:",        (ts.monitorNameColSize + ts.extraColPadding),
                    L"Current Resolution:",  (ts.resolutionColSize + ts.extraColPadding),
                    L"Comments:",            (ts.commentsColSize + ts.extraColPadding)
                 )
                 .println(ts.boxSpace)
                 .printMenuOptions(menuOptions, true);
        }
        // Update the Main Menu to reflect any changes.
        else if (selectedMonitorNum != previousSelectedMonitorNum) {
            if ( !menuOptions.getCursorStartPos() )
                throw std::logic_error("The Cursor Start Position of the MenuOptionList has not been properly set.");

            auto markerPos = *menuOptions.getCursorStartPos();
                 markerPos.X += 6;

            previousSelectedMonitorNum = selectedMonitorNum;
            this->console->saveCursorPos();

            // Re-render the list of Connected Display Monitors to reflect any changes
            // made to the Active Display Monitor.
            for (size_t i = 0ULL; i < displayMonitors.size(); i++) {
                const DisplayMonitor& monitor = displayMonitors[i];
                Console::MenuOption& menuOption = menuOptions[i];

                this->console->setCursorPos(markerPos);

                if (monitor.displayNum != selectedMonitorNum) {
                    this->console->print(L' ');
                }
                else {
                    this->console->print(L'*');
                    menuOptions.setStatusMessage(L"Successfully set " + monitor.monitorName + L" as the Active Display Monitor!");
                }

                markerPos.Y += 1;
            }

            this->console->restoreSavedCursorPos();
        }

        // Wait for the user to make a valid selection and return the appropriate value.
        std::optional<size_t> selection = this->console->waitForSelection(menuOptions);

        if (selection) {
            // A Connected Display Monitor was selected to be made the Active Display Monitor.
            if ( *selection < displayMonitors.size() )
                return displayMonitors[*selection];

            // Another Menu Option was selected.
            else if ( menuOptions[*selection].option == L"Configuration File Backups" )
                return CONFIG_FILE_BACKUPS_MENU_OPTION;
            else if ( menuOptions[*selection].option == L"Settings" )
                return PROGRAM_SETTINGS_MENU_OPTION;
            else if ( menuOptions[*selection].option == L"Exit" )
                return EXIT_MENU_OPTION;
        }

        // `ESC` was used to exit the Main Menu (or an invalid option was somehow selected).
        return std::optional<MainMenuOption>();
    
    }

    std::optional<bool> UserInterface::promptForConfirmation (
        const std::wstring& title,
        const std::wstring& subtitle
    ) const {
    
        // The default `MenuOptionList` presented to the user.
        Console::MenuOptionList menuOptions = {
            {
                { L"Yes", L'y' },
                { L"No", L'n' }
            },
            Console::MenuOptionList::DEFAULT_ACTIONS,
            L"| ",
            L" |",
            this->textSizing.boxBorder,
            this->textSizing.consoleBoxWidth
        };

        return this->promptForConfirmation(menuOptions, title, subtitle);
    
    }
    std::optional<bool> UserInterface::promptForConfirmation (
        Console::MenuOptionList& menuOptions,
        const std::wstring& title,
        const std::wstring& subtitle
    ) const {
        
        if (programSettings.autoConfirmPrompts)
            return std::optional<bool>(true);

        this->console->createAltBuffer();
        this->console->toggleCursorVisibility(false);
        this->console
            ->println(this->textSizing.boxBorder)
             .printfln(
                 L"{:s}{:^{}s}{:s}",
                 menuOptions.getPrefix(),
                 title, this->textSizing.consoleBoxWidth,
                 menuOptions.getSuffix()
             )
             .printfln(
                 L"{:s}{:^{}s}{:s}",
                 menuOptions.getPrefix(),
                 subtitle, this->textSizing.consoleBoxWidth,
                 menuOptions.getSuffix()
             )
             .println(this->textSizing.boxBorder)
             .printMenuOptions(menuOptions, true);

        // Prompt the user for confirmation.
        std::optional<size_t> selection = this->console->waitForSelection(menuOptions);
        this->console->restorePreviousBuffer();

        return (
            selection
                // Only the first `MenuOption` will return `true`, while all others will return `false`.
                ? std::optional<bool>( !((bool) *selection) )
                : std::optional<bool>()
        );
    
    }

    // Helper Methods

    template <typename... Args>
    const UserInterface& UserInterface::printInterfaceHeader ( const Args&... subtitle ) const {

        const Console& console = *this->console;

        console.println(this->textSizing.boxBorder)
               .println(this->programTitle);

        // Executes the lambda function once per line of the `subtitle`.
        ([&] {
            console.printfln(L"| {:^{}s} |", subtitle, this->textSizing.consoleBoxWidth);
        } (), ...);
        
        console.println(this->textSizing.boxBorder);
        return *this;

    }
    template <typename... Args>
    UserInterface& UserInterface::printInterfaceHeader ( const Args&... subtitle ) {

        ((const UserInterface*) this)->printInterfaceHeader(subtitle...);
        return *this;

    }

    template <typename... Args> void UserInterface::printArgUsageMessage (
        const std::wstring& title,
        const std::wstring& usageStr,
        const Args&... description
    ) const {

        this->printInterfaceHeader(title)
             .console
            ->println()
             .println(L"Usage:")
             .printsp(L"TerrariaMonitorTool").println(usageStr)
             .println();

        // Executes the lambda function once per line of the `description`.
        ([&] {
            this->console->println().print(description);
        } (), ...);

    };

}