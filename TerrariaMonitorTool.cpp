/*
 * TerrariaMonitorTool.cpp
 *
 * The Primary Driver Source File for the Program.
 *
 * Contains the main (`wmain()`) method for the Program, as well as
 * any miscellanous functions needed to support `wmain()`.
 */


#include "Console.h"
#include "UserInterface.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <unordered_map>


namespace PROGRAM_NAMESPACE {

    /* Type Definitions */

    /**
     * A type representing a Map of Configuration Property Names to an `std::pair`
     * containing the old and new Configuration Property Values.
     * 
     * The Configuration Property Name and Property Values are all represented
     * by Wide-Character Strings.
     */
    typedef std::unordered_map< std::wstring, std::pair<std::wstring, std::wstring> > ChangedValuesMap;


    /* Global Variables */

    static Console::console_ptr_t console = {};         // The `Console` used to interact with the Windows Console.
    static ChangedValuesMap changedValues = {};         // A Map containing the Modified Configuration Properties.
    static std::wstring dryRunOutput = {};              // Used to store the output when performing a Dry Run (`--dry-run`).
    std::optional<std::wstring> configFilePath = {};    // The path to the Terraria Configuration File being used.


    /* Helper Functions */

    /**
     * Get the Connected Display Monitors that can be set as 
     * the Active Display Monitor in the Terraria Configuration File. 
     * 
     * @return  A `DisplayMonitorList` containing the Connected Display Monitors,
     *          wrapped in an `std::optional` object.
     * 
     *          If the Connected Display Monitors could not be retrieved due to
     *          an error with the Windows API, an empty `std::optional` will be returned.
     */
    static std::optional<DisplayMonitorList> getDisplayMonitors () {

        // The `std::optional<DisplayMonitorList>` returned by the method.
        std::optional<DisplayMonitorList> finalMonitorList = std::make_optional<DisplayMonitorList>();
        
        // A map of Display IDs to `DisplayMonitor` objects used
        // to temporarily store the `DisplayMonitor` objects before
        // they are ultimately moved to the `finalMonitorList`.
        std::map<std::wstring, DisplayMonitor> tempMonitorMap = {};

        // The Display Number of the current Display Monitor being processed. 
        DisplayMonitor::display_number_t currentMonitorNum = 1U;

        std::vector<DISPLAYCONFIG_PATH_INFO> configPaths;   // Configuration Paths to be used with the Windows API.
        std::vector<DISPLAYCONFIG_MODE_INFO> configModes;   // Configuration Modes to be used with the Windows API.
        UINT32 configFlags = QDC_ONLY_ACTIVE_PATHS;         // Configuration Flags to be passed to the Windows API.
        LONG result = ERROR_SUCCESS;                        // The result of the most recent Windows API operation.

        /**
         * A lambda function used to print the last Windows API Error
         * to the Console Error Output Buffer.
         * 
         * Depends on the `result` variable.
         * 
         * @param msg   The message to print to the Console prior to the error message
         *              returned by the Windows API, suffixed by ": ".
         */
        auto printWindowsApiError = [&result]( const std::wstring& msg ) {

            // A Null-Terminated Wide-Character String containing the error message returned by the Windows API.
            // Must be freed using `LocalFree()` according to the Windows API.
            LPWSTR errMsgBuf = nullptr;

            FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                result,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPWSTR) &errMsgBuf,
                0,
                NULL
            );

