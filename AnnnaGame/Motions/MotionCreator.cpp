#include "../stdafx.h"
#include "MotionCreator.h"
#include"../Component/Transform.h"
#include"../Game/Object.h"
#include"../Game/UI.h"
#include"Parts.h"
#include"../Game/Utilities.h"
#include"../Util/Cmd.hpp"
#include"../Util/CmdDecoder.h"
#include"MotionCmd.h"
#include"../Component/Draw.h"
#include"Motion.h"

namespace {
	constexpr auto UiZvalue = 0;//-100000;
	constexpr auto CameraZvalue = -200000;
	constexpr double UiPriority = Math::Inf;
	util::MouseObject* mouse = nullptr;
	Entity* clickedSurfaceUiEntity = nullptr;
	Entity* clickedSurfaceParts = nullptr;
	Array<Collider*>uiCollider;//ui
	Array<Collider*>partsCollider;//parts
	Array<Collider*>pointCollider;//rotatePoint/scalePoint
	mot::PartsManager* pmanager = nullptr;
	mot::Parts* selecting=nullptr;
	mot::Parts* grabbing=nullptr;
}

namespace mot
{
	class DrawPartsFrame :public IDraw2D
	{
	private:
		Parts* parts;
	public:

		DrawPartsFrame(DrawManager* manager, Parts* parts)
			:IDraw2D(manager), parts(parts)
		{
		}

