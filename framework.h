#pragma once


/*
 * framework.h
 *
 * Header File containing common Headers and declaring Global Macros, Types, and Constants.
 */


#include <cwctype>
#include <format>
#include <filesystem>
#include <map>
#include <regex>
#include <stdexcept>    // std::runtime_error
#include <optional>
#include <string>
#include <type_traits>
#include <vector>
#include <Windows.h>


/* Global Macros */

// Defines the Global Program Namespace.
#define PROGRAM_NAMESPACE FusedKush::TerrariaMonitorTool

// Defines the name of the Global Program Utilities Namespace.
#define UTILS_NAMESPACE Utils
// Defines the full Global Program Utilities Namespace.
#define FULL_UTILS_NAMESPACE PROGRAM_NAMESPACE::UTILS_NAMESPACE


namespace PROGRAM_NAMESPACE {

    
    /* Global Utility Types & Functions */
    namespace UTILS_NAMESPACE {

        /* Structure Types */

        /**
         * A structure type containing all of the customizable settings that control
         * the behavior of the program.
         * 
         * Each setting is a boolean value indicating whether the setting is
         * enabled (`true`) or disabled (`false`).
         * 
         * Each setting corresponds to one or more Command-Line Arguments that can be used
         * to change the value of the associated Program Setting.
         */
        typedef struct ProgramSettingsStruct {

            /**
             * Indicates if changes should actually be written to the
             * Terriaria Configuration File or output to the console instead.
             * 
             * Can be enabled using the `-d` or `--dry-run` flag when launching the program.
             */
            bool dryRun = false;
            /**
             * Indicates if the program should skip reading from or writing to any program files.
             * 
             * Any changes made when `statelessMode` is `true` will have no effect on
             * any existing program data or result in the creation or any new program data.
             * 
             * This option does *not* effect whether or not changes are made to the
             * Terraria Configuration FIle or any Backup Files (see `dryRun` instead).
             * 
             * Can be enabled using the `-s` or `--stateless` flag when launching the program.
             */
            bool statelessMode = false;
            /**
             * Indicates if Confirmation Prompts should be automatically confirmed
             * and dismissed instead of requiring the user to respond to them.
             * 
             * This option should be used with caution, as it permits for potentially dangerous
             * actions to proceed without any further confirmation from the user.
             * 
             * This option applies to Confirmation Prompts triggered in response to both
             * Command-Line Arguments, as well as Interactive Menu Selections.
             * 
             * Can be enabled using the `-y` or `--yes` flag when launching the program.
             */
            bool autoConfirmPrompts = false;

            /**
             * Indicates if Custom Buffer Behavior should be used.
             * 
             * When `true`, applies to the following aspects of the `Console`'s Output and Error Output Buffers:
             * 
             *      - Alternate Output Buffers are managed by `clear()`ing the `Console` and maintaining
             *        an inner buffer of all of the contents of each Output Buffer, all within
             *        the original underlying Console Output Buffer.
             * 
             *      - Calling `clear()` uses a Virtual Terminal Sequence to clear the characters between
             *        the Current Cursor Position and the Recorded Cursor Position marking the beginning
             *        of the Output Buffer. 
             * 
             * While the Custom Buffer Behavior can make for a smoother and more-intuitive experience,
             * it is suspectible to changes in the Visible Console Viewport or an incompatible Console Window Size,
             * which can prevent the Custom Buffer Behavior from working properly.
             * 
             * When `false`, the standard behavior for Alternate Output Buffers and Clearing the Console
             * will be used instead. This behavior is recommended if the Custom Buffer Behavior does not appear
             * to be working properly due to changes in the Visible Console Viewport or an incompatible Console Window Size.
             * 
             * Can be disabled using the `-b` or `--disable-custom-buffer-behavior` flag when launching the program.
             */
            bool useCustomBufferBehavior = true;

            /**
             * Indicates if Debug-Friendly Mode is enabled or not.
             * 
             * When `true`, applies to the following aspects of the Program:
             * 
             *      - A brief delay is introduced prior to the program launching to allow for
             *        a debugger to be connected to an existing process.
             * 
             *      - Enables additional output and error logging.
             * 
             * Can be enabled using the `--debug` flag when launching the program.
             */
            bool debugMode = false;

        } ProgramSettings;

    
        /* Global Helper Functions */
        // String Functions

