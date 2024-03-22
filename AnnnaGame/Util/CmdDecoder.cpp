#include "../stdafx.h"
#include"CmdDecoder.h"
//#include"../Prg/Action.h"

CmdDecoder* CmdDecoder::input(const String& input, char spliter)
{
	return this->input(input.split(spliter));
}

CmdDecoder* CmdDecoder::input(Array<String> tokens)
{
	token = tokens;
	return this;
}

ICMD* CmdDecoder::decode()
{
	String head;
	Array<String> arg;

	if (token.isEmpty())goto Err;

	head = token[0];

	token.pop_front();

	arg = token;

	token.clear();

	if (table.contains(head) and table[head].contains(arg.size()))return table[head][arg.size()](arg);	
Err:
//例外処理
	return nullptr;	
}

#include"../Motions/MotionCmd.h"
#include"../Motions/Motion.h"
#include"../Tools/ObjScript.h"

void DecoderSet::motionScriptCmd(mot::PartsManager* pmanager)
{
	using namespace mot;

	decoder->add<SetMotion<RotateTo>, String, String, double, double, double, bool, double>(U"SetAngle", pmanager);
	decoder->add<SetMotion<RotateTo>, String, String, double, double, double, bool>(U"SetAngle", pmanager);
	decoder->add<SetMotion<RotateTo>, String, String, double, double, double>(U"SetAngle", pmanager);
}

void DecoderSet::objScriptCmd(Object* obj)
{
	using namespace objScriptCmdAction;
	//textureをアタッチ
	decoder->add<AttachTexture, String, double, double, double, double, double, double, double, double, double, double>(U"texture", obj);
	decoder->add<AttachTexture, String, double, double, double, double, double, double, double, double>(U"texture", obj);
	decoder->add<AttachTexture, String, double, double, double, double>(U"texture", obj);
	decoder->add<AttachTexture, String, double, double, double>(U"texture", obj);
	decoder->add<AttachTexture, String, double, double>(U"texture", obj);
	//rectをアタッチ
	decoder->add<AttachRectF, double, double, double, double, double, double, double, double, double, double,double,double>(U"rect", obj);
	decoder->add<AttachRectF, double, double, double, double, double, double, double, double, double, double>(U"rect", obj);
	decoder->add<AttachRectF, double, double, double, double, double, double, double, double, double>(U"rect", obj);
	decoder->add<AttachRectF, double, double, double, double, double, double, double, double>(U"rect", obj);
	decoder->add<AttachRectF, double, double, double, double, double, double, double>(U"rect", obj);
	decoder->add<AttachRectF, double, double, double, double, double, double>(U"rect", obj);
	decoder->add<AttachRectF, double, double, double, double, double>(U"rect", obj);
	decoder->add<AttachRectF, double, double, double, double>(U"rect", obj);
	//triangleをアタッチ
	decoder->add<AttachTriangle, double, double, double, double, double, double, double, double, double, double, double, double, double>(U"triangle", obj);
	decoder->add<AttachTriangle, double, double, double, double, double, double, double, double, double, double, double>(U"triangle", obj);
	decoder->add<AttachTriangle, double, double, double, double, double, double, double, double, double, double>(U"triangle", obj);
	decoder->add<AttachTriangle, double, double, double, double, double, double, double, double, double>(U"triangle", obj);
	decoder->add<AttachTriangle, double, double, double, double, double, double, double, double>(U"triangle", obj);
	decoder->add<AttachTriangle, double, double, double, double, double, double,double>(U"triangle", obj);
	decoder->add<AttachTriangle, double, double, double, double, double, double>(U"triangle", obj);
	decoder->add<AttachTriangle, double, double, double, double, double>(U"triangle", obj);
	//circleをアタッチ
	decoder->add<AttachCircle, double, double, double, double, double, double, double, double, double, double, double>(U"circle", obj);
	decoder->add<AttachCircle, double, double, double, double, double, double, double, double, double>(U"circle", obj);
	decoder->add<AttachCircle, double, double, double, double, double, double, double, double>(U"circle", obj);
	decoder->add<AttachCircle, double, double, double, double, double, double, double>(U"circle", obj);
	decoder->add<AttachCircle, double, double, double, double, double, double>(U"circle", obj);
	decoder->add<AttachCircle, double, double, double, double, double>(U"circle", obj);
	decoder->add<AttachCircle, double, double, double, double>(U"circle", obj);
	decoder->add<AttachCircle, double, double, double>(U"circle", obj);
}

void DecoderSet::registerMakePartsCmd(mot::PartsManager* pmana,bool createHitbox, const EventFunction<mot::MakeParts>& e)
{
	using namespace mot;

	decoder->add_event_cmd<MakeParts, String, String, double, double>(U"mkpar",e, pmana,createHitbox);
	decoder->add_event_cmd<MakeParts, String, String, double, double, double, double>(U"mkpar",e, pmana,createHitbox);
	decoder->add_event_cmd<MakeParts, String, String, double, double, double, double,double>(U"mkpar",e, pmana, createHitbox);
	decoder->add_event_cmd< MakeParts, String, String, double, double, double>(U"mkpar",e, pmana,createHitbox);
}

void DecoderSet::registerMakePartsCmd(mot::PartsManager* pmanager, MakePartsPostProcessing processing, const EventFunction<mot::MakeParts>& e)
{
	using namespace mot;
	decoder->add_original<MakeParts, String, String, double, double>(U"mkpar",e, processing);
	decoder->add_original<MakeParts, String, String, double, double, double, double>(U"mkpar",e, processing);
	decoder->add_original<MakeParts, String, String, double, double, double, double, double>(U"mkpar",e, processing);
	decoder->add_original<MakeParts, String, String, double, double, double>(U"mkpar", e,processing);
}
