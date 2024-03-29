#pragma once
#include<fstream>

namespace util
{
	template<class T> using uPtr = std::unique_ptr<T>;
	template<class T> using sPtr = std::shared_ptr<T>;
	template<class T> using wPtr = std::weak_ptr<T>;

	//極座標
	static Vec2 polar(double rad, double len = 1)
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

	static std::string str(StringView s);

	static String uStr(const std::string& s);

	class EditFile
	{
	private:
		uPtr<std::fstream> file;		
	public:

		EditFile() = default;

		EditFile(FilePathView path);
		//上書き
		void overwrite(StringView text);
		//fileの中身をすべて読みだす
		String readAll()const;
	};

	static EditFile createFile(FilePath path);

}