        /**
         * Convert all of the applicable characters of a Wide-Character String to Lowercase.
         * 
         * Each character of the specified Wide-Character String is effectively passed to `std::towlower()`,
         * concatenated together, and returned as a new Wide-Character String.
         * 
         * @param str   The Wide-Character String being converted to lowercase.
         * 
         * @return      A new Wide-Character String containing the lowercase equivalent
         *              of the specified Wide-Character `str`.
         */
        std::wstring stringToLowercase ( const std::wstring& str );
        /**
         * Convert all of the applicable characters of a Wide-Character String to Uppercase.
         * 
         * Each character of the specified Wide-Character String is effectively passed to `std::towlower()`,
         * concatenated together, and returned as a new Wide-Character String.
         * 
         * @param str   The Wide-Character String being converted to uppercase.
         * 
         * @return      A new Wide-Character String containing the uppercase equivalent
         *              of the specified Wide-Character `str`.
         */
        std::wstring stringToUppercase ( const std::wstring& str );

        /**
         * Trim extraneous whitespace characters from the beginning 
         * and end of the specified Wide-Character String.
         * 
         * Any characters considered to be whitespace by the `std::iswspace()` function
         * at the beginning or end of the specified Wide-Character `str` will be
         * stripped from the returned string.
         * 
         * @param str   The Wide-Character String being trimmed.
         * 
         * @returns     A new Wide-Character String containing the trimmed contents
         *              of the specified Wide-Character `str`.
         */
        std::wstring trimString ( const std::wstring& str );

        /**
         * Truncate a Wide-Character String to the specified number of characters.
         * 
         * If the `length()` of the specified `str` is greater than the
         * specified `maxLength`, the returned string will be truncated to a
         * size of `maxLength - 3` characters (or zero, whichever is greater)
         * and suffixed by three period characters ("...").
         * 
         * For example, `truncateString(L"FooBarBaz", 8);` returns `L"FooBa..."`
         * and `truncateString(L"FooBarBaz", 6);` returns `L"Foo..."`.
         * 
         * @param str           The Wide-Character String being truncated.
         *  
         * @param maxLength     The maximum length of the specified `str` before truncating it.
         *  
         * @returns             A new Wide-Character String containing the truncated string.
         *  
         *                      If the `length()` of the specified `str` is less than or
         *                      equal to the specified `maxLength`, a copy of the
         *                      specified `str` will be returned without truncation.
         */
        std::wstring truncateString ( const std::wstring& str, size_t maxLength );
        /**
         * Truncate a Wide-Character Path String to the specified number of characters.
         * 
         * If the `length()` of the specified `pathStr` is greater than the
         * specified `maxLength`, the returned string will be truncated to a
         * maximum size of `maxLength - 3` characters. The truncation will take place
         * beginning from the first character following the first path separator character
         * (e.g., `\` or `/`) in the specified `pathStr` up until the next path separator
         * character encountered in which the `length()` of the resulting string
         * is less than or equal to `maxLength -3`. The contents that are truncated
         * from the specified `str` are replaced with three period characters ("...").
         * 
         * For example, `truncatePathString(L"foo/bar/baz/test", 12);` returns `L"foo/.../test"`
         * and `truncatePathString(L"foo/bar/baz/test", 9);` returns `L".../test"`.
         * 
         * @param str           The Wide-Character String being truncated.
         *  
         * @param maxLength     The maximum length of the specified `pathStr` before truncating it.
         *  
         * @returns             A new Wide-Character String containing the truncated string.
         *  
         *                      If the `length()` of the specified `pathStr` is less than or
         *                      equal to the specified `maxLength`, a copy of the
         *                      specified `pathStr` will be returned without truncation.
         */
        std::wstring truncatePathString ( const std::wstring& pathStr, size_t maxLength );


        // File Functions

        /**
         * Create a new Temporary File for writing.
         * 
         * The Temporary File is created by the OS and may be located
         * in the Temporary File Directory as returned by `GetTempPath()`,
         * as well as in the Current Working Directory.
         * 
         * @returns     A Wide-Character String containing the path to the
         *              newly-created Temporary File, wrapped in an `std::optional` object.
         * 
         *                  - If the returned path does not contain any path information and
         *                    is prefixed by a backslash, the Temporary File was created
         *                    in the Current Working Directory, and the returned path will
         *                    be relative to the CWD.
         * 
         *              If a Temporary File could not be successfully created,
         *              an empty `std::optional` object is returned.
         */
        std::optional<std::wstring> createTempFile ();
        
    }


    /* Global Concept, Type, & Structure Definitions */

    /**
     * A Concept specifying a Numeric Type that arithmetic operations can be performed on.
     * 
     * In order for a type, `T`, to be considered a valid `Numeric`, the value
     * of `std::is_arithmetic<T>::value` must be `true`.
     * 
     * @tparam T    The type being checked.
     */
    template <typename T> concept Numeric = std::is_arithmetic_v<T>;
    /**
     * A Concept specifying a String Type that can be converted
     * to a Wide-Character String (`std::wstring`).
     * 
     * In order for a type, `T`, to be considered a valid `StringConvertible`,
     * the Constraint `std::convertible_to<T, std::wstring>` must be satisfied.
     * 
     * @tparam T    The type being checked.
     */
	template <typename T> concept StringConvertible = std::convertible_to<T, std::wstring>;

