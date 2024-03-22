#include "../stdafx.h"
#include "UI.h"
#include"Scenes.h"
#include"../Component/Transform.h"

namespace ui
{
	Card::~Card()
	{
		for (auto& card : cards)card->die();
	}

	void Card::start()
	{
		Object::start();

		for (auto& card : cards)card = scene->birthObjectNonHitbox();

		Rect rect{ 0,0,90,130 };
		//ドローマネージャー
		auto dm = scene->getDrawManager();

		int32 h, w;
		h = 145;
		w = 100;
		Vec2 pos4[4] = { {0,h},{0,-h},{w,0},{-w,0} };

		for (int32 i = 0; i < sizeof(cards) / sizeof(Object*); ++i)
		{
			auto& card = cards[i];
			card->transform->setParent(transform);
			card->transform->setLocalPos({ pos4[i], 0 });
			auto tex = card->addComponent<DrawTexture>(dm, assets[i]);
			tex->aspect.setScale(0.4);
		}

		transform->scale.setScale(0.6);

		transform->setPos({ Scene::Width() - 200, 500 ,0 });
	}

	/*TextBox::~TextBox()
	{
		textObject->die();
	}*/

	void TextBox::fitSize()
	{
		const auto& tex = font->drawing(font->text);
		RectF rect = tex.region();
		rect.w += x_margin*2;
		rect.h += y_margin*2;
		box->drawing = rect;
	}

	TextBox* TextBox::setting(const String& text, const int32& fontSize, const Vec2& pos, double w, double h, const ColorF& fontColor, const ColorF& boxColor)
	{
		font->drawing = Font{ fontSize };
		font->text = text;
		transform->setPos({ pos ,0 });
		box->drawing = RectF{ 0,0,w,h };
		font->color = fontColor;
		box->color = boxColor;
		return this;
	}

	TextBox* TextBox::setting(const String& text, const int32& fontSize, const Vec2& pos, const ColorF& fontColor, const ColorF& boxColor)
	{
		TextBox::setting(text, fontSize, pos, 0, 0, fontColor, boxColor);
		fitSize();
		return this;
	}

	TextBox* TextBox::setting(const String& text, const String& assetName, const Vec2& pos, double w, double h, const ColorF& fontColor, const ColorF& boxColor)
	{
		TextBox::setting(text, 0, pos, w, h, fontColor, boxColor);
		font->drawing = FontAsset(assetName);
		return this;
	}

	TextBox* TextBox::setting(const String& text, const String& assetName, const Vec2& pos, const ColorF& fontColor, const ColorF& boxColor)
	{
		TextBox::setting(text, assetName, pos, 0, 0, fontColor, boxColor);
		fitSize();
		return this;
	}

	void TextBox::setText(const String& text)
	{
		font->text = text;
	}

	void TextBox::start()
	{
		Object::start();

		textObject = scene->birthObjectNonHitbox<Object>();
		textObject->transform->setParent(transform);
		setSameDestiny(textObject);
		auto draw_manager = scene->getDrawManager();
		box = makeUiLike(addComponent<DrawRectF>(draw_manager, RectF{}));

		font = makeUiLike(textObject->addComponent<DrawFont>(draw_manager, Font(20)));

		font->color = Palette::White;

		frame = makeUiLike(addComponent<DrawFrame>(scene->getDrawManager(),RectF{},3));

		frame->color = Palette::Gray;

		frame->relative.z = box->relative.z - 0.5;
	}

	void TextBox::update(double dt)
	{
		Object::update(dt);

		font->viewport = box->getDrawing();
		const auto& tex = font->drawing(font->text);
		const auto& rect = box->drawing;
		double w = tex.region(0, 0).w;
		double h = tex.region(0, 0).h;

		frame->drawing.figure = box->drawing;

		switch (arrange)
		{
		case ui::Arrangement::tl:
			font->transform->setLocalPos({ 0,0,0 });
			break;
		case ui::Arrangement::top:
			font->transform->setLocalPos({ (rect.w - w) / 2,0 ,0 });
			break;
		case ui::Arrangement::tr:
			font->transform->setLocalPos({ rect.w - w,0 ,0 });
			break;
		case ui::Arrangement::left:
			font->transform->setLocalPos({ 0,(rect.h - h) / 2 ,0 });
			break;
		case ui::Arrangement::center:
			font->transform->setLocalPos({ (rect.w - w) / 2,(rect.h - h) / 2  ,0 });
			break;
		case ui::Arrangement::right:
			font->transform->setLocalPos({ rect.w - w, (rect.h - h) / 2,0 });
			break;
		case ui::Arrangement::bl:
			font->transform->setLocalPos({ 0,rect.h - h ,0 });
			break;
		case ui::Arrangement::bottom:
			font->transform->setLocalPos({ (rect.w - w) / 2 ,rect.h - h ,0 });
			break;
		case ui::Arrangement::br:
			font->transform->setLocalPos({ rect.w - w,rect.h - h ,0 });
			break;
		default:
			break;
		}
		//箱より上に描く
		font->transform->moveBy({ x_margin,y_margin,-1 });
	}

