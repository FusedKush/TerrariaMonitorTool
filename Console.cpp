/*
* Console.cpp
*
* Source File defining the `Console` class and all associated
* Internal and Inner Classes, including:
*    - `AConsoleBuffer`
*    - `AConsoleInput`
*    - `AConoleOutput`
*    - `Console::InputBuffer`
*    - `Console::OutputBuffer`
* 
* The `Console` class is used frequently throughout the entire program to interact with
* the Windows Console via reading input writing formatted output.
*/


#include "Console.h"
#include <algorithm> // min(), max()


namespace PROGRAM_NAMESPACE {

	/* Internal Helper Functions */

	/**
	 * The Handler Function registered with the Windows API
	 * responsible for handling the `CTRL + C` and `CTRL + BREAK` Signals.
	 * 
	 * When registered, receiving either the `CTRL + C` or `CTRL + BREAK`
	 * Signals will cause the program to exit using the Registered Exit Handler. 
	 * 
	 * @param dwCtrlType	An integer representing the type of event being handled.
	 * 
	 * @returns				If `dwCtrlType` is `CTRL_C_EVENT` or `CTRL_BREAK_EVENT`,
	 * 						the handler will cause the program to exit by calling `std::exit()`.
	 * 
	 * 						Otherwise, returns `FALSE`.
	 */
	static BOOL WINAPI consoleCtrlHandler ( _In_ DWORD dwCtrlType ) {

		// Only handle events where the user might still see the Console.
		if ( dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT )
			std::exit(ProgramStatusCode::TERMINATED);

		return FALSE;

	}
	/**
	 * An internal helper function that gets the lowercase variant of the
	 * specified Wide Character, wrapped in an `std::optional` object.
	 * 
	 * @param hotkey	A Wide-Character corresponding to the hotkey being evaluated,
	 * 					wrapped within an `std::optional` object.
	 * 
	 * @returns			The lowercase variant of the Wide Character Hotkey contained within the
	 *					specified `std::optional`, which is itself wrapped in an `std::optional` object.
	 * 
	 *					If the specified `std::optional` object is empty, an empty `std::optional` object
	 *					will be returned as well.
	 */
	static std::optional<wchar_t> getLowercaseHotkey ( const std::optional<wchar_t>& hotkey ) {
	
		if (!hotkey)
			return hotkey;

		return std::optional( std::towlower(*hotkey) );

	}


	/* Utils::AConsoleBuffer */
	namespace UTILS_NAMESPACE {

		// Class Constructors
		
		AConsoleBuffer::AConsoleBuffer ( win_conbuf_t iBufferHandle ) : bufferHandle(iBufferHandle) {}

		// Instance Methods

		const AConsoleBuffer::win_conbuf_t& AConsoleBuffer::getBufferHandle () const {
		
			return this->bufferHandle;
		
		}

	}


	/* Console::OutputBuffer */
	// Class Constructors & Destructors

	Console::OutputBuffer::OutputBuffer ( win_conbuf_t iBufferHandle )
		: AConsoleBuffer(iBufferHandle), mainBufferData({ .handle = iBufferHandle }), altBufferData()
	{
	
		// We set the Cursor Starting Position here to ensure that the
		// Windows Buffer Handle will be properly returned by `getBufferHandle()`.
		this->mainBufferData.cursorStartPos = this->getCursorPos();
	
	}

	Console::OutputBuffer::~OutputBuffer () {
	
		// Clear all Alternate Output Buffers and revert the
		// console to the Original Output Buffer.
		if ( !programSettings.useCustomBufferBehavior )
			while ( !this->altBufferData.empty() )
				this->restorePreviousBuffer();

		// Ensure the Console Cursor is made visible again.
		this->toggleCursorVisibility(true);
	
	}

	// Implemented Output Buffer Management Instance Methods

	std::optional<Console::OutputBuffer::buffer_number_t> Console::OutputBuffer::createAltBuffer () {
	
		// The Alternate Output Buffer Number, wrapped in an `std::optional`.
		std::optional<buffer_number_t> altBuffer = {};
		BufferData altBufferData = {
			.handle = CreateConsoleScreenBuffer(
				GENERIC_READ | GENERIC_WRITE,
				0,
				NULL,
				CONSOLE_TEXTMODE_BUFFER,
				NULL
			)
		};

		if ( altBufferData.handle != NULL && altBufferData.handle != INVALID_HANDLE_VALUE ) {
			if (programSettings.useCustomBufferBehavior) {
				this->clear();

				if ( !this->getCurrentBufferData().cursorIsVisible )
					this->synchronizeCursorVisibility(true, false);
			}
			else {	
				SetConsoleActiveScreenBuffer(altBufferData.handle);
			}

			altBuffer = this->getCurrentBufferNum();
			this->altBufferData.push(altBufferData);
		}

		return altBuffer;
	
	}

	Console::OutputBuffer::buffer_number_t Console::OutputBuffer::getCurrentBufferNum () const {
	
		return (buffer_number_t) this->altBufferData.size();
	
	}

	Console::OutputBuffer::buffer_number_t Console::OutputBuffer::restorePreviousBuffer () {

		if ( !this->altBufferData.empty() ) {
			if (programSettings.useCustomBufferBehavior) {
				bool prevBufferCursorVisibility = this->getCurrentBufferData().cursorIsVisible;

				this->clear(true);
				this->altBufferData.pop();

				auto& bufferData = this->getCurrentBufferData();
				
				for ( size_t i = 0; i < bufferData.contents.size(); i++ ) {
					this->print(bufferData.contents[i], false);

					if ( i < (bufferData.contents.size() - 1ULL) )
						this->println(false);
				}
				
				if ( prevBufferCursorVisibility != bufferData.cursorIsVisible ) {
					this->synchronizeCursorVisibility(bufferData.cursorIsVisible, false);
				}
			}
			else {
				CloseHandle(this->altBufferData.top().handle);
				this->altBufferData.pop();
				SetConsoleActiveScreenBuffer(this->getBufferHandle());
			}
		}

		return this->getCurrentBufferNum();

	}

	// Implemented Cursor Position Management Instance Methods

	Console::OutputBuffer::WinConsoleCursorCoordinates Console::OutputBuffer::getCursorPos () const {
	
		// A structure containing details about the Console Output Buffer returned by the Windows API.
		CONSOLE_SCREEN_BUFFER_INFO currentConsoleBufferInfo = {};

		GetConsoleScreenBufferInfo(this->getBufferHandle(), &currentConsoleBufferInfo);
		return currentConsoleBufferInfo.dwCursorPosition;
	
	}

	const short& Console::OutputBuffer::getCursorScrollOffset () const {

		return this->getCurrentBufferData().cursorScrollOffset;

	}

