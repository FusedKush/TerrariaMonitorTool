#pragma once


/*
* UserInterface.h
*
* Header File defining the `UserInterface` class, which encapsulates
* all of the functionality used to provide a fluid and
* consistent User Interface for the program.
*/


#include "framework.h"
#include "Console.h"
#include <list>


namespace PROGRAM_NAMESPACE {

	/**
	 * A class encapsulating all of the functionality used to provide a
	 * fluid and consistent User Interface for the program.
	 */
	class UserInterface {

		/* Enumeration & Type Definitions */
		public:
			/**
			 * An enumeration defining the available menu options,
			 * not including the "options" used to select a Connected Display Monitor
			 * to make the new Active Display Monitor.
			 */
			enum MainMenuOption {

				CONFIG_FILE_BACKUPS_MENU_OPTION,	// Navigate to the Configuration File Backups Menu.
				PROGRAM_SETTINGS_MENU_OPTION,		// Navigate to the Program Settings Menu.
				EXIT_MENU_OPTION					// Exit the Program.

			};

			/**
			 * A Type-Safe Union representing the selection
			 * made in the Main Menu of the User Interface.
			 * 
			 * The `std::variant` will contain either a `DisplayMonitor`
			 * representing a Connected Display Monitor to make the new Active Display Monitor,
			 * or a `MainMenuOption` representing an alternate menu option that has been selected.
			 */
			typedef std::variant<MainMenuOption, DisplayMonitor> MainMenuSelection;


		/* Inner Classes & Structure Types */
		protected:
			/**
			 * A internal structure type containing all of the properties associated with the
			 * sizing, spacing, and appearance of the User Interface in the console.
			 * 
			 * @internal	This structure type and all of its associated functionality are for
			 * 				internal use only and are subject to change at any time.
			 */
			typedef struct TextSizingStruct {

				/* Type Definitions */
				public:
					// An unsigned integer type representing the width or size
					// of a line of output in the User Interface.
					typedef unsigned short text_sizing_t;


				/* Structure Fields */
				public:
					text_sizing_t leftColSize;			// The width of the leftmost column of a Connected Display Monitor Menu Option.
					text_sizing_t displayIdColSize;		// The width of the "Display ID" column of a Connected Display Monitor Menu Option.
					text_sizing_t monitorNameColSize;	// The width of the "Monitor Name" column of a Connected Display Monitor Menu Option.
					text_sizing_t resolutionColSize;	// The width of the "Current Resolution" column of a Connected Display Monitor Menu Option.
					text_sizing_t commentsColSize;		// The width of the "Comments" column of a Connected Display Monitor Menu Option.
					text_sizing_t extraColPadding;		// The amount of extra padding to account for when calculating the `consoleBoxWidth`.

					text_sizing_t consoleBoxWidth;		// The total width of a single line of the User Interface.
					std::wstring boxBorder;				// A Wide-Character String containing the "border line" for the User Interface.
					std::wstring boxSpace;				// A Wide-Character String containing a "blank line" for the User Interface.


				/* Structure Constructors */
				public:
					/**
					 * Construct a new, empty `TextSizingStruct`.
					 * 
					 * Each of the fields will be initialized to an appropriate default value.
					 */
					TextSizingStruct ();

					/**
					 * Construct a new `TextSizingStruct`.
					 * 
					 * @param iLeftColSize			The width of the leftmost column of a Connected Display Monitor Menu Option.
					 * @param iDisplayIdColSize		The width of the "Display ID" column of a Connected Display Monitor Menu Option.
					 * @param iMonitorNameColSize	The width of the "Monitor Name" column of a Connected Display Monitor Menu Option.
					 * @param iResolutionColSize	The width of the "Current Resolution" column of a Connected Display Monitor Menu Option.
					 * @param iCommentsColSize		The width of the "Comments" column of a Connected Display Monitor Menu Option.
					 * @param iExtraColPadding		The amount of extra padding to account for when calculating the value for the `consoleBoxWidth` field.
					 */
					TextSizingStruct (
						text_sizing_t iLeftColSize,
						text_sizing_t iDisplayIdColSize,
						text_sizing_t iMonitorNameColSize,
						text_sizing_t iResolutionColSize,
						text_sizing_t iCommentsColSize,
						text_sizing_t iExtraColPadding
					);


				/* Structure Methods */
				public:
					/**
					 * Get a copy of this `TextSizingStruct` where the
					 * `monitorNameColSize` of this `TextSizingStruct` has
					 * been replaced by the specified `newMonitorNameColSize`.
					 * 
					 * Given that the specified `newMonitorNameColSize` is not the same as the
					 * `monitorNameColSize` of this `TextSizingStruct`, any automatically-generated fields
					 * that depend on this value (e.g., `consoleBoxWidth` or `boxBorder`) will differ
					 * from that of this `TextSizingStruct`, as they will instead reflect the specified `newMonitorNameColSize`.
					 * 
					 * @param newMonitorColSize The new width of the "Monitor Name" column of a Connected Display Monitor Menu Option.
					 * 
					 * @returns					A new `TextSizingStruct` constructed using the relevant fields
					 * 							of this `TextSizingStruct` and the specified `newMonitorNameColSize`.
					 */
					TextSizingStruct withNewMonitorNameColSize ( text_sizing_t newMonitorNameColSize ) const;

			} TextSizing;

