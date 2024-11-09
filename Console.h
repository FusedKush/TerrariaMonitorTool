#pragma once


/*
* Console.h
*
* Header File declaring the `Console` class and all associated
* Internal and Inner Classes, including:
*    - `Utils::AConsoleBuffer`
*    - `Utils::AConsoleInput`
*    - `Utils::AConoleOutput`
*    - `Console::InputBuffer`
*    - `Console::OutputBuffer`
* 
* The `Console` class is used frequently throughout the entire program to interact with
* the Windows Console via reading input writing formatted output.
*/


#include "framework.h"

#include <functional>	// std::function
#include <stack>
#include <variant>


namespace PROGRAM_NAMESPACE {

	/* Internal Utility Classes */
	namespace UTILS_NAMESPACE {

		/*
		 * An Internal, Abstract Class providing an interface for 
		 * storing and retrieving a handle to the underlying Windows Console.
		 * 
		 * @internal	This class and all of its associated functionality are for
		 * 				internal use only and are subject to change at any time.
		 */
		class AConsoleBuffer {
		
			/* Type Definitions */
			public:
				// A Windows Handle to an Input or Output Buffer for the underlying Windows Console.
				typedef HANDLE win_conbuf_t;


			/* Instance Properties */
			protected:
				// The handle to the Input or Output Buffer for the underlying Windows Console.
				win_conbuf_t bufferHandle;


			/* Class Constructors */
			protected:
				/**
				 * Construct a new `AConsoleBuffer`.
				 * 
				 * @param iBufferHandle	The handle to the Input or Output Buffer for the underlying Windows Console.
				 */
				AConsoleBuffer ( win_conbuf_t iBufferHandle );
		

			/* Instance Methods */
			public:
				/** 
				 * Get the handle to the Input or Output buffer for the underlying Windows Console.
				 * 
				 * @returns 	A constant reference to the Windows Handle for the Input
				 *				or Output buffer for the underlying Windows Console.
				 */
				virtual const win_conbuf_t& getBufferHandle () const;

		};
		