	bool Console::OutputBuffer::setCursorPos ( const WinConsoleCursorCoordinates& cursorPos ) {
	
		return ( SetConsoleCursorPosition(this->getBufferHandle(), cursorPos) == TRUE );
	
	}

	bool Console::OutputBuffer::saveCursorPos () {

		return this->saveCursorPos( this->getCursorPos() );

	}
	bool Console::OutputBuffer::saveCursorPos ( const WinConsoleCursorCoordinates& cursorPos ) {

		try {
			this->getCurrentBufferData().savedCursors.push_back(cursorPos);
			return true;
		}
		catch (...) {
			return false;
		}
	}

	std::optional<Console::OutputBuffer::WinConsoleCursorCoordinates> Console::OutputBuffer::restoreSavedCursorPos () {
	
		std::optional<WinConsoleCursorCoordinates> restoredCursorPos = {};
		auto& bufferData = this->getCurrentBufferData();
	
		if ( !bufferData.savedCursors.empty() ) {
			restoredCursorPos = bufferData.savedCursors.back();

			bufferData.savedCursors.pop_back();
			this->setCursorPos(*restoredCursorPos);
		}

		return restoredCursorPos;

	}

	Console::OutputBuffer& Console::OutputBuffer::toggleCursorVisibility () {

		return this->toggleCursorVisibility( !this->getCurrentBufferData().cursorIsVisible );

	}
	Console::OutputBuffer& Console::OutputBuffer::toggleCursorVisibility ( bool cursorIsVisible ) {

		if (cursorIsVisible != this->getCurrentBufferData().cursorIsVisible) {
			this->synchronizeCursorVisibility(cursorIsVisible);
			this->getCurrentBufferData().cursorIsVisible = cursorIsVisible;
		}

		return *this;

	}

	// Implemented Console Output Instance Methods

	Console::OutputBuffer& Console::OutputBuffer::clear ( bool clearBuffer, bool resetCursorPos ) {

		if (clearBuffer) {
			this->getCurrentBufferData().contents.clear();
			this->getCurrentBufferData().contentsCursorData.clear();
		}
		if (resetCursorPos) {
			this->setCursorPos(
				programSettings.useCustomBufferBehavior
					? this->getCurrentBufferData(true).cursorStartPos
					: WinConsoleCursorCoordinates()
			);
		}

		this->print(getVirtualTerminalSequence(L"[0J"), false);

		return *this;

	}

