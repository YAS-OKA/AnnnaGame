# include <Siv3D.hpp> // Siv3D v0.6.14
#include"GameMaster.h"
#include"Prg/Actions.h"
#include"Game/Object.h"
#include"Game/Chara/Character.h"
using namespace prg;

void Main()
{
	Window::Resize(1200, 675);
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

	EntityManager manager;

	manager.birth<GameMaster>();

	while (System::Update())
	{
		ClearPrint();

		manager.update(Scene::DeltaTime());
	}
}