	TextBox* createTextBox(my::Scene* scene, const String& text, const int32& fontSize, const Vec2& pos, double w, double h, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto textbox = scene->birthObjectNonHitbox<TextBox>();
		textbox->setting(text, fontSize, pos, w, h, fontColor, boxColor);
		return textbox;
	}

	TextBox* createTextBox(my::Scene* scene, const String& text, const int32& fontSize, const Vec2& pos, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto textbox = scene->birthObjectNonHitbox<TextBox>();
		textbox->setting(text, fontSize, pos, fontColor, boxColor);
		return textbox;
	}

	TextBox* createTextBox(my::Scene* scene, const String& text, const String& assetName, const Vec2& pos, double w, double h, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto textbox = scene->birthObjectNonHitbox<TextBox>();
		textbox->setting(text, assetName, pos, w, h, fontColor, boxColor);
		return textbox;
	}

	TextBox* createTextBox(my::Scene* scene, const String& text, const String& assetName, const Vec2& pos, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto textbox = scene->birthObjectNonHitbox<TextBox>();
		textbox->setting(text, assetName, pos, fontColor, boxColor);
		return textbox;
	}

	void InputBox::input(const int32& start, const int32& end, const String& t)
	{
		auto& text = box->font->text;
		auto b = start;
		auto e = end;

		if (b > e) std::swap(b, e);
		b = std::clamp(b, 0, int32(text.size()));
		e = std::clamp(e, 0, int32(text.size()));

		text.insert(e, t);
		text.erase(text.begin() + b, text.begin() + e);
		//cursorPosの更新
		if (e <= cursorPos and b < cursorPos)cursorPos -= e - b;

		cursorPos += t.size()-(e - b);
	}

	String InputBox::getText(int32 b, int32 e)const
	{
		String result=U"";
		auto& text = box->font->text;

		if (b > e) std::swap(b, e);
		b = std::clamp(b, 0, int32(text.size()));
		e = std::clamp(e, 0, int32(text.size()));

		for (int32 i = b; i < e; i++)
		{
			//選択範囲内の文字は返さない
			if (i < editS or editE <= i)result << text[i];
		}

		return result;
	}

	void InputBox::keyInput(const String& text)
	{
		double base_height = box->font->drawing.height();

		Array<String> tmp = box->font->text.split(U'\n');
		if (tmp.isEmpty())tmp << U"";
		//barPos barPointの更新
		for (int32 i=0;i<text.size();++i)
		{
			const auto& s = text[i];
			if (s == U'\n') {
				tmp[barPoint.x].insert(barPoint.y, U"\n");
				auto tmp2 = tmp[barPoint.x].split(U'\n');
				if (tmp2.isEmpty()) tmp2 = { U"",U"" };
				if (tmp2.size() == 1)tmp2 << U"";
				tmp[barPoint.x] = tmp2[1];
				tmp.insert(tmp.begin() + barPoint.x, tmp2[0]);
				barPos.y += base_height;
				barPos.x = box->font->transform->getPos().x;
				barPoint.y = 0;
				++barPoint.x;
			}
			else if (s == U'\b' and not (barPoint.y == 0 and barPoint.x == 0))
			{
				//端でバックスペースを押したか
				if (barPoint.y == 0)
				{
					barPos.y -= base_height;
					barPos.x = box->font->drawing(tmp[barPoint.x-1]).getXAdvances().sum();

					tmp[barPoint.x - 1] += tmp[barPoint.x];
					tmp.erase(tmp.begin() + barPoint.x);

					--barPoint.x;
					barPoint.y = tmp[barPoint.x].size()-1;
				}
				else {
					auto pop = tmp[barPoint.x][barPoint.y - 1];//消される文字を保持

					tmp[barPoint.x].erase(tmp[barPoint.x].begin() + barPoint.y-1);//消す
				
					barPos.x -= box->font->drawing(pop).getXAdvances()[0];//消した分減らす

					--barPoint.y;
				}
			}
			else {
				tmp[barPoint.x].insert(barPoint.y, Format(s));
				barPos.x += box->font->drawing(s).getXAdvances()[0];
				++barPoint.y;
			}
		}
		//tmp[barPoint.x].insert(barPoint.y, text);

		//textの更新
		box->font->text = U"";
		for (const auto& s : tmp)
		{
			box->font->text.append(s + U'\n');
		}
		box->font->text.pop_back();
	}

