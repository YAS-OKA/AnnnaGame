#include "../stdafx.h"
#include "Parts.h"
#include"../Game/Camera.h"
#include"../Util/Cmd.hpp"
#include"../Util/CmdDecoder.h"
#include"../Util/Util.h"
#include"MotionCmd.h"

namespace
{
	constexpr double frameZ = 10000; 
}

namespace mot
{
	void Parts::onTrashing()
	{
		Object::onTrashing();
		auto& arr = parts_manager->partsArray;
		//partsArrayから削除
		for (auto it = arr.begin(); it != arr.end();)
		{
			if (*it == this)
			{
				it = arr.erase(it);
			}
			else {
				++it;
			}
		}
		if (parts_manager->master == this)parts_manager->master = nullptr;
	}

	void Parts::setParent(const String& name, bool setLocalPos)
	{
		auto nextParent = parts_manager->find(name);
		if (nextParent == nullptr)return;

		partsRelation.setParent(nextParent);

		parent = nextParent;

		params.parent = parent->name;

		transform->setParent(parent->transform);
		if (setLocalPos) {
			//絶対を相対に！
			transform->setLocalX(transform->getPos().x);
			transform->setLocalY(transform->getPos().y);
			transform->setLocalDirection(transform->getDirection());
		}
		else {
			setPos(transform->getLocalPos().xy());
		}
	}

	void Parts::setTexture(const PartsDrawing& drawing)
	{
		if (tex != nullptr)remove(tex);
		switch (drawing.index())
		{
		case 0:
			tex = addComponent<DrawTexture>(&parts_manager->dm->drawing, std::get<0>(drawing));
			tex->relative = { -std::get<0>(drawing).size() / 2, 0 };
			break;
		case 1:
			tex = addComponent<Draw2D<Figure>>(&parts_manager->dm->drawing, std::get<1>(drawing));
			break;
		}
		params.drawing = drawing;
	}

	void Parts::build(const PartsParams& params)
	{
		if (transform == nullptr)return;
		//名前
		setName(params.name);
		//親
		if (name != U"master")setParent(params.parent, true);
		//path
		if (params.path)this->params.path = params.path;
		//座標
		setPos(params.pos);
		//アングル
		setAngle(params.angle);
		//スケール
		setScale(params.scale);
		//z
		setZ(params.z);
		//パーツのテクスチャ
		setTexture(params.drawing);
		//回転中心
		setRotatePos(params.rotatePos);
		//カラー
		tex->color = params.color;

		//base = params;

		//tex->in3D = true;
	}

	void Parts::update(double dt)
	{
		params.pos = transform->getXY();

		Object::update(dt);
	}

	Collider* Parts::createHitbox(const Vec2& pos,const MultiPolygon& fig)
	{
		//今はfig[0]を当たり判定にしてるけど、いつかMultiPolygon対応の当たり判定を作るべき
		if (collider != nullptr)remove(collider);

		collider = addComponent<Collider>(CollideBox::CollideFigure(fig[0]));

		collider->hitbox.relative = { fig[0].centroid()-pos,0 };

		return collider;
	}

	void PartsManager::scaleHelperParamsSetting(double baseLength, const Camera::DistanceType& distanceType)
	{
		scaleHelperBaseLength = baseLength;
		distanceTypeUsedInScaleHelper = distanceType;
	}

	Parts* PartsManager::birthParts()
	{
		return scene->birthObjectNonHitbox<Parts>(this);
	}

	Parts* PartsManager::addParts(const PartsParams& params)
	{
		if (master == nullptr and params.name != U"master")return nullptr;

		auto parts = birthParts();

		parts->build(params);

		parts->base = params;

		parts->params = params;

		if (params.name == U"master") {
			setMaster(parts);
		}
		else {
			partsArray << parts;
			parts->followDestiny(master);//masterが死んだら死ぬようにする
		}

		return parts;
	}

	Parts* PartsManager::setMaster(Parts* masterParts)
	{
		if (master != nullptr) {
			execute(new KillParts(U"master", true), this);
		}

		masterParts->setName(U"master");

		master = masterParts;

		partsArray << master;

		master->transform->setParent(transform);

		return master;
	}

	Parts* PartsManager::createMaster()
	{
		//masterパーツを作成
		auto m = birthParts();

		m->build(PartsParams(U"tmp"));

		m->tex->visible = false;

		m->setColor(ColorF{ 0,0,0,0 });

		return setMaster(m);
	}

	Parts* PartsManager::find(const String& name)
	{
		for (auto itr = partsArray.begin(), en = partsArray.end(); itr != en; ++itr)
		{
			if ((*itr)->name == name)return *itr;
		}
		return nullptr;
	}

	void PartsManager::killParts(Parts* parts)
	{
		parts->die();
	}

	void PartsManager::killParts(const String& name){
		killParts(find(name));
	}

	void PartsManager::setPartsVisibility(bool visible)
	{
		for (auto parts : partsArray)
		{
			parts->tex->visible = visible;
		}
	}

	void PartsManager::start()
	{
		Object::start();

		auto scene_draw_manager = scene->getDrawManager();

		dm = addComponent<Draw2D<DrawManager>>(scene_draw_manager, scene_draw_manager->getCamera(), MSRenderTexture{ 0,0,HasDepth::No }, ColorF{ 0,0,0,0 });

		createMaster();
	}

