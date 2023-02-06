/*
 * Copyright 2018, Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "FileWrapper.h"
#include "lsp-cpp/include/client.h"
#include <unistd.h>
#include <stdio.h>
#include <thread>
#include <debugger.h>

#include <Application.h>

ProcessLanguageClient* client = NULL;

bool initialized = false;

MapMessageHandler my;

std::thread thread;

//utility
void
GetCurrentLSPPosition(Editor* editor, Position& position)
{
	Sci_Position pos   = editor->SendMessage(SCI_GETCURRENTPOS,0,0);
	position.line      = editor->SendMessage(SCI_LINEFROMPOSITION, pos, 0);
	int end_pos        = editor->SendMessage(SCI_POSITIONFROMLINE, position.line, 0);
	position.character = editor->SendMessage(SCI_COUNTCHARACTERS, end_pos, pos);
}

void
OpenFile(std::string& uri, int32 line = -1)
{
	//fixe me if (uri.find("file://") == 0)
	{
		uri.erase(uri.begin(), uri.begin()+7);

		BEntry entry(uri.c_str());
		entry_ref ref;
		if (entry.Exists()) {
			BMessage	refs(B_REFS_RECEIVED);
			if (entry.GetRef(&ref) == B_OK)
			{
				refs.AddRef("refs", &ref);
				if (line != -1)
					refs.AddInt32("be:line", line + 1);
				be_app->PostMessage(&refs);
			}
			
		}

	}
}

int
PositionFromDuple(Editor* editor, int line, int character)
{
	int pos = 0;
	pos = editor->SendMessage(SCI_POSITIONFROMLINE, line, 0);
	pos = editor->SendMessage(SCI_POSITIONRELATIVE, pos,  character);
	return pos;
}
		

int
ApplyTextEdit(Editor* editor, json& textEdit)
{
		int e_line = textEdit["range"]["end"]["line"].get<int>();
		int s_line = textEdit["range"]["start"]["line"].get<int>();
		int e_char = textEdit["range"]["end"]["character"].get<int>();
		int s_char = textEdit["range"]["start"]["character"].get<int>();
		int s_pos  = PositionFromDuple(editor, s_line, s_char);
		int e_pos  = PositionFromDuple(editor, e_line, e_char);
		
		editor->SendMessage(SCI_SETTARGETRANGE, s_pos, e_pos); 
		int replaced = editor->SendMessage(SCI_REPLACETARGET, -1, (sptr_t)(textEdit["newText"].get<std::string>().c_str()));
		
		return s_pos + replaced;
}
		
//end - utility

void FileWrapper::Initialize(const char* rootURI) {

	client = new ProcessLanguageClient("clangd");

    thread = std::thread([&] {
        client->loop(my);
    });
    
	my.bindResponse("initialize", [&](json& j){
        initialized = true;
	});

	client->Initialize();
	
	while(!initialized)
	{
		fprintf(stderr, "Waiting for clangd initialization.. %d\n", initialized);
		sleep(1);
	}
}

FileWrapper::FileWrapper(std::string filenameURI)
{
	fFilenameURI = filenameURI;
}

void	
FileWrapper::didOpen(const char* text, long len) {
	if (!initialized)
		return;
		
	client->DidOpen(fFilenameURI.c_str(), text);
    client->Sync();
}


void	
FileWrapper::didClose() {
	if (!initialized)
		return;
		
	client->DidClose(fFilenameURI.c_str());
	fEditor = NULL;
}

void
FileWrapper::Dispose()
{
	my.bindResponse("shutdown", [&](json& j){
		initialized = false;
		thread.detach();
        //client->Exit();
        delete client;
		client = NULL;
	});
	
	if (initialized) {
		client->Shutdown();
		//client->Exit();
	}
		
    while(client!=NULL){sleep(1);}
    

}

void
FileWrapper::didChange(const char* text, long len, int s_line, int s_char, int e_line, int e_char) {
	if (!initialized)
		return;
	
	TextDocumentContentChangeEvent event;
	Range range;
		
	range.start.line = s_line;
	range.start.character = s_char;

	range.end.line = e_line;
	range.end.character = e_char;
	
	event.range = range;
	event.text.assign(text, len);
	
	std::vector<TextDocumentContentChangeEvent> changes{event};
//		changes[0] = event;
	
	client->DidChange(fFilenameURI.c_str(), changes, false);
}


void
FileWrapper::Format()
{
	if (!initialized)
		return;
		
	my.bindResponse("textDocument/formatting", [&](json& params){
		
		if (!fEditor)
			return;
			
		auto items = params;
		for (json::reverse_iterator it = items.rbegin(); it != items.rend(); ++it) {
				ApplyTextEdit(fEditor, (*it));
		}
	});
	client->Formatting(fFilenameURI.c_str());	
}

/* Working.. just need some UI
void
FileWrapper::GoToImplementation() {
	if (!initialized || !fEditor)
		return;

	Position position;
	GetCurrentLSPPosition(fEditor, position);
	
	my.bindResponse("textDocument/implementation", [&](json& items){
		if (!items.empty())
		{
			//TODO if more than one match??
			auto first=items[0];
			std::string uri = first["uri"].get<std::string>();
			
			int32 s_line = first["range"]["start"]["line"].get<int>();
			
			OpenFile(uri, s_line);	
		}
	});
	
	client->GoToImplementation(fFilenameURI.c_str(), position);
}
*/