            console->err().print(msg).print(L": ").println(errMsgBuf);
            LocalFree(errMsgBuf);

        };

        // Loop until the `configPaths` and `configModes` have been properly populated.
        do {
            UINT32 pathCount = 0U;  // The number of Display Paths returned by the Windows API.
            UINT32 modeCount = 0U;  // The number of Display Modes returned by the Windows API.

            result = GetDisplayConfigBufferSizes(configFlags, &pathCount, &modeCount);

            if (result == ERROR_SUCCESS) {
                configPaths.resize(pathCount);
                configModes.resize(modeCount);

                result = QueryDisplayConfig(configFlags, &pathCount, configPaths.data(), &modeCount, configModes.data(), nullptr);

                configPaths.resize(pathCount);
                configModes.resize(modeCount);
            }
        }
        while (result == ERROR_INSUFFICIENT_BUFFER);

        // Details about the Connected Display Monitors was successfully retrieved from the Windows API.
        if (result == ERROR_SUCCESS) {
            // A structure containing information about the Current Resolution
            // of the Current Display Monitor being processed.
            DEVMODEW displayMode = { .dmSize = sizeof DEVMODEW, .dmDriverExtra = 0UL };
            // A structure containing information about the Current Display Monitor being processed.
            DISPLAY_DEVICEW displayDevice = { .cb = sizeof DISPLAY_DEVICEW };

            // Retrieve details about each Connected Display Monitor from the Windows API.
            while ( EnumDisplayDevicesW( NULL, (DWORD) (currentMonitorNum - 1), &displayDevice, 0 ) == TRUE ) {
                // Exclude Virtual and Disconnected Display Monitors. 
                if (displayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
                    EnumDisplaySettingsW(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &displayMode);

                    // Store the details about the Current Display Monitor in the `tempMonitorMap`
                    // to be processed at the next stage.
                    tempMonitorMap.emplace(
                        displayDevice.DeviceName,
                        DisplayMonitor(
                            currentMonitorNum,
                            displayDevice.DeviceName,
                            L"",
                            displayMode.dmPelsWidth,
                            displayMode.dmPelsHeight,
                            displayMode.dmDisplayFrequency,
                            (displayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
                        )
                    );
                }

                currentMonitorNum++;
            }

            // Ensure the `currentMonitorNum` is properly reset before being used again.
            currentMonitorNum = 1U;

            // Retrieve additional information about each of the Connected Display Monitors from the Windows API.
            for (auto& path : configPaths) {
                // A structure type containing the Friendly Display Name of the Display Monitor.
                DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {
                    .header = {
                        .type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME,
                        .size = sizeof(DISPLAYCONFIG_TARGET_DEVICE_NAME),
                        .adapterId = path.targetInfo.adapterId,
                        .id = path.targetInfo.id
                    }
                };
                // A structure type containing the Display ID of the Display Monitor.
                DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName = {
                    .header = {
                        .type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME,
                        .size = sizeof(DISPLAYCONFIG_SOURCE_DEVICE_NAME),
                        .adapterId = path.sourceInfo.adapterId,
                        .id = path.sourceInfo.id
                    }
                };
                // DISPLAYCONFIG_ADAPTER_NAME adapterName = {
                //     .header = {
                //         .type = DISPLAYCONFIG_DEVICE_INFO_GET_ADAPTER_NAME,
                //         .size = sizeof(DISPLAYCONFIG_ADAPTER_NAME),
                //         .adapterId = path.sourceInfo.adapterId
                //     }
                // };

                result = DisplayConfigGetDeviceInfo(&targetName.header);
                result &= DisplayConfigGetDeviceInfo(&sourceName.header);
                // result &= DisplayConfigGetDeviceInfo(&adapterName.header);

                // Successfully retrieved information about the Current Display Monitor from the Windows API.
                if (result == ERROR_SUCCESS) {
                    // The node in the `tempMonitorMap` corresponding to the Current Display Monitor.
                    auto node = tempMonitorMap.extract(sourceName.viewGdiDeviceName);
                    // The Current Display Monitor being processed.
                    DisplayMonitor& displayMonitor = node.mapped();
                    // Indicates if the Display Monitor is an Internal Device or not.
                    bool isInternalDevice = (
                           targetName.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL
                        || targetName.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EMBEDDED
                        || targetName.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EMBEDDED
                    );

                    displayMonitor.monitorName = (
                        targetName.monitorFriendlyDeviceName[0] != L'\0'
                            ? targetName.monitorFriendlyDeviceName
                            : ( isInternalDevice ? L"Internal Display" : L"Unnamed Display" )
                    );
                    finalMonitorList->push_back( std::move(displayMonitor) );
                }
                // Failed to retrieve information about the Current Display Monitor from the Windows API.
                else {
                    printWindowsApiError(L"Failed to Query Display Information from the Windows API");
                    return std::optional<DisplayMonitorList>();
                }

                currentMonitorNum++;
            }
        }
        // Failed to retrieve details about the Connected Display Monitors from the Windows API.
        else {
            printWindowsApiError(L"Failed to Query Display Configuration from the Windows API");
            return std::optional<DisplayMonitorList>();
        }

        return finalMonitorList;

    }

    /**
     * Get the Active Display Monitor from the Specified Terraria Configuration File.
     * 
     * @param configFilePath    A Wide-Character String containing the path to the Terraria Configuration File.
     * 
     * @param displayMonitors   The `DisplayMonitorList` containing the Connected Display Monitors to 
     *                          compare to the Active Display Monitor in the Terraria Configuration File.
     * 
     * @returns                 On success, returns the zero-based index of the Connected Display Monitor
     *                          in the specified list of `displayMonitors` that is set as the Active Display Monitor
     *                          in the Terraria Configuration File, wrapped in an `std::optional` object.
     * 
     *                          If the specified Terraria Configuration File could not be found or opened,
     *                          or if the Active Display Monitor in the Specified Terraria Configuration File
     *                          was not found in the specified list of `displayMonitors`, 
     *                          an empty `std::optional` will be returned.
     */
    static std::optional<DisplayMonitor::display_number_t> getActiveMonitorFromConfigFile (
        const std::wstring& configFilePath,
        const DisplayMonitorList& displayMonitors
    ) {

        // The Regular Expression used to match the "Display" Configuration Property and Value.
        const std::wregex DISPLAY_REGEX = std::wregex(L"\"Display\": \"([^\"]+)\",");
        // The Regular Expression used to match Double-Escaped Backslashes in the Property Value.
        const std::wregex BACKSLASH_REGEX = std::wregex(L"\\\\\\\\");

        // The File Stream used to read from the Terraria Configuration File.
        std::wifstream configFileStream(configFilePath);
        // The index of the Active Display Monitor, if applicable.
        std::optional<DisplayMonitor::display_number_t> selectedMonitorNum = {};
        // Contains the current line of the Terraria Configuration File.
        std::wstring currentConfigFileLine = L"";
        // The Display ID of the Active Display Monitor.
        std::wstring selectedDisplayId = L"";
        // Contains the results of matching the `displayRegex` to the `currentConfigFileLine`.
        std::wsmatch matches;

        // Process each line of the Terraria Configuration File until
        // the Active Display Monitor has been determined.
        while ( configFileStream.good() && selectedDisplayId.empty() ) {
            std::getline(configFileStream, currentConfigFileLine);

            if ( std::regex_search(currentConfigFileLine, matches, DISPLAY_REGEX) ) {
                // Remove double-escaped backslashes.
                selectedDisplayId = std::regex_replace(matches[1].str(), BACKSLASH_REGEX, L"\\");
            }
        }

        // Attempt to match the Active Display Monitor to one of the provided `displayMonitors`.
        for (const auto& monitor : displayMonitors) {
            if (monitor.displayId == selectedDisplayId) {
                selectedMonitorNum = monitor.displayNum;
                break;
            }
        }

        return selectedMonitorNum;

    }

    /**
     * Set the Active Display Monitor in the Specified Terraria Configuration File.
     * 
     * @param configFilePath        A Wide-Character String containing the path to the Terraria Configuration File.
     * 
     * @param newSelectedMonitor    The `DisplayMonitor` corresponding to the Connected Display Monitor to
     *                              be set as the new Active Display Monitor.
     * 
     * @returns                     `true` on success and `false` on failure.
     */
    static bool setActiveMonitorInConfigFile (
        const std::wstring& configFilePath,
        const DisplayMonitor& newSelectedMonitor
    ) {

        // Catch any exceptions that are raised and return `false` on error.
        try {
            std::optional<std::wstring> tempFilePath = UTILS_NAMESPACE::createTempFile();

            if (tempFilePath) {
                // The File Stream used to read from the Current Terraria Configuration File.
                std::wifstream configFileStream(configFilePath);
                // The File Stream used to write to the Temporary File.
                std::wofstream tempConfigFileStream(*tempFilePath);
                // Contains the current line read in from the Current Terraria Configuration File.
                std::wstring currentLine = {};
                // A Regular Expression Pattern used to match specified Configuration Properties starting with `Display`. 
                std::wregex displayRegex = std::wregex(L"(^[^\"]*\"(Display(?:Width|Height|Screen)?)\"): \"?([^\"]+)\"?,");
                // Contains the results of matching the `displayRegex` against the `currentLine`.
                std::wsmatch matches;
                // Contains the Display ID of the Active Display Monitor, in which all
                // of the backslashes have already been double-escaped for writing.
                std::wstring selectedDisplayId = std::regex_replace( newSelectedMonitor.displayId, std::wregex(L"\\\\"), L"\\\\" );
                std::wstring outputData = {};

                if ( configFileStream.good() && tempConfigFileStream.good() ) {
                    if ( programSettings.dryRun && !dryRunOutput.empty() )
                        dryRunOutput.clear();

                    // Write each line of the Current Terraria Configuration File to the
                    // Temporary File, modifying the line as necessary.
                    while ( configFileStream.good() ) {
                        outputData.clear();
                        std::getline(configFileStream, currentLine);

                        // Copy unmatched lines from the Current Config File
                        // directly to the Temporary File without modification.
                        if ( !std::regex_search(currentLine, matches, displayRegex) ) {
                            outputData = currentLine;

                            if ( configFileStream.good() )
                                outputData.push_back(L'\n');
                        }
                        else {
                            std::wstring oldValueStr = matches[3];  // The old value to be added to the `changedValues` map.
                            std::wstring newValueStr = {};          // The new value to be added to the `changedValues` map.

                            if ( matches[2] == L"DisplayWidth" || matches[2] == L"DisplayHeight" ) {
                                // The previous width or height, converted to an integer type.
                                DWORD matchValue = std::stoul(matches[3]);

                                if ( matches[2] == L"DisplayWidth" && newSelectedMonitor.currentResolution.displayWidth != matchValue ) {
                                    outputData = std::format(
                                        L"{:s}: {:d},\n",
                                        matches[1].str(),
                                        newSelectedMonitor.currentResolution.displayWidth
                                    );
                                    newValueStr = std::to_wstring(newSelectedMonitor.currentResolution.displayWidth);
                                }
                                else if ( matches[2] == L"DisplayHeight" && newSelectedMonitor.currentResolution.displayHeight != matchValue ) {
                                    outputData = std::format(
                                        L"{:s}: {:d},\n",
                                        matches[1].str(),
                                        newSelectedMonitor.currentResolution.displayHeight
                                    );
                                    newValueStr = std::to_wstring(newSelectedMonitor.currentResolution.displayHeight);
                                }
                                else {
                                    outputData = currentLine + L'\n';
                                }
                            }
                            else if ( matches[2] == L"Display" || matches[2] == L"DisplayScreen" ) {
                                outputData = std::format( L"{:s}: \"{:s}\",\n", matches[1].str(), selectedDisplayId );
                                oldValueStr = (L'"' + oldValueStr + L'"');
                                newValueStr = (L'"' + selectedDisplayId + L'"');
                            }

                            // We assume that changes have been made anytime `newValueStr` is populated.
                            if ( !newValueStr.empty() ) {
                                if ( !changedValues.contains(matches[2]) ) {
                                    changedValues.emplace( matches[2], std::make_pair(oldValueStr, newValueStr) );
                                }
                                else if ( changedValues[matches[2]].first != newValueStr ) {
                                    changedValues[matches[2]].second = newValueStr;
                                }
                                else {
                                    changedValues.erase(matches[2]);
                                }
                            }
                        }

                        if ( !programSettings.dryRun )
                            tempConfigFileStream << outputData;
                        else
                            dryRunOutput += outputData;
                    }

                    if ( !programSettings.dryRun ) {
                        // Once we have finished writing the updated contents of the
                        // Terraria Configuration File to the Temporary File, we can
                        // replace the contents of the Config File with the Temporary File.
                        configFileStream.close();
                        tempConfigFileStream.close();
                        std::filesystem::rename(*tempFilePath, configFilePath);
                    }

                    return true;
                }
            }
        }
        catch (...) {}

        return false;

    }

    /**
     * Clear all of the files and folders associated with the program.
     * 
     * This function will clear *all* data created by the program,
     * including files used internally by the program, as well as
     * any existing Backup Configuration Files. This function does *not*
     * apply to any Terraria Configuration Files that are not being used as backups.
     * 
     * This action requires confirmation from the user via `UserInterface::promptForConfirmation()`
     * before any program data will be deleted. Note that if there is no existing
     * Program Data to be deleted, the method will simply return `true` without
     * prompting the user for any confirmation as there is nothing to be deleted.
     * 
     * @param ui    The `UserInterface` used to prompt the user for confirmation.
     * 
     * @returns     `true` if the program data was successfully deleted.
     * 
     *              Returns `false` if the user rejected the Confirmation Prompt
     *              or if the program data could not be successfully deleted.
     */
    static bool clearProgramData ( const UserInterface& ui ) {

        if ( !std::filesystem::exists(PROGRAM_DATA_PATH) )
            return true;

        std::optional<bool> confirmation = ui.promptForConfirmation(
            L"Clear Program Data?",
            L"All existing Program Data will be irrecoverably lost."
        );

        if (confirmation && *confirmation)
            return (std::filesystem::remove_all(PROGRAM_DATA_PATH) > 0);

        return false;

    }

    /**
     * The Program Exit Handler run at the end of `wmain()` and when calling `std::exit()`.
     *  
     * When invoked, all Alternate Output Buffers of the `Console` are cleared
     * followed by one of three messages being printed to the Primary Output Buffer
     * based on meeting one of the following criteria:
     * 
     *      - No Terraria Configuration File is specified.
     *      - A Terraria Configuration File was selected but no changes were made to it.
     *      - A Terraria Configuration File was selected and modified.
     * 
     * Note that for the purposes of deciding which criteria has been met,
     * a Terraria Configuration File is considered to have been "modified"
     * if one or more of the Configuration Properties was changed to 
     * a *different* value from when the program started. In other words,
     * changing one or more Configuration Properties and then changing them
     * back to their original values before exiting the program will
     * consider the Terraria Configuration File to be "unmodified", despite
     * the fact that it was overwritten two or more times.
     * 
     * If the `--dry-mode` Flag was used and the Terraria Configuration File
     * was "modified" one or more times, the output of the most recent operation
     * will be printed to the console prior to the final output message.
     */
    static void programExitHandler () {

        // Clear all Alternate Output Buffers
        while ( console->getCurrentBufferNum() > 0ULL )
            console->restorePreviousBuffer();

        if ( programSettings.dryRun && !dryRunOutput.empty() )
            console->println(dryRunOutput);

        if (configFilePath) {
            if ( !changedValues.empty() ) {
                console->print(L"Changes were made to ").print(*configFilePath).print(L':');

                for ( const auto& [key, values] : changedValues )
                    console->printf(L"\n   + {:14s} {:s} --> {:s}", key + L":", values.first, values.second);
            }
            else {
                console->print(L"No changes were made to ").print(*configFilePath).print(L'.');
            }
        }
        else {
            console->print(L"No changes were made to any Terraria Configuration Files.");
        }
    
    }

}