		/**
		 * An Internal, Abstract, Template Class providing an interface
		 * for writing formatted output to the Windows Console.
		 * 
		 * @tparam ThisT 	The inheriting class. Necessary for Compile-Time Polymorphism.
		 * 
		 * @internal		This class and all of its associated functionality are for
		 * 					internal use only and are subject to change at any time.
		 */
		template <class ThisT> class AConsoleOutput {

			/* Type Definitions */
			public:
				// An integer type representing one of the Output Buffers of the Console.
				typedef unsigned short buffer_number_t;
				// A Windows structure type containing the Cartesian Coordinates of the Console Cursor.
				typedef COORD WinConsoleCursorCoordinates;


			/* Class Constants */
			public:
				static constexpr wchar_t ARROW_LEFT = L'\u2190';		// Unicode Character for a Northern-Pointed Arrow.
				static constexpr wchar_t ARROW_UP = L'\u2191';			// Unicode Character for a Western-Pointed Arrow.
				static constexpr wchar_t ARROW_RIGHT = L'\u2192';		// Unicode Character for an Eastern-Pointed Arrow.
				static constexpr wchar_t ARROW_DOWN = L'\u2193';		// Unicode Character for a Southern-Pointed Arrow.
				static constexpr wchar_t ARROW_LEFT_RIGHT = L'\u2194';	// Unicode Character for a Northern-Eastern-Pointed Arrow.
				static constexpr wchar_t ARROW_UP_DOWN = L'\u2195';		// Unicode Character for a Southern-Western-Pointed Arrow.
				
				// The Escape Sequence denoting the beginning of a Virtual Terminal Sequence
				// used to query or modify one or more aspects of the Console.
				static constexpr wchar_t VIRTUAL_TERMINAL_SEQUENCE_ESCAPE = L'\x1B';

			protected:
				/**
				 * The maximum number of lines that can be used to render a `MenuOptionList`.
				 * 
				 * Higher values provide better readability while lower values
				 * provide better reliability is less likely to cause the User Interface to break
				 * at lower Console Window Resolutions. 
				 */
				static const unsigned short MAX_MENU_OPTION_LINES = 9U;


			/* Instance Methods */
			public:
				// Output Buffer Management

				/**
				 * Create and Switch to an Alternate Output Buffer.
				 * 
				 * @returns 	On success, returns an `std::optional` object containing a `buffer_number_t`
				 *				corresponding to the newly-created Output Buffer (i.e., the same `buffer_number_t`
				 *				that would be returned by immediately calling `getCurrentBufferNum()`
				 *				after this method returns).
				 * 	
				 *				On failure, returns an empty `std::optional` object.
				 */
				virtual std::optional<buffer_number_t> createAltBuffer () = 0;
				/**
				 * Get a number corresponding to the Output Buffer currently in-use by the `Console`.
				 * 
				 * @returns 	A `buffer_number_t` corresponding to the Output Buffer currently being used by the `Console`.
				 * 	
				 *					- A `buffer_number_t` equal to zero indicates that the Primary Buffer
				 *					  is currently being used and no Alternative Buffers are currently in-use.
				 * 	
				 *					- A `buffer_number_t` that is greater than zero indicates that an Alternative Buffer
				 *					  is currently being used.
				 */
				virtual buffer_number_t getCurrentBufferNum () const = 0;
				/**
				 * Switch to and Restore the Previous Output Buffer.
				 * 
				 * If the Current Output Buffer is an Alternate Output Buffer created
				 * using the `createAltBuffer()` method, it will be closed and it will no longer
				 * be possible to write to or read from the buffer.
				 * 
				 * If there are currently no active Alternative Output Buffers and the
				 * only Output Buffer in-use is the Primary Buffer, this method has no effect.
				 * 
				 * @returns 	A `buffer_number_t` corresponding to the Output Buffer currently being used by the `Console`.
				 * 	
				 *				The returned `buffer_number_t` may or may not be the same as that returned by a call to
				 *				the `getCurrentBufferNum()` method immediately prior to invoking this method, depending on
				 *				whether or not there are any Alternate Output Buffers currently in-use.
				 */
				virtual buffer_number_t restorePreviousBuffer () = 0;


				// Cursor Position Management

				/**
				 * Get the Current Position of the Console Cursor.
				 * 
				 * @returns 	A `WinConsoleCursorCoordinates` structure containing the Cartesian Coordinates
				 *				corresponding to the Current Position of the Console Cursor, relative
				 *				to the visible viewport of the Console.
				 */
				virtual WinConsoleCursorCoordinates getCursorPos () const = 0;
				/**
				 * Get the number of lines that the Output Buffer has scrolled since it was initialized.
				 * 
				 * This value is typically used to offset Console Cursor Positions to account
				 * for the console scrolling, particularly when repeatedly printing to the bottom
				 * row of the visible viewport.
				 * 
				 * @returns 	A constant reference to an integer value representing the Current Cursor Scroll Offset.
				 * 	
				 *				This value will be equal to zero when the Output Buffer is first initialized and will increase
				 *				any time the visible viewport of the Console is scrolled up. As such, this value
				 *				can increase between calls to `getCursorScrollOffset()`, but it will never decrease.
				 */
				virtual const short& getCursorScrollOffset () const = 0;

				/**
				 * Set the Position of the Console Cursor.
				 * 
				 * @param cursorPos A `WinConsoleCursorCoordinates` structure containing the Cartesian Coordinates
				 *					corresponding to the new Position of the Console Cursor, relative
				 *					to the visible viewport of the Console.
				 * 
				 * @returns			`true` on success and `false` on failure.
				 */
				virtual bool setCursorPos ( const WinConsoleCursorCoordinates& cursorPos ) = 0;
				/**
				 * Save the Current Position of the Console Cursor to be retrieved by
				 * a future call to `restoreSavedCursorPos()`.
				 * 
				 * @returns		`true` on success and `false` on failure.
				 */
				virtual bool saveCursorPos () = 0;
				/**
				 * Save the Specified Position of the Console Cursor to be retrieved by
				 * a future call to `restoreSavedCursorPos()`.
				 * 
				 * @param cursorPos A `WinConsoleCursorCoordinates` structure containing the Cartesian Coordinates
				 *					corresponding to the desired Position of the Console Cursor to be saved,
				 *					relative to the visible viewport of the Console.
				 * 
				 * @returns			`true` on success and `false` on failure.
				 */
				virtual bool saveCursorPos ( const WinConsoleCursorCoordinates& cursorPos ) = 0;
				/**
				 * Retrieve the Position of the Console Cursor that was stored by a previous call to `saveCursorPos()`.
				 * 
				 * @returns		On success, returns an `std::optional` object containing a `WinConsoleCursorCoordinates` structure
				 *				containing the Cartesian Coordinates of the saved Position of the Console Cursor,
				 *				relative to the visible viewport of the Console.
				 * 
				 *				If no cursor was previously saved by calling `saveCursorPos()`, an empty `std::optional` object is returned.
				 */
				virtual std::optional<WinConsoleCursorCoordinates> restoreSavedCursorPos () = 0;

				/**
				 * Toggle the Visibility of the Console Cursor.
				 * 
				 * @returns		A reference to this object to support method chaining.
				 */
				virtual ThisT& toggleCursorVisibility () = 0;
				/**
				 * Set the Visibility of the Console Cursor.
				 * 
				 * @param visible	Indicates whether or not the Console Cursor should be made Visible.
				 * 
				 * @returns			A reference to this object to support method chaining.
				 */
				virtual ThisT& toggleCursorVisibility ( bool visible ) = 0;


				// Console Output

				/**
				 * Clear the Current Output Buffer.
				 * 
				 * @param clearBuffer		Indicates whether the underlying string buffers for the 
				 *							Current Output Buffer should be cleared or not.
				 *	
				 * 							Useful for temporarily clearing the screen for presentational purposes.
				 * 
				 * @param resetCursorPos	Indicates if the position of the Console Cursor should be
				 * 							reset prior to clearing the screen, which is generally
				 * 							required to be able to clear the entire screen.
				 * 
				 * @returns					A reference to this object to support method chaining.
				 */
				virtual ThisT& clear ( bool clearBuffer = false, bool resetCursorPos = true ) = 0;

				/**
				 * Print the specified Wide Character to the Current Output Buffer.
				 * 
				 * @param ch			The Wide Character to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer	Indicates if the specified Wide Character should be added to
				 * 						the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				const ThisT& print ( wchar_t ch, bool addToBuffer = true ) const {

					wchar_t chstr[2] = { ch, L'\0' };
					return this->print(chstr, addToBuffer);

				}
				/**
				 * Print the specified Wide Character to the Current Output Buffer.
				 * 
				 * @param ch			The Wide Character to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer	Indicates if the specified Wide Character should be added to
				 * 						the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				ThisT& print ( wchar_t ch, bool addToBuffer = true ) {

					wchar_t chstr[2] = { ch, L'\0' };
					return this->print(chstr, addToBuffer);

				}
				
				/**
				 * Print the specified number to the Current Output Buffer.
				 * 
				 * @param num			The Integer or Floating-Point Number to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer	Indicates if the specified number should be added to
				 * 						the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template <Numeric NumberT>
				const ThisT& print ( NumberT num, bool addToBuffer = true ) const {

					return this->print(std::to_wstring(num), addToBuffer);

				}
				/**
				 * Print the specified number to the Current Output Buffer.
				 * 
				 * @param num			The Integer or Floating-Point Number to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer	Indicates if the specified number should be added to
				 * 						the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template <Numeric NumberT>
				ThisT& print ( NumberT num, bool addToBuffer = true ) {

					return this->print(std::to_wstring(num), addToBuffer);

				}

				/**
				 * Print the specified Wide-Character String to the Current Output Buffer.
				 * 
				 * @param str					The Wide-Character String to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer			Indicates if the specified Wide-Character String should be added to
				 * 								the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns						A reference to this object to support method chaining.
				 */
				const ThisT& print ( const std::wstring& str, bool addToBuffer = true ) const {
				
					return this->print(str.c_str(), addToBuffer);
				
				}
				/**
				 * Print the specified Wide-Character String to the Current Output Buffer.
				 * 
				 * @param str					The Wide-Character String to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer			Indicates if the specified Wide-Character String should be added to
				 * 								the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns						A reference to this object to support method chaining.
				 */
				ThisT& print ( const std::wstring& str, bool addToBuffer = true ) {
				
					return this->print(str.c_str(), addToBuffer);
				
				}

				/**
				 * Print the specified Null-Terminated Wide-Character String to the Current Output Buffer.
				 * 
				 * @param str					The Null-Terminated Wide-Character String to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer			Indicates if the specified Wide-Character String should be added to
				 * 								the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns						A reference to this object to support method chaining.
				 */
				virtual const ThisT& print ( const wchar_t* str, bool addToBuffer = true ) const = 0;
				/**
				 * Print the specified Null-Terminated Wide-Character String to the Current Output Buffer.
				 * 
				 * @param str					The Null-Terminated Wide-Character String to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer			Indicates if the specified Wide-Character String should be added to
				 * 								the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns						A reference to this object to support method chaining.
				 */
				virtual ThisT& print ( const wchar_t* str, bool addToBuffer = true ) = 0;


				/**
				 * Print a Newline Character to the Current Output Buffer.
				 * 
				 * @param addToBuffer	Indicates if the Newline Character should be added
				 * 						to the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				const ThisT& println ( bool addToBuffer = true ) const {

					return this->print(L'\n', addToBuffer);

				}
				/**
				 * Print a Newline Character to the Current Output Buffer.
				 * 
				 * @param addToBuffer	Indicates if the Newline Character should be added
				 * 						to the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				ThisT& println ( bool addToBuffer = true ) {

					return this->print(L'\n', addToBuffer);

				}

				/**
				 * Print the specified Wide Character to the Current Output Buffer
				 * immediately followed by a Newline Character.
				 * 
				 * @param ch			The Wide Character to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer	Indicates if the specified Wide Character and appended Newline Character
				 * 						should be added to the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				const ThisT& println ( wchar_t ch, bool addToBuffer = true ) const {

					return this->print(ch, addToBuffer).println(addToBuffer);

				}
				/**
				 * Print the specified Wide Character to the Current Output Buffer
				 * immediately followed by a Newline Character.
				 * 
				 * @param ch			The Wide Character to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer	Indicates if the specified Wide Character and appended Newline Character
				 * 						should be added to the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				ThisT& println ( wchar_t ch, bool addToBuffer ) {

					return this->print(ch, addToBuffer).println(addToBuffer);

				}
				
				/**
				 * Print the specified number to the Current Output Buffer
				 * immediately followed by a Newline Character.
				 * 
				 * @param num			The Integer or Floating-Point Number to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer	Indicates if the specified number and appended Newline Character
				 * 						should be added to the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template <Numeric NumberT>
				const ThisT& println ( NumberT num, bool addToBuffer = true ) const {

					return this->print(num, addToBuffer).println(addToBuffer);

				}
				/**
				 * Print the specified number to the Current Output Buffer
				 * immediately followed by a Newline Character.
				 * 
				 * @param num			The Integer or Floating-Point Number to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer	Indicates if the specified number and appended Newline Character
				 * 						should be added to the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template <Numeric NumberT>
				ThisT& println ( NumberT num, bool addToBuffer = true ) {

					return this->print(num, addToBuffer).println(addToBuffer);

				}
				
				/**
				 * Print the specified Wide-Character String to the Current Output Buffer
				 * immediately followed by a Newline Character.
				 * 
				 * @param str			The Wide-Character String to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer	Indicates if the specified Wide-Character String and appended Newline Character
				 * 						should be added to the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template<StringConvertible StringT>
				const ThisT& println ( const StringT& str, bool addToBuffer = true ) const {

					return this->print(str, addToBuffer).println(addToBuffer);

				}
				/**
				 * Print the specified Wide-Character String to the Current Output Buffer
				 * immediately followed by a Newline Character.
				 * 
				 * @param str			The Wide-Character String to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer	Indicates if the specified Wide-Character String and appended Newline Character
				 * 						should be added to the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template<StringConvertible StringT>
				ThisT& println ( const StringT& str, bool addToBuffer = true ) {

					return this->print(str, addToBuffer).println(addToBuffer);

				}


				/**
				 * Print a Space Character to the Current Output Buffer.
				 * 
				 * 
				 * @param addToBuffer	Indicates if the Space Character should be added
				 * 						to the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				const ThisT& printsp ( bool addToBuffer = true ) const {

					return this->print(L' ', addToBuffer);

				}
				/**
				 * Print a Space Character to the Current Output Buffer.
				 * 	
				 * @param addToBuffer	Indicates if the Space Character should be added
				 * 						to the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				ThisT& printsp ( bool addToBuffer = true ) {

					return this->print(L' ', addToBuffer);

				}

				/**
				 * Print the specified Wide Character to the Current Output Buffer
				 * immediately followed by a Space Character.
				 * 
				 * @param ch			The Wide Character to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer	Indicates if the specified Wide Character and appended Space Character
				 * 						should be added to the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				const ThisT& printsp ( wchar_t ch, bool addToBuffer = true ) const {

					return this->print(ch, addToBuffer).printsp(addToBuffer);

				}
				/**
				 * Print the specified Wide Character to the Current Output Buffer
				 * immediately followed by a Space Character.
				 * 
				 * @param ch			The Wide Character to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer	Indicates if the specified Wide Character and appended Space Character
				 * 						should be added to the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				ThisT& printsp ( wchar_t ch, bool addToBuffer = true ) {

					return this->print(ch, addToBuffer).printsp(addToBuffer);

				}
				
				/**
				 * Print the specified number to the Current Output Buffer
				 * immediately followed by a Space Character.
				 * 
				 * @param num			The Integer or Floating-Point Number to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer	Indicates if the specified number and appended Space Character
				 * 						should be added to the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template <Numeric NumberT>
				const ThisT& printsp ( NumberT num, bool addToBuffer = true ) const {

					return this->print(num, addToBuffer).printsp(addToBuffer);

				}
				/**
				 * Print the specified number to the Current Output Buffer
				 * immediately followed by a Space Character.
				 * 
				 * @param num	The Integer or Floating-Point Number to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer	Indicates if the specified number and appended Space Character
				 * 						should be added to the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns		A reference to this object to support method chaining.
				 */
				template <Numeric NumberT>
				ThisT& printsp ( NumberT num, bool addToBuffer = true ) {

					return this->print(num, addToBuffer).printsp(addToBuffer);

				}

				/**
				 * Print the specified Wide-Character String to the Current Output Buffer
				 * immediately followed by a Space Character.
				 * 
				 * @param str			The Wide-Character String to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer	Indicates if the specified Wide-Character String and appended Space Character
				 * 						should be added to the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template <StringConvertible StringT>
				const ThisT& printsp ( const StringT& str, bool addToBuffer = true ) const {

					return this->print(str, addToBuffer).printsp(addToBuffer);

				}
				/**
				 * Print the specified Wide-Character String to the Current Output Buffer
				 * immediately followed by a Space Character.
				 * 
				 * @param str			The Wide-Character String to be printed to the Current Output Buffer.
				 * 
				 * @param addToBuffer	Indicates if the specified Wide-Character String and appended Space Character
				 * 						should be added to the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template <StringConvertible StringT>
				ThisT& printsp ( const StringT& str, bool addToBuffer = true ) {

					return this->print(str, addToBuffer).printsp(addToBuffer);

				}

				
				/**
				 * Print Formatted Data to the Current Output Buffer.
				 * 
				 * This method uses the same syntax and provides the same functionality
				 * as the `std::format()` and `std::vformat()` functions.
				 * 
				 * By default, the resulting Wide-Character String will also be
				 * added to the underlying buffer for the Current Output Buffer.
				 * 
				 * @tparam StringT		The type of the specified `formatStr`.
				 * @tparam ArgsT		The types of the individual Format Arguments.
				 * 
				 * @param formatStr		The Wide-Character Format String.
				 * @param args			Zero or more Format Arguments.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template <StringConvertible StringT, typename... ArgsT>
				const ThisT& printf ( const StringT& formatStr, ArgsT... args ) const {
				
					return this->printf(true, formatStr, args...);
				
				}
				/**
				 * Print Formatted Data to the Current Output Buffer.
				 * 
				 * This method uses the same syntax and provides the same functionality
				 * as the `std::format()` and `std::vformat()` functions.
				 * 
				 * By default, the resulting Wide-Character String will also be
				 * added to the underlying buffer for the Current Output Buffer.
				 * 
				 * @tparam StringT		The type of the specified `formatStr`.
				 * @tparam ArgsT		The types of the individual Format Arguments.
				 * 
				 * @param formatStr		The Wide-Character Format String.
				 * @param args			Zero or more Format Arguments.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template <StringConvertible StringT, typename... ArgsT>
				ThisT& printf ( const StringT& formatStr, ArgsT... args ) {
				
					return this->printf(true, formatStr, args...);
				
				}
				/**
				 * Print Formatted Data to the Current Output Buffer.
				 * 
				 * This method uses the same syntax and provides the same functionality
				 * as the `std::format()` and `std::vformat()` functions.
				 * 
				 * @tparam StringT		The type of the specified `formatStr`.
				 * @tparam ArgsT		The types of the individual Format Arguments.
				 * 
				 * @param addToBuffer	Indicates if the resulting Wide-Character String should be added to
				 * 						the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @param formatStr		The Wide-Character Format String.
				 * 
				 * @param args			Zero or more Format Arguments.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template <StringConvertible StringT, typename... ArgsT>
				const ThisT& printf ( bool addToBuffer, const StringT& formatStr, ArgsT... args ) const {
				
					return this->print(
						std::vformat( formatStr, std::make_wformat_args(args...) ).c_str(),
						addToBuffer
					);
				
				}
				/**
				 * Print Formatted Data to the Current Output Buffer.
				 * 
				 * This method uses the same syntax and provides the same functionality
				 * as the `std::format()` and `std::vformat()` functions.
				 * 
				 * @tparam StringT		The type of the specified `formatStr`.
				 * @tparam ArgsT		The types of the individual Format Arguments.
				 * 
				 * @param addToBuffer	Indicates if the resulting Wide-Character String should be added to
				 * 						the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @param formatStr		The Wide-Character Format String.
				 * 
				 * @param args			Zero or more Format Arguments.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template <StringConvertible StringT, typename... ArgsT>
				ThisT& printf ( bool addToBuffer, const StringT& formatStr, ArgsT... args ) {
				
					return this->print(
						std::vformat( formatStr, std::make_wformat_args(args...) ).c_str(),
						addToBuffer
					);
				
				}
				
				/**
				 * Print Formatted Data to the Current Output Buffer
				 * immediately followed by a Newline Character.
				 * 
				 * This method is based on and provides the same functionality
				 * as the `std::format()` and `std::vformat()` functions.
				 * 
				 * By default, the resulting Wide-Character String and
				 * the appended Newline Character will also be added to
				 * the underlying buffer for the Current Output Buffer.
				 * 
				 * @tparam StringT		The type of the specified `formatStr`.
				 * @tparam ArgsT		The types of the individual Format Arguments.
				 * 
				 * @param formatStr		The Wide-Character Format String.
				 * @param args			Zero or more Format Arguments.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template <StringConvertible StringT, typename... ArgsT>
				const ThisT& printfln ( const StringT& formatStr, ArgsT... args ) const {

					return this->printfln(true, formatStr, args...);

				}
				/**
				 * Print Formatted Data to the Current Output Buffer
				 * immediately followed by a Newline Character.
				 * 
				 * This method is based on and provides the same functionality
				 * as the `std::format()` and `std::vformat()` functions.
				 * 
				 * By default, the resulting Wide-Character String and
				 * the appended Newline Character will also be added to
				 * the underlying buffer for the Current Output Buffer.
				 * 
				 * @tparam StringT		The type of the specified `formatStr`.
				 * @tparam ArgsT		The types of the individual Format Arguments.
				 * 
				 * @param formatStr		The Wide-Character Format String.
				 * @param args			Zero or more Format Arguments.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template <StringConvertible StringT, typename... ArgsT>
				ThisT& printfln ( const StringT& formatStr, ArgsT... args ) {

					return this->printfln(true, formatStr, args...);

				}
				/**
				 * Print Formatted Data to the Current Output Buffer
				 * immediately followed by a Newline Character.
				 * 
				 * This method is based on and provides the same functionality
				 * as the `std::format()` and `std::vformat()` functions.
				 * 
				 * @tparam StringT		The type of the specified `formatStr`.
				 * @tparam ArgsT		The types of the individual Format Arguments.
				 * 
				 * @param addToBuffer	Indicates if the resulting Wide-Character String and
				 * 						the appended Newline Character should be added to
				 * 						the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @param formatStr		The Wide-Character Format String.
				 * 
				 * @param args			Zero or more Format Arguments.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template <StringConvertible StringT, typename... ArgsT>
				const ThisT& printfln ( bool addToBuffer, const StringT& formatStr, ArgsT... args ) const {

					return this->printf(formatStr, args..., addToBuffer).println(addToBuffer);

				}
				/**
				 * Print Formatted Data to the Current Output Buffer
				 * immediately followed by a Newline Character.
				 * 
				 * This method is based on and provides the same functionality
				 * as the `std::format()` and `std::vformat()` functions.
				 * 
				 * @tparam StringT		The type of the specified `formatStr`.
				 * @tparam ArgsT		The types of the individual Format Arguments.
				 * 
				 * @param addToBuffer	Indicates if the resulting Wide-Character String and
				 * 						the appended Newline Character should be added to
				 * 						the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @param formatStr		The Wide-Character Format String.
				 * 
				 * @param args			Zero or more Format Arguments.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template <StringConvertible StringT, typename... ArgsT>
				ThisT& printfln ( bool addToBuffer, const StringT& formatStr, ArgsT... args ) {

					return this->printf(formatStr, args..., addToBuffer).println(addToBuffer);

				}
				
				/**
				 * Print Formatted Data to the Current Output Buffer
				 * immediately followed by a Space Character.
				 * 
				 * This method is based on and provides the same functionality
				 * as the `std::format()` and `std::vformat()` functions.
				 * 
				 * By default, the resulting Wide-Character String and
				 * the appended Space Character will also be added to
				 * the underlying buffer for the Current Output Buffer.
				 * 
				 * @tparam StringT		The type of the specified `formatStr`.
				 * @tparam ArgsT		The types of the individual Format Arguments.
				 * 
				 * @param formatStr		The Wide-Character Format String.
				 * @param args			Zero or more Format Arguments.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template <StringConvertible StringT, typename... ArgsT>
				const ThisT& printfsp ( const StringT& formatStr, ArgsT... args ) const {

					return this->printfsp(true, formatStr, args...);

				}
				/**
				 * Print Formatted Data to the Current Output Buffer
				 * immediately followed by a Space Character.
				 * 
				 * This method is based on and provides the same functionality
				 * as the `std::format()` and `std::vformat()` functions.
				 * 
				 * By default, the resulting Wide-Character String and
				 * the appended Space Character will also be added to
				 * the underlying buffer for the Current Output Buffer.
				 * 
				 * @tparam StringT		The type of the specified `formatStr`.
				 * @tparam ArgsT		The types of the individual Format Arguments.
				 * 
				 * @param formatStr		The Wide-Character Format String.
				 * @param args			Zero or more Format Arguments.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template <StringConvertible StringT, typename... ArgsT>
				ThisT& printfsp ( const StringT& formatStr, ArgsT... args ) {

					return this->printfsp(true, formatStr, args...);

				}
				/**
				 * Print Formatted Data to the Current Output Buffer
				 * immediately followed by a Space Character.
				 * 
				 * This method is based on and provides the same functionality
				 * as the `std::format()` and `std::vformat()` functions.
				 * 
				 * @tparam StringT		The type of the specified `formatStr`.
				 * @tparam ArgsT		The types of the individual Format Arguments.
				 * 
				 * @param addToBuffer	Indicates if the resulting Wide-Character String and
				 * 						the appended Space Character should be added to
				 * 						the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @param formatStr		The Wide-Character Format String.
				 * 
				 * @param args			Zero or more Format Arguments.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template <StringConvertible StringT, typename... ArgsT>
				const ThisT& printfsp ( bool addToBuffer, const StringT& formatStr, ArgsT... args ) const {

					return this->printf(formatStr, args..., addToBuffer).printsp(addToBuffer);

				}
				/**
				 * Print Formatted Data to the Current Output Buffer
				 * immediately followed by a Space Character.
				 * 
				 * This method is based on and provides the same functionality
				 * as the `std::format()` and `std::vformat()` functions.
				 * 
				 * @tparam StringT		The type of the specified `formatStr`.
				 * @tparam ArgsT		The types of the individual Format Arguments.
				 * 
				 * @param addToBuffer	Indicates if the resulting Wide-Character String and
				 * 						the appended Space Character should be added to
				 * 						the underlying buffer for the Current Output Buffer or not.
				 * 
				 * @param formatStr		The Wide-Character Format String.
				 * 
				 * @param args			Zero or more Format Arguments.
				 * 
				 * @returns				A reference to this object to support method chaining.
				 */
				template <StringConvertible StringT, typename... ArgsT>
				ThisT& printfsp ( bool addToBuffer, const StringT& formatStr, ArgsT... args ) {

					return this->printf(formatStr, args..., addToBuffer).printsp(addToBuffer);

				}


			/* Overloaded Operators */
			public:
				/**
				 * Print the specified Wide Character to the Current Output Buffer.
				 * 
				 * This operator is functionally equivalent to the `print()` method.
				 * 
				 * @param ch	The Wide Character to be printed to the Current Output Buffer.
				 * 
				 * @returns		A reference to this object to support operator chaining.
				 */
				const ThisT& operator << ( wchar_t ch ) const {

					return this->print(ch);

				}
				/**
				 * Print the specified Wide Character to the Current Output Buffer.
				 * 
				 * This operator is functionally equivalent to the `print()` method.
				 * 
				 * @param ch	The Wide Character to be printed to the Current Output Buffer.
				 * 
				 * @returns		A reference to this object to support operator chaining.
				 */
				ThisT& operator << ( wchar_t ch ) {

					return this->print(ch);

				}
				
				/**
				 * Print the specified number to the Current Output Buffer.
				 * 
				 * This operator is functionally equivalent to the `print()` method.
				 * 
				 * @param ch	The Integer or Floating-Point Number to be printed to the Current Output Buffer.
				 * 
				 * @returns		A reference to this object to support operator chaining.
				 */
				template <Numeric NumberT>
				const ThisT& operator << ( NumberT num ) const {

					return this->print(num);

				}
				/**
				 * Print the specified number to the Current Output Buffer.
				 * 
				 * This operator is functionally equivalent to the `print()` method.
				 * 
				 * @param ch	The Integer or Floating-Point Number to be printed to the Current Output Buffer.
				 * 
				 * @returns		A reference to this object to support operator chaining.
				 */
				template <Numeric NumberT>
				ThisT& operator << ( NumberT num ) {

					return this->print(num);

				}

				/**
				 * Print the specified Wide-Character String to the Current Output Buffer.
				 * 
				 * This operator is functionally equivalent to the `print()` method.
				 * 
				 * @param str	The Wide-Character String to be printed to the Current Output Buffer.
				 * 
				 * @returns		A reference to this object to support operator chaining.
				 */
				const ThisT& operator << ( const std::wstring& str ) const {

					return this->print(str);

				}
				/**
				 * Print the specified Wide-Character String to the Current Output Buffer.
				 * 
				 * This operator is functionally equivalent to the `print()` method.
				 * 
				 * @param str	The Wide-Character String to be printed to the Current Output Buffer.
				 * 
				 * @returns		A reference to this object to support operator chaining.
				 */
				ThisT& operator << ( const std::wstring& str ) {

					return this->print(str);

				}

				/**
				 * Print the specified Null-Terminated Wide-Character String to the Current Output Buffer.
				 * 
				 * This operator is functionally equivalent to the `print()` method.
				 * 
				 * @param str	The Null-Terminated Wide-Character String to be printed to the Current Output Buffer.
				 * 
				 * @returns		A reference to this object to support operator chaining.
				 */
				const ThisT& operator << ( const wchar_t* str ) const {

					return this->print(str);

				}
				/**
				 * Print the specified Null-Terminated Wide-Character String to the Current Output Buffer.
				 * 
				 * This operator is functionally equivalent to the `print()` method.
				 * 
				 * @param str	The Null-Terminated Wide-Character String to be printed to the Current Output Buffer.
				 * 
				 * @returns		A reference to this object to support operator chaining.
				 */
				ThisT& operator << ( const wchar_t* str ) {

					return this->print(str);

				}


			/* Miscellaneous Helper Methods */
			public:
				/** 
				 * Get a Virtual Terminal Sequence used to query or modify one or more aspects of the Console.
				 * 
				 * @param sequence			The main portion of the Virtual Terminal Sequence
				 * 							that follows the Escape Sequence.
				 * 
				 * @returns					A new Wide-Character String containing the Virtual Terminal Sequence.
				 */
				static std::wstring getVirtualTerminalSequence ( const std::wstring& sequence ) {

					return (VIRTUAL_TERMINAL_SEQUENCE_ESCAPE + sequence);

				}

		};

