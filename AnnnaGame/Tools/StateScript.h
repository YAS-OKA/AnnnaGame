#pragma once
#include"../Util/DataSaver.h"

namespace tool
{
	class StateMachine
	{
	public:
		static void Create(FilePath path, StringView name);
		static void CreateFromText(const String& text, StringView name);
	};
}