	const Console::OutputBuffer& Console::OutputBuffer::print ( const wchar_t* str, bool addToBuffer ) const {
		
		/**
		 * A Regular Expression Pattern used to attempt to automatically
		 * detect and match Virtual Terminal Sequences.
		 */
		std::wregex TERMINAL_SEQUENCE_REGEX = std::wregex(
			std::format(
				L"^{:c}{:s}",
				VIRTUAL_TERMINAL_SEQUENCE_ESCAPE,
				L"(?:[\\w=>]|\\([0B]|(?:\\[(?:!p|(?:\\??\\d{0,4} ?[a-zA-Z])|(?:\\d{1,3};\\d{1,3}[fH])|(?:[34]8;[25];\\d{1,3};\\d{1,3};\\d{1,3})))|(?:\\]4;\\d{1,3};rgb;\\d{1,3};\\d{1,3};\\d{1,3}\x07))"
			)
		);
		
		auto& bufferData = this->getCurrentBufferData();	// The `BufferData` for the Current Output Buffer.
		short newlineCount = 0;								// The number of newlines counted in the specified `str`.
		DWORD strLength = 0UL;								// The counted length of the specified `str`.
		size_t strPos = 0ULL;								// Our current position in the `str`.
		wchar_t currentChar;								// The current Wide Character being processed.
		auto currentCursorPos = this->getCursorPos();		// The Current Console Cursor Position.
		size_t termSeqLength = 0ULL;						// The length of the Current Virtual Terminal Sequence being processed.
		bool isDetectedTerminalSequence = false;			// Indicates if a Detected Virtual Terminal Sequence is currently being processed.
		size_t termSeqEndPos = 0ULL;						// The position of the last character of the Detected Virtual Terminal Sequence.
		std::optional<bool> atEndOfBuffer = {};
		CONSOLE_SCREEN_BUFFER_INFO bufferInfo = {			// Contains information about the Current Output Buffer.
			.dwSize = sizeof(CONSOLE_SCREEN_BUFFER_INFO)
		};

		GetConsoleScreenBufferInfo(bufferData.handle, &bufferInfo);

		// A lambda function used to write the specified `str` to the Current Output Buffer.
		auto writeToConsole = [this, &str, &strLength]() -> BOOL {

			if (strLength == 0)
				return true;

			return WriteConsoleW(
				this->getBufferHandle(),
				str,
				(DWORD) strLength,
				NULL,
				NULL
			);

		};

		// Count newline characters and add the provided `str` to the 
		// `bufferData` for the Current Output Buffer if applicable.
		while ( (currentChar = str[strPos]) != L'\0' ) {
			if ( isDetectedTerminalSequence && strPos > termSeqEndPos )
				isDetectedTerminalSequence = false;

			// Attempt to match the Virtual Terminal Sequence and update the Output Buffer accordingly.
			if (currentChar == VIRTUAL_TERMINAL_SEQUENCE_ESCAPE) {
				std::wcmatch matches = {};

				if ( std::regex_search(&str[strPos], matches, TERMINAL_SEQUENCE_REGEX) ) {
					isDetectedTerminalSequence = true;
					termSeqEndPos = (strPos + (size_t) matches.length() - 1ULL);
				}
			}

			if (addToBuffer) {
				// The Vertical Cursor Position, relative to the Starting Cursor Position.
				short cursorYPos = (currentCursorPos.Y - this->getCurrentBufferData(true).cursorStartPos.Y);

				// Populate the `contents` and `contentsCursorData` arrays if necessary.
				while ( (size_t) cursorYPos >= bufferData.contents.size() ) {
					bufferData.contents.emplace_back();
					bufferData.contentsCursorData.emplace_back();
				}
				
				// The Output Buffer Contents Buffer for the Current Line.
				auto& bufferDataContents = bufferData.contents[cursorYPos];
				// The Output Buffer Contents Cursor Position Array for the Current Line. 
				auto& bufferDataContentsCursorPos = bufferData.contentsCursorData[cursorYPos];

				if ( !atEndOfBuffer )
					atEndOfBuffer = ( bufferDataContentsCursorPos.size() <= (currentCursorPos.X + 1ULL) );

				// Populate the `bufferDataContents` and `bufferDataContentsCursorPos`
				// arrays if necessary.
				while ( (size_t) currentCursorPos.X > bufferDataContents.size() ) {
					bufferDataContentsCursorPos.push_back(bufferDataContents.size());
					bufferDataContents.push_back(L' ');
				}
				// Continue populating the `bufferDataContentsCursorPos` array if necessary.
				while ( (size_t) currentCursorPos.X > bufferDataContentsCursorPos.size() ) {
					bufferDataContentsCursorPos.push_back(
						!bufferDataContentsCursorPos.empty()
							? (bufferDataContentsCursorPos.back() + 1ULL)
							: (0ULL)
						);
				}

				// If the Console Cursor is located at the end of the Output Buffer, 
				// we can append the current Wide Character to the end of the character buffer.
				if ( *atEndOfBuffer ) {
					if ( !isDetectedTerminalSequence ) {
						bufferDataContentsCursorPos.push_back(bufferDataContents.size());
					}
					if ( currentChar != L'\n' ) {
						bufferDataContents.push_back(currentChar);
					}
				}
				// Otherwise, we can insert the current Wide Character into the character buffer
				// at the location corresponding to the Current Console Cursor Position.
				else {
					// The current Character Buffer Cursor Position, according to
					// the `bufferDataContentsCursorPos` array.
					auto& currentCharCursorPos = bufferDataContentsCursorPos[currentCursorPos.X];
					// The index of the first Wide Character in the potential Virtual Terminal Sequence.
					size_t startPos = (
						currentCursorPos.X > 0ULL
							? bufferDataContentsCursorPos[currentCursorPos.X - 1]
							: 0ULL
					);
					// The total length of the potential Virtual Terminal Sequence.
					size_t sequenceLength = (currentCharCursorPos - startPos);

					// Erase existing Virtual Terminal Sequences that are being overwritten.
					if (sequenceLength > 1ULL) {
						bufferDataContents.erase(startPos, sequenceLength);

						for ( size_t j = startPos; j < bufferDataContentsCursorPos.size(); j++ ) {
							bufferDataContentsCursorPos[j] -= sequenceLength;
						}
					}
					else if ( currentCursorPos.X == (bufferDataContentsCursorPos.size() - 1ULL) ) {
						if ( bufferDataContents.size() > bufferDataContentsCursorPos.back() ) {
							bufferDataContents.erase( bufferDataContentsCursorPos.back() + 1ULL );
						}
					}

					if (currentChar != L'\n') {
						// Add the current Wide Character to the Current Output Buffer Contents Buffer.
						bufferDataContents[currentCharCursorPos + termSeqLength] = currentChar;
					}
				}

				if (currentChar != L'\n') {
					if ( !isDetectedTerminalSequence ) {
						currentCursorPos.X++;
						termSeqLength = 0ULL;

						if (currentCursorPos.X == bufferInfo.dwMaximumWindowSize.X) {
							currentCursorPos.X = 0;
							currentCursorPos.Y++;
						}
					}
					else {
						termSeqLength++;
					}
				}
				else {
					currentCursorPos.X = 0;
					currentCursorPos.Y++;
					termSeqLength = 0ULL;
				}

			}
			if (currentChar == L'\n')
				newlineCount++;

			strLength++;
			strPos++;
		}

		// If there are one or more newlines in the specified `str`, we need
		// to check if the newlines caused the console to scroll.
		if (newlineCount > 0) {
			WinConsoleCursorCoordinates initialCursorPos = this->getCursorPos();
			writeToConsole();
			WinConsoleCursorCoordinates finalCursorPos = this->getCursorPos();

			// Calculate the number of lines scrolled in the console by the newlines.
			short scrolledLines = ( newlineCount - (finalCursorPos.Y - initialCursorPos.Y) );
			
			if (scrolledLines > 0 && this->getCurrentBufferData(true).cursorStartPos.Y > 0) {
				bufferData.cursorScrollOffset += scrolledLines;

				if (programSettings.useCustomBufferBehavior)
					this->mainBufferData.cursorStartPos.Y = std::max<short>(
						this->mainBufferData.cursorStartPos.Y - scrolledLines,
						0
					);

				// Update the position of any Saved Cursors in the current Output Buffer
				// to reflect the console being scrolled.
				for ( auto& savedCursorPos : bufferData.savedCursors )
					savedCursorPos.Y = std::max<short>(savedCursorPos.Y - scrolledLines, 0);
			}
		}
		// If there are no newlines in the specified `str`, we can 
		// simply write the contents of the `str` to the console.
		else {
			writeToConsole();
		}

		return *this;

	}
	Console::OutputBuffer& Console::OutputBuffer::print ( const wchar_t* str, bool addToBuffer ) {

		((const OutputBuffer*) this)->print(str, addToBuffer);
		return *this;

	}

	// Overridden Instance Methods

	const Console::OutputBuffer::win_conbuf_t& Console::OutputBuffer::getBufferHandle () const {
	
		return this->getCurrentBufferData(true).handle;
	
	}

	// Helper Methods

	Console::OutputBuffer::BufferData& Console::OutputBuffer::getCurrentBufferData ( bool currentHandleBuffer ) const {
	
		if ( programSettings.useCustomBufferBehavior && currentHandleBuffer )
			return this->mainBufferData;

		return (
			this->altBufferData.empty()
				? this->mainBufferData
				: this->altBufferData.top()
		);
	
	}

	Console::OutputBuffer& Console::OutputBuffer::synchronizeCursorVisibility ( bool cursorIsVisible, bool addToBuffer ) {

		this->print( getVirtualTerminalSequence(cursorIsVisible ? L"[?25h" : L"[?25l"), addToBuffer );
		return *this;

	}


	/* Console::InputBuffer */
	// Class Constants

	const DWORD Console::InputBuffer::MAX_INPUT_EVENT_BUFFER_SIZE = 10UL;

	// Class Constructors

	Console::InputBuffer::InputBuffer ( win_conbuf_t iBufferHandle ) : AConsoleBuffer(iBufferHandle) {}

	// Implemented Instance Methods

