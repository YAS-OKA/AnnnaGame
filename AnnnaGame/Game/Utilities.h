#pragma once
#include"../Prg/Action.h"
#include"../EC.hpp"
#include"Object.h"
#include"Camera.h"

class Collider;
class Character;
class DrawManager;
class Transform;
class Influence;

namespace mot {
	class PartsManager;
}

namespace my{
	class Scene;
}

namespace util
{
	enum class ProjectionType
	{
		Parallel,
		Parse,
	};

	void loadTexture(my::Scene* scene, const String& name, const String& path);
	//三次元に投影
	class Convert2DTransform
	{
	public:
		ProjectionType type;

		DrawManager* dManager;

		Convert2DTransform(DrawManager* dManager, const ProjectionType& type=ProjectionType::Parallel);

		Vec3 convert(const Vec3& pos, const BasicCamera3D& camera)const;

		Vec3 convert(const Vec3& pos, const ProjectionType& type)const;

		Vec3 convert(const Vec3& pos)const;

		Vec3 convert(Transform* transform)const;
	};
	//カメラからの距離をもとにスケーリングする　遠近法
	class Convert2DScale
	{
	public:
		Camera::DistanceType type;

		Camera* camera = nullptr;

		DrawManager* dManager;

		double baseLength;
		//カメラの距離に応じて
		Convert2DScale(double baseLength, DrawManager* dManager);
		//typeに応じて
		Convert2DScale(double baseLength,DrawManager* dManager, const Camera::DistanceType& type);
		//baseLength/distanceを返す
		double distanceRate(const Vec3& pos, const Camera::DistanceType& type)const;
		double distanceRate(const Vec3& pos)const;

		Vec3 convert(const Vec3&scale, const Vec3& pos, const Camera::DistanceType& type)const;

		Vec3 convert(const Vec3& scale,const Vec3& pos)const;

		Vec3 convert(Transform* transform)const;
	};

	constexpr auto sc = s3d::Scene::Center;
	constexpr auto sw = s3d::Scene::Width;
	constexpr auto sh = s3d::Scene::Height;

	class MouseObject :public Object
	{
	public:
		DrawManager* manager;

		Collider* mouseHitbox;

		void start()override;

		void update(double dt)override;

		Vec3 getCursorPos(Influence* dManagerInfluence)const;

		Array<Entity*> getClickedObjects(Array<Collider*> colliders) const;

		enum SurfaceType {
			depth,
			z,
		};
		//クリックされた表面のエンティティを返す
		Entity* getClickedSurfaceObject(Array<Collider*> collider, const SurfaceType& type = depth);
		//クリックされた表面のエンティティを返す　2D(FigureCollider)を優先する
		Entity* getClickedSurfaceObject2DPrior(Array<Collider*> colliders, const SurfaceType& type2d = z, const SurfaceType& type3d = depth);
	};	
}