	void PartsManager::update(double dt)
	{
		dm->drawing.update();//drawManagerをアプデ
		Object::update(dt);
	}
}

namespace mot
{
	void loadPartsTexture(my::Scene* scene, const String& relativePath)
	{
		//アセットが登録されていなければ
		if (not scene->r.textures.contains(relativePath))
		{
			scene->r.type = RegisterAssets::AssetType::texture;
			using namespace resource;
			texture::Register(scene->r(relativePath), localPath + relativePath, TextureDesc::Mipped);
			resource::texture::Load(relativePath);
		}
	}

	bool savePartsJson(PartsManager* pm, const String& path)
	{
		Array<PartsParams> many_parts;
		for (const auto& parts : pm->partsArray)
		{
			many_parts << PartsParams(parts->params);
		}

		JSON json;

		for (const auto& parts : many_parts)
		{
			json[parts.name][U"parent"] = parts.parent;
			String texture=U"";
			if (parts.path)
			{
				texture = *parts.path;
			}
			else
			{
				const auto& fig = std::get<1>(parts.drawing);
				const auto& figName = fig.getName();
				if (figName == U"Circle") {
					texture = U"cir {}"_fmt(fig.getCircle().r);
				}
				else if (figName == U"RectF") {
					const auto& rect = fig.getRectF();
					texture = U"rect {} {}"_fmt(rect.w, rect.h);
				}
				else if (figName == U"Triangle")
				{
					const auto& tri = fig.getTriangle();
					const auto& deg = util::angleOf2Vec(tri.p1 - tri.p0, tri.p2 - tri.p0) / 1_deg;
					const auto& len1 = (tri.p0 - tri.p1).length();
					const auto& len2 = (tri.p0 - tri.p2).length();
					texture = U"tri {} {} {}"_fmt(deg, len1, len2);
				}
			}
			json[parts.name][U"texture"] = texture;
			json[parts.name][U"pos"] = parts.pos;
			json[parts.name][U"z"] = parts.z;
			json[parts.name][U"scale"] = parts.scale;
			json[parts.name][U"scalePos"] = parts.scalePos;
			json[parts.name][U"angle"] = parts.angle;
			json[parts.name][U"rotatePos"] = parts.rotatePos;
			json[parts.name][U"color"] = parts.color;
		}

		return json.save(path);
	}

	PartsManager* LoadParts::create(const String& jsonPath)
	{
		auto pm = m_scene->birthObjectNonHitbox<PartsManager>();

		return create(jsonPath, pm);
	}

	PartsManager* LoadParts::create(const String& jsonPath,PartsManager* pmanager)
	{
		auto& pm = pmanager;

		JSON json = JSON::Load(jsonPath);

		HashTable<Parts*, String> parent_list;
		HashTable<Parts*, double>partsAngle;

		CmdDecoder deco;

		DecoderSet(&deco).registerMakePartsCmd(pm,true);

		for (const auto& elms : json)
		{
			Parts* createdParts = nullptr;

			if (elms.key != U"master")
			{
				auto tmp = elms.value[U"texture"].getString();

				auto texArray = tmp.split(' ');
				texArray.insert(texArray.begin() + 1, U"0 0");
				for (tmp = U""; const auto & tex : texArray)
				{
					tmp += U" " + tex;
				}
				//mkpar name pos param をうちこむ
				deco.input(U"mkpar " + elms.key + tmp)->decode()->execute();
				//作成したパーツを取得
				createdParts = pm->partsArray.back();
			}
			else
			{
				createdParts = pm->master;
			}

			createdParts->setPos(elms.value[U"pos"].get<Vec2>());
			createdParts->setScalePos(elms.value[U"scalePos"].get<Vec2>());
			createdParts->setScale(elms.value[U"scale"].get<Vec2>());
			createdParts->setRotatePos(elms.value[U"rotatePos"].get<Vec2>());
			//createdParts->pureRotate(elms.value[U"angle"].get<double>());
			createdParts->setZ(elms.value[U"z"].get<double>());
			createdParts->setColor(elms.value[U"color"].get<ColorF>());

			partsAngle[createdParts] = elms.value[U"angle"].get<double>();

			if (elms.key != U"master")parent_list.emplace(createdParts, elms.value[U"parent"].getString());
		}
		for (auto& p : parent_list)p.first->setParent(p.second);//親子関係は最後に結ぶ(存在しない親を参照してしまうから)

		for (auto& [p, v] : partsAngle)
		{
			p->pureRotate(v, true);		//その場で回転させる
			p->params.angle = v;
		}

		return pm;
	}
}

Vec2 draw_helper::CameraScaleOfParts::getScalePos() const
{
	return parts->getPos() + parts->getScalePos().rotate(parts->getAbsAngle() * 1_deg);
}

double draw_helper::CameraScaleOfParts::operator () () const
{
	return (*helper)();
}

draw_helper::PartsShallow::PartsShallow(mot::PartsManager* p, IDraw2D* d)
	:pmanager(p), DrawShallow(d)
{

}

double draw_helper::PartsShallow::getDepth() const
{
	return 0.0;
}
