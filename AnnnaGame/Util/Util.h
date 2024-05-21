#pragma once
#include<fstream>
#include<string>

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


	static String slice(StringView arr, size_t a, size_t b)
	{
		String ret = U"";
		for (auto itr = arr.begin() + a, en = arr.begin() + b; itr != en; ++itr)ret << *itr;
		return ret;
	}
	//配列に配列を挿入する(i番目に挿入する)
	template<class A>
	static void append(Array<A>& arr, Array<A> _ins, size_t i)
	{
		for (auto itr = _ins.rbegin(), en = _ins.rend(); itr != en; ++itr)
		{
			arr.insert(arr.begin() + i, *itr);
		}
	}

	static bool strEqual(StringView str, size_t from, size_t to, StringView cstr)
	{
		auto s = util::slice(str, from, to);

		if (not s.isEmpty() and s == cstr)return true;
		return false;
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

	std::string toStr(StringView s);

	String toUStr(const std::string& s);

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
