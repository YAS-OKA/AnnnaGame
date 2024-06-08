#include "../../stdafx.h"
#include "Character.h"
#include"../Skill/Skills.h"
#include"../Scenes.h"
#include"../../Util/DataSaver.h"

#include"../../Motions/Parts.h"

void CharaProperty::printParams() const
{
	Print << U"power:" << power;
	Print << U"ready_rate:" << ready_rate;
	Print << U"guard:" << guard;
}

void CharacterParams::printParams() const
{
	Print <<U"hp:" << hp;
	Print << U"level:" << level.level.value;
	Print << U"quickness:" << quickness;	
}

void CharacterParams::LoadFile(const String& path,const String& name)
{
	DataSaver saver = *DataSaver(U"", TextReader(path).readAll()).getDataSaver(name);

	hp=saver.getDataSaver(U"hp")->get<double>(0,1);

	physics.power = saver.getDataSaver(U"p_power")->get<double>(0, 1);
	physics.ready_rate = saver.getDataSaver(U"p_recover")->get<double>(0, 1);
	physics.guard = saver.getDataSaver(U"p_guard")->get<double>(0, 1);

	magic.power = saver.getDataSaver(U"m_power")->get<double>(0, 1);
	magic.ready_rate = saver.getDataSaver(U"m_recover")->get<double>(0, 1);
	magic.guard = saver.getDataSaver(U"m_guard")->get<double>(0, 1);

	quickness=saver.getDataSaver(U"quick")->get<double>(0, 1);

	DataSaver z = *saver.getDataSaver(U"zokusei");
	zokusei.fire = z.get<double>(0, 1);
	zokusei.water = z.get<double>(1, 1);
	zokusei.soil = z.get<double>(2, 1);
	zokusei.plant = z.get<double>(3, 1);
	zokusei.animal = z.get<double>(4, 1);
}

void Character::start()
{
	Object::start();
}

void Character::update(double dt)
{
	Object::update(dt);
	Print << name<<U"sHp:" << param.hp;
}

Borrow<Field<HashTable<String, Borrow<skill::Skill>>>> Character::getSkills()
{
	auto skills = getComponent<Field<HashTable<String, skill::Skill*>>>(U"Skills");

	if (not skills)skills = addComponentNamed<Field<HashTable<String, skill::Skill*>>>(U"Skills");

	return skills;
}

Borrow<skill::Skill> Character::setSkill(const Borrow<skill::Skill>& skill, StringView name)
{
	auto skills = getSkills();

	skills->value.emplace(name, skill);

	ACreate(name) += FuncAction([=] { skill->act(); });//スキル発動

	return skill;
}

void CharaUtil::SetAnimator(const Borrow<Object>& obj, Borrow<mot::PartsManager> pman, FilePath srcPath, Optional<String> standard, Array<AnimeArg> args)
{
	using namespace prg;
	using namespace state;

	HashSet<String> stateSet;
	//起点パラメータのセット
	auto param = obj->addComponentNamed<Field<HashTable<String, bool>>>(U"AnimatorParam");
	for (const auto& arg : args)
	{
		stateSet.emplace(arg.to);
		param->value[arg.to] = false;
	}
	if (standard)stateSet.emplace(*standard);

	SCreatorContainer dict;
	//状態遷移を定義
	dict[U"State"] = [&,args](In info, A act)->A
		{
			if (standard)
			{
				act |= dict[*standard](info);//default
			}
			for (const auto& arg : args)
			{
				act |= dict[arg.to](info);
			}

			for (const auto& arg : args)
			{
				for (const auto& state : arg.from)
				{
					if (not stateSet.contains(state))
					{
						throw Error{ U"存在しないfromが指定されました\nfrom={}\nto={}"_fmt(state,arg.to) };
					}
				}

				act.relate(arg.from, arg.to).andActiveIf([=] {return param->value[arg.to]; });
			}

			return F(act);
		};
	//デコーダー作成
	auto decoder = std::make_shared<CmdDecoder>();
	DecoderSet(decoder.get()).motionScriptCmd(pman, nullptr);
	if (standard)args << AnimeArg{ .from = {},.to = *standard };//スタンダード(default)のモーションを入れておく
	//各状態(モーション)のセット
	for (const auto& arg : args)
	{
		//モーションをロード
		decoder->input(U"load {} {}"_fmt(srcPath, arg.to))->decode()->execute();

		dict[arg.to] = [=](In info, A act)->A
			{
				act |= FuncAction(
					[=]
					{
						String cmd = U"start {}"_fmt(arg.to);
						if (arg.loop)cmd += U" true";//ループする
						//モーション開始
						decoder->input(cmd)->decode()->execute();
					},
					[=] {
						if (pman->actman(arg.to) and pman->actman[arg.to].isActive())
						{
							pman->actman[arg.to].end();
						}
					}, none);

				return F(act);
			};
	}

	obj->ACreate(U"Animator", true) += dict[U"State"]();
}