void
FileWrapper::GoToDefinition() {
	if (!initialized || !fEditor)
		return;

	Position position;
	GetCurrentLSPPosition(fEditor, position);
	
	my.bindResponse("textDocument/definition", [&](json& items){
		if (!items.empty())
		{
			//TODO if more than one match??
			auto first=items[0];
			std::string uri = first["uri"].get<std::string>();
			
			int32 s_line = first["range"]["start"]["line"].get<int>();
			
			OpenFile(uri, s_line);	
		}
	});
	
	client->GoToDefinition(fFilenameURI.c_str(), position);
}

void
FileWrapper::GoToDeclaration() {
	if (!initialized || !fEditor)
		return;

	Position position;
	GetCurrentLSPPosition(fEditor, position);
	
	my.bindResponse("textDocument/declaration", [&](json& items){
		if (!items.empty())
		{
			//TODO if more than one match??
			auto first=items[0];
			std::string uri = first["uri"].get<std::string>();
			
			int32 s_line = first["range"]["start"]["line"].get<int>();
			
			OpenFile(uri, s_line);	
		}
	});
	
	client->GoToDeclaration(fFilenameURI.c_str(), position);
}

void
FileWrapper::SwitchSourceHeader() {
	if (!initialized)
		return;
		
	my.bindResponse("textDocument/switchSourceHeader", [&](json& result){		
		std::string uri = result.get<std::string>();
		OpenFile(uri);		
	});
	
	client->SwitchSourceHeader(fFilenameURI.c_str());
}

void	
FileWrapper::SelectedCompletion(const char* text){
	if (!initialized || !fEditor)
		return;	
	

	//int cur = fEditor->SendMessage(SCI_AUTOCGETCURRENT, 0, 0);
	//printf("SelectedCompletion([%s]) - [%s] - cur[%d]\n", text, fCurrentCompletion.dump().c_str(),cur);
	
	if (this->fCurrentCompletion != json({}))
	{
		auto items = fCurrentCompletion;
		std::string list;
		for (json::iterator it = items.begin(); it != items.end(); ++it) {
			
			if ((*it)["label"].get<std::string>().compare(std::string(text)) == 0)
			{
				//first let's clean the text entered until the combo is selected:
				Sci_Position pos = fEditor->SendMessage(SCI_GETCURRENTPOS,0,0);
				
				 if (pos > fCompletionPosition) {
					fEditor->SendMessage(SCI_SETTARGETRANGE, fCompletionPosition, pos); 
					fEditor->SendMessage(SCI_REPLACETARGET, -1, (sptr_t)"");
				}
				
				int endpos = ApplyTextEdit(fEditor, (*it)["textEdit"]);
				fEditor->SendMessage(SCI_SETEMPTYSELECTION, endpos, 0);
				fEditor->SendMessage(SCI_SCROLLCARET, 0 ,0);
				break;
			}
			
		}		
	}
	fEditor->SendMessage(SCI_AUTOCCANCEL, 0, 0);
	this->fCurrentCompletion = json({});
}

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

void	
FileWrapper::StartCompletion(){
	if (!initialized || !fEditor)
		return;
		
	//let's check if a completion is ongoing
	
	if (this->fCurrentCompletion != json({}))
	{
		//let's close the current Scintilla listbox
		fEditor->SendMessage(SCI_AUTOCCANCEL, 0, 0);
		//let's cancel any previous request running on clangd
		// --> TODO: cancel previous clangd request!
		
		//let's clean-up current request details:
		this->fCurrentCompletion = json({});
	}
	
	Position position;
	GetCurrentLSPPosition(fEditor, position);
	CompletionContext context;
	
	this->fCompletionPosition = fEditor->SendMessage(SCI_GETCURRENTPOS,0,0);

	my.bindResponse("textDocument/completion", [&](json& params){
		auto items = params["items"];
		std::string list;
		for (json::iterator it = items.begin(); it != items.end(); ++it) {
			std::string label = (*it)["label"].get<std::string>();
			ltrim(label);
			fprintf(stderr, "completion: [%s]\n", label.c_str());
			if (list.length() > 0)
				list += "@";
			list += label;
			(*it)["label"] = label;
			
		}
		if (list.length() > 0) {
			this->fCurrentCompletion = items;
			fEditor->SendMessage(SCI_AUTOCSETSEPARATOR, (int)'@', 0);
			fEditor->SendMessage(SCI_AUTOCSHOW, 0, (sptr_t)list.c_str());
			//printf("Dump([%s])\n", fCurrentCompletion.dump().c_str());
		}
			
	});
	client->Completion(fFilenameURI.c_str(), position, context);
}