		void draw()const override
		{
			std::get<2>(parts->collider->hitbox.shape).getScaledFig().drawFrame(2, color);
		}
	};
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
			act.startIf<prg::ButtonChecker>(button)
				.andStartIf([=] {return button->collider->owner == clickedSurfaceUiEntity;});
			act.add(events[0]);
		}
		else {
			auto& act = act_owner->ACreate(eventName, true, true);

			for (const auto& e : events)
			{
				//ボタンが押され、かつそのボタンが前面にあるなら
				act.add(e)
					.startIf<prg::ButtonChecker>(button)
					.andStartIf([=] {return button->collider->owner == clickedSurfaceUiEntity; });
			}
		}

		button->box->transform->setZ(UiZvalue - CameraZvalue);

		uiCollider << button->collider;

		//Buttonを先にアップデートしないとclickedSurfaceUiEntityがnullptrにされちゃうよ		そんなことない？
		button->priority.setPriority(UiPriority);

		return button;
	}

	template<class... Args>
	ui::Button* ButtonEvent(Object* act_owner, const String& eventName, const Event& event, Args&& ...args)
	{
		return ButtonEvent(act_owner, eventName, Array<Event>{ event }, args...);
	}

	class PartsDetailWindow:public Object
	{
	private:
		void createTextBoxs(DrawManager* dm,double box_w)
		{
			for (const auto& elems : all_params_key)
			{
				for (const auto& elem : elems)
				{
					params[elem] = ui::createSimpleInputBox(scene, { 0,0 }, box_w / elems.size());
					params[elem]->transform->setParent(transform);

					auto f = ui::makeUiLike(params[elem]->owner->addComponent<DrawFont>(dm, 16));
					f->text = elem;
					f->color = Palette::Black;
					f->relative = { -f->getEndX() - 5,5,0 };

					uiCollider << params[elem]->owner->getComponent<Collider>();
				}
			}
		}

		std::tuple<double,double> textBoxSetting(DrawManager* dm,double init_x,double init_y,double w,double box_w)
		{
			const double inter = 40;
			double x = init_x;
			double y = init_y;
			for (const auto& elems : all_params_key)for (const auto& elem : elems)
			{
				params[elem]->owner->getComponent<DrawFont>()->visible = false;
				params[elem]->visible = false;
			}

			for (const auto& elems : params_key)
			{
				x = init_x;
				for (const auto& elem : elems)
				{
					double size = elems.size();
					auto f = params[elem]->owner->getComponent<DrawFont>();
					x -= f->relative.x;
					params[elem]->transform->setLocalPos({ x,y,-1});
					params[elem]->owner->getComponent<DrawFont>()->visible = true;
					params[elem]->visible = true;
					x += box_w / size + 10;
				}
				y += inter;
			}
			return { x,y };
		}

		enum class PartsType {
			master,
			texture,
			rect,
			triangle,
			circle,
		};
		void partsParamsSetting(PartsType type){
			this->type = type;
			switch (type)
			{
			case PartsType::master:
				params_key = all_params_key.filter([=](Array<String> p) {
					return not(
						p == Array<String>{U"W", U"H"}
					or p == Array<String>{U"Radius"}
					or p == Array<String>{U"Deg"}
					or p == Array<String>{U"Len1", U"Len2"}
					or p == Array<String>{U"Texture"});
					});
				break;
			case PartsType::texture:
				params_key = all_params_key.filter([=](Array<String> p) {
					return not(
						p == Array<String>{U"W", U"H"}
					or p == Array<String>{U"Radius"}
					or p == Array<String>{U"Deg"}
					or p == Array<String>{U"Len1", U"Len2"});
					});
				break;
			case PartsType::rect:
				params_key = all_params_key.filter([=](Array<String> p) {
					return not(
						p == Array<String>{U"Texture"}
					or p == Array<String>{U"Radius"}
					or p == Array<String>{U"Deg"}
					or p == Array<String>{U"Len1", U"Len2"});
				});
				break;
			case PartsType::triangle:
				params_key = all_params_key.filter([=](Array<String> p) {
					return not(
						p == Array<String>{U"Texture"}
					or p == Array<String>{U"Radius"}
					or p == Array<String>{U"W",U"H"});
				});
				break;
			case PartsType::circle:
				params_key = all_params_key.filter([=](Array<String> p) {
					return not(
						p == Array<String>{U"Texture"}
					or p == Array<String>{U"W", U"H"}
					or p == Array<String>{U"Deg"}
					or p == Array<String>{U"Len1", U"Len2"});
				});
				break;
			}			
		}

		void windowSetting(DrawManager* dm)
		{
			//画面の構成
			auto [x, y] = textBoxSetting(dm, init_x, init_y, w, box_w);

			window->drawing.w = w;
			window->drawing.h = y;

			std::get<2>(windowCollider->hitbox.shape) = RectF{ 0,0,w,y };

			fitButton->transform->setLocalPos({ init_x, y + 10, 0 });
		}
	public:
		bool visible=true;
		HashTable<String, ui::SimpleInputBox*> params;
		Array<Array<String>> all_params_key{
			{U"Name"},
			{U"Parent"},
			{U"Texture"},
			{U"W",U"H"},
			{U"Deg"},
			{U"Len1",U"Len2" },
			{U"Radius"},
			{U"X"},
			{U"Y"},
			{U"Z"},
			{U"Angle"},
			{U"RP X",U"RP Y"},
			{U"S X",U"S Y"},
			{U"SP X",U"SP Y"},
			{U"R",U"G",U"B",U"A"}
		};
		Array<Array<String>> params_key;
		Object* frame = nullptr;
		Object* rotatePoint = nullptr;
		Object* scalePoint = nullptr;
		prg::IAction* selectAct;
		PartsType type;

		String getText(const String& key) {
			return params[key]->getText();
		}

		void createPartsFrame()
		{
			frame = scene->birthObjectNonHitbox();
			auto tmp = frame->addComponent<DrawPartsFrame>(scene->getDrawManager(), selecting);
			tmp->transform->setPos(selecting->transform->getPos());
			tmp->transform->setParent(selecting->transform);
			tmp->color = Palette::Cyan;
		}

		void createRotatePoint()
		{
			rotatePoint = scene->birthObjectNonHitbox();
			const double r = 6;
			auto tmp = rotatePoint->addComponent<DrawCircle>(scene->getDrawManager(), r);
			tmp->color = Palette::Orange;
			auto& act = rotatePoint->ACreate(U"move", true);
			act.add(
				//アドレスのコピー　これをしないとthis->...を参照してしまう
				[rp = rotatePoint, s = selecting](double dt) {
					rp->transform->setPos(
						{ s->getRotatePos().rotated(s->getAbsAngle()*1_deg) + s->getPos() + s->transform->getParent()->getPos().xy(),UiZvalue/2 - 11}
					);
				}
			);
			pointCollider << rotatePoint->addComponent<Collider>(CollideBox::CollideFigure(Circle{ 0,0,r }));
			//つかんだ時の処理
			auto& gr = rotatePoint->ACreate(U"grab");
			std::shared_ptr<Vec2> offset{ new Vec2{0,0} };
			gr.add(
				[=, rp = rotatePoint]
				{
					*offset = rp->transform->getPos().xy() - (mouse->getCursorPos(&rp->getComponent<Field<Influence>>(U"dManagerInfluence")->value)).xy();
				},
				[=, rp = rotatePoint, s = selecting](double dt)
				{
					s->setRotatePos(((mouse->getCursorPos(&rp->getComponent<Field<Influence>>(U"dManagerInfluence")->value)).xy() + *offset - s->transform->getPos().xy()).rotate(-s->getAngle() * 1_deg));
					setText(U"RP X", Format(s->getRotatePos().x));
					setText(U"RP Y", Format(s->getRotatePos().y));
				}
			).endIf([] {return MouseL.up(); });
		}

		void createScalePoint() {
			scalePoint = scene->birthObjectNonHitbox();
			const double r=6;
			auto tmp = scalePoint->addComponent<DrawCircle>(scene->getDrawManager(), r);
			tmp->color = Palette::Green;
			auto& act = scalePoint->ACreate(U"move", true);
			act.add(
				[sp = scalePoint, s = selecting](double dt) {
					sp->transform->setPos(
						{ s->getScalePos().rotated(s->getAbsAngle() * 1_deg) + s->getPos() + s->transform->getParent()->getPos().xy(),UiZvalue/2 - 10 }
					);
				}
			);
			pointCollider << scalePoint->addComponent<Collider>(CollideBox::CollideFigure(Circle{ 0,0,r }));

			auto& gr = scalePoint->ACreate(U"grab");
			std::shared_ptr<Vec2> offset{ new Vec2{0,0} };
			gr.add<prg::FuncAction>(
				[=, sp = scalePoint]
				{
					*offset = sp->transform->getPos().xy() - (mouse->getCursorPos(&sp->getComponent<Field<Influence>>(U"dManagerInfluence")->value)).xy();
				},
				[=, sp = scalePoint, s = selecting](double dt)
				{
					s->setScalePos(((mouse->getCursorPos(&sp->getComponent<Field<Influence>>(U"dManagerInfluence")->value)).xy() + *offset - s->transform->getPos().xy()).rotate(-s->getAngle() * 1_deg));
					setText(U"SP X", Format(s->getScalePos().x));
					setText(U"SP Y", Format(s->getScalePos().y));
				}
			).endIf([] {return MouseL.up(); });
		}

		const double init_x = 10;
		const double init_y = 10;
		const double w = 330;
		const double box_w = 200;
		DrawRectF* window;//自分のコンポーネント
		Collider* windowCollider;
		ui::Button* fitButton;

		void start()override
		{
			Object::start();

			DrawManager* dm = scene->getDrawManager();
			//画面の構成
			window = ui::makeUiLike(addComponent<DrawRectF>(dm, 0, 0, 0, 0));
			windowCollider = addComponent<Collider>(CollideBox::CollideFigure{ RectF{ 0,0,0,0 } }, Vec3{ 0,0,0 });
			uiCollider << windowCollider;

			//textBoxのセット
			createTextBoxs(scene->getDrawManager(), box_w);

			partsParamsSetting(PartsType::master);

			auto button = ButtonEvent(this, U"fitting", [=] {
				if (selecting != nullptr)fit();
			}, U"適応する", 18, Vec2{ 0,0 });

			button->transform->setParent(transform);

			fitButton = button;

			windowSetting(dm);

			//座標をセット カメラを追従するようにする カメラを動かした後zがUiZvalueとなるように-CameraZvalueをおいておく
			transform->setPos({ util::sw() - w,0,UiZvalue });

			//パーツ選択
			auto& selectParts = ACreate(U"selectParts");
			selectParts.add<prg::FuncAction>(
				[=] {
					auto pre = selecting;
					Parts* selectedParts = nullptr;
					for (const auto& parts : pmanager->partsArray)
					{
						if (parts == clickedSurfaceParts)
						{
							selectedParts = parts;
							//選択済みのパーツだったら　つかむ
							if (pre == selectedParts) {
								grabbing = selectedParts;
							}
							break;
						};
					}
					select(selectedParts);
				}
			);

			//パーツを動かす
			auto& act = ACreate(U"move");

			act.startIf([=] {return grabbing != nullptr; });

			std::shared_ptr<Vec2> offset{ new Vec2{0,0} };
			act.add(
				[=]
				{
				*offset = grabbing->getPos() - (mouse->getCursorPos(&grabbing->getComponent<Field<Influence>>(U"dManagerInfluence")->value) - grabbing->transform->getParent()->getPos()).xy();
				},
					[=](double dt, prg::FuncAction*)
				{
					grabbing->setAbsPos(mouse->getCursorPos(&grabbing->getComponent<Field<Influence>>(U"dManagerInfluence")->value).xy() + *offset);/*.rotate(-grabbing->getAngle() * 1_deg));*/
					auto p = grabbing->getPos();
					setText(U"X", Format(p.x));
					setText(U"Y", Format(p.y));
					setText(U"RP X", Format(grabbing->getRotatePos().x));
					setText(U"RP Y", Format(grabbing->getRotatePos().y));
				},
					[=]
				{
					grabbing = nullptr;
				}
			).endIf([] {return MouseL.up(); });
		}

		void update(double dt)override
		{
			//set();

			//アクションをアップデートするより先に
			if (MouseL.down())
			{
				clickedSurfaceUiEntity = nullptr;
				clickedSurfaceUiEntity = mouse->getClickedSurfaceObject(uiCollider);
				if (clickedSurfaceUiEntity == nullptr) {
					//rotatePosとかscalePosとか
					auto ent = mouse->getClickedSurfaceObject(pointCollider);
					if (ent != nullptr) {
						dynamic_cast<Object*>(ent)->startAction(U"grab");
					}
					else {
						clickedSurfaceParts = nullptr;
						clickedSurfaceParts = mouse->getClickedSurfaceObject(partsCollider);
						startAction(U"selectParts");
					}
				}
			}

			if (KeyControl.pressed() and KeyEnter.down())
				startAction(U"fitting");

			Object::update(dt);
		}

		void setText(const String& param,const String& te)
		{
			params[param]->tex.text = te;
		};

		void select(Parts* target)
		{
			selecting = target;
			//フレームなどを描く

			if (frame != nullptr) {
				frame->die();
				frame = nullptr;
			}
			if (rotatePoint != nullptr) {
				rotatePoint->die();
				rotatePoint = nullptr;
			}
			if (scalePoint) {
				scalePoint->die();
				scalePoint = nullptr;
			}

			pointCollider.clear();

			set();

			if (selecting != nullptr)
			{
				//フレームを生成
				if(selecting->collider != nullptr) createPartsFrame();

				//rotatePointの生成
				createRotatePoint();

				//scalePointの生成
				createScalePoint();
			}
		}

		void set()
		{
			for (const auto& param : params)param.second->tex.text = U"";

			if (selecting == nullptr)return;

			setText(U"Name", selecting->getName());
			setText(U"Parent", selecting->getParent());
			if (selecting->params.name == U"master")
			{
				partsParamsSetting(PartsType::master);
			}
			else if (selecting->params.path)
			{
				setText(U"Texture", *selecting->params.path);
				partsParamsSetting(PartsType::texture);
			}
			else
			{
				const auto& fig = std::get<1>(selecting->params.drawing);
				auto index = fig.getIndex();
				if (index == 0)
				{
					//circle
					setText(U"Radius", Format(fig.getCircle().r));
					partsParamsSetting(PartsType::circle);
				}
				else if (index == 1)
				{
					const auto& rect = fig.getRectF();
					setText(U"W", Format(rect.w));
					setText(U"H", Format(rect.h));
					partsParamsSetting(PartsType::rect);
				}
				else if(index==3)
				{
					const auto& tri = fig.getTriangle();
					setText(U"Deg", Format(util::angleOf2Vec(tri.p1 - tri.p0, tri.p2 - tri.p0)/1_deg));
					setText(U"Len1", Format((tri.p0 - tri.p1).length()));
					setText(U"Len2", Format((tri.p0 - tri.p2).length()));
					partsParamsSetting(PartsType::triangle);
				}
			}
			setText(U"Angle", Format(selecting->getAngle()));
			setText(U"X", Format(selecting->getPos().x));
			setText(U"Y", Format(selecting->getPos().y));
			setText(U"Z", Format(selecting->getZ()));
			setText(U"SP X", Format(selecting->getScalePos().x));
			setText(U"SP Y", Format(selecting->getScalePos().y));
			setText(U"S X", Format(selecting->transform->scale.aspect.vec.x));
			setText(U"S Y", Format(selecting->transform->scale.aspect.vec.y));
			setText(U"RP X", Format(selecting->getRotatePos().x));
			setText(U"RP Y", Format(selecting->getRotatePos().y));
			const auto& color = selecting->getColor();
			setText(U"R", Format(color.r));
			setText(U"G", Format(color.g));
			setText(U"B", Format(color.b));
			setText(U"A", Format(color.a));

			windowSetting(scene->getDrawManager());
		}
		/* TODO: 例外処理をつけるため、警告ポップアップを作ろう*/
		void fit()
		{
			if (type == PartsType::texture) {
				//存在しないパスだとバグる　例外処理をつけよう createHitbox
				const auto& path = getText(U"Texture");
				bool changed = false;
				if (selecting->params.path) {
					changed = selecting->params.path != path;
				}
				else {
					changed = not path.isEmpty();
				}
				if (changed) {
					partsCollider.remove(selecting->collider);
					selecting->remove(selecting->collider);
					auto pos = Texture{ AssetManager::myAsset(localPath + path) }.size() / 2;
					
					partsCollider << selecting->createHitbox(
						pos,
						Image{ AssetManager::myAsset(localPath + path) }.alphaToPolygons().rotateAt(pos, selecting->getAbsAngle() *1_deg)
					);

					frame->die();
					createPartsFrame();
				}
				selecting->params.path = path;
				loadPartsTexture(scene, path);
				selecting->setTexture(resource::texture(path));
			}
			else if (type == PartsType::rect)
			{
				Vec2 hw{ Parse<double>(getText(U"W")),Parse<double>(getText(U"H")) };
				RectF rect{ -hw / 2,hw };

				partsCollider.remove(selecting->collider);
				selecting->remove(selecting->collider);
				partsCollider << selecting->createHitbox({ 0,0 }, { rect.asPolygon().rotate(selecting->getAbsAngle() * 1_deg)});

				frame->die();
				createPartsFrame();

				selecting->setTexture(rect);
			}
			else if (type == PartsType::circle)
			{
				double r=Parse<double>(getText(U"Radius"));
				Circle cir{ 0,0,r };

				partsCollider.remove(selecting->collider);
				selecting->remove(selecting->collider);
				partsCollider << selecting->createHitbox({ 0,0 }, { cir.asPolygon() });

				frame->die();
				createPartsFrame();

				selecting->setTexture(cir);
			}
			else if (type == PartsType::triangle)
			{
				double deg = Parse<double>(getText(U"Deg"));
				double l1 = Parse<double>(getText(U"Len1"));
				double l2 = Parse<double>(getText(U"Len2"));
				auto p0 = std::get<1>(selecting->params.drawing).getTriangle().p0;
				Triangle tri{ p0,p0 + util::polar(l1,-deg * 1_deg / 2),p0 + util::polar(l2,deg * 1_deg / 2) };

				partsCollider.remove(selecting->collider);
				selecting->remove(selecting->collider);
				partsCollider << selecting->createHitbox({ 0,0 }, { tri.asPolygon().rotate(selecting->getAbsAngle() * 1_deg) });

				frame->die();
				createPartsFrame();

				selecting->setTexture(tri);
			}

			selecting->setName(params[U"Name"]->getText());

			selecting->setRotatePos({ Parse<double>(params[U"RP X"]->getText()), Parse<double>(params[U"RP Y"]->getText()) });
			selecting->setPos({ Parse<double>(params[U"X"]->getText()),Parse<double>(params[U"Y"]->getText()) });

			selecting->setParent(params[U"Parent"]->getText());
			//parentをセットすると相対座標が変わるので更新
			setText(U"X", Format(selecting->getPos().x));
			setText(U"Y", Format(selecting->getPos().y));

			selecting->setScalePos({ Parse<double>(params[U"SP X"]->getText()), Parse<double>(params[U"SP Y"]->getText()) });
			selecting->setScale({ Parse<double>(params[U"S X"]->getText()),Parse<double>(params[U"S Y"]->getText()) });
			setText(U"SP X", Format(selecting->getScalePos().x));
			setText(U"SP Y", Format(selecting->getScalePos().y));
			setText(U"RP X", Format(selecting->getRotatePos().x));
			setText(U"RP Y", Format(selecting->getRotatePos().y));

			selecting->setZ(Parse<double>(params[U"Z"]->getText()));
			selecting->setColor({
				Parse<double>(params[U"R"]->getText()),
				Parse<double>(params[U"G"]->getText()),
				Parse<double>(params[U"B"]->getText()),
				Parse<double>(params[U"A"]->getText()) });

			selecting->setAngle(Parse<double>(params[U"Angle"]->getText()));
			//回転すると変わるので更新
			setText(U"X", Format(selecting->getPos().x));
			setText(U"Y", Format(selecting->getPos().y));
			setText(U"Z", Format(selecting->getZ()));
		}
	};

	class CmdArea :public Object
	{
	public:
		ui::SimpleInputArea* area;
		CmdDecoder decoder;
		PartsDetailWindow* pd;

		void start()override
		{
			Object::start();

			pd = scene->getEntityManager()->findOne<PartsDetailWindow>();

			double w, h;
			Vec2 pos;

			w = 600;
			h = 40;
			pos = { (util::sc().x - w / 2)/2, util::sh() - h - 15 };

			area = ui::createSimpleInputArea(scene, {0,0}, w, h);

			area->transform->setPos({ pos,UiZvalue });
			//area->transform->setParent(transform);

			uiCollider << area->owner->getComponent<Collider>();

			auto f = ui::makeUiLike(area->owner->addComponent<DrawFont>(scene->getDrawManager(), Font(20, Typeface::Bold)));
			f->text = U"Command >";
			f->relative = { -130,0,0 };
			f->color = Palette::Gray;

			area->transform->setPos({ pos,UiZvalue });

			buildDecoder();			
		}

		// クラス特有の命令一覧
		// mkpar
		// find
		// kill
		//
		void buildDecoder()
		{
			DecoderSet sets(&decoder);
			//パーツを作る命令
			sets.registerMakePartsCmd(pmanager, true, [=](MakeParts* p) { partsCollider << p->getCreatedParts()->collider; });
			//パーツを選択する命令
			decoder.add_event_cmd<FindParts, String>(U"find", [=](FindParts* act) { pd->select(act->getFindParts());}, pmanager);
			//パーツを削除する命令
			EventFunction<KillParts> killPartsEvent
				= [=](KillParts* act) {
				auto killedParts = act->getKilledParts();
				for (const auto& p : killedParts)
				{
					partsCollider.remove(p->collider);
					if (selecting == p)pd->select(nullptr);
					//マスターが殺されてたら自動生成
					if (p->getName() == U"master")pmanager->createMaster();
				}
			};
			decoder.add_event_cmd<KillParts, String, bool>(U"kill", killPartsEvent, pmanager);
			decoder.add_event_cmd<KillParts, String>(U"kill", killPartsEvent, pmanager);
		}

		void update(double dt)override
		{
			if (area->getText() == U"\n")area->tex.text.clear();
			if (area->isActive() and KeyEnter.down())
			{
				if (area->getText() != U"") {
					decoder.input(area->getText());
					area->tex.text.clear();
					decoder.decode()->execute();
				}
			}
			Object::update(dt);
		}
	};

	class CameraController:public Object
	{
	private:
		bool touch_thumb;
		bool touch_bar_thumb;
		double x, y, r;
		DrawRectF* bar_thumb;
		DrawRectF* scale_bar;
		DrawCircle* range;
		DrawCircle* thumb;
		double sensitivity;
		double mouse_sensitivity;

		double _scale_func(double t)
		{
			if (t > 0)
			{
				return 1.0 + scale_range * t;
			}
			else
			{
				return 1.0 + (1 - 1.0 / scale_range) * t;
			}
		}

		Collider* c1;
		Collider* c2;
	public:
		DrawManager* manager;

		double get_x() { return x; }
		double get_y() { return y; }

		double scale_range;

		void start()override
		{
			Object::start();
			manager = scene->getDrawManager();
			transform->setParent(scene->getDrawManager()->getCamera()->transform);
			transform->setZ(UiZvalue-CameraZvalue);
			sensitivity = 0.1;
			mouse_sensitivity = -10;
			scale_range = 5.0;
			touch_thumb = touch_bar_thumb = false;
			double distance_from_Scene = 20, bar_h = 20, bar_thumb_w = 15, bar_from_range = 10;
			r = 80;
			x = util::sw() - r - distance_from_Scene;
			y = util::sh() - r;
			range = ui::makeUiLike(addComponent<DrawCircle>(scene->getDrawManager(), x, y, r));
			thumb = ui::makeUiLike(addComponent<DrawCircle>(scene->getDrawManager(), x, y, r / 4));
			bar_thumb = ui::makeUiLike(addComponent<DrawRectF>(scene->getDrawManager(), RectF{ Arg::center(x, y - r - bar_h / 2 - bar_from_range),bar_thumb_w,bar_h }));
			scale_bar = ui::makeUiLike(addComponent<DrawRectF>(scene->getDrawManager(), x - r, y - r - bar_h - bar_from_range, 2 * r, bar_h));

			c1 = addComponent<Collider>(CollideBox::CollideFigure(range->drawing));
			c2 = addComponent<Collider>(CollideBox::CollideFigure(scale_bar->drawing));
			c1->hitbox.relative = { x, y,0 };
			c2->hitbox.relative = { x, y - r - bar_h/2 - bar_from_range,0 };

			uiCollider << c1;
			uiCollider << c2;

			bar_thumb->relative.z = -1;

			scale_bar->color=ColorF(0.2, 0.2, 0.2);
			bar_thumb->color=Palette::Gray;
			range->color=ColorF(0.2, 0.2, 0.2);
			thumb->color = Palette::Gray;
		}

		void update(double dt)override
		{
			//UIの操作
			if (MouseL.up()) {
				touch_thumb = touch_bar_thumb = false;
				thumb->drawing.setCenter(x, y);
			}
			else if (MouseL.down()) {
				touch_thumb = thumb->drawing.leftClicked();
				if ((not touch_thumb) and range->drawing.leftClicked())
				{
					Vec2 p{ x,y };
					p += (Cursor::Pos() - p).setLength(Min((Cursor::Pos() - p).length(), range->drawing.r - thumb->drawing.r - 1));
					thumb->drawing.setCenter(p);
					touch_thumb = true;
				}
				touch_bar_thumb = bar_thumb->drawing.leftClicked();
				if ((not touch_bar_thumb) and scale_bar->drawing.leftClicked())
				{
					Vec2 p{ scale_bar->drawing.center() };
					if (abs(Cursor::Pos().x - p.x) < (scale_bar->drawing.w - bar_thumb->drawing.w) / 2) {
						p.x += Cursor::Pos().x - p.x;
					}
					else {
						p.x += Cursor::Pos().x - p.x > 0 ? (scale_bar->drawing.w - bar_thumb->drawing.w) / 2 : -(scale_bar->drawing.w - bar_thumb->drawing.w) / 2;
					}
					bar_thumb->drawing.setCenter(p);
					touch_bar_thumb = true;
				}
			};
			//UIを動かす
			if (touch_thumb) {
				if (thumb->drawing.movedBy(Cursor::DeltaF()).intersectsAt(range->drawing).has_value()
					and thumb->drawing.movedBy(Cursor::DeltaF()).intersectsAt(range->drawing)->isEmpty())
						thumb->drawing.moveBy(Cursor::DeltaF());
			}
			else if (touch_bar_thumb) {
				if (scale_bar->drawing.intersects(bar_thumb->drawing.movedBy(Cursor::DeltaF().x, 0).left()) and scale_bar->drawing.intersects(bar_thumb->drawing.movedBy(Cursor::DeltaF().x, 0).right()))
					bar_thumb->drawing.moveBy(Cursor::DeltaF().x, 0);
			}

			if (scale_bar->drawing.intersects(bar_thumb->drawing.movedBy(mouse_sensitivity * int32(Mouse::Wheel()), 0).left())
				and scale_bar->drawing.intersects(bar_thumb->drawing.movedBy(mouse_sensitivity * int32(Mouse::Wheel()), 0).right()))
			{
				bar_thumb->drawing.moveBy(mouse_sensitivity * int32(Mouse::Wheel()), 0);
			}

			manager->translate -= sensitivity * (thumb->drawing.center - Vec2{ x,y });

			manager->scale = Vec2{1,1}*_scale_func((bar_thumb->drawing.centerX() - scale_bar->drawing.centerX()) * 2.0 / (scale_bar->drawing.w - bar_thumb->drawing.w));
		}
	};

	class PartsEditor::Impl :public Object
	{
	public:
		PartsDetailWindow* dw;
		CmdArea* c;

		Parts* addParts(const PartsParams& params,const String& p)
		{
			auto parts = pmanager->addParts(params);
			partsCollider << parts->createHitbox(resource::texture(p).size()/2, Image{AssetManager::myAsset(localPath+p)}.alphaToPolygons());
			return parts;
		}

		void birthPartsManager()
		{
			if (pmanager != nullptr)pmanager->die();
			pmanager = scene->birthObjectNonHitbox<PartsManager>();
			pmanager->scaleHelperParamsSetting(Abs(CameraZvalue), Camera::Z);
		}

		void start()override
		{
			Object::start();
			birthPartsManager();
			mouse = scene->birthObjectNonHitbox<util::MouseObject>();
			dw = scene->birthObjectNonHitbox<PartsDetailWindow>();
			dw->priority.setPriority(priority.getPriority() + 1);
			scene->birthObjectNonHitbox<CameraController>();
			c = scene->birthObjectNonHitbox<CmdArea>();

			//画像読み込み
			ButtonEvent(this, U"readImg", [=] {
				Array<String>path = Dialog::OpenFiles({ FileFilter::AllImageFiles() });
				if (path.isEmpty())return;
				//パーツ追加
				for (const auto& tmp : path)
				{
					auto p = FileSystem::RelativePath(tmp, FileSystem::CurrentDirectory() + localPath);
					auto name = FileSystem::BaseName(tmp);
					c->decoder.input(U"mkpar " + name + U" " + p + U" 0 0")->decode()->execute();
				}
			}, U"画像読み込み", 20, Vec2{ 10,10 });

			//JSON読み込み
			ButtonEvent(this, U"loadJSON", [=] {
				Optional<FilePath> path = Dialog::OpenFile({ FileFilter::JSON() });
				if (not path)return;
				//masterパーツを殺して新しく構築
				c->decoder.input(U"kill master")->decode()->execute();
				scene->partsLoader->create(*path, pmanager);
				//当たり判定を付与
				for (auto& p : pmanager->partsArray)
				{
					if (p == pmanager->master)continue;

					if (p->base.path) partsCollider << p->createHitbox(resource::texture(*p->base.path).size() / 2, Image{ AssetManager::myAsset(localPath + *p->base.path) }.alphaToPolygons());
					else partsCollider << p->createHitbox({ 0,0 }, { std::get<1>(p->base.drawing).asPolygon() });
				}
			}, U"JSON読み込み", 20, Vec2{ 10,50 });

			//保存
			ButtonEvent(this, U"save", [=] {
				Optional<FilePath> path = Dialog::SaveFile({ FileFilter::JSON() });
				if (path)savePartsJson(pmanager, *path);
			}, U"保存", 20, Vec2{ 10,90 });

			//操作方法切り替え
			ButtonEvent(this, U"keyMoveModeChange", Array<Event>{
				[=] {
					auto button = scene->findOne<ui::Button>(U"keyMoveModeChangeButton");
					button->setText(U"キー操作終了");
					button->box->fitSize();
					scene->findOne<CmdArea>()->area->canInput = false;
					startAction(U"keyMove");
				}, [=] {
					auto button = scene->findOne<ui::Button>(U"keyMoveModeChangeButton");
					button->setText(U"キー操作");
					button->box->fitSize();
					scene->findOne<CmdArea>()->area->canInput = true;
					stopAction(U"keyMove");
				}
			}, U"キー操作", 20, Vec2{ util::sw() - 470,10 });

			ButtonEvent(this, U"pauseModeChange", Array<Event>{
				[=] {
					auto button = scene->findOne<ui::Button>(U"pauseModeChangeButton");
					button->setText(U"モデル編集");
					button->box->fitSize();

				}, [=] {
					auto button = scene->findOne<ui::Button>(U"pauseModeChangeButton");
					button->setText(U"モーション作成");
					button->box->fitSize();
				}
			}, U"モーション作成", 20, Vec2{ util::sw() - 630,10 });

			//パーツをキー入力で移動させる　回転　拡大させる
			ACreate(U"keyMove").add(
				[=](double dt) {
					if (selecting == nullptr)return;
					Vec2 moving{ 0,0 };
					double dAngle = 0;
					Vec2 dZoom{ 0,0 };
					double speed = 100;
					if (KeyShift.pressed())speed *= 2.5;
					if (KeyA.pressed())moving += Vec2{ -1,0 };
					if (KeyD.pressed())moving += Vec2{ 1,0 };
					if (KeyS.pressed())moving += Vec2{ 0,1 };
					if (KeyW.pressed())moving += Vec2{ 0,-1 };
					moving.setLength(speed);
					selecting->setPos(selecting->getPos() + moving * dt);
					if (KeyQ.pressed())dAngle -= 1;
					if (KeyE.pressed())dAngle += 1;
					selecting->setAngle(selecting->getAngle() + dAngle * speed * dt);
					if (KeyJ.pressed())dZoom += Vec2{ -1,0 };
					if (KeyL.pressed())dZoom += Vec2{ 1,0 };
					if (KeyI.pressed())dZoom += Vec2{ 0,1 };
					if (KeyK.pressed())dZoom += Vec2{ 0,-1 };
					dZoom.setLength(speed / 150);
					selecting->setScale(selecting->getScale() + dZoom * dt);
					dw->set();
				}
			);

			auto plusMark = scene->birthObjectNonHitbox();

			ui::makeUiLike(plusMark->addComponent<Draw2D<Polygon>>(scene->getDrawManager(), Shape2D::Plus(6, 1).asPolygon()));

			plusMark->transform->setPos({ util::sc(),0 });
		}

		void update(double dt)override
		{
			Object::update(dt);
		}

		Array<PartsParams> createAllPartsParams()const
		{
			Array<PartsParams> ret;
			for (const auto& parts : pmanager->partsArray)
			{
				ret << PartsParams(parts->params);
			}
			return ret;
		}
	};
		
	void PartsEditor::start()
	{
		camera = birthObjectNonHitbox<Camera>(BasicCamera3D(drawManager.getRenderTexture().size(), 50_deg, Vec3{ 0,0,-10 }));

		camera->type = Camera::Z;

		drawManager.setting(camera);

		impl = birthObjectNonHitbox<PartsEditor::Impl>();
		camera->transform->setZ(CameraZvalue);

		drawManager.translate = -Vec2{ util::sw(),util::sh() }/2;
	}

	void MotionCreator::start()
	{
		camera = birthObjectNonHitbox<Camera>(SimpleFollowCamera3D(drawManager.getRenderTexture().size(), 50_deg, Vec3{ 0,0,10 }, 0_deg, 40, 12));
		drawManager.setting(camera);
	}
}
