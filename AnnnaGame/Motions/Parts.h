#pragma once
#include"../Component/Transform.h"
#include"../Game/Object.h"
#include"../Component/Draw.h"
#include"../Game/Scenes.h"
#include"../Figure.h"
//#include"../Game/Utilities.h"

class Camera;

namespace draw_helper {
	class CameraScaleOfParts;
	class ZShallow;
}

namespace mot
{
	static String localPath{ U"asset/motion/" };

	struct PartsParams
	{
		String name;
		Optional<String> path;
		std::variant<Texture, Figure> drawing;
		String parent;
		Vec2 pos;
		Vec2 scale;
		Vec2 scalePos;
		double angle;
		double z;
		Vec2 rotatePos;
		ColorF color;

		PartsParams() {};

		PartsParams(const PartsParams& other)
		{
			build(other.name, other.parent, other.pos, other.scale, other.scalePos, other.angle, other.z, other.rotatePos, other.color, other.path, other.drawing);
		}

		PartsParams(
			const String& _name,
			const Optional<String>& _path = none,
			const std::variant<Texture, Figure>& _drawing = RectF{},
			const String& _parent = U"",
			const Vec2& _pos = { 0,0 },
			const Vec2& _scale = { 1,1 },
			const Vec2& _scalePos = { 0,0 },
			double _angle = 0,
			double _z = 100,
			const Vec2& _rotatePos = { 0,0 },
			const ColorF& _color = Palette::White)
		{
			build(_name,
				_parent,
				_pos,
				_scale,
				_scalePos,
				_angle,
				_z,
				_rotatePos,
				_color,
				_path,
				_drawing);
		}

		void build(
			const String& _name,
			const String& _parent,
			const Vec2& _pos,
			const Vec2& _scale,
			const Vec2& _scalePos,
			double _angle,
			double _z,
			const Vec2& _rotatePos,
			const ColorF& _color,
			const Optional<String> _path,
			const std::variant<Texture, Figure>& _drawing)
		{
			name = _name;
			parent = _parent;
			pos = _pos;
			scale = _scale;
			scalePos = _scalePos;
			angle = _angle;
			z = _z;
			rotatePos = _rotatePos;
			color = _color;
			path = _path;
			drawing = _drawing;
		};
	};

	class PartsManager;

	using PartsDrawing = std::variant<Texture, Figure>;

	class Parts :public Object
	{
	public:
		PartsParams base;
		
		PartsParams params;

		IDraw2D* tex = nullptr;

		//draw_helper::CameraScaleOfParts* scaleHelper=nullptr;

		Collider* collider = nullptr;

		PartsManager* parts_manager;

		Parts* parent=nullptr;

		util::PCRelationship<Parts> partsRelation;

		Parts(PartsManager* manager)
			:parts_manager(manager),partsRelation(this) {};
		
		void onTrashing()override;

		Collider* createHitbox(const Vec2& pos, const MultiPolygon& fig);

		void setParent(const String& name, bool setLocalPos = false);

		void setZ(double z)
		{
			//この値は子に影響しないようにする
			transform->affectToChildren = false;
			params.z = z;
			transform->setPos({ transform->getPos().xy(),z });
			transform->affectToChildren = true;
		}

		void setPos(const Vec2& pos)
		{
			transform->setLocalXY(pos);
		}

		void setAbsPos(const Vec2& pos)
		{
			transform->setXY(pos);
		}

	private:
		void rotateCollider(double ang)
		{
			if(collider!=nullptr)std::get<2>(collider->hitbox.shape).rotateAt({ 0,0 }, ang * 1_deg);
			for (auto& c : partsRelation.getChildren())
			{
				c->rotateCollider(ang);
			}
		}

	public:
		void setAngle(double angle)
		{
			double ang = Math::Fmod(angle - getAngle(), 360);

			auto rp = transform->getPos().xy() + getRotatePos().rotated(getAbsAngle()*1_deg);

			transform->rotateAt({ rp, 0}, {0,0,1}, ang * 1_deg);

			rotateCollider(ang);

			params.angle = angle;

			setScale(transform->scale.aspect.vec.xy());
		}
		//ただその場で回る　ローテートポスを無視　recursive=trueなら子のpureRotateを呼ぶ
		void pureRotate(double angle, bool recursive = true)
		{
			double ang = Math::Fmod(angle, 360);
			auto tmp = transform->affectToChildren;
			transform->affectToChildren = false;
			transform->rotate({ 0,0,1 }, ang * 1_deg);
			transform->affectToChildren = tmp;
			if (collider != nullptr)std::get<2>(collider->hitbox.shape).rotateAt({ 0,0 }, ang * 1_deg);
			setScale(transform->scale.aspect.vec.xy());
			//子どもも回す
			if (recursive)for (auto& c : partsRelation.getChildren())dynamic_cast<Parts*>(c)->pureRotate(ang, true);
		}

