/*
 * Copyright 2018 A. Mosca <amoscaster@gmail.com>
 *
 * Adapted from Expander::ExpanderThread class
 * Copyright 2004-2010, Jérôme Duval. All rights reserved.
 *
 * Distributed under the terms of the MIT License.
 * Original code from ZipOMatic by jonas.sundstrom@kirilla.com
 */

/*
 * ConsoleIOThread is the worker class for console I/O (build log, terminal
 * program input/output, etc.).
 * It gets the command (via message) from main window, executes it in pipes
 * and sends the streams to the visual class, ConsoleIOView (via messages).
 * Some logic is also sent, like enabling and disabling Stop button, and start,
 * end, error banners.
 * When the thread is over, or in case of error, a message is sent to the main
 * window.
 * The only exception is when the user presses the stop button. In that case it
 * is the visual class itself that sends a message to main window.
 * All end messages sent to main window contain the command type in order to
 * apply some logic (reenable build/run buttons and menus).
 * stdin is handled via DispatchMessage in main window.
 */
#ifndef CONSOLE_THREAD_H
#define CONSOLE_THREAD_H

#include <Message.h>
#include <Messenger.h>
#include <String.h>

#include "GenericThread.h"
#include <stdio.h>
#include <stdlib.h>


enum {
	CONSOLEIOTHREAD_ENABLE_STOP_BUTTON	= 'Cesb',
	CONSOLEIOTHREAD_ERROR				= 'Cerr',
	CONSOLEIOTHREAD_EXIT				= 'Cexi',
	CONSOLEIOTHREAD_CMD_TYPE			= 'Ccty',
	CONSOLEIOTHREAD_PRINT_BANNER		= 'Cpba',
	CONSOLEIOTHREAD_STDOUT				= 'Csou',
	CONSOLEIOTHREAD_STDERR				= 'Cser'
};

class ConsoleIOThread : public GenericThread {
public:
								ConsoleIOThread(BMessage* cmd_message,
									const BMessenger& consoleTarget);

								~ConsoleIOThread();

			status_t			SuspendExternal();
			status_t			ResumeExternal();
			status_t			InterruptExternal();

			void				PushInput(BString text);

private:
			void				ClosePipes();
	virtual	status_t			ThreadStartup();
	virtual	status_t			ExecuteUnit();
	virtual	status_t			ThreadShutdown();

	virtual	void				ThreadStartupFailed(status_t a_status);
	virtual	void				ExecuteUnitFailed(status_t a_status);
	virtual	void				ThreadShutdownFailed(status_t a_status);

			thread_id			PipeCommand(int argc, const char** argv,
									int& in, int& out, int& err,
									const char** envp = (const char**)environ);

			void				_BannerMessage(BString status);
			void				_CleanPipes();

			BMessenger			fConsoleTarget;

			thread_id			fThreadId;
			int					fStdIn;
			int					fStdOut;
			int					fStdErr;
			FILE*				fConsoleOutput;
			FILE*				fConsoleError;
			char				fConsoleOutputBuffer[LINE_MAX];
			BString 			fCmdType;
};


#endif	// CONSOLE_THREAD_H