	int32 InputBox::getCharaNum(Point pos)const
	{
		const auto& splitedText = box->font->text.split(U'\n');
		int32 result = 0;
		for (int32 i = 0; i < pos.y-1; ++i)
		{
			//+1は改行の分
			result += splitedText[i].size()+1;
		}
		result += pos.x;
		return result;
	}

	void InputBox::setBarPosition()
	{
		const auto& pos = Cursor::Pos();
		const auto& region = box->font->drawing(box->font->text).region();
		double base_height = box->font->drawing.height();
		//カーソルに最も近い行を求める
		int32 row =
			Max(
				1,
				Min(
					int32(box->font->text.split(U'\n').size()),
					int32((pos.y - box->font->transform->getPos().y) / base_height)+1
				)
			);
		//一番左のｘ距離
		double tmp = box->font->transform->getPos().x;
		double distance = (pos.x - tmp) * (pos.x - tmp);
		//barPosを行の一番左側にセット
		barPos = box->font->transform->getPos().xy() + Vec2{ 0,base_height * (row - 1) };
		barPoint = Point{ row - 1,0 };//行,列
		if (box->font->text.isEmpty())return;//空だったらここで終了
		for (auto p : box->font->drawing(box->font->text.split(U'\n')[row - 1]).getXAdvances())
		{
			tmp += p;
			double next_distance = (tmp - pos.x)* (tmp - pos.x);
			//x距離が遠くなったら
			if (distance < next_distance)
			{
				return;
			}
			//x距離の更新とbarPos,barPointの更新
			distance = next_distance;
			barPos.x += p;
			++barPoint.y;
		}

		cursorPos = getCharaNum(barPoint);
	}

	void InputBox::start()
	{
		Object::start();

		box = scene->birthObjectNonHitbox<TextBox>();

		setSameDestiny(box);
		//左揃え
		box->arrange = Arrangement::tl;

		transform->setParent(box->transform);
		//w:h=1:10の長方形
		auto bar = makeUiLike(addComponent<DrawRectF>(scene->getDrawManager(), RectF{ 0,0,1,10 }));

		bar->visible = false;

		bar->color = Palette::Black;

		//キー入力 入力したら発動
		ACreate(U"input", false, true)
			.startIf([=] {return isActive(); })
			.endIf([=] {return not isActive(); });
		actman[U"input"].add(
			[=]{
				Print << cursorPos;
				const String& pre = getText(cursorPos - 1, cursorPos + 1);
				String t = pre;
				TextInput::UpdateText(t, t.size() > 0 ? 1 : 0);
				if (t == pre)return;

				if (editS != editE)
				{
					input(editS, editE, U"");
					//eraseかdeleteを押してなかったら
					if (t.size() > pre.size()) {
						input(cursorPos - 1, cursorPos + 1, t);
					};					
					editS = editE;
				}
				else {
					input(cursorPos - 1, cursorPos + 1, t);
				}
			}
		);
		//棒の点滅アクション  入力できる設定&テキストボックスの幅が0より大きい&入力待ち状態　なら開始
		ACreate(U"barFlashing", false, true)
			.startIf([=] {return isActive(); })
			.endIf([=] {return not isActive(); });
		//棒を0.5秒表示
		actman[U"barFlashing"].add(
			[=] {
				//棒を表示
				bar->visible = true;
				transform->setPos({ barPos,box->transform->getPos().z - 1 });
			}, [=](double dt) {
				double base_height = box->font->drawing.height();
				bar->aspect.setScale(base_height / bar->drawing.h * 0.95);
				transform->setPos({ barPos,box->transform->getPos().z - 1 });
			}, [=] {
				//棒を非表示
				bar->visible = false;
			}, 0.5
		);
		actman[U"barFlashing"].add<prg::Wait>(0.4);
	}

	bool InputBox::isActive()const
	{
		return canInput and box->box->drawing.w > 0 and active;
	}

	void InputBox::update(double dt)
	{
		Object::update(dt);

		//箱をクリックしたら入力待ち状態
		auto fig = box->box->getDrawing();
		if (fig.leftClicked())
		{
			active = true;

			setBarPosition();
		}
	}