	std::optional<Console::InputBuffer::WinConsoleInputKey> Console::InputBuffer::waitForInput ( bool flushBuffer, DWORD maxWaitTime ) const {
	
		static WinConsoleInput inputBuf[MAX_INPUT_EVENT_BUFFER_SIZE];	// A buffer containing recently-emitted Console Input Records.
		static DWORD inputBufSize = 0UL;								// The current size of the `inputBuf`.
		static DWORD inputBufPos = 0UL;									// The current index in the `inputBuf`.
		static bool inputBufIsEmpty = true;								// Indicates whether or not the `inputBuf` is considered to be empty.

		const win_conbuf_t& bufferHandle = this->getBufferHandle();		// The handle to the underlying Console Input Buffer.

		if (flushBuffer) {
			FlushConsoleInputBuffer(bufferHandle);
			inputBufIsEmpty = true;
		}

		// Wait until valid console input is received.
		while (true) {
			// As long as the local `inputBuf` is not empty or fully-traversed,
			// it will take priority over calling the Windows API.
			if (!inputBufIsEmpty) {
				// Continue iterating through the `inputBuf` looking for valid Input Records.
				for ( ; inputBufPos < inputBufSize; inputBufPos++ ) {
					// The current Console Input Record being evaluated.
					WinConsoleInput& input = inputBuf[inputBufPos];

					// Only Key Input Records and Key Down Events are considered to be valid.
					if ( input.EventType == KEY_EVENT && input.Event.KeyEvent.bKeyDown ) {
						inputBufPos++;
						return std::make_optional( std::move(input.Event.KeyEvent) );
					}
				}

				inputBufIsEmpty = true;
			}
			else {
				// The number of Console Event Records available in the underlying Console Input Buffer.
				DWORD inputRecordsAvailable = 0UL;

				GetNumberOfConsoleInputEvents(bufferHandle, &inputRecordsAvailable);

				if (inputRecordsAvailable > 0UL) {
					inputBufSize = 0UL;
					inputBufPos = 0UL;

					// Retrieve Console Event Records from the underlying Console Input Buffer and
					// store them in the internal record buffer for processing. 
					ReadConsoleInputW(bufferHandle, inputBuf, MAX_INPUT_EVENT_BUFFER_SIZE, &inputBufSize);
					inputBufIsEmpty = (inputBufSize == 0UL);
				}
				else {
					// If no Console Input Records are available, we can idle the Current Thread
					// until the user interacts with the console again.
					if ( WaitForSingleObject(bufferHandle, maxWaitTime) == WAIT_TIMEOUT ) {
						return std::optional<WinConsoleInputKey>();
					}
				}
			}
		}

	}

	std::optional<size_t> Console::InputBuffer::waitForInputData ( _Out_ wchar_t* oStrBuf, size_t maxInputLength ) const {

		// The number of characters read in from the console.
		DWORD charsRead = 0UL;
		// A structure used to control the behavior of the console when
		// requesting input data from the user.
		CONSOLE_READCONSOLE_CONTROL inputControl = {
			.nLength = sizeof(CONSOLE_READCONSOLE_CONTROL),
			.nInitialChars = 0UL,
			// Return on `Enter` or `ESC` (in theory)
			.dwCtrlWakeupMask = ( (1 << VK_RETURN) | (1 << VK_ESCAPE) ),
			.dwControlKeyState = 0UL
		};

		ReadConsoleW(
			this->getBufferHandle(),
			oStrBuf,
			(DWORD) maxInputLength + 1UL,
			&charsRead,
			&inputControl
		);

		if ( charsRead > 0UL ) {
			// Discard extra data beyond the `maxInputLength` from the Input Buffer.
			if ( charsRead > maxInputLength && oStrBuf[charsRead - 1ULL] != L'\r' ) {
				WinConsoleInput input[10] = {};
				DWORD recordsRead = 0UL;
				std::wstring tempBuf = {};
						     tempBuf.resize(maxInputLength + 1UL);
				DWORD tempCharsRead = 0ULL;

				oStrBuf[charsRead-- - 1UL] = L'\0';

				do {
					ReadConsoleW(
						this->getBufferHandle(),
						tempBuf.data(),
						(DWORD) maxInputLength + 1UL,
						&tempCharsRead,
						NULL
					);
				} while ( tempCharsRead > maxInputLength && tempBuf.back() != L'\r' );
			}
			else {
				// Remove Trailing Newlines
				oStrBuf[charsRead-- - 1ULL] = L'\0';
			}

		}
		
		return std::optional<size_t>(charsRead);

	}


	/* Console::MenuOptionStruct */
	// Structure Constructors

	Console::MenuOptionStruct::MenuOptionStruct (
		const std::wstring& iOption,
		std::optional<wchar_t> iHotkey,
		bool iDisabled,
		const MenuOptionPadding& iPadding
	) : option(iOption),
		hotkey( getLowercaseHotkey(iHotkey) ),
		disabled(iDisabled),
		padding(iPadding)
	{}

	// Structure Methods

	unsigned short Console::MenuOptionStruct::getTotalLineCount () const {
	
		return (
			1U
			+ (unsigned short) this->padding.top
			+ (unsigned short) this->padding.bottom
		);
	
	}


	/* Console::MenuOptionList::MenuOptionListActionStruct */

	Console::MenuOptionList::MenuOptionListActionStruct::MenuOptionListActionStruct (
		ActionCallbackFunction iActionFn, 
		const std::wstring& instructionLine
	) : MenuOptionListActionStruct( iActionFn, InstructionLineList({ instructionLine }) ) {}

	Console::MenuOptionList::MenuOptionListActionStruct::MenuOptionListActionStruct (
		ActionCallbackFunction iActionFn, 
		const InstructionLineList& iInstructions
	) : actionFn(iActionFn), instructions(iInstructions) {}


	/* Console::MenuOptionList */
	// Class Constants

