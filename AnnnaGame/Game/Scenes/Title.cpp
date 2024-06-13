#include "../../stdafx.h"
#include "Title.h"
#include"../UI.h"
#include"../StateMachine/StateProvider.h"
#include"../../Motions/BodyEditor.h"

void Title::start()
{
	Scene::start();

	camera = birth<Camera>(BasicCamera3D(drawManager.getRenderTexture().size(), 50_deg, Vec3{ 0,0,-20 }));

	camera->type = Camera::Z;

	drawManager.setting(camera);

	/*state::SCreatorContainer dict;

	auto text = birth<ui::Text>()->setting(U"hello", 24, { 200,200 });

	auto text1 = birth<ui::Text>()->setting(U"world", 24, { 200,250 });

	auto sele = birth<ui::Selection>();

	sele->addSelection(new Actions(),
		new FuncAction([=] {
			text->font->color = Palette::White;
			}, none),
		new FuncAction([=] {
			text->font->color = Palette::Black;
			})
	);

	sele->addSelection(new Actions(), new FuncAction([=] {text1->font->color = Palette::White; },none), new FuncAction([=] {text1->font->color = Palette::Black; }));

	sele->startDirection().start();

	sele->ACreate(U"select", true).add([=](double) {
		if (Key1.down())sele->selecting = 0;
		if (Key2.down())sele->selecting = 1;
		if (Key3.down())sele->determined = true;
		});*/

	auto obj = birth<bodyEditor::BodyEditor>();
}

void Title::update(double dt)
{
	Scene::update(dt);
	
}