		/**
		 * An Internal, Abstract, Template Class providing an interface
		 * for retrieving input from the Windows Console.
		 * 
		 * @tparam ThisT	The inheriting class. Necessary for Compile-Time Polymorphism.
		 * 
		 * @internal		This class and all of its associated functionality are for
		 * 					internal use only and are subject to change at any time.
		 */
		template <class ThisT> class AConsoleInput {

			/* Type Definitions */
			public:
				typedef INPUT_RECORD WinConsoleInput;			// A Console Input Record returned by the Windows API.
				typedef KEY_EVENT_RECORD WinConsoleInputKey;	// A Console Input Key Record returned by the Windows API.
				typedef DWORD wait_time_t;						// An integer type representing the amount of time to wait for input.


			/* Class Constants */
			public:
				// Indicates that the console should wait indefinitely for the
				// user to interact with the console or provide input data.
				static constexpr DWORD INFINITE_WAIT_TIME = INFINITE;
				// The default maximum amount of time to wait for input.
				static constexpr DWORD DEFAULT_MAX_INPUT_WAIT_TIME = INFINITE_WAIT_TIME;


			/* Instance Methods */
			public:
				/** 
				 * Wait for the user to interact with and provide input to the console.
				 * 
				 * This method will block until the user interacts with the console and
				 * triggers a valid Console Input Event to be emitted.
				 * 
				 * @param flushBuffer	Indicates whether the underlying Input Buffers should be flushed
				 *						prior to attempting to retrieve any input from the console.
				 * 
				 * @param maxWaitTime	The maximum amount of time to wait for input in milliseconds.
				 * 
				 * 						Specifying `INFINITE_WAIT_TIME` will cause this method to block and
				 * 						wait indefinitely for the user to interact with the console.
				 * 
				 * @returns				On success, returns a `WinConsoleInputKey` structure representing the
				 * 						input provided to the console by the user, wrapped in an `std::optional` object.
				 * 
				 * 						If the `maxWaitTime` is reached before the user interacts with
				 * 						or provides valid input to the console, an empty `std::optional`
				 * 						object will be returned.
				 */
				virtual std::optional<WinConsoleInputKey> waitForInput (
					bool flushBuffer = false,
					DWORD maxWaitTime = DEFAULT_MAX_INPUT_WAIT_TIME
				) const = 0;

				/** 
				 * Wait for the user to provide an Input Character to the Console.
				 * 
				 * This method will block until the user interacts with the console and
				 * triggers a valid Console Input Key Event to be emitted.
				 * 
				 * @param flushBuffer	Indicates whether the underlying Input Buffers should be flushed
				 *						prior to attempting to retrieve any input from the console.
				 * 
				 * @param maxWaitTime	The maximum amount of time to wait for input in milliseconds.
				 * 
				 * 						Specifying `INFINITE_WAIT_TIME` will cause this method to block and
				 * 						wait indefinitely for the user to interact with the console.
				 * 
				 * 						Note that this time is "reset" each time the user triggers an
				 * 						invalid Console Input Event to be emitted, resulting in a
				 * 						potentially-longer wait time than specified if the user
				 * 						continues to interact with the console in ways that
				 * 						do not generate an equivalent Wide Character of Input Data.
				 * 
				 * @returns				On success, returns the Wide Character specified by the user,
				 * 						wrapped in an `std::optional` object.
				 * 
				 * 						If the `maxWaitTime` is reached before the user interacts with
				 * 						or provides valid input to the console, or if the user uses
				 * 						the `ESC` key to exit the prompt, an empty `std::optional`
				 * 						object will be returned.
				 */
				std::optional<wchar_t> waitForInputChar (
					bool flushBuffer = false,
					DWORD maxWaitTime = DEFAULT_MAX_INPUT_WAIT_TIME
				) {

					std::optional<WinConsoleInputKey> input = {};
					std::optional<wchar_t> inputChar = {};

					do {
						input = this->waitForInput(flushBuffer, maxWaitTime);

						if ( input && input->uChar.UnicodeChar != L'\0' )
							inputChar = input->uChar.UnicodeChar;
						if (flushBuffer)
							flushBuffer = false;
					} while ( !inputChar && input && input->wVirtualKeyCode != VK_ESCAPE );

					return inputChar;

				}

				/**
				 * Wait for the user to provide input data to the console.
				 * 
				 * This method will block until the user presses the `Enter` key after optionally
				 * providing input data to the console in the form of one or more printable characters.
				 * 
				 * @param maxInputLength	The maximum number of input characters to accept.
				 * 
				 * 							The returned Wide-Character String will be no more
				 * 							than `maxInputLength` characters in length, though the actual
				 * 							length is dependent on the amount of input data provided by the user.
				 * 
				 * @returns					A Wide-Character String containing the input data provided
				 *							by the user and retrieved from the Console, wrapped in an `std::optional` object.
				 * 
				 *							If the user immediately presses the `Enter` key, an empty string
				 *							wrapped in an `std::optional` object will be returned.
				 * 
				 *							If the user presses the `ESC` key to exit the prompt, 
				 *							an empty `std::optional` object will be returned.
				 */
				std::optional<std::wstring> waitForInputData ( size_t maxInputLength = 512UL ) const {

					std::wstring inputData = {};
					std::optional<size_t> charsWritten = this->waitForInputData(inputData, maxInputLength);

					if ( !charsWritten || *charsWritten == 0 )
						return std::optional<std::wstring>();

					return std::optional<std::wstring>(inputData);

				}
				/**
				 * Wait for the user to provide input data to the console.
				 * 
				 * This method will block until the user presses the `Enter` key after optionally
				 * providing input data to the console in the form of one or more printable characters.
				 * 
				 * @param oStrBuf			The Wide-Character String where the user input data is to be written.
				 * 
				 * @param maxInputLength	The maximum number of input characters to accept.
				 * 
				 * 							The designated `oStrBuf` will be no more than `maxInputLength` characters in length,
				 * 							though its actual length will be dependent on the amount of input data provided by the user.
				 * 
				 * @returns					An unsigned integer representing the number of characters entered
				 *							by the user and retrieved from the Console, or the new length of
				 *							the specified `oStrBuf` Wide-Character String, wrapped in an `std::optional` object.
				 * 
				 *							If the user immediately presses the `Enter` key, a value of `0`
				 *							wrapped in an `std::optional` object will be returned.
				 * 
				 *							If the user presses the `ESC` key to exit the prompt, 
				 *							an empty `std::optional` object will be returned.
				 */
				std::optional<size_t> waitForInputData ( _Out_ std::wstring& oStrBuf, size_t maxInputLength = 512UL ) const {

					if ( oStrBuf.capacity() < maxInputLength )
						oStrBuf.resize(maxInputLength);
					
					std::optional<size_t> charsWritten = this->waitForInputData(oStrBuf.data(), maxInputLength);

					if ( charsWritten && *charsWritten > 0ULL )
						oStrBuf.resize(*charsWritten);

					return charsWritten;

				}
				/**
				 * Wait for the user to provide input data to the console.
				 * 
				 * This method will block until the user presses the `Enter` key after optionally
				 * providing input data to the console in the form of one or more printable characters.
				 * 
				 * @tparam SizeT			The length of the designated `oStrBuf` Wide-Character String Buffer.
				 * 
				 * @param oStrBuf			The Wide-Character String Buffer where the user input data is to be written.
				 * 
				 * @param maxInputLength	The maximum number of input characters to accept.
				 * 
				 * 								- If the `maxInputLength` is less than or equal to the length
				 * 								  of the designated Wide-Character String Buffer, no more than
				 * 								  `maxInputLength` characters will be written to the `oStrBuf`.
				 * 
				 * 								- If the `maxInputLength` is greater than than the length of the
				 * 								  designated Wide-Character String Buffer, no more than
				 * 								  `SizeT - 1` characters will be written to the `oStrBuf`.
				 * 
				 * 							Note that the actual number of characters written to the designated
				 * 							Wide-Character String Buffer will be dependent on the amount of
				 * 							input data provided by the user.
				 * 
				 * @returns					An unsigned integer representing the number of characters entered
				 *							by the user and retrieved from the Console, or the number of characters
				 *							written to the designated `oStrBuf` Wide-Character String Buffer,
				 *							wrapped in an `std::optional` object.
				 * 
				 *							If the user immediately presses the `Enter` key, a value of `0`
				 *							wrapped in an `std::optional` object will be returned.
				 * 
				 *							If the user presses the `ESC` key to exit the prompt, 
				 *							an empty `std::optional` object will be returned.
				 */
				template <size_t SizeT>
				std::optional<size_t> waitForInputData ( _Out_ wchar_t (&oStrBuf)[SizeT], size_t maxInputLength = 512UL ) const {

					return this->waitForInputData(
						oStrBuf,
						(SizeT >= maxInputLength ? maxInputLength : SizeT)
					);

				}
				/**
				 * Wait for the user to provide input data to the console.
				 * 
				 * This method will block until the user presses the `Enter` key after optionally
				 * providing input data to the console in the form of one or more printable characters.
				 * 
				 * @param oStrBuf			The Wide-Character String Buffer where the user input data is to be written.
				 * 
				 * @param maxInputLength	The maximum number of input characters to accept.
				 * 
				 * 							No more than `maxInputLength` characters will be written to the
				 * 							designated Wide-Character String Buffer, though the actual number
				 * 							of characters written will be dependent on the amount of input data
				 * 							provided by the user.
				 * 
				 * @returns					An unsigned integer representing the number of characters entered
				 *							by the user and retrieved from the Console, or the number of characters
				 *							written to the designated `oStrBuf` Wide-Character String Buffer,
				 *							wrapped in an `std::optional` object.
				 * 
				 *							If the user immediately presses the `Enter` key, a value of `0`
				 *							wrapped in an `std::optional` object will be returned.
				 * 
				 *							If the user presses the `ESC` key to exit the prompt, 
				 *							an empty `std::optional` object will be returned.
				 */
				virtual std::optional<size_t> waitForInputData ( _Out_ wchar_t* oStrBuf, size_t maxInputLength ) const = 0;


			/* Overloaded Operators */
			public:
				/**
				 * Wait for the user to provide a Input Character to the Console.
				 * 
				 * This operator is simply an alias for the `waitForInputChar()` method
				 * but lacks the ability to specify whether or not the underlying Input Buffers
				 * should be flushed before attempting to collect any input.
				 * 
				 * In addition, this operator will always wait indefinitely for
				 * the user to provide a valid Input Character and does *not*
				 * permit for the `ESC` key to be used to exit the prompt.
				 * 
				 * @param oStrbuf	A reference to the Wide-Character where the
				 * 					Input Character retrieved from the Console is to be stored.
				 * 
				 * @returns			A reference to this object to support operator chaining.
				 */
				const ThisT& operator >> ( _Out_ wchar_t& oChar ) const {

					std::optional<wchar_t> inputChar = {};

					while (!inputChar)
						inputChar = this->waitForInputChar();

					oChar = *inputChar;
					return *this;

				}
				/**
				 * Wait for the user to provide Input Data to the Console.
				 * 
				 * This operator is simply an alias for the `waitForInputData()` method
				 * but lacks the ability to specify the maximum length of the retrieved input data.
				 * 
				 * @param oStrbuf	A reference to the Wide-Character String where the retrieved
				 *					input data is to be written.
				 * 
				 * @returns			A reference to this object to support operator chaining.
				 */
				ThisT& operator >> ( _Out_ wchar_t& oChar ) {

					((const ThisT*) this) >> oChar;
					return *this;

				}
				/**
				 * Wait for the user to provide Input Data to the Console.
				 * 
				 * This operator is simply an alias for the `waitForInputData()` method
				 * but lacks the ability to specify the maximum length of the retrieved input data.
				 * 
				 * @param oStrbuf	A reference to the Wide-Character String where the retrieved
				 *					input data is to be written.
				 * 
				 * @returns			A reference to this object to support operator chaining.
				 */
				const ThisT& operator >> ( _Out_ std::wstring& oStrBuf ) const {

					this->waitForInputData(oStrBuf);
					return *this;

				}
				/**
				 * Wait for the user to provide Input Data to the Console.
				 * 
				 * This operator is simply an alias for the `waitForInputData()` method
				 * but lacks the ability to specify the maximum length of the retrieved input data.
				 * 
				 * @param oStrbuf	A reference to the Wide-Character String where the retrieved
				 *					input data is to be written.
				 * 
				 * @returns			A reference to this object to support operator chaining.
				 */
				ThisT& operator >> ( _Out_ std::wstring& oStrBuf ) {

					return this->waitForInputData(oStrBuf);

				}
				/**
				 * Wait for the user to provide Input Data to the Console.
				 * 
				 * This operator is simply an alias for the `waitForInputData()` method
				 * but lacks the ability to specify the maximum length of the retrieved input data.
				 * 
				 * @tparam SizeT	The size of the Wide-Character String Buffer being written to.
				 * 
				 * @param oStrbuf	The Wide-Character String Buffer
				 * 					where the retrieved input data is to be written.
				 * 
				 * 					The size of the buffer must be known to use this overload.
				 * 
				 * @returns			A reference to this object to support operator chaining.
				 */
				template <size_t SizeT>
				const ThisT& operator >> ( _Out_ wchar_t (&oStrBuf)[SizeT] ) const {

					return this->waitForInputData(oStrBuf);

				}
				/**
				 * Wait for the user to provide Input Data to the Console.
				 * 
				 * This operator is simply an alias for the `waitForInputData()` method
				 * but lacks the ability to specify the maximum length of the retrieved input data.
				 * 
				 * @tparam SizeT	The size of the Wide-Character String Buffer being written to.
				 * 
				 * @param oStrbuf	The Wide-Character String Buffer
				 * 					where the retrieved input data is to be written.
				 * 
				 * 					The size of the buffer must be known to use this overload.
				 * 
				 * @returns			A reference to this object to support operator chaining.
				 */
				template <size_t SizeT>
				ThisT& operator >> ( _Out_ wchar_t (&oStrBuf)[SizeT] ) {

					return this->waitForInputData(oStrBuf);

				}

		};

	}


