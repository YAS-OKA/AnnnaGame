#include"../stdafx.h"
#include"MotionCmd.h"
#include "../Util/Cmd.hpp"
#include"../Game/Scenes.h"
#include"Parts.h"
#include"Motion.h"

namespace mot
{
	void MakeParts::start()
	{
		IAction::start();
		String parentName = pmanager->partsArray.isEmpty() ? U"" : U"master";

		if (path)
		{
			loadPartsTexture(pmanager->scene, *path);

			createdParts = pmanager->addParts(PartsParams(name, *path, TextureAsset(*path), parentName, { x,y }));

			if (createHitbox)createdParts->createHitbox(TextureAsset(*path).size() / 2.0, Image{ localPath + *path }.alphaToPolygons());
		}
		else
		{
			createdParts = pmanager->addParts(PartsParams(name, none, fig, parentName, { x,y }));
			if (createHitbox)createdParts->createHitbox({ 0,0 }, { fig.asPolygon() });
		}
	}

	Parts* MakeParts::getCreatedParts(bool keepParts)
	{
		auto ret = createdParts;
		if (not keepParts)createdParts = nullptr;
		return ret;
	}
	Parts* FindParts::getFindParts(bool keepParts)
	{
		auto ret = foundParts;
		if (not keepParts)foundParts = nullptr;
		return ret;
	}

	void FindParts::start()
	{
		foundParts = pmanager->find(name);
	}

	HashSet<Parts*> KillParts::getKilledParts(bool keepParts)
	{
		HashSet<Parts*> ret = killedParts;
		if (not keepParts)killedParts = {};
		return ret;
	}

	void KillParts::start()
	{
		auto parts = pmanager->find(name);

		if (parts == nullptr)return;

		if (killChildren or parts->getName() == U"master")
		{
			//子を殺す
			prg::Actions actions;
			Array<KillParts*>acts;
			for (const auto& child : parts->partsRelation.getChildren())
			{
				acts << actions.addParallel<KillParts>(child->name, true).build(pmanager);
			}
			actions.start(true);

			for (const auto& act : acts)
			{
				for (const auto& p : act->getKilledParts())
					killedParts.emplace(p);
			}
		}
		else {
			auto parent = parts->partsRelation.getParent();

			auto child = parts->partsRelation.getChild();
			//自分の親を自分の子の親にする
			if (child != nullptr and parent != nullptr)child->setParent(parent->getName());
		}
		pmanager->killParts(parts);

		killedParts.emplace(parts);
	}
	StartMotion::StartMotion(String motionName)
		:motionName(motionName)
	{
	}
	void StartMotion::start()
	{
		bool flag = false;
		for (auto& parts : pMan->partsArray)
		{
			if (parts->actman(motionName))
			{
				flag = true;
				parts->actman.act(motionName);
			}
		}
		if (not flag)Console << U"モーションがセットされてません:{}"_fmt(motionName);
	}
	LoadMotionScript::LoadMotionScript(FilePath path,String motionName)
		:path(path),motionName(motionName)
	{
	}

	LoadMotionScript* LoadMotionScript::build(PartsManager* pman)
	{
		this->pman = pman;
		return this;
	}
	void LoadMotionScript::start()
	{
		if (not MotionScript().LoadFile(pman, path, motionName))
		{
			Console << U"モーションスクリプト読み込みに失敗\n\
					指定されたパス:{}"_fmt(path);
		}
	}
}