		void setScale(const Vec2& s)
		{
			transform->scale.setAspect({ s,1 }, 0, 0, getAngle() * 1_deg);

			if (params.scale.x != 0 and params.scale.y != 0)
			{
				setRotatePos(params.rotatePos * s / params.scale);
				setScalePos(params.scalePos * s / params.scale);
			}

			params.scale = s;
		}

		void setScalePos(const Vec2& pos)
		{
			params.scalePos = pos;
		}

		void setColor(const ColorF& color)
		{
			tex->color = color;
			params.color = color;
		}

		void setTexture(const PartsDrawing& drawing);

		void setRotatePos(const Vec2& pos)
		{
			params.rotatePos = pos;
		}

		void setName(const String& name)
		{
			this->name = name;
			params.name = name;
		}

		void build(const PartsParams& params);

		String getParent()const { return transform->getParent()->owner->name; }

		Vec2 getPos()const { return transform->getLocalPos().xy(); }

		String getName()const { return params.name; }

		double getZ()const { return transform->getPos().z; }

		double getAngle()const {
			return params.angle;
		};

		double getAbsAngle()const {
			if (parent != nullptr)return params.angle + parent->getAbsAngle();
			else return getAngle();
		}

		Vec2 getRotatePos()const { return params.rotatePos; }

		ColorF getColor()const { return tex->color; }

		Vec2 getScalePos()const { return params.scalePos; }

		Vec2 getScale()const { return transform->scale.aspect.vec.xy(); }
		
		void update(double dt)override;
	};

	class PartsManager:public Object
	{
	public:
		//マスターパーツ
		Parts* master=nullptr;
		Array<Parts*> partsArray;
		Draw2D<DrawManager>* dm;

		Camera::DistanceType distanceTypeUsedInScaleHelper=Camera::Screen;
		double scaleHelperBaseLength=100;

		//EntityManager ema;

		void scaleHelperParamsSetting(double baseLength, const Camera::DistanceType& distanceType = Camera::Screen);

		Parts* birthParts();

		Parts* addParts(const PartsParams& params);

		Parts* setMaster(Parts* masterParts);

		Parts* createMaster();

		Parts* find(const String& name);

		void killParts(Parts* parts);

		void killParts(const String& name);

		void setPartsVisibility(bool visible=false);

		void start()override;

		void update(double dt)override;
	};
}

namespace mot
{
	//path->localPath+relativePath
	void loadPartsTexture(my::Scene* scene, const String& relativePath);

	bool savePartsJson(PartsManager* pm, const String& path);

	class LoadParts
	{
	private:
		my::Scene* m_scene;
	public:
		LoadParts(my::Scene* scene) :m_scene(scene) {};

		void setScene(my::Scene* scene) { m_scene = scene; };

		PartsManager* create(const String& jsonPath);

		PartsManager* create(const String& jsonPath, PartsManager* pmanager);
	};
}

namespace draw_helper
{
	//Masterパーツの情報を参照してスケール。
	class CameraScaleOfParts :public ScaleHelper2D
	{
	private:
		ScaleHelper2D* helper = nullptr;
		mot::Parts* parts;
	public:
		CameraScaleOfParts(mot::Parts* parts)
			: parts(parts)
		{
		}

		~CameraScaleOfParts() { delete helper; }

		template <class ScaleHelper, class... Args>
		ScaleHelper* setHelper(Args&&... args)
		{
			if (helper != nullptr)delete helper;

			auto ret = new ScaleHelper(args...);

			helper = ret;
			
			return ret;
		}

		virtual Vec2 getScalePos()const override;

		virtual double operator () () const override;
	};

	class PartsShallow :public DrawShallow
	{
	public:
		mot::PartsManager* pmanager;

		PartsShallow(mot::PartsManager* p,IDraw2D* d);

		double getDepth()const override;
	};
}
