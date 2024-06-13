#include "../stdafx.h"
#include "BodyEditor.h"
#include"Motion.h"
#include"../Game/UI.h"
#include"../Game/Utilities.h"
#include"../Tools/ObjScript.h"
#include"../UI/UIProducts.h"

/*
* 目的
	*キャラクターを実際に3D空間に配置して、より実際のゲームに近い状況を用意する。
* 機能
	* キャラクターの大きさ
	* キャラクターの当たり判定
	* モーション実行
	* モーション中に生成される当たり判定の確認
	* よりゲームに忠実な操作を用意
	* 複数オブジェクトの配置など
* 操作
	* 初期スケールの設定
	* 当たり判定の設置
	* ファイル出力
	* オブジェクトを指定
	* モーションを実行
	* リロード
	* オブジェクトの生成
*
*/

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

using namespace BunchoUI;

class bodyEditor::Impl
{
public:
	Borrow<Object> editor;
	Borrow<my::Scene> scene;

	double h = 70;
	double w = 500;
	const double downOffset = 15;

	Borrow<Object> window;

	Impl(const Borrow<Object>& e):editor(e){
		scene = editor->scene;
	}

	void start()
	{
		auto dm = scene->getDrawManager();

		window = scene->birth();

		auto canvas = window->addComponent<UICanvas>(dm);

		auto& manager = canvas->uiManager;

		auto rect = RectUI::Create({
			.size = SizeF{100,100},
			.color = Palette::Red,
			.relative = Relative::TopLeft()
		});

		manager.setChildren(
			{
				RectUI::Create({
					.size = SizeF{100,100},
					.color = Palette::Red,
					.margine = Margin(0.1,0.1,0.3,0.3),
				}),
			}
		);
	}

	void update(double dt)
	{

	}
};

void bodyEditor::BodyEditor::start()
{
	Object::start();
	impl = new Impl(*this);
	impl->start();
}

void bodyEditor::BodyEditor::update(double dt)
{
	Object::update(dt);
	impl->update(dt);
}