/**
 * The Primary Entry Point for the Program.
 * 
 * @param argc	The total number of Command-Line Arguments available in `argv`.
 * 
 * @param argv 	An array of Null-Terminated Wide-Character Strings corresponding
 * 				to the Command-Line Arguments passed to the Program.
 * 
 * @returns     `0` on success.
 * 
 *              If the user terminated the program using the `ESC` key
 *              or the `CTRL + C` signal, returns `-1`.
 * 
 *              If an error occurred and the program did not complete successfully,
 *              returns a positive, nonzero integer.
 */
int wmain ( int argc, const wchar_t* argv[] ) {

    using namespace PROGRAM_NAMESPACE;              // Import all custom code.
    using namespace std::chrono_literals;           // Import Chrono Literals for specifying thread timeouts.

    struct ProgramFlagsStruct {
        // Indicates if the `--help` flag was used.
        bool helpMode = false;
        // Indicates if the `--version` flag was used.
        bool versionMode = false;
        // Indicates if the `--clear-program-data` flag was used.
        bool clearProgramDataAtStart = false;
    } programFlags;                                 // A structure containing the status of each of the Program Flags.
    int statusCode = ProgramStatusCode::SUCCESS;    // The Result Status Code returned by the Program.


    // Process Command-Line Arguments
    for ( int i = 1; i < argc; i++ ) {
        std::wstring arg(argv[i]);
        std::wstring lcArg = UTILS_NAMESPACE::stringToLowercase(arg);

        if (i == 1) {
            // Display Help/Usage Information
            if ( arg == L"/?" || lcArg == L"--help" || lcArg == L"--usage" ) {
                programFlags.helpMode = true;
                break;
            }
            // Display Version Information
            else if ( arg == L"-v" || lcArg == L"--version" ) {
                programFlags.versionMode = true;
                break;
            }
        }

        // Enable Dry Run Mode
        if ( arg == L"-d" || lcArg == L"--dry-run" ) {
            programSettings.dryRun = true;
        }
        // Enable Stateless Mode
        else if ( arg == L"-s" || lcArg == L"--stateless" ) {
            programSettings.statelessMode = true;
        }
        // Enable Auto-Confirm Prompts Mode
        else if ( arg == L"-y" || lcArg == L"--yes" ) {
            programSettings.autoConfirmPrompts = true;
        }
        // Disable Custom Buffer Behavior
        else if ( arg == L"-b" || lcArg == L"--disable-custom-buffer-behavior" ) {
            programSettings.useCustomBufferBehavior = false;
        }
        // Clear Program Data at Start
        else if ( lcArg == L"--clear-program-data" ) {
            programFlags.clearProgramDataAtStart = true;
        }
        // Enable Debug-Friendly Mode
        else if ( lcArg == L"--debug" ) {
            programSettings.debugMode = true;
        }
    }


    if ( !(console = Console::getConsole()) ) {
        std::wcerr << L"Failed to initialize the Console via the Windows API.";
        return ProgramStatusCode::CONSOLE_CREATION_FAILURE;
    }

    // The `UserInterface` responsible for providing the Program's User Interface.
    UserInterface ui = { console };


    // Display message if Debug-Friendly Mode is enabled and pause
    // to give time to attach a debugger to an existing process.
    if (programSettings.debugMode) {
        // The list of debug properties displayed during the Debug Loading Screen.
        std::vector< std::pair<std::wstring, std::wstring> > debugPropMap = {
            { L"Version", PROGRAM_VERSION },
            { L"Dry Run", programSettings.dryRun ? L"Yes" : L"No" },
            { L"Stateless Mode", programSettings.statelessMode ? L"Yes" : L"No" },
            { L"Custom Buffer Behavior", programSettings.useCustomBufferBehavior ? L"Enabled" : L"Disabled" },
            { L"Auto-Confirm Prompts", programSettings.autoConfirmPrompts ? L"Yes" : L"No" },
            { L"Clear Program Data at Start", programFlags.clearProgramDataAtStart ? L"Yes" : L"No" }  
        };

        console->createAltBuffer();
        console->toggleCursorVisibility(false)
                .println(L"Debug Mode Enabled!");

        for ( const auto& [property, value] : debugPropMap )
            console->printfln(L"   {:29s} {:s}", (property + L':'), value);

        console->print(L"Waiting for debuggers to attach...");

        // Wait for 5 seconds, updating the countdown timer in the console once per second.
        for (short i = 5; i > 0; i--) {
            console->print(i, false);
            std::this_thread::sleep_for(1s);
            console->print(Console::getVirtualTerminalSequence(L"[1D"), false);
        }

        console->restorePreviousBuffer();
    }

    if (programFlags.helpMode) {
        ui.printUsageMessage(argc, argv);
        return ProgramStatusCode::SUCCESS;
    }
    else if (programFlags.versionMode) {
        console->print(PROGRAM_VERSION);
        return ProgramStatusCode::SUCCESS;
    }
    else if (programFlags.clearProgramDataAtStart) {
        clearProgramData(ui);
    }


    // Once the `Console` has been initialized and we're sure that the program isn't
    // in `helpMode` or `versionMode`, we can register the Program Exit Handler.
    std::atexit(programExitHandler);


    // The Connected Display Monitors.
    std::optional<DisplayMonitorList> displayMonitors = getDisplayMonitors();
    // The current width of the "Monitor Name" column of a Connected Display Monitor Menu Option.
    auto monitorNameColSize = ui.getTextSizing().monitorNameColSize;

    if (!displayMonitors) {
        console->err().print(L"Failed to retrieve the Connected Display Monitors from the Windows API.");
        return ProgramStatusCode::DISPLAY_MONITOR_QUERY_FAILURE;
    }

    // Determine the width of the "Monitor Name" column based on the length
    // of the longest monitor in the list of `displayMonitors`.
    for (const auto& monitor : *displayMonitors) {
        unsigned short monitorNameLength = (unsigned short) monitor.monitorName.length();

        if (monitorNameLength > monitorNameColSize)
            monitorNameColSize = monitorNameLength;
    }

    if ( monitorNameColSize != ui.getTextSizing().monitorNameColSize )
        ui.changeTextSizing( ui.getTextSizing().withNewMonitorNameColSize(monitorNameColSize) );


    // Get the path to the Terraria Configuration File to use from the user.
    console->createAltBuffer();
    configFilePath = ui.promptForConfigFilePath();

    if (configFilePath) {
        // The Active Display Monitor, if a valid one is currently set.
        std::optional<DisplayMonitor::display_number_t> selectedMonitorNum = getActiveMonitorFromConfigFile(
            *configFilePath, *displayMonitors
        );
        // The last selection from the Main Menu of the Program.
        std::optional<UserInterface::MainMenuSelection> selection = {};
        // Indicates whether the `selection` contains a `DisplayMonitor` or not.
        bool isMonitorSelection = false;

        // Repeatedly draw the Main Menu until an Alternative Menu Option is selected
        // (i.e., `selection` does not contain a `DisplayMonitor`).
        do {
            selection = ui.mainMenu(
                *configFilePath,
                *displayMonitors,
                !selection.has_value(),
                selectedMonitorNum ? *selectedMonitorNum : 1U
            );

            isMonitorSelection = selection && std::holds_alternative<DisplayMonitor>(*selection);

            if ( !selection )
                statusCode = ProgramStatusCode::TERMINATED;

            if (isMonitorSelection) {
                // The `DisplayMonitor` selected by the user.
                const DisplayMonitor& selectedMonitor = std::get<DisplayMonitor>(*selection);

                // Only update the Terraria Configuration File when the selected
                // Display Monitor is not already the Active Display Monitor.
                if ( !selectedMonitorNum || selectedMonitor.displayNum != *selectedMonitorNum ) {
                    if ( setActiveMonitorInConfigFile(*configFilePath, selectedMonitor) ) {
                        selectedMonitorNum = selectedMonitor.displayNum;
                    }
                    else {
                        console->err().println(L"Failed to Set the Display Monitor in the Terraria Configuration File.");
                    }
                }
            }
        }
        while (isMonitorSelection);

        // Remove the Main Menu at the end of the program.
        console->restorePreviousBuffer();
        console->toggleCursorVisibility(true);
    }
    else {
        statusCode = ProgramStatusCode::TERMINATED;
    }

    // Print the results of the program via
    // the `programExitHandler()` and exit.
    return statusCode;

}