			/**
			 * An internal class comprising a collection of `std::filesystem::path` objects
			 * that represents the history of previously-used Configuration File Paths.
			 * 
			 * This class is an extension of an `std::list` of `std::filesystem::path` objects
			 * that inherits and exports all of its public members, including the `std::list` constructors.
			 * 
			 * @internal	This class and all of its associated functionality are for
			 * 				internal use only and are subject to change at any time.
			 */
			class ConfigurationPathHistory : public std::list<std::filesystem::path> {

				/* Class Constants */
				protected:
					static const std::wstring PATH_HISTORY_FILE_NAME;			// The name of the file used to store the Configuration Path History.
					static const std::filesystem::path PATH_HISTORY_FILE_PATH;	// The path to the file used to store the Configuration Path History.

			
				/* Class Constructors */
				public:
					using list::list;	// Inherit and export the `std::list` constructors.


				/* Serialization & Persistence to File */
				public:
					/**
					 * Fetch the Configuration Path History from the saved 
					 * Configuration Path History File, if applicable.
					 * 
					 * The Configuration Path History is stored in a file located at `PATH_HISTORY_FILE_PATH`.
					 * 
					 * @returns		A new `ConfigurationPathHistory` object.
					 * 
					 * 				If the Configuration Path History File located at `PATH_HISTORY_FILE_PATH`
					 * 				contains one or more valid Configuration File Paths, the returned `ConfigurationPathHistory` object
					 * 				will contain one or more `std::filesystem::path`s corresponding to the saved Configuration File Paths.
					 * 
					 * 				If the Configuration Path History File does not exist or contains no
					 * 				valid Configuration File Paths, an empty `ConfigurationPathHistory` object will be returned.
					 */
					static ConfigurationPathHistory fetchFromFile ();
					
					/**
					 * Save the Configuration File Paths stored in this object to a file.
					 * 
					 * The Configuration Path History is stored in a file located at `PATH_HISTORY_FILE_PATH`.
					 * 
					 * @return		`true` on success and `false` on failure.
					 */
					bool saveToFile ();
					/**
					 * Delete the Configuration Path History File.
					 * 
					 * This method does *not* call or replicate the behavior of
					 * the `clear()` method inherited from `std::list`, meaning that
					 * all of the Configuration File Paths currently stored in this object
					 * will still be accessible even after invoking this method and
					 * deleting the Configuration Path History File.
					 * 
					 * All of the Configuration File Paths currently stored in the
					 * Configuration Path History File located at `PATH_HISTORY_FILE_PATH`
					 * will be irrecoverably lost unless they still exist within this object.
					 * 
					 * @return		`true` if the Configuration Path History File was successfully
					 * 				deleted or does not currently exist.
					 * 
					 * 				Returns `false` if the Configuration Path History File
					 * 				could not be successfully deleted.
					 */
					bool deleteSavedData ();
			
			};


