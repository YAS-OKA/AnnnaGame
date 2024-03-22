﻿#pragma once
#include"Base.h"

class Lender;
namespace {
	//参照先のポインタを持っている
	template<class C>
	class BorrowImpl :public BaseBorrow
	{
		C* borrow_obj = nullptr;
	public:
		Lender* cast_lender = nullptr;//キャストしたBorrowを作るときについ買う

		BorrowImpl(Lender* lender)
			:BaseBorrow(lender), cast_lender(lender) {}

		void _init()
		{
			borrow_obj = static_cast<C*>(lender->owner);
		}

		C* get()const
		{
			//貸出者が生きてたら借りたものを返す
			if (lender)return borrow_obj;
			return nullptr;
		}
	};
}
//_Borrowのスマポ。sharedだからコピーも可
template<class C>
class Borrow
{
	std::shared_ptr<BorrowImpl<C>> ptr;
public:
	Borrow(BorrowImpl<C>* ptr)
		:ptr(std::forward<BorrowImpl<C>*>(ptr))
	{}
	Borrow() = default;
	//コピー可
	Borrow(const Borrow<C>& other) = default;
	Borrow<C>& operator = (const Borrow<C>&) = default;

	//借用先が削除されてたらfalse
	operator bool()const
	{
		return ptr and ptr.get()->get();//nullptrかポインタ
	}

	operator C* ()const
	{
		return ptr.get()->get();
	}

	C* operator -> ()const
	{
		return ptr.get()->get();
	}

	template<class T, std::enable_if_t<std::is_base_of_v<BaseBorrowable, T>>* = nullptr>
	operator Borrow<T>()const;
};
//貸出機能など
class Lender :public BaseLender
{
public:
	using BaseLender::BaseLender;

	template<class C, std::enable_if_t<std::is_base_of_v<BaseBorrowable, C>>* = nullptr>
	Borrow<C> lend()
	{
		auto bor = new BorrowImpl<C>(this);//thisを使って貸し出す
		bor->_init();
		callbacks.emplace(bor, [=](BaseLender* next) {bor->lender = next; bor->_init(); });
		return Borrow<C>(bor);
	};
};
//キャスト
template<class C>
template<class T, std::enable_if_t<std::is_base_of_v<BaseBorrowable, T>>*>
Borrow<C>::operator Borrow<T>()const
{
	if (ptr->get())return ptr->cast_lender->lend<T>();
	else return Borrow<T>(nullptr);
};

//これを継承するだけ
class Borrowable :public BaseBorrowable
{
public:
	using BaseBorrowable::BaseBorrowable;

	template<class Self>
	Borrow<Self> lend(this Self& self)
	{
		return self.lender->lend<Self>();
	};

	template<class T>
	Borrow<T> lend()
	{
		return lender->lend<T>();
	};

	template<class T>
	operator Borrow<T>()
	{
		return lender->lend<T>();
	}
};
