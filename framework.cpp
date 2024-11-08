/*
 * framework.cpp
 *
 * Source File defining Global Types, and Constants.
 */


#include "framework.h"


namespace PROGRAM_NAMESPACE {

    /* Global Utility Functions */
    namespace UTILS_NAMESPACE {

        // String Functions

        std::wstring stringToLowercase ( const std::wstring& str ) {

            std::wstring lcStr = {};

            for ( const wchar_t& ch : str )
                lcStr.push_back( std::towlower(ch) );

            return lcStr;

        }
        std::wstring stringToUppercase ( const std::wstring& str ) {

            std::wstring ucStr = {};

            for ( const wchar_t& ch : str )
                ucStr.push_back( std::towupper(ch) );

            return ucStr;

        }

        std::wstring trimString ( const std::wstring& str ) {

            std::wsmatch matches;
            std::regex_match( str, matches, std::wregex(L"^\\s*(.+?)\\s*$") );

            // Return the match for the first Capture Group in the Regular Expression.
            return matches[1];
            
        }
        std::wstring truncateString ( const std::wstring& str, size_t maxLength ) {
        
            if ( str.length() <= maxLength )
                return str;

            std::wstring truncStr = str.substr(0, maxLength - 3ULL);
                         truncStr.append(L"...");

            return truncStr;
        
        }
        std::wstring truncatePathString ( const std::wstring& str, size_t maxLength ) {

            if ( str.length() <= maxLength )
                return str;

            std::wstring truncStr = str;
            size_t index = 0ULL;

            // Start the truncation after the first Path Segment in the Path String.
            while ( index < truncStr.length() && truncStr[index] != L'\\' && truncStr[index] != L'/' ) {
                index++;
            }

            index++;

            // Attempt to trim Path Segments from the Path String until the
            // length of the Path String is less than `maxLength`.
            while ( index < truncStr.length() ) {
                truncStr.erase(index, 1);

                if ( (truncStr[index] == L'\\' || truncStr[index] == L'/') && truncStr.length() <= (maxLength - 3ULL) )
                    break;
            }

            // If we were successfully able to truncate the Path String, 
            // we just need to insert the ellipsis to indicate where the truncation occurred.
            if ( index < truncStr.length() ) {
                truncStr.insert(index, L"..."); 
            }
            // If we weren't able to truncate the Path String, we will return the
            // truncated File or Directory Name instead.
            else {
                wchar_t currentChar;

                truncStr = L"...";
                index = (str.length() - 1ULL);

                while ( index >= 0ULL && (currentChar = str[index--]) != L'\\' && currentChar != L'/' )
                    truncStr.insert(3ULL, 1ULL, currentChar);
            }

            return truncStr;

        }

        // File Functions

        std::optional<std::wstring> createTempFile () {

            // The Wide-Character String wrapped in an `std::optional`
            // returned by the function.
            std::optional<std::wstring> tempFilePath = {};
            // A Null-Terminated Wide-Character String containing the path
            // to the newly-created Temporary File.
            WCHAR tempFile[L_tmpnam] = {};

            if ( _wtmpnam_s(tempFile) == 0 )
                tempFilePath = tempFile;

            return tempFilePath;

        }

    }


    /* DisplayMonitorStruct::DisplayResolutionStruct */
    // Structure Constructors

    DisplayMonitorStruct::DisplayResolutionStruct::DisplayResolutionStruct (
        DWORD iDisplayWidth, DWORD iDisplayHeight, DWORD iRefreshRate
    ) : displayWidth(iDisplayWidth), displayHeight(iDisplayHeight), refreshRate(iRefreshRate),
        resolutionString( std::format(L"{:d} x {:d} @ {:d} Hz", iDisplayWidth, iDisplayHeight, iRefreshRate) )
    {}


    /* DisplayMonitorStruct */
    // Structure Constructors

    DisplayMonitorStruct::DisplayMonitorStruct (
        display_number_t iDisplayNum,
        std::wstring iDisplayId,
        std::wstring iMonitorName,
        DWORD displayWidth,
        DWORD displayHeight,
        DWORD refreshRate,
        bool isMainDisplay
    ) :
        displayNum(iDisplayNum),
        displayId(iDisplayId),
        monitorName(iMonitorName),
        currentResolution(displayWidth, displayHeight, refreshRate),
        comments(isMainDisplay ? L"Main Display" : L"")
    {}


    /* Global Variables */

    UTILS_NAMESPACE::ProgramSettings programSettings = {};


    /* Global Helper Functions */

    bool ensureProgramDataDirectoryExists () {

        // Don't create any new directories in Stateless Mode.
        if ( programSettings.statelessMode || std::filesystem::exists(PROGRAM_DATA_PATH) )
            return true;

        return std::filesystem::create_directories(PROGRAM_DATA_PATH);

    }
    bool ensureProgramDataDirectoryExists ( _Out_ std::optional<std::exception>* oExceptionPtr ) noexcept {

        try {
            bool result = ensureProgramDataDirectoryExists();

            if (oExceptionPtr != nullptr) {
                oExceptionPtr->reset();
            }

            return result;
        }
        catch ( const std::exception& ex ) {
            if (oExceptionPtr != nullptr) {
                *oExceptionPtr = ex;
            }

            return false;
        }
        
    }

}