		/* Instance Properties */
		private:
			// The `TextSizing` structure containing all of the properties associated with
			// the sizing, spacing, and appearance of the User Interface in the console.
			TextSizing textSizing;
			// A Smart Pointer to the `Console` object used to interact with the Windows Console.
			mutable Console::console_ptr_t console;

			/**
			 * The formatted title of the program, to be displayed in the User Interface.
			 * 
			 * @invariant 	This field is dependent on the `textSizing` properties of the `UserInterface`, and thus,
			 * 				must generally be updated anytime the `textSizing` instance property is modified
			 * 				(e.g., via the `changeTextSizing()` method).
			 */
			std::wstring programTitle = {};


		/* Class Constructors */
		public:
			/**
			 * Construct a new `UserInterface`.
			 * 
			 * @param iConsolePtr 	A Smart Pointer to the `Console` object used
			 * 						to interact with the Windows Console.
			 */
			UserInterface ( const Console::console_ptr_t& iConsolePtr );


		/* Instance Methods */
		public:
			// Getters & Setters

			/**
			 * Get the `TextSizing` structure containing all of the properties associated with
			 * the sizing, spacing, and appearance of the User Interface in the console.
			 * 
			 * @return 	A constant reference to the `TextSizing` structure associated with the `UserInterface`.
			 */
			const TextSizing& getTextSizing () const;
			/**
			 * Change the `TextSizing` structure defining all of the properties associated with
			 * the sizing, spacing, and appearance of the User Interface in the console.
			 * 
			 * Because the `programTitle` instance property is dependent on the `TextSizing`
			 * associated with the `UserInterface`, the `programTitle` property will be updated
			 * to reflect the `newTextSizing` as well.
			 * 
			 * @param newTextSizing		The new `TextSizing` structure to associate with the `UserInterface`.
			 * 
			 * @return 					The old `TextSizing` structure.
			 */
			TextSizing changeTextSizing ( const TextSizing& newTextSizing );


			// User Interface Procedures

			/**
			 * Print Usage and Help Information to the Console, optionally for a
			 * specified Command-Line Argument, Flag, or Switch, depending
			 * on the provided Command-Line Arguments.
			 * 				
			 * @param argc	The total number of Command-Line Arguments available in `argv`.
			 * 
			 * @param argv 	An array of Null-Terminated Wide-Character Strings corresponding
			 * 				to the Command-Line Arguments passed to the Program.
			 */
			void printUsageMessage ( int argc, LPCWSTR argv[] ) const;

			/**
			 * Prompt the user for the path to the Terraria Configuration File.
			 * 
			 * An Alternate Output Buffer will be created to display
			 * the Selection Interface, and the previous Output Buffer
			 * and User Interface will be restored before this method returns.
			 * 
			 * The user will have the option to select a path from a list
			 * of recently-used paths or use the default path
			 * when there are no recently-used paths available,
			 * as well as specify a custom path to the Terraria Configuration File.
			 * 
			 * 		- The user will also have the option to modify the Configuration Path History,
			 *   	  including the ability to remove specific Configuration File Paths in the
			 *   	  Configuration Path History, as well as clearing the Configuration File Path History.
			 * 
			 *  	- Depending on the actions of the user, the Configuration Path History File may be created,
			 *    	  modified, and/or deleted in the Program Data Directory before this method returns.
			 * 
			 * 		- All specified paths will be checked to ensure that they are valid and point 
			 * 		  to a directory containing a valid Terraria Configuration File.
			 * 
			 * @returns		A Wide-Character String containing the path to the Terraria Configuration File,
			 * 				wrapped in an `std::optional` object.
			 * 
			 * 				If the user chooses to exit the program using the `ESC` key,
			 * 				an empty `std::optional` object is returned.
			 */
			std::optional<std::wstring> promptForConfigFilePath () const;
			/**
			 * Render the Main Menu for the Program.
			 * 
			 * An Alternate Output Buffer will be created to display
			 * the Selection Interface, and the previous Output Buffer
			 * and User Interface will be restored before this method returns.
			 * 
			 * The user will have the option to select the Active Display Monitor
			 * from the list of Connected Display Monitors, as well as navigate
			 * to the Configuration File Backups Menu, navigate to the Program Settings Menu,
			 * or exit the program.
			 * 
			 * This method is intended to be repeatedly invoked each time
			 * that a `DisplayMonitor` is returned and until a `MainMenuOption`
			 * is returned instead. When using the method in this manner, the `renderMenu` argument
			 * should be `true` for the *first call only*, and it should be made `false` for
			 * *all subsequent calls* until a `MainMenuOption` is returned.
			 * 
			 * @param configFilePath		The current path to the Terraria Configuration File being used.
			 * 
			 * @param displayMonitors		A `DisplayMonitorList` containing the Connected Display Monitors to choose from.
			 * 
			 * @param renderMenu			Indicates whether the Main Menu should be rendered for the first time (`true`),
			 * 								or if the Main Menu should be updated to reflect any changes to
			 * 								the Active Display Monitor (`false`).
			 * 
			 * @param selectedMonitorNum	The Display Number of the Active Display Monitor.
			 * 
			 * @returns						Returns a Type-Safe `MainMenuSelection` Union, 
			 * 								wrapped in an `std::optional` object.
			 * 
			 * 								If the user selects the *Exit* option in the Main Menu,
			 * 								`MainMenuSelection::EXIT_MENU_OPTION` will be returned, wrapped in an `std::optional` object.
			 * 								However, if the user exits the program by pressing the `ESC` key,
			 * 								an empty `std::optional` object will be returned instead.
			 */
			std::optional<UserInterface::MainMenuSelection> mainMenu (
				const std::wstring& configFilePath,
				const DisplayMonitorList& displayMonitors,
				bool renderMenu = true,
				DisplayMonitor::display_number_t selectedMonitorNum = 1U
			) const;