	/**
	 * A Class providing a rich interface for interacting with the Windows Console
	 * via reading input and writing formatted output, including through Interactive Menus.
	 * 
	 * The `Console` Class extends the `AConsoleInput` and `AConsoleOutput` classes, providing
	 * easy access to the Input and Output Operations that can be performed on the
	 * Console Input and Output Buffers directly from the `Console` class.
	 * In other words, `console.println(L"Hello, World!");` has the exact
	 * same effect as `console.out().println(L"Hello, World!");`.
	 */
	class Console :
		public UTILS_NAMESPACE::AConsoleInput<Console>,
		public UTILS_NAMESPACE::AConsoleOutput<Console>
	{

		/* Type Definitions */
		public:
			/**
			 * A type representing a Smart Pointer to a `Console` object.
			 */
			typedef std::shared_ptr<Console> console_ptr_t;


		/* Inner Classes & Structure Types */
		public:
			/**
			 * An Inner Class providing an interface for writing formatted output to the Windows Console.
			 * 
			 * The `OutputBuffer` extends the `AConsoleBuffer` and `AConsoleOutput` Classes,
			 * providing a concrete implementation for the Output Operations of the `Console`.
			 */
			class OutputBuffer : 
				public UTILS_NAMESPACE::AConsoleBuffer,
				public UTILS_NAMESPACE::AConsoleOutput<OutputBuffer>
			{

				/* Friends */
				public:
					// Provides internal property access to the Console.
					friend Console;


				/* Type Definitions */
				protected:
					// A Stack containing the handles to each of the underlying Console Output Buffers.
					typedef std::stack<win_conbuf_t> ConsoleBufferStack;
					// A Stack containing the printed contents of each of the underlying 
					// Console Output Buffers as Wide-Character Strings.
					typedef std::stack<std::wstring> ConsoleBufferContents;

					/**
					 * A Stack containing `WinConsoleCursorCoordinates` structures representing the
					 * previously-saved positions of the Console Cursor.
					 * 
					 * Rather than using an `std::stack`, an `std::vector` is used to allow
					 * for any Saved Cursors to be updated if and when the console is scrolled.
					 */
					typedef std::vector<WinConsoleCursorCoordinates> CursorPositionStack;


				/* Inner Structure Types */
				protected:
					/**
					 * A structure type containing all of the data and information
					 * associated with an Output Buffer of the underlying Windows Console.
					 * 
					 * @internal	This structure type and all of its associated functionality are for
			 		 * 				internal use only and are subject to change at any time.
					 */
					typedef struct BufferDataStruct {

						/**
						 * The handle to an Input or Output Buffer for the underlying Windows Console.
						 * 
						 * Generally only used when `programSettings.useCustomBufferBehavior` is `false`.
						 */
						win_conbuf_t handle = NULL;

						/**
						 * The contents of the Buffer Data Output Buffer, which is used
						 * to re-`print()` the contents when switching between Output Buffers.
						 * 
						 * Generally only used when `programSettings.useCustomBufferBehavior` is `true`.
						 */
						std::vector<std::wstring> contents = {};
						/**
						 * A Three-Dimensional Dynamic Array containing the index of the character
						 * in a Wide-Character String of the `contents` array corresponding to the
						 * Console Cursor Position at each point.
						 * 
						 * In other words, a Console Cursor Position of `(2, 1)` corresponds to
						 * the Wide Character located at the index specified by the element located
						 * at Index `2` of the Dynamic Array at Index `1` of `contentsCursorData`.
						 * 
						 * This mechanism makes it possible to "jump" over Virtual Terminal Sequences
						 * and match the Cursor Position returned by the Windows API to the proper
						 * character in the `contents` buffer.
						 */
						std::vector< std::vector<size_t> > contentsCursorData = {};

						// The Saved Cursor Position Stack
						CursorPositionStack savedCursors = {};
						// Output Buffer Cursor Starting Position
						WinConsoleCursorCoordinates cursorStartPos = {
							.X = 0,
							.Y = 0
						};
						// The number of lines the Current Output Buffer has been scrolled.
						short cursorScrollOffset = 0U;
						// Indicates whether or not the Console Cursor is currently visible.
						bool cursorIsVisible = true;

					} BufferData;


				/* Instance Properties */
				private:
					mutable BufferData mainBufferData;				// Buffer Data for the Main Console Output Buffer.
					mutable std::stack<BufferData> altBufferData;	// Alternate Output Buffer Data Stack


				/* Class Constructors & Destructors */
				public:
					/**
					 * Construct a new `OutputBuffer`.
					 * 
					 * @param iBufferHandle The handle to the Output Buffer for the underlying Windows Console.
					 */
					OutputBuffer ( win_conbuf_t iBufferHandle );
					
					/**
					 * Destruct the `OutputBuffer`.
					 * 
					 * All of the Output Buffers for the underlying Windows Console
					 * will be flushed, all Alternate Output Buffers will be cleared,
					 * and the Primary Output Buffer will be restored.
					 */
					~OutputBuffer ();


				/* Inherited Instance Methods */
				public:
					// Ensure all overloads of print() are accessible.
					using AConsoleOutput::print;


				/* Implemented Instance Methods */
				public:
					// Output Buffer Management
					
					/**
					 * Create and Switch to an Alternate Output Buffer.
					 * 
					 * @returns 	On success, returns an `std::optional` object containing a `buffer_number_t`
					 *				corresponding to the newly-created Output Buffer (i.e., the same `buffer_number_t`
					 *				that would be returned by immediately calling `getCurrentBufferNum()`
					 *				after this method returns).
					 * 	
					 *				On failure, returns an empty `std::optional` object.
					 */
					virtual std::optional<buffer_number_t> createAltBuffer () override;
					/**
					 * Get a number corresponding to the Output Buffer currently in-use by the `Console`.
					 * 
					 * @returns 	A `buffer_number_t` corresponding to the Output Buffer currently being used by the `Console`.
					 * 	
					 *					- A `buffer_number_t` equal to zero indicates that the Primary Buffer
					 *					  is currently being used and no Alternative Buffers are currently in-use.
					 * 	
					 *					- A `buffer_number_t` that is greater than zero indicates that an Alternative Buffer
					 *					  is currently being used.
					 */
					virtual buffer_number_t getCurrentBufferNum () const override;
					/**
					 * Switch to and Restore the Previous Output Buffer.
					 * 
					 * If the Current Output Buffer is an Alternate Output Buffer created
					 * using the `createAltBuffer()` method, it will be closed and it will no longer
					 * be possible to write to or read from the buffer.
					 * 
					 * If there are currently no active Alternative Output Buffers and the
					 * only Output Buffer in-use is the Primary Buffer, this method has no effect.
					 * 
					 * @returns 	A `buffer_number_t` corresponding to the Output Buffer currently being used by the `Console`.
					 * 	
					 *				The returned `buffer_number_t` may or may not be the same as that returned by a call to
					 *				the `getCurrentBufferNum()` method immediately prior to invoking this method, depending on
					 *				whether or not there are any Alternate Output Buffers currently in-use.
					 */
					virtual buffer_number_t restorePreviousBuffer () override;


					// Cursor Position Management

					/**
			 		 * Get the Current Position of the Console Cursor.
			 		 * 
			 		 * @returns 	A `WinConsoleCursorCoordinates` structure containing the Cartesian Coordinates
			 		 *				corresponding to the Current Position of the Console Cursor, relative
			 		 *				to the visible viewport of the Console.
			 		 */
					virtual WinConsoleCursorCoordinates getCursorPos () const override;	
					/**
			 		 * Get the number of lines that the Output Buffer has scrolled since it was initialized.
			 		 * 
			 		 * This value is typically used to offset Console Cursor Positions to account
			 		 * for the console scrolling, particularly when repeatedly printing to the bottom
			 		 * row of the visible viewport.
			 		 * 
			 		 * @returns 	A constant reference to an integer value representing the Current Cursor Scroll Offset.
			 		 * 	
			 		 *				This value will be equal to zero when the Output Buffer is first initialized and will increase
			 		 *				any time the visible viewport of the Console is scrolled up. As such, this value
			 		 *				can increase between calls to `getCursorScrollOffset()`, but it will never decrease.
			 		 */
					virtual const short& getCursorScrollOffset () const override;

					/**
					 * Set the Position of the Console Cursor.
					 * 
					 * @param cursorPos A `WinConsoleCursorCoordinates` structure containing the Cartesian Coordinates
					 *					corresponding to the new Position of the Console Cursor, relative
					 *					to the visible viewport of the Console.
					 * 
					 * @returns			`true` on success and `false` on failure.
					 */
					virtual bool setCursorPos ( const WinConsoleCursorCoordinates& cursorPos ) override;
					/**
					 * Save the Current Position of the Console Cursor to be retrieved by
					 * a future call to `restoreSavedCursorPos()`.
					 * 
					 * @returns			`true` on success and `false` on failure.
					 */
					virtual bool saveCursorPos () override;
					/**
			 		 * Save the Specified Position of the Console Cursor to be retrieved by
			 		 * a future call to `restoreSavedCursorPos()`.
			 		 * 
			 		 * @param cursorPos A `WinConsoleCursorCoordinates` structure containing the Cartesian Coordinates
			 		 *					corresponding to the desired Position of the Console Cursor to be saved,
			 		 *					relative to the visible viewport of the Console.
			 		 * 
			 		 * @returns			`true` on success and `false` on failure.
			 		 */
					virtual bool saveCursorPos ( const WinConsoleCursorCoordinates& cursorPos ) override;
					/**
					 * Retrieve the Position of the Console Cursor that was stored by a previous call to `saveCursorPos()`.
					 * 
					 * @returns		On success, returns an `std::optional` object containing a `WinConsoleCursorCoordinates` structure
					 *				containing the Cartesian Coordinates of the saved Position of the Console Cursor,
					 *				relative to the visible viewport of the Console.
					 * 
					 *				If no cursor was previously saved by calling `saveCursorPos()`, an empty `std::optional` object is returned.
					 */
					virtual std::optional<WinConsoleCursorCoordinates> restoreSavedCursorPos () override;

					/**
					 * Toggle the Visibility of the Console Cursor.
					 * 
					 * @returns		A reference to this object to support method chaining.
					 */
					virtual OutputBuffer& toggleCursorVisibility () override;
					/**
					 * Set the Visibility of the Console Cursor.
					 * 
					 * @param visible	Indicates whether or not the Console Cursor should be made Visible.
					 * 
					 * @returns			A reference to this object to support method chaining.
					 */
					virtual OutputBuffer& toggleCursorVisibility ( bool visible ) override;

					
					// Console Output

					/**
					 * Clear the Current Output Buffer.
					 * 
					 * @param clearBuffer		Indicates whether the underlying string buffers for the 
					 *							Current Output Buffer should be cleared or not.
					 *
					 * @param resetCursorPos	Indicates if the position of the Console Cursor should be
				 	 * 							reset prior to clearing the screen, which is generally
				 	 * 							required to be able to clear the entire screen.
					 * 
					 * @returns					A reference to this object to support method chaining.
					 */
					virtual OutputBuffer& clear ( bool clearBuffer = false, bool resetCursorPos = true ) override;

					/**
					 * Print the specified Null-Terminated Wide-Character String to the Current Output Buffer.
					 * 
					 * @param str					The Null-Terminated Wide-Character String to be
					 * 								printed to the Current Output Buffer.
					 * 
					 * @param addToBuffer			Indicates if the specified Wide-Character String should be added to
					 * 								the underlying buffer for the Current Output Buffer or not.
					 * 
					 * @returns						A reference to this object to support method chaining.
					 */
					virtual const OutputBuffer& print ( const wchar_t* str, bool addToBuffer = true ) const override;
					/**
					 * Print the specified Null-Terminated Wide-Character String to the Current Output Buffer.
					 * 
					 * @param str					The Null-Terminated Wide-Character String to be printed to the Current Output Buffer.
					 * 
					 * @param addToBuffer			Indicates if the specified Wide-Character String should be added to
					 * 								the underlying buffer for the Current Output Buffer or not.
					 * 
					 * @returns						A reference to this object to support method chaining.
					 */
					virtual OutputBuffer& print ( const wchar_t* str, bool addToBuffer = true ) override;


				/* Overridden Instance Methods */
				public:
					/** 
					 * Get the handle to the Output Buffer for the underlying Windows Console.
					 * 
					 * @returns 	A constant reference to the handle to the 
					 *				Output Buffer for the underlying Windows Console.
					 */
					virtual const win_conbuf_t& getBufferHandle () const override;


				/* Helper Methods */
				protected:
					/**
					 * Get the `BufferData` for the Current Output Buffer.
					 * 
					 * @param currentHandleBuffer	Indicates whether or not the returned `BufferData` should reflect the
					 * 								Output Buffer whose `handle` is currently being used by the `Console`,
					 * 								which is dependent on the current value of `programSettings.useCustomBufferBehavior`.
					 * 
					 *								When `true`, the `BufferData` for the Main Output Buffer will always be returned
					 *								when `programSettings.useCustomBufferBehavior` is `true`.
					 * 
					 *								When `false` or when `programSettings.useCustomBufferBehavior` is `false`,
					 *								the `BufferData` for the Current Alternate Output Buffer or the Main Output Buffer
					 *								will be returned, depending on whether or not any Alternate Output Buffers
					 *								are currently in use.
					 * 
					 * @returns						A reference to this object to support method chaining.
					 */
					Console::OutputBuffer::BufferData& getCurrentBufferData ( bool currentHandleBuffer = false ) const;

					/**
					 * Synchronize the Visibility of the Console Cursor with the Current Output Buffer.
					 * 
					 * This method is primarily invoked when switching between the Main/Shared/Global Output Buffer
					 * and one or more Alternate Output Buffers, ensuring that the current visibility
					 * of the Console Cursor is synchronized between Output Buffers.
					 * 
					 * @param cursorIsVisible	The Current Visibility of the Console Cursor.
					 * 
					 * @param addToBuffer		Indicates if the Virtual Terminal Sequence used to set the
					 * 							Cursor Visibility should be added to the underlying buffer
					 * 							for the Current Output Buffer or not.
					 * 
					 * @returns					A reference to this object to support method chaining.
					 */
					Console::OutputBuffer& synchronizeCursorVisibility ( bool cursorIsVisible, bool addToBuffer = true );

			};

			/**
			 * An Inner Class providing an interface for for retrieving input from the Windows Console.
			 * 
			 * The `InputBuffer` class extends the `AConsoleBuffer` and `AConsoleInput` classes,
			 * providing a concrete implementation for the Input Operations of the `Console`.
			 */
			class InputBuffer : 
				public UTILS_NAMESPACE::AConsoleBuffer,
				public UTILS_NAMESPACE::AConsoleInput<InputBuffer>
			{

				/* Friends */
				public:
					// Provides internal property access to the Console.
					friend Console;


				/* Class Constants */
				protected:
					// The maximum number of Console Input Records to retrieve from 
					// the Windows API and store in an internal buffer for reading.
					static const DWORD MAX_INPUT_EVENT_BUFFER_SIZE;


				/* Class Constructors */
				public:
					/**
					 * Construct a new `InputBuffer`.
					 * 
					 * @param iBufferHandle	The handle to the Input Buffer for the underlying Windows Console.
					 */
					InputBuffer ( win_conbuf_t iBufferHandle );


				/* Inherited Instance Methods */
				public:
					// Ensure all overloads of waitForInputData() are accessible.
					using AConsoleInput::waitForInputData;


				/* Implemented Instance Methods */
				public:
					/** 
					 * Wait for the user to interact with and provide input to the console.
					 * 
					 * This method will block until the user interacts with the console and
					 * triggers a valid Console Input Event to be emitted.
					 * 
					 * @param flushBuffer	Indicates whether the underlying Input Buffers should be flushed
					 *						prior to attempting to retrieve any input from the console.
					 * 
					 * @param maxWaitTime	The maximum amount of time to wait for input in milliseconds.
					 * 
					 * 						Specifying `INFINITE_WAIT_TIME` will cause this method to block and
					 * 						wait indefinitely for the user to interact with the console.
					 * 
					 * @returns				On success, returns a `WinConsoleInputKey` structure representing the
					 * 						input provided to the console by the user, wrapped in an `std::optional` object.
					 * 
					 * 						If the `maxWaitTime` is reached before the user interacts with
					 * 						or provides valid input to the console, an empty `std::optional` object will be returned.
					 */
					virtual std::optional<WinConsoleInputKey> waitForInput (
						bool flushBuffer = false,
						DWORD maxWaitTime = DEFAULT_MAX_INPUT_WAIT_TIME
					) const override;

					/**
					 * Wait for the user to provide input data to the console.
					 * 
					 * This method will block until the user presses the `Enter` key after optionally
					 * providing input data to the console in the form of one or more printable characters.
					 * 
					 * @param oStrBuf			The Wide-Character String Buffer where the user input data is to be written.
					 * 
					 * @param maxInputLength	The maximum number of input characters to accept.
					 * 
					 * 							No more than `maxInputLength` characters will be written to the
					 * 							designated Wide-Character String Buffer, though the actual number
					 * 							of characters written will be dependent on the amount of input data
					 * 							provided by the user.
					 * 
					 * @returns					An unsigned integer representing the number of characters entered
					 *							by the user and retrieved from the Console, or the number of characters
					 *							written to the designated `oStrBuf` Wide-Character String Buffer,
					 *							wrapped in an `std::optional` object.
					 * 
					 *							If the user immediately presses the `Enter` key, a value of `0`
					 *							wrapped in an `std::optional` object will be returned.
					 * 
					 *							If the user presses the `ESC` key to exit the prompt, 
					 *							an empty `std::optional` object will be returned.
					 */
					virtual std::optional<size_t> waitForInputData ( _Out_ wchar_t* oStrBuf, size_t maxInputLength ) const override;

			};

			/**
			 * A structure type representing a Selectable Option in an Interative Console Menu.
			 */
			typedef struct MenuOptionStruct {

				/* Friends */
				public:
					// Provides internal property access to the Console.
					friend Console;


				/* Inner Structure Types */
				public:
					/**
					 * A structure type containing the padding information for
					 * a Selectable Option in an Interactive Console Menu.
					 * 
					 * The value of all fields defaults to `false`.
					 */
					typedef struct MenuOptionPaddingStruct {
						
						bool top = false;		// Indicates if a line of whitespace should be inserted before the Menu Option.
						bool left = false;		// Indicates if multiple spaces of whitespace should be prepended to the Menu Option.
						bool right = false;		// Indicates if multiple spaces of whitespace should be appended to the Menu Option.
						bool bottom = false;	// Indicates if a line of whitespace should be inserted after the Menu Option.

					} MenuOptionPadding;


				/* Structure Fields */
				public:
					std::wstring option;			// A Wide-Character String containing the contents of the Menu Option.
					std::optional<wchar_t> hotkey;	// An optional Wide-Character Hotkey associated with the Menu Option.
					bool disabled;					// Indicates whether or not the Menu Option is currently disabled.
					MenuOptionPadding padding;		// A `MenuOptionPadding` structure containing the padding information for the Menu Option.


				/* Structure Constructors */
				public:
					/**
					 * Construct a new `MenuOptionStruct`.
					 * 
					 * @param iOption		A Wide-Character String containing the contents of the Menu Option.
					 * @param iHotkey		An optional Wide-Character Hotkey associated with the Menu Option.
					 * @param iDisabled		Indicates whether or not the Menu Option is currently disabled.
					 * @param iPadding		A MenuOptionPadding structure containing the padding associated with the Menu Option.
					 */
					MenuOptionStruct (
						const std::wstring& iOption,
						std::optional<wchar_t> iHotkey = {},
						bool iDisabled = false,
						const MenuOptionPadding& iPadding = {}
					);


				/* Structure Methods */
				public:
					/**
					 * Get the total number of lines occupied by this Menu Option.
					 * 
					 * @returns 	A positive integer in the range `1 <= returnValue <= 3`
					 *				depending on the padding associated with the Menu Option.
					 */
					unsigned short getTotalLineCount () const;

			} MenuOption;
			
			/**
			 * A Class representing a Collection of `MenuOption` objects.
			 * 
			 * A MenuOptionList is an extension of an `std::vector` of `MenuOption` objects,
			 * providing several useful fields and methods for working with `MenuOption` objects.
			 */
			class MenuOptionList : public std::vector<MenuOption> {

				/* Friends */
				public:
					// Provides internal property access to the Console.
					friend Console;


				/* Inner Structure Types */
				public:
					/**
					 * A structure type representing one or more actions associated with a `MenuOptionList`.
					 */
					typedef struct MenuOptionListActionStruct {
						
						/* Type Definitions */
						public:
							/**
							 * An `std::pair` of `bool` values representing the result of input
							 * being processed by an `MenuOptionListAction`.
							 * 
							 * The first `bool` value indicates if processing for the current input key
							 * should be halted after the current Action has finished processing it
							 *		- This value is ignored when the second `bool` value is true.
							 * 
							 * The second `bool` value indicates if processing for the current `MenuOptionList`
							 * should be halted after the current Action has finished processing it.
							 */
							typedef std::pair<bool, bool> InputProcessingResult;
							/**
							 * The Function Signature of the Action Callback Function,
							 * which is used to process and handle Console Input.
							 * 
							 * @param key					A `WinConsoleInputKey` structure representing the Console Input being processed.
							 * @param menuOptions			A reference to the `MenuOptionList` being processed.
							 * @param console				A reference to the `Console` being used.
							 * @param currentSelectionNum	The optional Current Selection Number indicating which 
							 *								`MenuOption` is currently selected, if any.
							 * 
							 * @returns						An `InputProcessingResult`.
							 */
							typedef std::function< InputProcessingResult (
								const WinConsoleInputKey&,
								MenuOptionList&,
								Console&,
								std::optional<size_t>&
							) > ActionCallbackFunction;

							/**
							 * A Collection of Wide-Character Strings representing the
							 * Human-Readable Instructions associated with the MenuOptionListAction.
							 * 
							 * Each Wide-Character String corresponds to a separate instruction
							 * and distinct line when printed to the Output Buffer.
							 */
							typedef std::vector<std::wstring> InstructionLineList;
							

						/* Structure Fields */
						public:
							ActionCallbackFunction actionFn;	// The Action Callback Function used to process and handle Console Input.
							InstructionLineList instructions;	// The Human-Readable Instructions associated with the `MenuOptionListAction`.


						/* Structure Constructors */
						public:
							/**
							 * Construct a new `MenuOptionListAction`.
							 * 
							 * @param iActionFn			The Action Callback Function used to process and handle Console Input.
							 * 
							 * @param instructionLine	A Wide-Character String representing a single Human-Readable Instruction
							 *							to be associated with the `MenuOptionListAction`.
							 */
							MenuOptionListActionStruct ( ActionCallbackFunction iActionFn, const std::wstring& instructionLine );
							/**
							 * Construct a new `MenuOptionListAction`.
							 * 
							 * @param iActionFn			The Action Callback Function used to process and handle Console Input.
							 * 
							 * @param iInstructions		A Collection of Wide-Character Strings representing the
							 *							Human-Readable Instructions associated with the `MenuOptionListAction`.
							 */
							MenuOptionListActionStruct ( ActionCallbackFunction iActionFn, const InstructionLineList& iInstructions );

					} MenuOptionListAction;


				/* Class Constants */

				public:
					/**
					* The Default `MenuOptionListAction` responsible for handling all basic navigation actions,
					* including Arrow-Key Selection and `MenuOption` Hotkeys.
					* 
					* @see DEFAULT_ESCAPE_ACTION
					* @see DEFAULT_ACTIONS
					*/
					static const MenuOptionListAction DEFAULT_NAVIGATION_ACTIONS;
					/**
					 * The Default `MenuOptionListAction` responsible for handling basic escape actions.
					 * which simply stops further processing of the `MenuOptionList` when the `ESC` key is pressed.
					 * 
					 * @see DEFAULT_NAVIGATION_ACTIONS
					 * @see DEFAULT_ACTIONS
					 */
					static const MenuOptionListAction DEFAULT_ESCAPE_ACTION;
					/**
					 * The Default Collection of `MenuOptionListAction` objects responsible for handling all basic navigation
					 * and escape actions, including Arrow-Key Selection, `MenuOption` Hotkeys, and halting further processing
					 * of the `MenuOptionList` when the `ESC` key is pressed.
					 * 
					 * @see DEFAULT_NAVIGATION_ACTION
					 * @see DEFAULT_ESCAPE_ACTION
					 */
					static const std::vector<struct MenuOptionListActionStruct> DEFAULT_ACTIONS;

				protected:
					/**
					 * The maximum amount of time in seconds that Status Messages associated
					 * with a `MenuOptionList` should be displayed in the console before expiring.
					 */
					static const size_t STATUS_MESSAGE_LIFETIME = 5ULL;


				/* Instance Properties */

				private:
					std::vector<MenuOptionListAction> actions;	// The Collection of `MenuOptionListAction` objects associated with this `MenuOptionlist`.
					
					std::wstring prefix;						// The Wide-Character String prepended to all `MenuOption` objects.
					std::wstring suffix;						// The Wide-Character String appended to all `MenuOption` objects.
					std::wstring separator;						// The Wide-Character String used to separate distinguish distinct sections of output.
					unsigned short width;						// The Minimum Width of all `MenuOption`s in the `MenuOptionList`.
					unsigned short maxMenuOptionLines;			// The maximum number of lines to use to render the `MenuOption`'s in the `MenuOptionList`.

					/**
					 * A `WinConsoleCursorCoordinates` object wrapped in an `std::optional`
					 * representing the Starting Position of the Console Cursor
					 * for the `MenuOptionsList`, which corresponds to the location
					 * of the Selection Marker for the first MenuOption in the `MenuOptionsList`.
					 * 
					 * This field will contain an empty `std::optional` until the Cursor Starting Position
					 * has been explicitly set via a call to `setCursorStartPos()`.
					 */
					std::optional<WinConsoleCursorCoordinates> cursorStartPos;
					/**
					 * A valid index for the `MenuOptionList` wrapped in an `std::optional`
					 * representing the Currently-Selected `MenuOption` in the `MenuOptionsList`.
					 * 
					 * This field will contain an empty `std::optional` until the Selected Option Number
					 * has been explicitly set via a call to `setSelectedOptionNum()`.
					 */
					std::optional<size_t> selectedOptionNum = {};
					// A valid index for the `MenuOptionList` representing the first
					// `MenuOption` in the Visible Console Viewport.
					size_t topMenuOptionNum = 0ULL;
					// A valid index for the `MenuOptionList` representing the last
					// `MenuOption` in the Visible Console Viewport.
					size_t bottomMenuOptionNum = 0ULL;
					
					/**
					 * Contains the pending Status Message associated with this `MenuOptionList`, if any.
					 * 
					 * When this field is empty, there is currently no Status Message associated with this
					 * `MenuOptionList` or it has already been printed to the console.
					 * 
					 * @see statusMessageIssueTime
					 */
					std::wstring statusMessage = {};
					/**
					 * A Unix Timestamp indicating when the Status Message
					 * associated with this `MenuOptionList` was first issued.
					 * 
					 * When this value is equal to zero, there is currently no Status Message
					 * associated with this `MenuOptionList`.
					 * 
					 * When this field is populated with a nonzero value, the Status Message has already
					 * been printed to the console and may or may not have expired since it was first issued.
					 */
					mutable std::time_t statusMessageIssueTime = 0;


				/* Class Constructors */
				public:
					/**
					 * Construct a new `MenuOptionList`.
					 * 
					 * @param iActions				The `MenuOptionListAction` objects associated with this `MenuOptionList`.
					 * @param iPrefix				The Wide-Character String prepended to all `MenuOption`s.
					 * @param iSuffix				The Wide-Character String appended to all `MenuOption`s.
					 * @param iSeparator			The Wide-Character String used to separate distinguish distinct sections of output.
					 * @param iWidth				The Minimum Width of all `MenuOption`s.
					 * @param iMaxMenuOptionLines	The maximum number of lines to use to render the `MenuOption`s in the `MenuOptionList`.
					 */
					MenuOptionList (
						const std::vector<MenuOptionListAction>& iActions = DEFAULT_ACTIONS,
						const std::wstring& iPrefix = L"",
						const std::wstring& iSuffix = L"",
						const std::wstring& iSeparator = L"",
						unsigned short iWidth = 0U,
						unsigned short iMaxMenuOptionLines = MAX_MENU_OPTION_LINES
					);
					/**
					 * Construct a new `MenuOptionList`.
					 * 
					 * @param initList				The Initializer List to be passed to the `std::vector` of `MenuOption` objects.
					 * @param iActions				The `MenuOptionListAction` objects associated with this `MenuOptionList`.
					 * @param iPrefix				The Wide-Character String prepended to all `MenuOption` objects.
					 * @param iSuffix				The Wide-Character String appended to all `MenuOption` objects.
					 * @param iSeparator			The Wide-Character String used to separate distinguish distinct sections of output.
					 * @param iWidth				The Minimum Width of all `MenuOption` objects.
					 * @param iMaxMenuOptionLines	The maximum number of lines to use to render the `MenuOption`s in the `MenuOptionList`.
					 */
					MenuOptionList (
						std::initializer_list<MenuOption> initList,
						const std::vector<MenuOptionListAction>& iActions = DEFAULT_ACTIONS,
						const std::wstring& iPrefix = L"",
						const std::wstring& iSuffix = L"",
						const std::wstring& iSeparator = L"",
						unsigned short iWidth = 0U,
						unsigned short iMaxMenuOptionLines = MAX_MENU_OPTION_LINES
					);


				/* Instance Methods */
				// Basic Instance Getter Methods
				public:

					/**
					 * Get the `MenuOptionListActions` associated with the `MenuOptionList`.
					 * 
					 * @returns 	A reference to a Collection of `MenuOptionListAction` structures
					 *				currently associated with this `MenuOptionList`.
					 */
					std::vector<MenuOptionListAction>& getActions ();
					/**
					 * Get the `MenuOptionListActions` associated with the `MenuOptionList`.
					 * 
					 * @returns 	A constant reference to a Collection of `MenuOptionListAction` structures
					 *				currently associated with this `MenuOptionList`.
					 */
					const std::vector<MenuOptionListAction>& getActions () const;

					/**
					 * Get the Prefix String prepended to each `MenuOption` in the `MenuOptionList`.
					 * 
					 * @returns 	A constant reference to a Wide-Character String containing the Prefix String
					 *				to be prepended to each `MenuOption`.
					 * 
					 *				An empty string indicates that no prefix is to be prepended to each of the `MenuOption` objects.
					 */
					const std::wstring& getPrefix () const;
					/**
					 * Get the Suffix String appended to each `MenuOption` in the `MenuOptionList`.
					 * 
					 * @returns 	A constant reference to a Wide-Character String containing the Suffix String.
					 * 
					 *				An empty string indicates that no suffix is to be appended to the each of the `MenuOption` objects.
					 */
					const std::wstring& getSuffix () const;
					/**
					 * Get the Empty-Space String to be inserted between one or
					 * more `MenuOption` objects in the `MenuOptionList`.
					 * 
					 * @returns 	A Wide-Character String containing the Empty-Space String.
					 */
					std::wstring getSpace () const;
					/**
					 * Get the Separator String appended to be inserted before, after, 
					 * and between one or more of the `MenuOption` objects of the `MenuOptionList`.
					 * 
					 * @returns 	A constant reference to a Wide-Character String containing the Separator String.
					 */
					const std::wstring& getSeparator () const;
					/**
					 * Get the Minimum Width of each printed `MenuOption` in the `MenuOptionList`.
					 * 
					 * @returns 	A constant reference to a positive integer containing the Minimum Width
					 *				of the `MenuOption` objects in the `MenuOptionList`.
					 * 
					 *				A return value equal to zero indicates that there is no Minimum Width
					 *				enforced on the `MenuOption` objects in the `MenuOptionList`.
					 */
					const unsigned short& getWidth () const;
					/**
					 * Get the maximum number of lines to use to render the `MenuOption`s in the `MenuOptionList`.
					 * 
					 * @returns 	A constant reference to a positive integer containing the 
					 *				maximum number of lines to use to render the `MenuOption`s in the `MenuOptionList`.
					 */
					const unsigned short& getMaxMenuOptionLines () const;

					/**
					 * Get the Starting Position of the Console Cursor for the `MenuOptionsList`,
					 * which corresponds to the location of the Selection Marker for the 
					 * first MenuOption in the `MenuOptionsList`.
					 * 
					 * @returns 	A `WinConsoleCursorCoordinates` object wrapped in an `std::optional`
					 *				representing the Starting Position of the Console Cursor
					 *				for the `MenuOptionsList`.
					 * 
					 *				If the Cursor Starting Position has not yet been explicitly set via
					 *				a call to `setCursorStartPos()`, returns an empty `std::optional` object.
					 */
					const std::optional<WinConsoleCursorCoordinates>& getCursorStartPos () const;
					/**
					 * Set the Starting Position of the Console Cursor for the `MenuOptionsList`,
					 * which corresponds to the location of the Selection Marker for the 
					 * first `MenuOption` in the `MenuOptionsList`.
					 * 
					 * @param cursorStartPos	A `WinConsoleCursorCoordinates` object wrapped in an `std::optional`
					 *							representing the Starting Position of the Console Cursor
					 *							for the `MenuOptionsList`.
					 * 
					 *							Currently, passing an empty `std::optional` object will clear any currently-set
					 *							Cursor Starting Position and replace it with the empty `std::optional`. However,
					 *							this behavior is discouraged and is subject to change at any time.
					 * 
					 * @returns					A reference to this object to support method chaining.
					 */
					MenuOptionList& setCursorStartPos ( const WinConsoleCursorCoordinates& cursorStartPos );


				// Menu Option Selection Methods
				public:
					/**
					 * Get the Currently-Selected `MenuOption` in the `MenuOptionList`.
					 * 
					 * @returns 	On success, returns an `std::optional` object containing the zero-based index
					 *				of the Currently-Selected `MenuOption` in the `MenuOptionList`.
					 * 	
					 *				If the `MenuOptionList` is empty or none of the added `MenuOption` objects have
					 *				been selected via a previous call to `setSelectedOption()`,
					 *				an empty `std::optional` object will be returned.
					 */
					std::optional<size_t> getSelectedOption () const;
					/**
					 * Get the current `MenuOption` in the `MenuOptionList` located
					 * at the top of the Visible Console Viewport.
					 * 
					 * @returns 	The zero-based index of the `MenuOption` currently at the 
					 *				top of the Visible Console Viewport.
					 * 	
					 *				The returned value generally has no meaning whenever
					 *				the `MenuOptionList` is not being actively used to render
					 *				an Interactive Menu in the Console.
					 */
					const size_t& getTopMenuOptionNum () const;
					/**
					 * Get the current `MenuOption` in the `MenuOptionList` located
					 * at the bottom of the Visible Console Viewport.
					 * 
					 * @returns 	The zero-based index of the `MenuOption` currently at the 
					 *				bottom of the Visible Console Viewport.
					 * 	
					 *				The returned value generally has no meaning whenever
					 *				the `MenuOptionList` is not being actively used to render
					 *				an Interactive Menu in the Console.
					 */
					const size_t& getBottomMenuOptionNum () const;

					/**
					 * Set the Currently-Selected `MenuOption` in the `MenuOptionList`.
					 * 
					 * @param menuOptionNum The zero-based index of the `MenuOption`
					 *						to be selected in the `MenuOptionList`.
					 * 
					 * @returns				A reference to this object to support method chaining.
					 */
					MenuOptionList& setSelectedOption ( size_t menuOptionNum );
					/**
					 * Set the `MenuOption` in the `MenuOptionList` located
					 * at the top of the Visible Console Viewport.
					 * 
					 * In general, there is no value to using this method
					 * when the `MenuOptionList` is not being actively used to render
					 * an Interactive Menu in the Console.
					 * 
					 * @param menuOptionNum The zero-based index of the `MenuOption` located at the 
					 *						top of the Visible Console Viewport.
					 * 
					 * @returns				A reference to this object to support method chaining.
					 */
					MenuOptionList& setTopMenuOptionNum ( size_t menuOptionNum );
					/**
					 * Set the `MenuOption` in the `MenuOptionList` located
					 * at the bottom of the Visible Console Viewport.
					 * 
					 * In general, there is no value to using this method
					 * when the `MenuOptionList` is not being actively used to render
					 * an Interactive Menu in the Console.
					 * 
					 * @param menuOptionNum The zero-based index of the `MenuOption` located at the 
					 *						bottom of the Visible Console Viewport.
					 * 
					 * @returns				A reference to this object to support method chaining.
					 */
					MenuOptionList& setBottomMenuOptionNum ( size_t menuOptionNum );


				// Status Message Management
				public:
					/**
					 * Set a Status Message to be displayed in the User Interface.
					 * 
					 * In general, Status Messages will only be printed to the Console
					 * when using the `Console::waitForSelection()` method as part of its main loop.
					 * Outside of the `waitForSelection()` method, a Status Message set using
					 * this method will simply sit in the `MenuOptionList` until 
					 * it is either replaced by a subsequent call to `setStatusMessage()`,
					 * or by later calling the `Console::waitForSelection()` method.
					 * 
					 * @param message	The Status Message to be displayed in the User Interface.
					 * 
					 * @returns			A reference to this object to support method chaining.
					 */
					MenuOptionList& setStatusMessage ( const std::wstring& message );

				protected:
					/**
					 * Check if the `MenuOptionList` currently has an Active Status Message
					 * being displayed in the User Interface or not.
					 * 
					 * @returns		`true` if a Status Message is currently being displayed
					 * 				in the User Interface.
					 * 
					 * 				Otherwise, returns `false`.
					 */
					bool hasActiveStatusMessage () const;
					/**
					 * Check if the `MenuOptionList` has an Active Status Message
					 * being displayed in the User Interface that has expired or not.
					 * 
					 * Internally, calling this method results in the Active Status Message
					 * being dismissed, meaning that this method will return `true`
					 * only once per Status Message.
					 * 
					 * @returns		`true` if an Expired Status Message is currently being
					 * 				displayed in the User Interface.
					 * 
					 * 				Otherwise, returns `false`.
					 */
					bool hasExpiredStatusMessage () const;
					
					/**
					 * Indicate that the Current Status Message has been displayed
					 * in the User Interface and its Expiration Timer should be started.
					 * 
					 * This method effectively removes the Current Status Message from
					 * the `MenuOptionList` and sets a Unix Timestamp corresponding to
					 * the current time, which is used to automatically expire and clear
					 * the Status Message from the User Interface in the near future.
					 * 
					 * @returns		`true` if the Current Status Message was successfully recorded
					 * 				as issued to the User Interface.
					 * 
					 * 				If no Status Message in the `MenuOptionList` is currently ready
					 * 				to be issued, returns `false`.
					 */
					bool issueStatusMessage ();


				// Miscellaneous Helper Methods
				public:
					/**
					 * Get the Position of the Console Cursor for the Selection Marker of
					 * the specified `MenuOption` in the `MenuOptionsList`.
					 * 
					 * @warning 			Note that this method does *not* attempt to validate that the specified
					 * 						`MenuOption` lies within the Visible Console Viewport, and attempting to retrieve
					 * 						Cursor Positions for `MenuOption` objects outside of the Visible Console Viewport
					 * 						may produce invalid Console Cursor Coordinates.
					 * 
					 * @param menuOptionNum The zero-based index of the `MenuOption` in the `MenuOptionList`
					 *						whose Cursor Position is to be determined.
					 * 
					 * @returns				On success, returns a `WinConsoleCursorCoordinates` structure wrapped 
					 *						in an `std::optional` representing the Position of the Console Cursor
					 *						for the specified `MenuOption` in the `MenuOptionsList`.
					 * 
					 *						If the specified `MenuOption` is invalid or the Cursor Position could not be
					 *						determined for the specified `MenuOption` for any other reason, an 
					 *						empty `std::optional` object will be returned.
					 * 
					 *						The returned value generally has no meaning whenever
					 *						the `MenuOptionList` is not being actively used to render
					 *						an Interactive Menu in the Console.
					 */
					std::optional<WinConsoleCursorCoordinates> getCursorPos ( size_t menuOptionNum ) const;
					/**
					 * Get a Wide-Character String containing the Human-Readable Instructions 
					 * corresponding to the `MenuOptionListAction` objects associated with the `MenuOptionList`.
					 * 
					 * @returns		A Wide-Character String containing the Human-Readable Instructions
					 *				associated with the set `MenuOptionListAction` objects, formatted according
					 *				to the custom properties of the `MenuOptionList`.
					 * 	
					 *				Note that the returned string is *not* terminated by a Newline Character.
					 */
					std::wstring getInstructionString () const;


				/* Internal Helper Methods */
				protected:
					/**
					 * Check if the specified index value corresponds to a valid `MenuOption` in the `MenuOptionList`.
					 * 
					 * @param menuOptionNum The zero-based `MenuOption` index being evaluated.
					 * 
					 * @returns				Returns `true` if the specified index corresponds to a
					 *						valid `MenuOption` in the `MenuOptionList`.
					 *
					 *						Otherwise, returns `false`.
					 */
					bool isValidOption ( size_t menuOptionNum ) const;

			};

		
		/* Static Class Properties */
		private:
			static console_ptr_t instancePtr;	// A Smart Pointer to the shared `Console` instance associated with the Current Thread.


		/* Instance Properties */
		private:
			InputBuffer conInBuf;				// The `InputBuffer` responsible for the Console Input Buffer.
			OutputBuffer conOutBuf;				// The `OutputBuffer` responsible for the Console Output Buffer.
			OutputBuffer conErrBuf;				// The `OutputBuffer` responsible for the Console Error Output Buffer.


		/* Class Constructors & Destructors */
		public:
			/**
			 * Destruct the `Console`.
			 * 
			 * All of the Output Buffers for the underlying Windows Console
			 * will be flushed, all Alternate Output Buffers will be cleared,
			 * and the Primary Output Buffer will be restored.
			 * 
			 * The Title of the Console will automatically be reset to the
			 * value from before the `Console` instance was constructed,
			 * and the `CTRL + C` Handler registered for the `Console` instance
			 * will be unregistered from the Windows API.
			 */
			~Console ();

		protected:
			/**
			 * Construct a new `Console`.
			 * 
			 * The Title of the Console will automatically be set to the `PROGRAM_TITLE`
			 * and a `CTRL + C` Handler for the `Console` instance will be registered to the Windows API.
			 * 
			 * @throws `std::runtime_error`		If a handle to an Input or Output Buffer for the underlying Windows Console
			 *									could not be retrieved through the Windows API.
			 */
			Console ();

			/**
			 * `Console` objects cannot be Copy-Constructed.
			 * 
			 * @see getConsole() to get the shared `Console` instance associated with the Current Thread. 
			 */
			Console ( const Console& ) = delete;
			/**
			 * `Console` objects cannot be Move-Constructed.
			 * 
			 * @see getConsole() to get the shared `Console` instance associated with the Current Thread. 
			 */
			Console ( Console&& ) = delete;


		/* Static Class Methods */
		public:
			/**
			 * Get the shared `Console` instance associated with the Current Thread. 
			 * 
			 * If no such `Console` instance exists, one will first be created.
			 * 
			 * @returns		A Smart Pointer to the shared `Console` instance
			 * 				associated with the Current Thread.
			 * 
			 * 				If a new `Console` instance could not be created due to
			 * 				an error with the underlying Windows API,
			 * 				an Empty Smart Pointer (Null Pointer) will be returned.
			 */
			static console_ptr_t getConsole ();


		/* Instance Methods */
		public:
			/**
			 * Get the Input Buffer used to retrieve input from the console.
			 * 
			 * @returns		A constant reference to the `InputBuffer` responsible for retrieving
			 * 				input from the Input Buffer of the underlying Windows Console.
			 */
			const InputBuffer& in () const;
			/**
			 * Get the Output Buffer used to write formatted output to the console.
			 * 
			 * @returns		A constant reference to the `OutputBuffer` responsible for writing
			 * 				formatted output to the Output Buffer of the underlying Windows Console.
			 */
			const OutputBuffer& out () const;
			/**
			 * Get the Error Buffer used to write formatted errors to the console.
			 * 
			 * @returns		A constant reference to the `OutputBuffer` responsible for writing
			 * 				formatted output to the Error Output Buffer of the underlying Windows Console.
			 */
			const OutputBuffer& err () const;

			/**
			 * Print the specified `MenuOption` to the console, formatted according to the specified arguments.
			 * 
			 * @param menuOption	The `MenuOption` being printed to the console.
			 * 
			 * @param optionNum		The one-based number hotkey to be associated with
			 * 						the `menuOption` if it does not specify its own hotkey.
			 * 
			 * @param width			The minimum width of the printed line.
			 * 
			 * @returns				A reference to this object to support method chaining.
			 */
			Console& printMenuOption ( MenuOption& menuOption, unsigned int optionNum, unsigned short width = 0U );
			/**
			 * Print the specified `MenuOptionList` to the console.
			 * 
			 * @param menuOptions			The `MenuOptionList` being printed to the console.
			 * 								
			 * 								The `MenuOptionList` will be modified to 
			 * 
			 * @param printInstructions		Whether or not to print the instructions block to the console.
			 * 
			 * @returns						A reference to this object to support method chaining.
			 */
			Console& printMenuOptions ( MenuOptionList& menuOptions, bool printInstructions = false );

			/**
			 * Wait for the user to make a selection in the specified `MenuOptionList`,
			 * automatically updating the on-screen menu to allow the user to navigate the menu
			 * using arrow keys or using distinct hotkeys.
			 * 
			 * If at any point before this method returns a Status Message is set in the
			 * specified list of `menuOptions` using the `MenuOptionList::setStatusMessage()` method,
			 * it will be printed to the User Interface below the Interactive Menu and the
			 * specified list of `menuOptions` will be updated accordingly.
			 * 
			 * @warning				This method assumes that the `printMenuOption()` or `printMenuOptions()` methods have been
			 *						called prior to invoking this method in order to print the menu options to the console.
			 *						Failing to do so will result in the Console Output Buffer being corrupted and some
			 *						of the written characters being overwritten.
			 *
			 * @param menuOptions	The `MenuOptionList` the user is selecting a choice from.
			 * 
			 * @param maxWaitTime	The maximum number of seconds to wait for a selection in milliseconds.
			 * 
			 * @returns				The zero-based index of the selected `MenuOption` in the specified `MenuOptionList`,
			 * 						wrapped in an `std::optional` object.
			 * 
			 * 						If the specified `maxWaitTime` was reached or no valid selection was otherwise made,
			 * 						an empty `std::optional` object will be returned.
			 */
			std::optional<size_t> waitForSelection ( MenuOptionList& menuOptions, DWORD maxWaitTime = DEFAULT_MAX_INPUT_WAIT_TIME );


		/* Inherited Instance Methods */
		public:
			using AConsoleInput::waitForInputData;	// Ensure all overloads of waitForInputData() are accessible.
			using AConsoleOutput::print;			// Ensure all overloads of print() are accessible.


		/* Implemented Instance Methods */
		public:
			// Input Methods

			/** 
			 * Wait for the user to interact with and provide input to the console.
			 * 
			 * This method will block until the user interacts with the console and
			 * triggers a valid Console Input Event to be emitted.
			 * 
			 * @param flushBuffer	Indicates whether the underlying Input Buffers should be flushed
			 *						prior to attempting to retrieve any input from the console.
			 * 
			 * @param maxWaitTime	The maximum amount of time to wait for input in milliseconds.
			 * 
			 * 						Specifying `INFINITE_WAIT_TIME` will cause this method to block and
			 * 						wait indefinitely for the user to interact with the console.
			 * 
			 * @returns				On success, returns a `WinConsoleInputKey` structure representing the
			 * 						input provided to the console by the user, wrapped in an `std::optional` object.
			 * 
			 * 						If the `maxWaitTime` is reached before the user interacts with
			 * 						or provides valid input to the console, an empty `std::optional` object will be returned.
			 */
			virtual std::optional<WinConsoleInputKey> waitForInput (
				bool flushBuffer = false,
				DWORD maxWaitTime = DEFAULT_MAX_INPUT_WAIT_TIME
			) const override;

			/**
			 * Wait for the user to provide input data to the console.
			 * 
			 * This method will block until the user presses the `Enter` key after optionally
			 * providing input data to the console in the form of one or more printable characters.
			 * 
			 * @param oStrBuf			The Wide-Character String Buffer where the user input data is to be written.
			 * 
			 * @param maxInputLength	The maximum number of input characters to accept.
			 * 
			 * 							No more than `maxInputLength` characters will be written to the
			 * 							designated Wide-Character String Buffer, though the actual number
			 * 							of characters written will be dependent on the amount of input data
			 * 							provided by the user.
			 * 
			 * @returns					An unsigned integer representing the number of characters entered
			 *							by the user and retrieved from the Console, or the number of characters
			 *							written to the designated `oStrBuf` Wide-Character String Buffer,
			 *							wrapped in an `std::optional` object.
			 * 
			 *							If the user immediately presses the `Enter` key, a value of `0`
			 *							wrapped in an `std::optional` object will be returned.
			 * 
			 *							If the user presses the `ESC` key to exit the prompt, 
			 *							an empty `std::optional` object will be returned.
			 */
			virtual std::optional<size_t> waitForInputData ( _Out_ wchar_t* oStrBuf, size_t maxInputLength ) const override;


			// Output Methods

			/**
			 * Create and Switch to an Alternate Output Buffer.
			 * 
			 * @returns 	On success, returns an `std::optional` object containing a `buffer_number_t`
			 *				corresponding to the newly-created Output Buffer (i.e., the same `buffer_number_t`
			 *				that would be returned by immediately calling `getCurrentBufferNum()`
			 *				after this method returns).
			 * 	
			 *				On failure, returns an empty `std::optional` object.
			 */
			virtual std::optional<buffer_number_t> createAltBuffer () override;
			/**
			 * Get a number corresponding to the Output Buffer currently in-use by the `Console`.
			 * 
			 * @returns 	A `buffer_number_t` corresponding to the Output Buffer currently being used by the `Console`.
			 * 	
			 *					- A `buffer_number_t` equal to zero indicates that the Primary Buffer
			 *					  is currently being used and no Alternative Buffers are currently in-use.
			 * 	
			 *					- A `buffer_number_t` that is greater than zero indicates that an Alternative Buffer
			 *					  is currently being used.
			 */
			virtual buffer_number_t getCurrentBufferNum () const override;
			/**
			 * Switch to and Restore the Previous Output Buffer.
			 * 
			 * If the Current Output Buffer is an Alternate Output Buffer created
			 * using the `createAltBuffer()` method, it will be closed and it will no longer
			 * be possible to write to or read from the buffer.
			 * 
			 * If there are currently no active Alternative Output Buffers and the
			 * only Output Buffer in-use is the Primary Buffer, this method has no effect.
			 * 
			 * @returns 	A `buffer_number_t` corresponding to the Output Buffer currently being used by the `Console`.
			 * 	
			 *				The returned `buffer_number_t` may or may not be the same as that returned by a call to
			 *				the `getCurrentBufferNum()` method immediately prior to invoking this method, depending on
			 *				whether or not there are any Alternate Output Buffers currently in-use.
			 */
			virtual buffer_number_t restorePreviousBuffer () override;

			/**
			 * Get the Current Position of the Console Cursor.
			 * 
			 * @returns 	A `WinConsoleCursorCoordinates` structure containing the Cartesian Coordinates
			 *				corresponding to the Current Position of the Console Cursor, relative
			 *				to the visible viewport of the Console.
			 */
			virtual WinConsoleCursorCoordinates getCursorPos () const override;
			/**
			 * Get the number of lines that the console has scrolled since it was initialized.
			 * 
			 * This value is typically used to offset Console Cursor Positions to account
			 * for the console scrolling, particularly when repeatedly printing to the bottom
			 * row of the visible viewport.
			 * 
			 * @returns 	A constant reference to an integer value representing the Current Cursor Scroll Offset.
			 * 	
			 *				This value will be equal to zero when the console is first initialized and will increase
			 *				any time the visible viewport of the Console is scrolled up. As such, this value
			 *				can increase between calls to `getCursorScrollOffset()`, but it will never decrease.
			 */
			virtual const short& getCursorScrollOffset () const override;
			
			/**
			 * Set the Position of the Console Cursor.
			 * 
			 * @param cursorPos A `WinConsoleCursorCoordinates` structure containing the Cartesian Coordinates
			 *					corresponding to the new Position of the Console Cursor, relative
			 *					to the visible viewport of the Console.
			 * 
			 * @returns			`true` on success and `false` on failure.
			 */
			virtual bool setCursorPos ( const WinConsoleCursorCoordinates& cursorPos ) override;
			/**
			 * Save the Current Position of the Console Cursor to be retrieved by
			 * a future call to `restoreSavedCursorPos()`.
			 * 
			 * @returns		`true` on success and `false` on failure.
			 */
			virtual bool saveCursorPos () override;
			/**
			 * Save the Specified Position of the Console Cursor to be retrieved by
			 * a future call to `restoreSavedCursorPos()`.
			 * 
			 * @param cursorPos A `WinConsoleCursorCoordinates` structure containing the Cartesian Coordinates
			 *					corresponding to the desired Position of the Console Cursor to be saved,
			 *					relative to the visible viewport of the Console.
			 * 
			 * @returns			`true` on success and `false` on failure.
			 */
			virtual bool saveCursorPos ( const WinConsoleCursorCoordinates& cursorPos ) override;
			/**
			 * Retrieve the Position of the Console Cursor that was stored by a previous call to `saveCursorPos()`.
			 * 
			 * @returns		On success, returns an `std::optional` object containing a `WinConsoleCursorCoordinates` structure
			 *				containing the Cartesian Coordinates of the saved Position of the Console Cursor,
			 *				relative to the visible viewport of the Console.
			 * 
			 *				If no cursor was previously saved by calling `saveCursorPos()`, an empty `std::optional` object is returned.
			 */
			virtual std::optional<WinConsoleCursorCoordinates> restoreSavedCursorPos () override;

			/**
			 * Toggle the Visibility of the Console Cursor.
			 * 
			 * @returns		A reference to this object to support method chaining.
			 */
			virtual Console& toggleCursorVisibility () override;
			/**
			 * Set the Visibility of the Console Cursor.
			 * 
			 * @param visible	Indicates whether or not the Console Cursor should be made Visible.
			 * 
			 * @returns			A reference to this object to support method chaining.
			 */
			virtual Console& toggleCursorVisibility ( bool visible ) override;

			/**
			 * Clear the Current Output Buffer.
			 * 
			 * @param clearBuffer	Indicates whether the underlying string buffers for the 
			 *						Current Output Buffer should be cleared or not.
			 * 
			 * @returns				A reference to this object to support method chaining.
			 */
			virtual Console& clear ( bool clearBuffer = false, bool resetCursorPos = true ) override;

			/**
			 * Print the specified Null-Terminated Wide-Character String to the Current Output Buffer.
			 * 
			 * @param str			The Null-Terminated Wide-Character String to be printed to the Current Output Buffer.
			 * 
			 * @param addToBuffer	Indicates if the specified Wide-Character String should be added to
			 * 						the underlying buffer for the Current Output Buffer or not.
			 * 
			 * @returns				A reference to this object to support method chaining.
			 */
			virtual const Console& print ( const wchar_t* str, bool addToBuffer = true ) const override;
			/**
			* Print the specified Null-Terminated Wide-Character String to the Current Output Buffer.
			* 
			* @param str			The Null-Terminated Wide-Character String to be printed to the Current Output Buffer.
			* 
			* @param addToBuffer	Indicates if the specified Wide-Character String should be added to
			* 						the underlying buffer for the Current Output Buffer or not.
			* 
			* @returns				A reference to this object to support method chaining.
			*/
			virtual Console& print ( const wchar_t* str, bool addToBuffer = true ) override;


		/* Overloaded Operators */
		public:
			/**
			 * `Console` objects do not support the Copy Operator.
			 * 
			 * @see getConsole() to get the shared `Console` instance associated with the Current Thread. 
			 */
			Console& operator = ( const Console& ) = delete;
			/**
			 * `Console` objects do not support the Move Operator.
			 * 
			 * @see getConsole() to get the shared `Console` instance associated with the Current Thread. 
			 */
			Console& operator = ( Console&& ) = delete;

	};

}