    // A structure type containing all of the relevant properties of a Connected Display Monitor.
    typedef struct DisplayMonitorStruct {

        /* Inner Type Definitions & Structure Types */
        public:
            // An integer type representing the Display Number of a Connected Monitor.
            typedef unsigned short display_number_t;

            /**
             * A structure type containing all of the properties associated
             * with the Current Display Resolution of a Connected Monitor.
             */
            typedef struct DisplayResolutionStruct {

                /* Structure Fields */
                public:
                    DWORD displayWidth;             // The Display Width of the Connected Monitor in Pixels.
                    DWORD displayHeight;            // The Display Height of the Connected Monitor in Pixels.
                    DWORD refreshRate;              // The Refresh Rate of the Connected Monitor.
                    std::wstring resolutionString;  // The Full Resolution String in the format "Width x Height @ Refresh Rate".


                /* Structure Constructors */
                public:
                    /**
                     * Construct a new DisplayResolutionStruct.
                     * 
                     * @param iDisplayWidth     The Display Width of the Connected Monitor in Pixels.
                     * @param iDisplayHeight    The Display Height of the Connected Monitor in Pixels.
                     * @param iRefreshRate      The Refresh Rate of the Connected Monitor.
                     */
                    DisplayResolutionStruct ( DWORD iDisplayWidth, DWORD iDisplayHeight, DWORD iRefreshRate );

            } DisplayResolution;    


        /* Structure Fields */
        public:
            display_number_t displayNum;            // The Display Number of the Connected Monitor, which corresponds to the `displayId`.
            std::wstring displayId;                 // The "Unique" Display Identifier of the Connected Monitor.
            std::wstring monitorName;               // The Human-Readable Name of the Connected Monitor.
            DisplayResolution currentResolution;    // The Current Display Resolution of the Connected Monitor.

            /**
             * Additional Comments about the Connected Monitor.
             * 
             * In general, this field will either contain the string "Main Monitor" 
             * or it will contain an empty string.
             */
            std::wstring comments;


        /* Structure Constructors */
        public:
            /**
             * Construct a new DisplayMonitorStruct.
             * 
             * @param iDisplayNum       The Display Number of the Connected Monitor.
             * @param iDisplayId        The "Unique" Display Identifier of the Connected Monitor.
             * @param iMonitorName      The Human-Readable Name of the Connected Monitor.
             * @param displayWidth      The Display Width of the Connected Monitor in Pixels.
             * @param displayHeight     The Display Height of the Connected Monitor in Pixels.
             * @param refreshRate       The Refresh Rate of the Connected Monitor.
             * @param isMainDisplay     Indicates whether the Connected Monitor is the Main/Primary Device.
             */
            DisplayMonitorStruct (
                display_number_t iDisplayNum,
                std::wstring iDisplayId,
                std::wstring iMonitorName,
                DWORD displayWidth,
                DWORD displayHeight,
                DWORD refreshRate,
                bool isMainDisplay = false
            );

    } DisplayMonitor;

    // A collection of `DisplayMonitor` objects representing the Connected Display Monitors.
    typedef std::vector<DisplayMonitor> DisplayMonitorList;


    /* Global Constants */

    /**
     * A Wide-Character String containing the Current Version Number for the Program.
     */
    const std::wstring PROGRAM_VERSION = L"1.0.0";

    /**
     * A Wide-Character String containing the Primary Title of the Program.
     * 
     * @see PROGRAM_TITLE for the Full Title of the Program.
     */
    const std::wstring PRIMARY_PROGRAM_TITLE = L"Terraria Monitor Tool";
    /**
     * A Wide-Character String containing the Full Title of the Program.
     * 
     * @see PRIMARY_PROGRAM_TITLE for the Primary Title of the Program.
     * @see PROGRAM_VERSION for the Version Number String for the Program.
     */
    const std::wstring PROGRAM_TITLE = std::format(L"{:s} by FusedKush (v{:s})", PRIMARY_PROGRAM_TITLE, PROGRAM_VERSION);

    // The filename of the Terraria Configuration File.
    const std::wstring CONFIG_FILE_NAME = L"config.json";