	const Console::MenuOptionList::MenuOptionListAction
	Console::MenuOptionList::DEFAULT_NAVIGATION_ACTIONS = {
		[](
			const WinConsoleInputKey& key,
			MenuOptionList& menuOptions,
			Console& console,
			std::optional<size_t>& currentSelectionNum
		) -> Console::MenuOptionList::MenuOptionListAction::InputProcessingResult {

			// The index of the previous `MenuOption` that was selected prior to invoking this function.
			size_t prevSelectionNum = currentSelectionNum ? *currentSelectionNum : 1ULL;
			// The index of the new `MenuOption` to be selected after invoking this function.
			size_t newSelectionNum = prevSelectionNum;
			// The index of the `MenuOption` at the top of the Visible Console Viewport.
			const size_t& topMenuOptionNum = menuOptions.getTopMenuOptionNum();
			// The index of the `MenuOption` at the bottom of the Visible Console Viewport.
			const size_t& bottomMenuOptionNum = menuOptions.getBottomMenuOptionNum();
			// The value returned by the lambda function.
			Console::MenuOptionList::MenuOptionListAction::InputProcessingResult stopProcessingInput = std::make_pair(false, false);

			// Move the cursor up or down using the arrow keys.
			if (key.wVirtualKeyCode == VK_DOWN || key.wVirtualKeyCode == VK_UP) {
				// Avoid selecting disabled `MenuOption`s.
				do {
					// Move the cursor down using the down arrow.
					if (key.wVirtualKeyCode == VK_DOWN) {
						// Only move the cursor if it is above the last `MenuOption` in the list of `menuOptions`.
						if ( newSelectionNum < (menuOptions.size() - 1ULL) ) {
							newSelectionNum++;
							stopProcessingInput = std::make_pair(true, false);
						}
					}
					// Move the cursor up using the up arrow.
					else {
						// Only move the cursor if it is below the first `MenuOption` in the list of `menuOptions`.
						if (newSelectionNum > 0U) {
							newSelectionNum--;
							stopProcessingInput = std::make_pair(true, false);
						}
					}
				}
				while ( menuOptions[newSelectionNum].disabled && newSelectionNum > 0U && newSelectionNum < (menuOptions.size() - 1ULL) );
			}
			// Select one of the visible options without a dedicated hotkey using a numeric hotkey.
			else if ( 0x31 <= key.wVirtualKeyCode && key.wVirtualKeyCode <= 0x39 ) {
				// The integer corresponding to the number key that was used.
				size_t numberKey = (key.wVirtualKeyCode - 0x30ULL);

				newSelectionNum = topMenuOptionNum;

				// Determine which `MenuOption` in the list of `menuOptions` corresponds
				// to the specified `numberKey`, if possible.
				for (
					size_t i = 1U;
					( newSelectionNum < menuOptions.size() && i < numberKey );
					i++
				) {
					if (menuOptions[newSelectionNum].hotkey)
						continue;

					newSelectionNum++;
				}

				// If a valid selection is made, stop further processing of the list of `menuOptions`.
				if ( newSelectionNum < menuOptions.size() )
					stopProcessingInput = std::make_pair(true, true);
			}
			// Select one of the options using a dedicated hotkey.
			else if ( key.uChar.UnicodeChar != L'\0' ) {
				// The character that was specified, converted to its lowercase equivalent if applicable.
				wchar_t keyChar = std::towlower(key.uChar.UnicodeChar);

				// Determine which `MenuOption` in the list of `menuOptions` corresponds
				// to the specified `keyChar`, if possible.
				for ( size_t i = 0ULL; i < menuOptions.size(); i++ ) {
					const MenuOption& menuOption = menuOptions[i];

					if ( menuOption.hotkey && *menuOption.hotkey == keyChar && !menuOption.disabled ) {
						newSelectionNum = i;

						// If a valid selection is made, stop further processing of the list of `menuOptions`.
						stopProcessingInput = std::make_pair(true, true);
						break;
					}
				}
			}

			// Only bother updating the User Interface if the selection has actually changed.
			if (newSelectionNum != prevSelectionNum) {
				// Update the Selected `MenuOption` in the list of `menuOptions`.
				menuOptions.setSelectedOption(newSelectionNum);

				// The cursor is moving to a new location outside of the Visible Console Viewport
				if ( newSelectionNum < topMenuOptionNum || bottomMenuOptionNum < newSelectionNum ) {
					// The position of the Console Cursor where the Main Menu is to be re-drawn from.
					WinConsoleCursorCoordinates menuStartPos = *menuOptions.getCursorStartPos();
												menuStartPos.X -= 2;

					console.saveCursorPos();
					console.setCursorPos(menuStartPos);

					// Determine the new `MenuOption` that should appear at
					// the top of the Visible Console Viewport after scrolling.
					if (newSelectionNum < topMenuOptionNum) {
						if (topMenuOptionNum > 0ULL) {
							size_t diff = (prevSelectionNum - newSelectionNum);

							menuOptions.setTopMenuOptionNum(topMenuOptionNum - diff);
						}
					}
					else {
						size_t lastMenuOptionNum = (menuOptions.size() - 1ULL);

						if (bottomMenuOptionNum < lastMenuOptionNum) {
							size_t diff = (newSelectionNum - prevSelectionNum);
							size_t newTopMenuOptionNum = topMenuOptionNum;
							size_t newBottomMenuOptionNum = bottomMenuOptionNum;

							/*
							 * There is probably a better way to do this, but we are currently repeatedly manually calculating
							 * the expected number of lines taken up by each `MenuOption` in the list of `menuOptions`
							 * until the selected `MenuOption` is expected to be visible in the Visible Console Viewport.
							 */
							while (true) {
								newTopMenuOptionNum++;

								unsigned short lineCount = 0U;
								size_t i = newTopMenuOptionNum;

								for ( ; i <= newSelectionNum; i++ ) {
									lineCount += menuOptions[i].getTotalLineCount();

									if (i == newTopMenuOptionNum && menuOptions[newTopMenuOptionNum].padding.top)
										lineCount--;
									else if (i == newSelectionNum && menuOptions[newSelectionNum].padding.bottom)
										lineCount--;

									if (lineCount >= MAX_MENU_OPTION_LINES)
										break;
								}

								// Only break out of the loop once the selected `MenuOption` is
								// expected to be visible in the Visible Console Viewport.
								if (i > newSelectionNum || (lineCount < MAX_MENU_OPTION_LINES && !menuOptions[i].padding.top))
									break;
							}

							menuOptions.setTopMenuOptionNum(newTopMenuOptionNum);
						}
					}

					// Re-draw the Main Menu Options to reflect the
					// Menu Cursor having moved to a new `MenuOption`.
					console.printMenuOptions(menuOptions, false);
					console.restoreSavedCursorPos();
				}
				// The cursor is moving to a new location within the Visible Console Viewport.
				else {
					WinConsoleCursorCoordinates prevCursorPos = *menuOptions.getCursorPos(prevSelectionNum);
					WinConsoleCursorCoordinates newCursorPos = *menuOptions.getCursorPos(newSelectionNum);

					console.saveCursorPos();
					console.setCursorPos(prevCursorPos);
					console.print(L" ");
					console.setCursorPos(newCursorPos);
					console.print(L">");
					console.restoreSavedCursorPos();
				}

				currentSelectionNum = newSelectionNum;
			}

			return stopProcessingInput;

		},
		 L"Use a Hotkey or the Up/Down Key and Enter to select an option."
		//std::wstring(L"Use a Hotkey or ") + ARROW_UP_DOWN + L" and press Enter to select an option."
	};

	const Console::MenuOptionList::MenuOptionListAction
	Console::MenuOptionList::DEFAULT_ESCAPE_ACTION = {
		[](
			const WinConsoleInputKey& key,
			MenuOptionList& menuOptions,
			Console& console,
			std::optional<size_t>& currentSelectionNum
		) -> Console::MenuOptionList::MenuOptionListAction::InputProcessingResult {

			if ( key.wVirtualKeyCode == VK_ESCAPE ) {
				currentSelectionNum = std::optional<size_t>();
				// Halt processing for the `MenuOptionList` entirely.
				return std::make_pair(true, true);
			}

			return std::make_pair(false, false);

		},
		L"Press ESC to return to the previous menu."
	};

	const std::vector<Console::MenuOptionList::MenuOptionListAction> Console::MenuOptionList::DEFAULT_ACTIONS = {
		Console::MenuOptionList::DEFAULT_NAVIGATION_ACTIONS,
		Console::MenuOptionList::DEFAULT_ESCAPE_ACTION
	};

