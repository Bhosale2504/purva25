/*
 * Copyright 2023, Andrea Anzani <andrea.anzani@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef ActionManager_H
#define ActionManager_H


#include <SupportDefs.h>
#include <map>
#include <String.h>
#include <MenuItem.h>
#include "ToolBar.h"

class Action;

class ActionManager {
public:

			~ActionManager();

	static status_t RegisterAction(int32   msgWhat, 
							BString label, 
							BString toolTip = "",
							BString iconResource = "",
							char shortcut = 0, uint32 modifiers = 0);
	
	static BMenuItem*	CreateMenuItem(int32 msgWhat);
	
	static status_t		AddItem(int32 msgWhat, BMenu*);
	static status_t		AddItem(int32 msgWhat, ToolBar*);
	
	static status_t		SetEnabled(int32 msgWhat, bool enabled);
	static status_t	SetPressed(int32 msgWhat, bool pressed);
	
	static bool		IsPressed(int32 msgWhat);

private:

	typedef std::map<int32, Action*> ActionMap;
	ActionMap	fActionMap;
	
	static ActionManager instance;
	
	ActionManager() {};
};


#endif // ActionManager_H