			/**
			 * Prompt the user for confirmation to proceed with an action.
			 * 
			 * An Alternate Output Buffer will be created to display
			 * the Confirmation Interface, and the previous Output Buffer
			 * and User Interface will be restored before this method returns.
			 * 
			 * @warning 		To avoid rendering issues, passing using newline characters (`\n`) in the
			 * 					specified `title` or `subtitle` strings is strongly discouraged unless they
			 * 					properly account for the `textSizing` properties associated with the `UserInterface`.
			 * 
			 * @param title 	A Wide-Character String containing the Title of the Confirmation Screen.
			 * 
			 * @param subtitle 	A Wide-Character String containing the Subtitle of the Confirmation Screen.
			 * 
			 * @return 			A boolean indicating whether or not the user confirmed the action,
			 * 					wrapped in an `std::optional` object.
			 * 
			 * 						- If the user confirmed the action, the `std::optional` will contain a `true` value.
			 * 
			 * 						- If the user rejected the action, the `std::optional` will contain a `false` value.
			 * 
			 * 					If the user backed out of the Confirmation Screen using the `ESC` key,
			 * 					an empty `std::optional` object will be returned.
			 */
			std::optional<bool> promptForConfirmation (
				const std::wstring& title = L"Are You Sure?",
				const std::wstring& subtitle = L"This action cannot be reversed."
			) const;
			/**
			 * Prompt the user for confirmation to proceed with an action.
			 * 
			 * An Alternate Output Buffer will be created to display
			 * the Confirmation Interface, and the previous Output Buffer
			 * and User Interface will be restored before this method returns.
			 * 
			 * @warning 			To avoid rendering issues, passing using newline characters (`\n`) in the
			 * 						specified `title` or `subtitle` strings is strongly discouraged unless they
			 * 						properly account for the `textSizing` properties associated with the `UserInterface`.
			 * 
			 * @param menuOptions	The `MenuOptionList` containing the `MenuOption`s to be presented to the user.
			 * 
			 * 						The *first* `MenuOption` in the `MenuOptionList` is always considered
			 * 						to be the "confirm" or "yes" option that corresponds to a return value of `true`.
			 * 	
			 * 						All other `MenuOption`s in the `MenuOptionList`, of which there should generally
			 * 						only be two in total, will correspond to a "reject" or "no" option 
			 * 						that corresponds to a return value of `false`.
			 * 
			 * @param title 		A Wide-Character String containing the Title of the Confirmation Screen.
			 * 
			 * @param subtitle 		A Wide-Character String containing the Subtitle of the Confirmation Screen.
			 * 	
			 * @return 				A boolean indicating whether or not the user confirmed the action,
			 * 						wrapped in an `std::optional` object.
			 * 	
			 * 							- If the user confirmed the action, the `std::optional` will contain a `true` value.
			 * 	
			 * 							- If the user rejected the action, the `std::optional` will contain a `false` value.
			 * 	
			 * 						If the user backed out of the Confirmation Screen using the `ESC` key,
			 * 						an empty `std::optional` object will be returned.
			 */
			std::optional<bool> promptForConfirmation (
				Console::MenuOptionList& menuOptions,
				const std::wstring& title = L"Are You Sure?",
				const std::wstring& subtitle = L"This action cannot be reversed."
			) const;