    /**
     * The path to the directory where most required Program Data is to be stored.
     * 
     * The location of the Program Data Directory is dependent on the name
     * of the Current Working Directory, which defaults to the location of the
     * Program Executable.
     * 
     *      - If the CWD contains the name `Terraria Monitor Tool`,
     *        separated my an arbitrary number of spaces, dashes, or underscores,
     *        the Program Data Directory is located at the relative path `./data`.
     * 
     *      - Otherwise, the Program Data Directory will be located at
     *        the relative path `./TerrariaMonitorTool/data`.
     * 
     * Note that `PROGRAM_DATA_PATH` is *not* automatically created when the program
     * is launched if it does not already exist. To ensure that `PROGRAM_DATA_PATH` exists before 
     * attempting to write to it, consider using the `ensureProgramDataDirectoryExists()` function.
     */
    const std::filesystem::path PROGRAM_DATA_PATH = (
        std::regex_search(
            std::filesystem::current_path().parent_path().filename().c_str(),
            std::wregex(L"Terraria[ _-]*Monitor[ _-]*Tool", std::regex_constants::icase)
        )
            ? ( std::filesystem::current_path() / L"data" )
            : ( std::filesystem::current_path() / L"Terraria Monitor Tool" / L"data" )
    );

    // An enumeration defining some of the potential status codes 
    // that can be returned by the program via the `wmain()` and `std::exit()` functions.
    enum ProgramStatusCode : int {
    
        /**
         * Indicates that the program was explicitly terminated by the
         * user, either by using the `ESC` key in an applicable Interactive Menu,
         * or by using `CTRL + C`.
         */
        TERMINATED = -1,
        /**
         * Indicates that the program was successfully run.
         * 
         * Note that this does *not* necessarily mean that any Terraria Configuration Files
         * were successfully modified, and a `SUCCESS` Status Code can be returned in situations
         * where no Terraria Configuration Files were modified by the program.
         */
        SUCCESS = 0,
        /**
         * Indicates that the `Console` instance responsible for the Console
         * could not be created due to an error with the Windows API.
         */
        CONSOLE_CREATION_FAILURE = 0x10,
        /**
         * Indicates that the Connected Display Monitors could not
         * be retrieved from the Windows API due to an error.
         */
        DISPLAY_MONITOR_QUERY_FAILURE = 0x20
    
    };


    /* Global Variables */

    /**
     * The customizable settings that control the behavior of the program.
     * 
     * These Program Settings are initialized along with the program and
     * are then modified according to the specified Command-Line Arguments.
     * 
     * @warning     Following initialization and any updates due to Command-Line Arguments
     *              being applied, this variable should generally *not* be modified any further,
     *              and dependent code is *not* required to immediately or automatically
     *              reflect further changes to any Program Settings. 
     */
    extern UTILS_NAMESPACE::ProgramSettings programSettings;


    /* Global Helper Functions */

    /**
     * Ensures the main Program Data Directory exists, creating it if necessary.
     * 
     * This is the Throwing Overload of `ensureProgramDataDirectoryExists()`.
     * 
     * If `PROGRAM_DATA_DIRECTORY` does not exist when this function is invoked,
     * the directory and any missing parent directories will automatically be created.
     * 
     * If `PROGRAM_DATA_DIRECTORY` already exists, this function has no effect.
     * 
     * @returns     `true` if `PROGRAM_DATA_DIRECTORY` already exists or was successfully created.
     * 
     *              Otherwise, returns `false`.
     * 
     * @throws      `std::bad_alloc`, `std::filesystem::filesystem_error`
     *              May be thrown by `std::filesystem::create_directories()` on OS error.
     */
    bool ensureProgramDataDirectoryExists ();
    /**
     * Ensures the main Program Data Directory exists, creating it if necessary.
     * 
     * This is the Non-Throwing Overload of `ensureProgramDataDirectoryExists()`.
     * 
     * If `PROGRAM_DATA_DIRECTORY` does not exist when this function is invoked,
     * the directory and any missing parent directories will automatically be created.
     * 
     * If `PROGRAM_DATA_DIRECTORY` already exists, this function has no effect.
     * 
     * @param oExceptionPtr A Raw Pointer to an `std::exception` object wrapped in an `std::optional`.
     *                      
     *                      If the underlying call to `std::filesystem::create_directories()` throws
     *                      an `std::bad_alloc` or `std::filesystem::filesystem_error` due to an OS error, 
     *                      the `std::optional` object pointed to by the `exceptionPtr` argument will be populated
     *                      with the exception thrown by `std::filesystem::create_directories()`.
     * 
     *                      If the underlying function call does not raise any exceptions, the `std::optional` object will
     *                      be `reset()` to indicate that no exception was thrown, even if the method returns `false`,
     *                      making it possible to distinguish between generic failure and OS exceptions.
     * 
     *                      A `nullptr` can be explicitly passed to this argument, making it possible to use the
     *                      Non-Throwing Overload of `ensureProgramDataDirectoryExists()` without necessarily
     *                      having to capture any emitted exceptions.
     * 
     * @returns             `true` if `PROGRAM_DATA_DIRECTORY` already exists or was successfully created.
     * 
     *                      Otherwise, returns `false`.
     */
    bool ensureProgramDataDirectoryExists ( _Out_ std::optional<std::exception>* oExceptionPtr ) noexcept;

}
