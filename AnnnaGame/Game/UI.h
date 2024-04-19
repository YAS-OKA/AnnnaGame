#pragma once
#include"Object.h"
#include"../EC.hpp"
#include"../Component/Draw.h"
#include"Utilities.h"

namespace my {
	class Scene;
}

namespace ui
{
	/*class Card :public Object
	{
	public:
		Object* cards[4];

		String assets[4]{ U"カード裏.png",U"カード裏.png",U"カード裏.png",U"カード裏.png" };

		~Card();

		virtual void start()override;
	};*/

	class ProgressBar :public Object
	{
		double m_rate = 0;
		double m_w=0;
		std::variant<DrawRectF*, Draw2D<RoundRect>*> rect;
		IDraw2D* back;
	public:
		Array<std::pair<double, ColorF>> m_barColors = {
			{ 1.0, ColorF(0.1, 0.8, 0.2) }
		};

		ProgressBar() = default;

		void setting(const Vec3& pos, double w, double h, double round = 0.0);

		void setting(const Vec3& pos, double w, double h, const ColorF& backgroundColor, const ColorF& barColor, double round = 0.0);

		void setting(const Vec3& pos, double w, double h, const ColorF& backgroundColor, const Array<std::pair<double, ColorF>>& barColors, double round = 0.0);

		double rate(double r);

		double rate()const;

		bool visible(bool visible);

		bool visible()const;
	};

	template <class Draw>
	Draw* makeUiLike(Draw* draw_class,bool drawSurface = true)
	{
		draw_class->dManagerInfluence->value.SetInfluence(0, 0, 0);
		if (drawSurface)draw_class->shallow->layer = 10;
		return draw_class;
	}
	//左上から右下まで
	enum class Arrangement
	{
		tl,
		top,
		tr,
		left,
		center,
		right,
		bl,
		bottom,
		br,
	};


	class TextBox :public Object
	{
	public:
		using DrawFrame = Draw2D<Frame<RectF>>;

		Object* textObject;
		DrawRectF* box;
		DrawFont* font;
		DrawFrame* frame;
		double x_margin = 2;
		double y_margin = 2;

		//virtual ~TextBox();

		TextBox* setting( const String& text, const int32& fontSize, const Vec2& pos, double w, double h, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

		TextBox* setting( const String& text, const int32& fontSize, const Vec2& pos, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

		TextBox* setting( const String& text, const String& assetName, const Vec2& pos, double w, double h, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

		TextBox* setting( const String& text, const String& assetName, const Vec2& pos, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

		Arrangement arrange=Arrangement::center;

		void setText(const String& text);

		void fitSize();

		void start()override;

		void update(double dt)override;
	};

	TextBox* createTextBox(my::Scene* scene,const String& text, const int32& fontSize, const Vec2& pos, double w, double h, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

	TextBox* createTextBox(my::Scene* scene, const String& text, const int32& fontSize, const Vec2& pos, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

	TextBox* createTextBox(my::Scene* scene, const String& text, const String& assetName, const Vec2& pos, double w, double h, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

	TextBox* createTextBox(my::Scene* scene, const String& text, const String& assetName , const Vec2& pos, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

	class InputBox :public Object
	{
	private:
		bool active = false;
		//棒の場所
		Vec2 barPos;
		//棒が指す行と列の場所
		Point barPoint;
		//カーソルの場所
		int32 cursorPos = 0;
		//範囲入力 通常はs=e=cursorPos 必ずs<=e
		int32 editS;
		int32 editE;
		//begin~endまでの文字をtextに置き換える
		void input(const int32& b, const int32& e, const String& text);
		//begin~endまでの文字を取得
		String getText(int32 b, int32 e)const;

		int32 getCharaNum(Point pos)const;

		void keyInput(const String& text);

		void setBarPosition();
	public:
		bool canInput = true;

		TextBox* box;

		//virtual ~InputBox();
		bool isActive()const;

		void start()override;

		void update(double dt)override;
	};

	InputBox* createInputBox(my::Scene* scene, const int32& fontSize, const Vec2& pos, double w, double h, const String& text = U"", const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

	InputBox* createInputBox(my::Scene* scene, const String& assetName, const Vec2& pos, double w, double h, const String& text = U"", const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

	class SimpleInputArea:public IDraw2D
	{
	public:
		bool canInput = true;
		double h, w;

		mutable TextAreaEditState tex;

		SimpleInputArea(DrawManager* manager, double w, double h, const String& text = U"");

		bool isActive();

		String getText()const;

		void draw()const override;
	};

	SimpleInputArea* createSimpleInputArea(my::Scene* scene, const Vec2& pos, double w, double h, const String& text = U"");

	class SimpleInputBox :public IDraw2D
	{
	public:
		bool canInput = true;

		double w;

		mutable TextEditState tex;

		util::MouseObject* mouse=nullptr;

		SimpleInputBox(DrawManager* manager,double w, const String& text = U"");

		bool isEditing()const;

		String getText()const;

		void draw()const override;
	};

	SimpleInputBox* createSimpleInputBox(my::Scene* scene, const Vec2& pos, double w, const String& text = U"");

	class Button :public Object
	{
	private:
		bool flag;
	public:
		Collider* collider;

		bool active = true;

		TextBox* box;

		void setText(const String& text);

		void start()override;

		void update(double dt)override;

		bool pushed();
	};

	Button* createButton(my::Scene* scene, const String& text, const int32& fontSize, const Vec2& pos, double w, double h, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

	Button* createButton(my::Scene* scene, const String& text, const int32& fontSize, const Vec2& pos, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

	Button* createButton(my::Scene* scene, const String& text, const String& assetName, const Vec2& pos, double w, double h, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

	Button* createButton(my::Scene* scene, const String& text, const String& assetName, const Vec2& pos, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

}