		// Helper Methods
		protected:
			/**
			 * Print the Primary Header of the User Interface.
			 * 
			 * The Primary Header of the User Interface consists of the
			 * `PROGRAM_TITLE` and, optionally, one or more lines specified
			 * by the `subtitle`, all surrounded by a box.
			 * 
			 * @tparam Args 	The types of arguments passed as the `subtitle`.
			 * 
			 * @param subtitle 	One or more Wide-Character Strings specifying the optional Subtitle
			 * 					for the User Interface, each of which corresponds to a
			 * 					single line of the Subtitle.
			 *
			 * 					Each Wide-Character String is printed on its own line, including
			 * 					empty strings, which makes it possible to print empty lines
			 * 					between the `PROGRAM_TITLE` and `subtitle`, or between one or more
			 * 					lines of the `subtitle`.
			 * 
			 * 					If no `subtitle` is specified, only the `PROGRAM_TITLE` will be
			 * 					printed in the header and surrounded by the interface box.						
			 * 
			 * @returns			A reference to this object to support method chaining.
			 */
			template <typename... Args> const UserInterface& printInterfaceHeader ( const Args&... subtitle ) const;
			/**
			 * Print the Primary Header of the User Interface.
			 * 
			 * The Primary Header of the User Interface consists of the
			 * `PROGRAM_TITLE` and, optionally, one or more lines specified
			 * by the `subtitle`, all surrounded by a box.
			 * 
			 * @tparam Args 	The types of arguments passed as the `subtitle`.
			 * 
			 * @param subtitle 	One or more Wide-Character Strings specifying the optional Subtitle
			 * 					for the User Interface, each of which corresponds to a
			 * 					single line of the Subtitle.
			 *
			 * 					Each Wide-Character String is printed on its own line, including
			 * 					empty strings, which makes it possible to print empty lines
			 * 					between the `PROGRAM_TITLE` and `subtitle`, or between one or more
			 * 					lines of the `subtitle`.
			 * 
			 * 					If no `subtitle` is specified, only the `PROGRAM_TITLE` will be
			 * 					printed in the header and surrounded by the interface box.						
			 *
			 * @returns			A reference to this object to support method chaining.
			 */
			template <typename... Args> UserInterface& printInterfaceHeader ( const Args&... subtitle );

			/**
			 * Print the Help/Usage Message for a specific Command-Line Argument, Flag, or Switch.
			 * 
			 * @tparam Args 		The types of arguments passed as the `description`.
			 * 
			 * @param title			A Wide-Character String containing the title of the
			 * 						Command-Line Argument, Flag, or Switch.
			 * 
			 * @param usageStr		A Wide-Character String containing the Usage String for the
			 * 						Command-Line Argument, Flag, or Switch.
			 * 
			 * @param description 	One or more Wide-Character Strings specifying the Description
			 * 						for the specified Command-Line Argument, Flag, or Switch, 
			 * 						each of which corresponds to a single line of the Description.
			 * 
			 * 						Each Wide-Character String is printed on its own line, including
			 * 						empty strings, which makes it possible to print empty lines
			 * 						between one or more lines of the `description`.				
			 * 
			 * @returns				A reference to this object to support method chaining.
			 */
			template <typename... Args> void printArgUsageMessage (
				const std::wstring& title,
				const std::wstring& usageStr,
				const Args&... description
			) const;

	};

}