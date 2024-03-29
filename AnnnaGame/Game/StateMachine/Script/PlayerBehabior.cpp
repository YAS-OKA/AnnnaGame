#include"../../../stdafx.h"
#include"../StateProvider.h"
#include"PlayerBehavior.h"

using namespace state;

namespace {
	SCreatorContainer dict;
}
//
//SCreatorContainer createPlayerBehabior()
//{
//	//f1
//	dict[U"PlayerBehabior"] = [&](In info, A act)
//		{
//			act |= dict[U"Operable"](info)
//				+ dict[U"UnOperable"](info);
//			return F(act);
//		};
//	//f2
//	dict[U"Operatable"] = [&](In info, A act)
//		{
//			act |= dict[U"SetUp"](info)
//				+ dict[U"Run"](info)
//				+ dict[U"Jump"](info);
//			for (auto k : step(3)) act |= dict[U"Attack{}"_fmt(k)](info);
//
//			return F(act);
//		};
//	dict[U"UnOperatable"] = [&](In info, A act)
//		{
//			return F(act);
//		};
//	//f3
//	dict[U"SetUp"] = [&](In info, A act)
//		{
//			
//			return F(act);
//		};
//	dict[U"Run"] = [&](In info, A act)
//		{
//
//			return F(act);
//		};
//	dict[U"Jump"] = [&](In info, A act)
//		{
//			act |= dict[U"Avoid"](info);
//			return F(act);
//		};
//	for (auto k : step(3)) dict[U"Attack{}"_fmt(k)] = [&](In info, A act)
//		{
//			return F(act);
//		};
//
//}