	InputBox* createInputBox(my::Scene* scene, const int32& fontSize, const Vec2& pos, double w, double h, const String& text, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto inputbox = scene->birthObjectNonHitbox<InputBox>();
		inputbox->box->setting(text, fontSize, pos, w, h, fontColor, boxColor);
		return inputbox;
	}

	InputBox* createInputBox(my::Scene* scene, const String& assetName, const Vec2& pos, double w, double h, const String& text, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto inputbox = scene->birthObjectNonHitbox<InputBox>();
		inputbox->box->setting(text, assetName, pos, w, h, fontColor, boxColor);
		return inputbox;
	}

	SimpleInputArea::SimpleInputArea(DrawManager* manager,double w, double h, const String& text)
		:w(w),h(h),IDraw2D(manager)
	{
		tex.text = text;
	}

	bool SimpleInputArea::isActive()
	{
		return tex.active;
	}

	String SimpleInputArea::getText()const
	{
		return tex.text;
	}

	void SimpleInputArea::draw()const
	{
		const Transformer2D t1(getTransformer());

		SimpleGUI::TextArea(tex, {0,0}, {w,h}, 50000, canInput);
	}

	SimpleInputArea* createSimpleInputArea(my::Scene* scene, const Vec2& pos, double w, double h, const String& text)
	{
		auto area = scene->birthObject(RectF{ 0,0,w,h }, { 0,0,0 });

		area->transform->setPos({ pos,0 });

		return makeUiLike(area->addComponent<SimpleInputArea>(scene->getDrawManager(), w, h, text));
	}

	SimpleInputBox::SimpleInputBox(DrawManager* manager, double w, const String& text)
		:w(w),IDraw2D(manager)
	{
		tex.text = text;
	}

	String SimpleInputBox::getText()const
	{
		return tex.text;
	}

	bool SimpleInputBox::isEditing()const
	{
		return tex.active;
	}

	void SimpleInputBox::draw()const
	{
		const Transformer2D t1(IDraw2D::getTransformer());

		SimpleGUI::TextBox(tex, { 0,0 }, w, none, canInput);
	}

	SimpleInputBox* createSimpleInputBox(my::Scene* scene, const Vec2& pos, double w, const String& text)
	{
		auto area = scene->birthObject(RectF{ 0,0,w,40 }, { 0,0,0 });

		area->transform->setPos({ pos,0 });

		return makeUiLike(area->addComponent<SimpleInputBox>(scene->getDrawManager(), w, text));
	}

	void Button::setText(const String& text)
	{
		box->setText(text);
	}

	void Button::start()
	{
		Object::start();

		box = scene->birthObject<TextBox>(RectF{}, { 0,0,0 });

		setSameDestiny(box);

		box->name = U"Button";

		//box->box->visible = false;

		box->transform->setParent(transform);

		box->arrange = Arrangement::center;

		collider = box->getComponent<Collider>();
	}

	void Button::update(double dt)
	{
		Object::update(dt);
		flag = false;

		if (not active)return;

		auto fig = box->box->getDrawing();

		std::get<2>(collider->hitbox.shape).setWH(fig.w, fig.h);
		
		if (fig.leftClicked()) {
			flag = true;
		}
	}

	bool Button::pushed()
	{
		return flag;
	}

	Button* createButton(my::Scene* scene, const String& text, const int32& fontSize, const Vec2& pos, double w, double h, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto button = scene->birthObjectNonHitbox<Button>();
		button->transform->setPos({ pos, 0 });
		button->box->setting(text, fontSize, pos, w, h, fontColor, boxColor);
		return button;
	}

	Button* createButton(my::Scene* scene, const String& text, const int32& fontSize, const Vec2& pos, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto button = scene->birthObjectNonHitbox<Button>();
		button->transform->setPos({ pos, 0 });
		button->box->setting(text, fontSize, pos, fontColor, boxColor);
		return button;
	}

	Button* createButton(my::Scene* scene, const String& text, const String& assetName, const Vec2& pos, double w, double h, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto button = scene->birthObjectNonHitbox<Button>();
		button->transform->setPos({ pos, 0 });
		button->box->setting(text, assetName, pos, w, h, fontColor, boxColor);
		return button;
	}

	Button* createButton(my::Scene* scene, const String& text, const String& assetName, const Vec2& pos, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto button = scene->birthObjectNonHitbox<Button>();
		button->transform->setPos({ pos, 0 });
		button->box->setting(text, assetName, pos, fontColor, boxColor);
		return button;
	}
}