	// Class Constructors

	Console::MenuOptionList::MenuOptionList (
		const std::vector<MenuOptionListAction>& iActions,
		const std::wstring& iPrefix,
		const std::wstring& iSuffix,
		const std::wstring& iSeparator,
		unsigned short iWidth,
		unsigned short iMaxMenuOptionLines
	) : vector(), 
		actions(iActions), 
		prefix(iPrefix), 
		suffix(iSuffix), 
		separator(iSeparator), 
		width(iWidth),
		maxMenuOptionLines(iMaxMenuOptionLines)
	{}
	Console::MenuOptionList::MenuOptionList (
		std::initializer_list<MenuOption> initList,
		const std::vector<MenuOptionListAction>& iActions,
		const std::wstring& iPrefix,
		const std::wstring& iSuffix,
		const std::wstring& iSeparator,
		unsigned short iWidth,
		unsigned short iMaxMenuOptionLines
	) : vector(initList), 
		actions(iActions), 
		prefix(iPrefix), 
		suffix(iSuffix), 
		separator(iSeparator), 
		width(iWidth),
		maxMenuOptionLines(iMaxMenuOptionLines)
	{}

	// Basic Instance Getter Methods

	std::vector<Console::MenuOptionList::MenuOptionListAction>& Console::MenuOptionList::getActions () {

		return this->actions;

	}
	const std::vector<Console::MenuOptionList::MenuOptionListAction>& Console::MenuOptionList::getActions () const {
	
		return this->actions;
	
	}
	
	const std::wstring& Console::MenuOptionList::getPrefix () const {
	
		return this->prefix;
	
	}
	const std::wstring& Console::MenuOptionList::getSuffix () const {

		return this->suffix;

	}
	std::wstring Console::MenuOptionList::getSpace () const {

		return std::format(
			L"{:}{:{}}{:}",
			this->prefix,
			L"",
			this->width,
			this->suffix
		);

	}
	const std::wstring& Console::MenuOptionList::getSeparator () const {
	
		return this->separator;

	}
	const unsigned short& Console::MenuOptionList::getWidth () const {

		return this->width;

	}
	const unsigned short& Console::MenuOptionList::getMaxMenuOptionLines () const {
	
		return this->maxMenuOptionLines;
		
	}

	const std::optional<Console::WinConsoleCursorCoordinates>& Console::MenuOptionList::getCursorStartPos () const {
	
		return this->cursorStartPos;
	
	}
	Console::MenuOptionList& Console::MenuOptionList::setCursorStartPos ( const WinConsoleCursorCoordinates& cursorStartPos ) {
	
		this->cursorStartPos = cursorStartPos;
		return *this;
	
	}

	// Menu Option Selection Methods

	std::optional<size_t> Console::MenuOptionList::getSelectedOption () const {
	
		return this->selectedOptionNum;
	
	}
	const size_t& Console::MenuOptionList::getTopMenuOptionNum () const {

		return this->topMenuOptionNum;

	}
	const size_t& Console::MenuOptionList::getBottomMenuOptionNum () const {

		return this->bottomMenuOptionNum;

	}

	Console::MenuOptionList& Console::MenuOptionList::setSelectedOption ( size_t menuOptionNum ) {

		if (this->isValidOption(menuOptionNum))
			this->selectedOptionNum = menuOptionNum;

		return *this;

	}
	Console::MenuOptionList& Console::MenuOptionList::setTopMenuOptionNum ( size_t menuOptionNum ) {

		if (this->isValidOption(menuOptionNum))
			this->topMenuOptionNum = menuOptionNum;

		return *this;

	}
	Console::MenuOptionList& Console::MenuOptionList::setBottomMenuOptionNum ( size_t menuOptionNum ) {

		if (this->isValidOption(menuOptionNum))
			this->bottomMenuOptionNum = menuOptionNum;
		
		return *this;

	}
	
	bool Console::MenuOptionList::hasActiveStatusMessage () const {

		return (this->statusMessageIssueTime > 0);

	}
	bool Console::MenuOptionList::hasExpiredStatusMessage () const {

		if (this->statusMessageIssueTime > 0) {
			std::time_t currentTime = {};
			std::time(&currentTime);
			
			bool result = ( std::difftime(currentTime, this->statusMessageIssueTime) > STATUS_MESSAGE_LIFETIME );

			if (result)
				this->statusMessageIssueTime = 0;

			return result;
		}

		return false;

	}
	Console::MenuOptionList& Console::MenuOptionList::setStatusMessage ( const std::wstring& message ) {
	
		this->statusMessage = message;
		return *this;

	}
	bool Console::MenuOptionList::issueStatusMessage () {
	
		if ( !this->statusMessage.empty() ) {
			this->statusMessage.clear();
			std::time(&this->statusMessageIssueTime);

			return true;
		}

		return false;

	}

	// Miscellaneous Helper Methods

	std::optional<Console::WinConsoleCursorCoordinates>
	Console::MenuOptionList::getCursorPos ( size_t menuOptionNum ) const {

		// The starting position of the Console Cursor in the `MenuOptionList`.
		const std::optional<WinConsoleCursorCoordinates>& cursorStartPos = this->cursorStartPos;
		// The cursor position of the specified `MenuOption`.
		std::optional<WinConsoleCursorCoordinates> cursorPos = {};

		if ( cursorStartPos && menuOptionNum < this->size() ) {
			// The current `MenuOption` being processed.
			const MenuOption& menuOption = (*this)[menuOptionNum];
			
			cursorPos = cursorStartPos;
			
			for ( size_t i = this->topMenuOptionNum; i < menuOptionNum; i++ ) {
				cursorPos->Y += (*this)[i].getTotalLineCount();
			}

			if ( this->topMenuOptionNum > 0ULL )
				cursorPos->Y++;
			if ( menuOption.padding.top )
				cursorPos->Y++;
			if ( (*this)[this->topMenuOptionNum].padding.top )
				cursorPos->Y--;

			if (menuOption.padding.left)
				cursorPos->X += 3;

		}

		return cursorPos;

	}

	std::wstring Console::MenuOptionList::getInstructionString () const {
	
		// The Wide-Character String containing the instructions.
		std::wstring instructions = {};

		instructions += this->separator;
		instructions.push_back(L'\n');

		for (const MenuOptionListAction& action : this->actions)
			for (const std::wstring instruction : action.instructions)
				instructions += std::format(
					L"{:} - {:{}} {:}\n",
					this->prefix,
					instruction,
					std::max<unsigned short>(this->width - 4U, 0U),
					this->suffix
				);

		instructions += this->separator;
		return instructions;

	}

	// Internal Helper Methods

	bool Console::MenuOptionList::isValidOption ( size_t menuOptionNum ) const {
	
		return (menuOptionNum < this->size());
	
	}


