#include "../stdafx.h"
#include "MotionEditor.h"
#include"Motion.h"
#include"../Game/UI.h"
#include"../Game/Utilities.h"
#include"../Tools/ObjScript.h"

namespace motionEditor
{

	using Event = std::function<void()>;
	template<class... Args>
	ui::Button* ButtonEvent(Object* act_owner, const String& eventName, Array<Event> events, Args&& ...args)
	{
		auto button = ui::createButton(act_owner->scene, args...);
		button->name = eventName + U"Button";
		//eventの数によって条件を付ける場所を変える こうするとsizeが1のときショートカット実行がやりやすい
		if (events.size() == 1)
		{
			auto& act = act_owner->ACreate(eventName);
			//ボタンが押され、かつそのボタンが前面にあるなら
			act.startIf<prg::ButtonChecker>(button);
			act.add(events[0]);
		}
		else {
			auto& act = act_owner->ACreate(eventName, true, true);

			for (const auto& e : events)
			{
				//ボタンが押され、かつそのボタンが前面にあるなら
				act.add(e).startIf<prg::ButtonChecker>(button);
			}
		}
		return button;
	}

	template<class... Args>
	ui::Button* ButtonEvent(Object* act_owner, const String& eventName, const Event& event, Args&& ...args)
	{
		return ButtonEvent(act_owner, eventName, Array<Event>{ event }, args...);
	}
}

class motionEditor::Impl
{
public:
	Object* editor;
	my::Scene* scene;

	double h = 70;
	double w = 500;
	const double downOffset = 15;

	Object* window;

	Impl(Object* e):editor(e){
		scene = editor->scene;
	}

	void start()
	{
		
		auto dm = scene->getDrawManager();

		window=tool::ObjScript(scene).Load(U"asset/ui/test/test.txt",U"test");

		motionEditor::ButtonEvent(editor, U"load",[=] {
			window->die();
			window = tool::ObjScript(scene).Load(U"asset/ui/test/test.txt", U"test");
		}, U"ロード", 20, Vec2{ util::sw() - 100,20});
	}

	void update(double dt)
	{

	}
};

void motionEditor::MotionEditor::start()
{
	Object::start();
	impl = new Impl(this);
	impl->start();
}

void motionEditor::MotionEditor::update(double dt)
{
	Object::update(dt);
	impl->update(dt);
}
