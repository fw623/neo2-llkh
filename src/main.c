#define UNICODE

#include "main.h"

HHOOK keyhook = NULL;

void SetStdOutToNewConsole() {
	// allocate a console for this app
	AllocConsole();
	// redirect unbuffered STDOUT to the console
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	int fileDescriptor = _open_osfhandle((intptr_t)consoleHandle, _A_SYSTEM);
	FILE *fp = _fdopen(fileDescriptor, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);
	// give the console window a nicer title
	SetConsoleTitle(L"neo-llkh Debug Output");
	// give the console window a bigger buffer size
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(consoleHandle, &csbi)) {
		COORD bufferSize;
		bufferSize.X = csbi.dwSize.X;
		bufferSize.Y = 9999;
		SetConsoleScreenBufferSize(consoleHandle, bufferSize);
	}
}

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
	switch (fdwCtrlType) {
		// Handle the Ctrl-c signal.
		case CTRL_C_EVENT:
			printf("\nCtrl-c detected!\n");
			printf("Please quit by using the tray icon!\n\n");
			return TRUE;

		default:
			return FALSE;
	}
}

DWORD WINAPI hookThreadMain(void *user) {
	HINSTANCE base = GetModuleHandle(NULL);
	MSG msg;

	if (!base) {
		if (!(base = LoadLibrary((wchar_t *) user))) {
			return 1;
		}
	}
	/* Installs an application-defined hook procedure into a hook chain
	* 1st Parameter idHook: WH_KEYBOARD_LL - The type of hook procedure to be installed.
	* Installs a hook procedure that monitors low-level keyboard input events.
	* 2nd Parameter lpfn: LowLevelKeyboardProc - A pointer to the hook procedure.
	* 3rd Parameter hMod: hExe - A handle to the DLL containing the hook procedure pointed to by the lpfn parameter.
	* 4th Parameter dwThreadId: 0 - the hook procedure is associated with all existing threads running.
	* If the function succeeds, the return value is the handle to the hook procedure.
	* If the function fails, the return value is NULL.
	*/
	keyhook = SetWindowsHookEx(WH_KEYBOARD_LL, keyevent, base, 0);

	/* Message loop retrieves messages from the thread's message queue and dispatches them to the appropriate window procedures.
	* For more info http://msdn.microsoft.com/en-us/library/ms644928%28v=VS.85%29.aspx#creating_loop
	* Retrieves a message from the calling thread's message queue.
	*/
	while (GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	/* To free system resources associated with the hook and removes a hook procedure installed in a hook chain
	* Parameter hhk: hKeyHook - A handle to the hook to be removed.
	*/
	UnhookWindowsHookEx(keyhook);

	return 0;
}

void exitApplication() {
	trayicon_remove();
	PostQuitMessage(0);
}

bool fileExists(LPCSTR szPath) {
	DWORD dwAttrib = GetFileAttributesA(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES
		&& !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

int main (int argc, char *argv[]) {
  /**
	* find settings.ini (in same folder as neo-llkh.exe)
	*/
	// get path of neo-llkh.exe
	char ini[256];
	GetModuleFileNameA(NULL, ini, 256);
	//printf("exe: %s\n", ini);
	char * pch;
	// find last \ in path
	pch = strrchr(ini, '\\');
	// replace neo-llkh.exe by settings.ini
	strcpy(pch+1, "settings.ini");
	//printf("ini: %s\n", ini);

	/**
	* If settings.ini exists, read in settings.
	* Otherwise check for command line parameters.
	*/
	if (fileExists(ini)) {
		char returnValue[100];

		GetPrivateProfileStringA("Settings", "layout", "neo", layout, 100, ini);

		GetPrivateProfileStringA("Settings", "capsLockEnabled", "0", returnValue, 100, ini);
		capsLockEnabled = (strcmp(returnValue, "1") == 0);

		GetPrivateProfileStringA("Settings", "shiftLockEnabled", "0", returnValue, 100, ini);
		shiftLockEnabled = (strcmp(returnValue, "1") == 0);

		GetPrivateProfileStringA("Settings", "qwertzForShortcuts", "0", returnValue, 100, ini);
		qwertzForShortcuts = (strcmp(returnValue, "1") == 0);

		GetPrivateProfileStringA("Settings", "supportLevels5and6", "0", returnValue, 100, ini);
		supportLevels5and6 = (strcmp(returnValue, "1") == 0);

		GetPrivateProfileStringA("Settings", "debugWindow", "0", returnValue, 100, ini);
		debugWindow = (strcmp(returnValue, "1") == 0);

		if (capsLockEnabled)
			shiftLockEnabled = false;

		if (debugWindow)
			// Open Console Window to see printf output
			SetStdOutToNewConsole();

		printf("\nSettings from %s:\n", ini);
		printf(" Layout: %s\n", layout);
		printf(" capsLockEnabled: %d\n", capsLockEnabled);
		printf(" shiftLockEnabled: %d\n", shiftLockEnabled);
		printf(" qwertzForShortcuts: %d\n", qwertzForShortcuts);
		printf(" supportLevels5and6: %d\n", supportLevels5and6);
		printf(" debugWindow: %d\n\n", debugWindow);

	} else {
		printf("\nsettings.ini not found: %s\n\n", ini);
	}


	if (argc >= 2) {
		printf("Settings via command line:");
		char delimiter[] = "=";
		char *param, *value;
		for (int i=1; i< argc; i++) {
			if (strcmp(argv[i], "neo") == 0
				|| strcmp(argv[i], "adnw") == 0
				|| strcmp(argv[i], "adnwzjf") == 0
				|| strcmp(argv[i], "bone") == 0
				|| strcmp(argv[i], "koy") == 0
				|| strcmp(argv[i], "kou") == 0
				|| strcmp(argv[i], "kou-fw623") == 0
				|| strcmp(argv[i], "vou") == 0
				|| strcmp(argv[i], "qwertz") == 0) {
				strncpy(layout, argv[i], 100);
				printf("\n Layout: %s", layout);

			} else if (strstr(argv[i], "=") != NULL) {
				//printf("\narg%d: %s", i, argv[i]);
				param = strtok(argv[i], delimiter);
				if (param != NULL) {
					value = strtok(NULL, delimiter);
					if (value != NULL) {
						//printf("\n%s ist %s", param, value);
					}
				}

				if (strcmp(param, "debugWindow") == 0) {
					bool debugWindowAlreadyStarted = debugWindow;
					debugWindow = value==NULL ? false : (strcmp(value, "1") == 0);
					if (debugWindow && !debugWindowAlreadyStarted)
						// Open Console Window to see printf output
						SetStdOutToNewConsole();
					printf("\n debugWindow: %d", debugWindow);

				} else if (strcmp(param, "layout") == 0) {
					if (value != NULL) {
						strncpy(layout, value, 100);
						printf("\n Layout: %s", layout);
					}

				} else if (strcmp(param, "capsLockEnabled") == 0) {
					capsLockEnabled = value==NULL ? false : (strcmp(value, "1") == 0);
					printf("\n capsLockEnabled: %d", capsLockEnabled);

				} else if (strcmp(param, "shiftLockEnabled") == 0) {
					shiftLockEnabled = value==NULL ? false : (strcmp(value, "1") == 0);
					printf("\n shiftLockEnabled: %d", shiftLockEnabled);

				} else if (strcmp(param, "qwertzForShortcuts") == 0) {
					qwertzForShortcuts = value==NULL ? false : (strcmp(value, "1") == 0);
					printf("\n qwertzForShortcuts: %d", qwertzForShortcuts);

				} else if (strcmp(param, "supportLevels5and6") == 0) {
					supportLevels5and6 = value==NULL ? false : (strcmp(value, "1") == 0);
					printf("\n supportLevels5and6: %d", supportLevels5and6);

				} else {
					printf("\nUnknown parameter: %s", param);
				}
			} else {
				printf("\ninvalid arg: %s", argv[i]);
			}
		}
	}
	printf("\n\n");

	// TODO: implement this by checking dfkConfig
	// if (swapLeftCtrlAndLeftAlt || swapLeftCtrlLeftAltAndLeftWin)
	// 	// catch ctrl-c because it will send keydown for ctrl
	// 	// but then keyup for alt. Then ctrl would be locked.
	// 	SetConsoleCtrlHandler(CtrlHandler, TRUE);

	initLayout();

	setbuf(stdout, NULL);

	DWORD tid;

	/* Retrieves a module handle for the specified module.
	 * parameter is NULL, GetModuleHandle returns a handle to the file used to create the calling process (.exe file).
	 * If the function succeeds, the return value is a handle to the specified module.
	 * If the function fails, the return value is NULL.
	 */
	HINSTANCE hInstance = GetModuleHandle(NULL);
	trayicon_init(LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON)), APPNAME);
	trayicon_add_item(NULL, &toggleBypassMode);
	trayicon_add_item("Exit", &exitApplication);

	/* CreateThread function Creates a thread to execute within the virtual address space of the calling process.
	 * 1st Parameter lpThreadAttributes:  NULL - Thread gets a default security descriptor.
	 * 2nd Parameter dwStackSize:  0  - The new thread uses the default size for the executable.
	 * 3rd Parameter lpStartAddress:  KeyLogger - A pointer to the application-defined function to be executed by the thread
	 * 4th Parameter lpParameter:  argv[0] -  A pointer to a variable to be passed to the thread
	 * 5th Parameter dwCreationFlags: 0 - The thread runs immediately after creation.
	 * 6th Parameter pThreadId(out parameter): NULL - the thread identifier is not returned
	 * If the function succeeds, the return value is a handle to the new thread.
	 */
	HANDLE thread = CreateThread(0, 0, hookThreadMain, argv[0], 0, &tid);


	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		// Translates virtual-key messages into character messages.
		TranslateMessage(&msg);
		// Dispatches a message to a window procedure.
		DispatchMessage(&msg);
	}

	return 0;
}
