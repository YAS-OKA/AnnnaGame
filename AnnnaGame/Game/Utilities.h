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
	//極座標
	static Vec2 polar(double rad, double len=1)
	{
		return len * Vec2{ Cos(rad),Sin(rad) };
	}
	//ベクトル内積
	static double dotProduct(Vec2 vl, Vec2 vr) {
		return vl.x * vr.x + vl.y * vr.y;
	}

	static double angleOf2Vec(Vec2 A, Vec2 B)
	{
		//内積とベクトル長さを使ってcosθを求める
		double cos_sita = dotProduct(A, B) / (A.length() * B.length());

		//cosθからθを求める
		return Acos(cos_sita);
	}
	//x,yをひっくり返す
	static Vec2 invXY(const Vec2& p)
	{
		return { p.y,p.x };
	}

	static double getRad(const Vec2& p)
	{
		return p.getAngle() - 90_deg;
	}

	template<class T>
	static Array<T> slice(Array<T> arr, int32 a, int32 b)
	{
		Array<T> ret{};
		for (auto itr = arr.begin() + a, en = arr.begin() + b; itr != en; ++itr)ret << *itr;
		return ret;
	}

	static String slice(const String& arr, int32 a, int32 b)
	{
		String ret = U"";
		for (auto itr = arr.begin() + a, en = arr.begin() + b; itr != en; ++itr)ret << *itr;
		return ret;
	}

	enum class ProjectionType
	{
		Parallel,
		Parse,
	};

	//template<class T, class... Args>
	//static T* ptr(Args&& ...args)
	//{
	//	return new T(args...);
	//}

	void loadTexture(my::Scene* scene, const String& name, const String& path);

	class Convert2DTransform
	{
	public:
		ProjectionType type;

		DrawManager* dManager;

		Convert2DTransform(DrawManager* dManager, const ProjectionType& type=ProjectionType::Parallel);

		Vec3 convert(const Vec3& pos, const ProjectionType& type)const;

		Vec3 convert(const Vec3& pos)const;

		Vec3 convert(Transform* transform)const;
	};

	class Convert2DScale
	{
	public:
		Camera::DistanceType type;

		DrawManager* dManager;

		double baseLength;

		Convert2DScale(double baseLength,DrawManager* dManager, const Camera::DistanceType& type = Camera::DistanceType::Screen);
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

	//上限で止まる
	struct StopMax
	{
		double value;
		double max;

		StopMax() {};

		StopMax(double max, double value = 0);

		void add(double value);

		bool additionable()const;
	};

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
