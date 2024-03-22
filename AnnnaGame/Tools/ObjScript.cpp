#include "../stdafx.h"
#include "ObjScript.h"
#include"../Component/Draw.h"
#include"../Game/Scenes.h"
#include"../Asset/AssetManager.h"

Object* tool::ObjScript::create(const DataSaver& data,Optional<String> objName)
{
	auto obj = scene->birthObjectNonHitbox();

	if (objName)obj->name = *objName;

	DecoderSet(&deco).objScriptCmd(obj);

	auto en = data.data.indexOf(U"[");

	String cmd = data.name+U",";

	if (data.data.size() > en)
	{
		for (int32 i = 0; i < en; ++i)cmd << data.data[i];
	}
	else{
		cmd += data.data;
	}

	if (cmd.split(U',').size()>2) {//パラメータが書いてあったら
		auto colorSaver = data.getDataSaver(U"color");
		String color = U"";
		if (colorSaver) {
			color = U"," + colorSaver->data;
		}
		while (color.split(U',').size() <= 4)
		{
			color += U",1";
		}

		auto angleSaver = data.getDataSaver(U"angle");
		String angle = U"";
		if (angleSaver) {
			angle = U"," + angleSaver->data;
		}
		if (angle.isEmpty())angle = U",0";

		auto zSaver = data.getDataSaver(U"z");
		String z = U"";
		if (zSaver) {
			z = U"," + zSaver->data;
		}
		if (z.isEmpty())z = U",0";

		auto scaleSaver = data.getDataSaver(U"scale");
		String scale = U"";
		if (scaleSaver)
		{
			scale = U"," + scaleSaver->data;
		}
		while (scale.split(U',').size() >= 2)
		{
			scale += U",1";
		}

		cmd += color + angle + z + scale;
		auto act = deco.input(cmd, U',')->decode();
		if (act)act->execute();
	}

	//子を読み込む
	for (const auto& [key, value] : data.children)
	{
		auto name = key.split(U':');

		Object* child = nullptr;
		if (name.size() >= 2)
			child=create(value, name[1]);
		else
			child=create(value, none);

		if (child)
		{
			child->transform->setParent(obj->transform, false);
			child->transform->addZ(-1);
			child->setSameDestiny(obj);
		}
	}

	return obj;
}

Object* tool::ObjScript::_loadImpl(const String& text, const String& name)
{
	auto data = DataSaver(U"", text).getDataSaver(name);

	return create(*data, name);
}

void objScriptCmdAction::AttachTexture::start()
{
	const auto& name = FileSystem::FullPath(U"App/");
	util::loadTexture(obj->scene, FileSystem::RelativePath(path, name), path);

	auto drawing = obj->addComponent<DrawTexture>(obj->scene->getDrawManager(), resource::texture(name));
	drawing->color = ColorF{ r / 255.0,g / 255.0,b / 255.0, a };
	obj->transform->setPos({ x,y,z });
	obj->transform->setDirection({ util::polar(angle * 1_deg),0 });
	obj->transform->scale.aspect = Vec3{ scaleX,scaleY,1.0 };
}

void objScriptCmdAction::AttachRectF::start()
{
	auto drawing = obj->addComponent<DrawRectF>(obj->scene->getDrawManager(), fig);
	drawing->color = ColorF{ r / 255.0,g / 255.0,b / 255.0,a };
	obj->transform->setPos({ x,y,z });
	obj->transform->setDirection({ util::polar(angle * 1_deg),0 });
	obj->transform->scale.aspect = Vec3{ scaleX,scaleY,1.0 };
}

void objScriptCmdAction::AttachTriangle::start()
{
	auto drawing = obj->addComponent<Draw2D<Triangle>>(obj->scene->getDrawManager(), fig);
	drawing->color = ColorF{ r / 255.0,g / 255.0,b / 255.0,a };
	obj->transform->setPos({ x,y,z });
	obj->transform->setDirection({ util::polar(angle * 1_deg),0 });
	obj->transform->scale.aspect = Vec3{ scaleX,scaleY,1.0 };
}

void objScriptCmdAction::AttachCircle::start()
{
	auto drawing = obj->addComponent<DrawCircle>(obj->scene->getDrawManager(), fig);
	drawing->color = ColorF{ r / 255.0,g / 255.0,b / 255.0,a };
	obj->transform->setPos({ x,y,z });
	obj->transform->setDirection({ util::polar(angle * 1_deg),0 });
	obj->transform->scale.aspect = Vec3{ scaleX,scaleY,1.0 };
}
