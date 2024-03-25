#include "../stdafx.h"
#include "StateScript.h"

void tool::StateMachine::Create(FilePath path, StringView name)
{
	CreateFromText(TextReader{ path }.readAll());
}

void tool::StateMachine::CreateFromText(const String& text)
{
	
}