	/* Console */
	// Static Class Properties

	Console::console_ptr_t Console::instancePtr = {};

	// Class Constructors & Destructors

	Console::Console () : 
		conInBuf( GetStdHandle(STD_INPUT_HANDLE) ),
		conOutBuf( GetStdHandle(STD_OUTPUT_HANDLE) ),
		conErrBuf( GetStdHandle(STD_ERROR_HANDLE) )
	{

		// An array of Raw Pointers to the handles being validated.
		const UTILS_NAMESPACE::AConsoleBuffer::win_conbuf_t* handles[] = {
			&this->conInBuf.bufferHandle,
			&this->conOutBuf.bufferHandle,
			&this->conErrBuf.bufferHandle
		};

		for (auto handlePtr : handles)
			if ( *handlePtr == INVALID_HANDLE_VALUE || *handlePtr == NULL )
				throw new std::runtime_error("Failed to retrieve handles for the attached console.");

		SetConsoleTitle(PRIMARY_PROGRAM_TITLE.c_str());
		SetConsoleCtrlHandler(consoleCtrlHandler, true);

	}
	Console::~Console () {
	
		wchar_t originalTitle[512] = {};

		GetConsoleOriginalTitle(originalTitle, 512);
		SetConsoleTitle(originalTitle);
		SetConsoleCtrlHandler(consoleCtrlHandler, false);

	}

	// Static Class Methods

	Console::console_ptr_t Console::getConsole () {

		if ( !Console::instancePtr ) {
			try {
				Console::instancePtr = Console::console_ptr_t(new Console());
			}
			catch (...) {
				return Console::console_ptr_t();
			}
		}

		return Console::instancePtr;
		
	}

	// Instance Methods

	const Console::InputBuffer& Console::in () const {
	
		return this->conInBuf;
	
	}
	const Console::OutputBuffer& Console::out () const {

		return this->conOutBuf;

	}
	const Console::OutputBuffer& Console::err () const {

		return this->conErrBuf;

	}

	Console& Console::printMenuOption ( MenuOption& menuOption, unsigned int optionNum, unsigned short width ) {

		// Print Disabled `MenuOption`s in a gray color.
		if (menuOption.disabled)
			this->print(getVirtualTerminalSequence(L"[90m"));

		this->printf(
			L"{:<{}}",
			std::format(
				L"{:1}) {:}",
				menuOption.hotkey ? std::wstring(1, *menuOption.hotkey) : std::to_wstring(optionNum),
				menuOption.option
			),
			width
		);

		// Revert the text color to the default value if needed.
		if (menuOption.disabled)
			this->print(getVirtualTerminalSequence(L"[39m"));

		return *this;
	
	}

	Console& Console::printMenuOptions ( MenuOptionList& menuOptions, bool printInstructions ) {

		std::wstring prefix = menuOptions.getPrefix();		// Menu Option Prefix
		std::wstring suffix = menuOptions.getSuffix();		// Menu Option Suffix
		unsigned short width = menuOptions.getWidth();		// Minimum Menu Option Width
		std::wstring spaceStr = menuOptions.getSpace();		// Menu Option Space String
		
		// The Top `MenuOption` number for the list of `menuOptions`
		const size_t& topMenuOptionNum = menuOptions.getTopMenuOptionNum();
		// The Bottom `MenuOption` number for the list of `menuOptions`
		size_t bottomMenuOptionNum = topMenuOptionNum;
		// The Selected `MenuOption` number for the list of `menuOptions`
		size_t selectedOptionNum = menuOptions.getSelectedOption() ? *menuOptions.getSelectedOption() : 0ULL;
		// The Current Unassigned `MenuOption` Hotkey Number
		unsigned short currentOptionHotkeyNum = 1U;

		short initialScrollOffset = this->getCursorScrollOffset();	// The Cursor Scroll Offset prior to printing the `menuOptions`
		short finalScrollOffset = initialScrollOffset;				// The Cursor Scroll Offset after printing the `menuOptions`
		unsigned short menuOptionLines = 0U;						// The total number of lines that have been used to render the list of `menuOptions`
		unsigned short maxMenuLines = (								// The maximum number of lines that can be used to render the list of `menuOptions`.
			selectedOptionNum < (menuOptions.size() - 1ULL)
				? (menuOptions.maxMenuOptionLines - 1U)
				: menuOptions.maxMenuOptionLines
		);

		// Set the Cursor Starting Position in the `menuOptions` list if
		// it has not already been set.
		if ( !menuOptions.getCursorStartPos() ) {
			WinConsoleCursorCoordinates cursorStartPos = this->getCursorPos();
										cursorStartPos.X += 2;

			menuOptions.setCursorStartPos(cursorStartPos);
		}

		// Ensure each `MenuOption` is terminated by a Newline Character.
		if ( !suffix.ends_with(L"\n") )
			suffix.push_back(L'\n');

		// Print an Up Arrow if there are one or more `MenuOptions`
		// currently above the Visible Console Viewport.
		if (topMenuOptionNum > 0ULL) {
			this->print(prefix)
				 .printf(L"  {:<{}}", ARROW_UP, width - 2U)
				 .print(suffix);
			menuOptionLines++;
		}

		// Print the `MenuOption`s within the Visible Console Viewport.
		for ( size_t i = topMenuOptionNum; i < menuOptions.size(); i++ ) {
			// Break out of the loop once we have exceeded the Expected Visible Console Viewport.
			if (menuOptionLines >= maxMenuLines)
				break;

			// The current `MenuOption` being printed.
			MenuOption& menuOption = menuOptions[i];

			// Print Top Padding for the Current `MenuOption` if applicable.
			if ( menuOption.padding.top && i > topMenuOptionNum && menuOptionLines < maxMenuLines ) {
				this->println(spaceStr);
				menuOptionLines++;
			}

			// Print the contents of the `MenuOption`.
			if (menuOptionLines < maxMenuLines) {
				this->print(prefix)
					 .printsp( ((i != selectedOptionNum) ? L' ' : L'>') )
					 .printMenuOption(menuOption, currentOptionHotkeyNum, width - 2U)
					 .print(suffix);
			
				if (!menuOption.hotkey)
					currentOptionHotkeyNum++;

				menuOptionLines++;

				if (i != topMenuOptionNum)
					bottomMenuOptionNum++;
			}

			// Print Bottom Padding for the Current `MenuOption` if applicable.
			if ( menuOption.padding.bottom && menuOptionLines < maxMenuLines ) {
				this->println(spaceStr);
				menuOptionLines++;
			}
		}

		menuOptions.setBottomMenuOptionNum(bottomMenuOptionNum);

		// Print a Down Arrow if there are one or more `MenuOptions`
		// currently below the Visible Console Viewport.
		if ( bottomMenuOptionNum < (menuOptions.size() - 1ULL) ) {
			this->print(prefix)
				 .printf(L"  {:<{}}", ARROW_DOWN, width - 2U)
				 .print(suffix);
			menuOptionLines++;
		}

		if (printInstructions)
			this->print(menuOptions.getInstructionString());

		finalScrollOffset = this->getCursorScrollOffset();

		if (finalScrollOffset > initialScrollOffset) {
			std::optional<WinConsoleCursorCoordinates> currentCursorStartPos = menuOptions.getCursorStartPos();

			if (currentCursorStartPos) {
				menuOptions.setCursorStartPos({
					.X = currentCursorStartPos->X,
					.Y = std::max<short>( (currentCursorStartPos->Y - (finalScrollOffset - initialScrollOffset)), 0 )
				});
			}
		}

		return *this;
	
	}

