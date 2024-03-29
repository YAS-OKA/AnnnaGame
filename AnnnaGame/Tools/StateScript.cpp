#include "../stdafx.h"
#include "StateScript.h"
#include"../Util/Util.h"

String Path = U"../Game/StateMachine/Script/";

void tool::StateMachine::Create(FilePath path, StringView name)
{
	CreateFromText(TextReader{ path }.readAll(),name);
}

void tool::StateMachine::CreateFromText(const String& text,StringView name)
{	
	auto saver = DataSaver(U"", text);
	auto states = *saver.getDataSaver(name);
	auto edges = *saver.getDataSaver(U"edge:" + name);
	//cpp h ファイルの読み込み
	auto cf = util::EditFile(Path + name + U"/state.cpp");
	auto hf = util::EditFile(Path + name + U"/state.h");


}