	std::optional<size_t> Console::waitForSelection ( MenuOptionList& menuOptions, DWORD maxWaitTime ) {

		// The current selection in the list of `menuOptions`, if applicable.
		std::optional<size_t> currentSelectionNum = (
			menuOptions.getSelectedOption()
				? *menuOptions.getSelectedOption()
				: 0U
		);
		// The last input key received from the user.
		std::optional<WinConsoleInputKey> key = {};
		// The return value of the method, indicating whether or not to stop further processing.
		MenuOptionList::MenuOptionListAction::InputProcessingResult stopProcessingInput = std::make_pair(false, false);

		/**
		 * A lambda function to print the Status Message for the list of `menuOptions`,
		 * if applicable.
		 * 
		 * Depends on both this object and the list of `menuOptions`.
		 * 
		 * @returns		`true` if a Status Message was successfully displayed.
		 * 				
		 * 				Otherwise, returns `false`.
		 */
		auto printMenuOptionStatusMessage = [this, &menuOptions] () -> bool {

			if ( !menuOptions.statusMessage.empty() ) {
				auto initialScrollOffset = this->getCursorScrollOffset();

				if ( menuOptions.statusMessageIssueTime > 0 )
					this->clear(false, false);

				this->saveCursorPos();
				this->println(false).print(menuOptions.statusMessage, false);

				auto finalScrollOffset = this->getCursorScrollOffset();

				if (finalScrollOffset > initialScrollOffset) {
					auto newCursorPos = *menuOptions.getCursorStartPos();
						 newCursorPos.Y = std::max<short>((newCursorPos.Y - (finalScrollOffset - initialScrollOffset)), 0);

					menuOptions.setCursorStartPos(newCursorPos);
				}

				this->restoreSavedCursorPos();
				menuOptions.issueStatusMessage();
				return true;
			}

			return false;

		};
		/**
		 * A lambda function to wait for input from the user.
		 * 
		 * Depends on this object, as well as the `key`, `menuOptions`,
		 * and `maxWaitTime` variables, of which the `key` variable may
		 * be modified by this function.
		 * 
		 * @param flushBuffer	Indicates whether or not to flush the Console Input Buffer
		 * 						prior to waiting for user input.
		 */
		auto waitForValidInput = [this, &key, &menuOptions, maxWaitTime] ( bool flushBuffer = false ) -> void {

			key = this->waitForInput( flushBuffer, (menuOptions.hasActiveStatusMessage() ? 1UL : maxWaitTime) );

		};

		printMenuOptionStatusMessage();
		waitForValidInput(true);

		while ( !stopProcessingInput.second && ( ( key && key->wVirtualKeyCode != VK_RETURN ) || ( !key && menuOptions.hasActiveStatusMessage() ) ) ) {
			if (key) {
				auto actions(menuOptions.getActions());
			
				for ( const MenuOptionList::MenuOptionListAction& action : actions ) {
					stopProcessingInput = action.actionFn(*key, menuOptions, *this, currentSelectionNum);

					if (stopProcessingInput.first || stopProcessingInput.second)
						break;
				}
			}

			if ( !printMenuOptionStatusMessage() && menuOptions.hasExpiredStatusMessage() ) {
				this->clear(false, false);
			}

			if (!stopProcessingInput.second)
				waitForValidInput();
		}

		return currentSelectionNum;

	}

	// Implemented Instance Methods

	std::optional<Console::WinConsoleInputKey> Console::waitForInput ( bool flushBuffer, DWORD maxWaitTime ) const {
	
		return this->conInBuf.waitForInput(flushBuffer, maxWaitTime);
	
	}
	std::optional<size_t> Console::waitForInputData ( _Out_ wchar_t* oStrBuf, size_t maxInputLength ) const {
	
		return this->conInBuf.waitForInputData(oStrBuf, maxInputLength);
	
	}

	std::optional<Console::buffer_number_t> Console::createAltBuffer () {
	
		return this->conOutBuf.createAltBuffer();
	
	}
	Console::buffer_number_t Console::getCurrentBufferNum () const {
	
		return this->conOutBuf.getCurrentBufferNum();
	
	}
	Console::buffer_number_t Console::restorePreviousBuffer () {
	
		return this->conOutBuf.restorePreviousBuffer();
	
	}

	Console::WinConsoleCursorCoordinates Console::getCursorPos () const {
	
		return this->conOutBuf.getCursorPos();
	
	}
	const short& Console::getCursorScrollOffset () const {

		return this->conOutBuf.getCursorScrollOffset();

	}
	bool Console::setCursorPos ( const WinConsoleCursorCoordinates& cursorPos ) {
	
		return this->conOutBuf.setCursorPos(cursorPos);
	
	}
	bool Console::saveCursorPos () {
	
		return this->conOutBuf.saveCursorPos();
	
	}
	bool Console::saveCursorPos ( const WinConsoleCursorCoordinates& cursorPos ) {
	
		return this->conOutBuf.saveCursorPos(cursorPos);
	
	}
	std::optional<Console::WinConsoleCursorCoordinates> Console::restoreSavedCursorPos () {
	
		return this->conOutBuf.restoreSavedCursorPos();
	
	}
	Console& Console::toggleCursorVisibility () {

		this->conOutBuf.toggleCursorVisibility();
		return *this;

	}
	Console& Console::toggleCursorVisibility ( bool visible ) {

		this->conOutBuf.toggleCursorVisibility(visible);
		return *this;

	}

	Console& Console::clear ( bool clearBuffer, bool resetCursorPos ) {
	
		this->conOutBuf.clear(clearBuffer, resetCursorPos);
		return *this;
	
	}

	const Console& Console::print ( const wchar_t* str, bool addToBuffer ) const {
	
		this->conOutBuf.print(str, addToBuffer);
		return *this;
	
	}
	Console& Console::print ( const wchar_t* str, bool addToBuffer ) {
	
		this->conOutBuf.print(str, addToBuffer);
		return *this;
	
	}

}