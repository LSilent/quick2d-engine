// Copyright satoren
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <vector>
#include <map>
#include <cassert>
#include "kaguya/config.hpp"
#include "kaguya/lua_ref.hpp"
#include "kaguya/exception.hpp"
#include "kaguya/type.hpp"
#include "kaguya/utility.hpp"


#include "kaguya/detail/lua_function_def.hpp"
#include "kaguya/detail/lua_variant_def.hpp"

namespace kaguya
{
	class FunctionResults :public Ref::StackRef, public LuaVariantImpl<FunctionResults>
	{
		FunctionResults(lua_State* state, int startIndex) :Ref::StackRef(state, startIndex, true),state_(state), resultCount_(lua_gettop(state) + 1 - startIndex)
		{
		}
		FunctionResults(lua_State* state, int startIndex, int endIndex) :Ref::StackRef(state, startIndex, true), state_(state), resultCount_(endIndex - startIndex)
		{
		}
		friend class FunctionResultProxy;
	public:
		FunctionResults() :Ref::StackRef(0, 0, true), state_(0), resultCount_(0)
		{
		}
		FunctionResults(lua_State* state) :Ref::StackRef(state, 0, true), state_(state), resultCount_(0)
		{
		}
#if KAGUYA_USE_CPP11
		FunctionResults(FunctionResults&&src) : Ref::StackRef(std::move(src)), state_(src.state_), resultCount_(src.resultCount_)
		{
			src.state_ = 0;
		}
#else
		FunctionResults(const FunctionResults&src) : Ref::StackRef(src), state_(src.state_), resultCount_(src.resultCount_)
		{
			src.state_ = 0;
		}
#endif

		~FunctionResults()
		{
		}

		struct reference :public Ref::StackRef, public LuaVariantImpl<reference>
		{
			reference(lua_State* s, int index) :Ref::StackRef(s, index, false)
			{
			}

			const reference* operator->()const
			{
				return this;
			}
		};
		struct iterator
		{
			iterator(lua_State* s, int i) : state(s), stack_index(i)
			{
			}
			reference operator*()const
			{
				return reference(state, stack_index);
			}
			reference operator->()const
			{
				return reference(state, stack_index);
			}
			const iterator& operator++()
			{
				stack_index++;
				return *this;
			}
			iterator operator++(int)
			{
				return iterator(state, stack_index++);
			}

			iterator operator+=(int n)
			{
				stack_index += n;
				return iterator(state, stack_index);
			}
			bool operator==(const iterator& other)const
			{
				return state == other.state && stack_index == other.stack_index;
			}
			bool operator!=(const iterator& other)const
			{
				return !(*this == other);
			}
			int index() const { return stack_index; }
		private:
			lua_State* state;
			int stack_index;
		};
		typedef iterator const_iterator;
		typedef reference const_reference;

		iterator begin()
		{
			return iterator(state_, stack_index_);
		}
		iterator end()
		{
			return iterator(state_, stack_index_ + resultCount_);
		}
		const_iterator begin()const
		{
			return const_iterator(state_, stack_index_);
		}
		const_iterator end()const
		{
			return const_iterator(state_, stack_index_ + resultCount_);
		}
		const_iterator cbegin()const
		{
			return const_iterator(state_, stack_index_);
		}
		const_iterator cend()const
		{
			return const_iterator(state_, stack_index_ + resultCount_);
		}

		
		template<class Result>
		Result get_result()const
		{
			return util::get_result<Result>(state_, stack_index_);
		}
		lua_State* state()const { return state_; }

		template<typename T>
		typename lua_type_traits<T>::get_type result_at(size_t index)const
		{
			if (index >= result_size())
			{
				throw std::out_of_range("function result out of range");
			}
			return lua_type_traits<T>::get(state_, stack_index_ + static_cast<int>(index));
		}
		reference result_at(size_t index)const
		{
			if (index >= result_size())
			{
				throw std::out_of_range("function result out of range");
			}
			return reference(state_, stack_index_ + static_cast<int>(index));
		}

		size_t result_size()const
		{
			return resultCount_;
		}
		

	private:
		mutable lua_State* state_;
		int resultCount_;

	};

	template<typename RetType>
	inline RetType FunctionResultProxy::ReturnValue(lua_State* state, int retindex, types::typetag<RetType> tag)
	{
		return FunctionResults(state, retindex).get_result<RetType>();
	}
	inline FunctionResults FunctionResultProxy::ReturnValue(lua_State* state, int retindex, types::typetag<FunctionResults> tag)
	{
		return FunctionResults(state, retindex);
	}
	inline void FunctionResultProxy::ReturnValue(lua_State* state, int retindex, types::typetag<void> tag)
	{
		FunctionResults(state, retindex);
	}

#if KAGUYA_USE_CPP11
	template<typename Derived>template<class...Args>
	FunctionResults LuaFunctionImpl<Derived>::operator()(Args&&... args)
	{
		return this->template call<FunctionResults>(std::forward<Args>(args)...);
	}

	template<typename Derived>template<class...Args>
	FunctionResults LuaThreadImpl<Derived>::operator()(Args&&... args)
	{
		return this->template resume<FunctionResults>(std::forward<Args>(args)...);
	}
	template<typename Derived>template<class...Args>
	FunctionResults LuaVariantImpl<Derived>::operator()(Args&&... args)
	{
		int t = type();
		if (t == LUA_TTHREAD)
		{
			return this->template resume<FunctionResults>(std::forward<Args>(args)...);
		}
		else if (t == LUA_TFUNCTION)
		{
			return this->template call<FunctionResults>(std::forward<Args>(args)...);
		}
		else
		{
			except::typeMismatchError(state_(), " is not function or thread");
			return FunctionResults(state_());
		}
	}
#else
#define KAGUYA_TEMPLATE_PARAMETER(N)template<typename Derived>
#define KAGUYA_FUNCTION_ARGS_DEF(N)
#define KAGUYA_CALL_ARGS(N)
#define KAGUYA_PP_FARG(N) const KAGUYA_PP_CAT(A,N)& KAGUYA_PP_CAT(a,N)

#define KAGUYA_OP_FN_DEF(N) \
	KAGUYA_TEMPLATE_PARAMETER(N)\
	inline FunctionResults LuaFunctionImpl<Derived>::operator()(KAGUYA_FUNCTION_ARGS_DEF(N))\
	{\
			return this->template call<FunctionResults>(KAGUYA_CALL_ARGS(N)); \
	}\
	KAGUYA_TEMPLATE_PARAMETER(N)\
	inline FunctionResults LuaThreadImpl<Derived>::operator()(KAGUYA_FUNCTION_ARGS_DEF(N))\
	{\
			return this->template resume<FunctionResults>(KAGUYA_CALL_ARGS(N)); \
	}\
	KAGUYA_TEMPLATE_PARAMETER(N)\
	inline FunctionResults LuaVariantImpl<Derived>::operator()(KAGUYA_FUNCTION_ARGS_DEF(N))\
	{\
			int t = type();\
			if (t == LUA_TTHREAD)\
			{\
				return this->template resume<FunctionResults>(KAGUYA_CALL_ARGS(N)); \
			}\
			else if (t == LUA_TFUNCTION)\
			{\
				return this->template call<FunctionResults>(KAGUYA_CALL_ARGS(N)); \
			}\
			else\
			{\
				except::typeMismatchError(state_(), " is not function or thread");\
				return FunctionResults(state_());\
			}\
	}

	KAGUYA_OP_FN_DEF(0)

#undef KAGUYA_TEMPLATE_PARAMETER
#undef KAGUYA_FUNCTION_ARGS_DEF
#undef KAGUYA_CALL_ARGS
#define KAGUYA_TEMPLATE_PARAMETER(N) template<typename Derived> template<KAGUYA_PP_REPEAT_ARG(N,KAGUYA_PP_TEMPLATE)>
#define KAGUYA_FUNCTION_ARGS_DEF(N) KAGUYA_PP_REPEAT_ARG(N,KAGUYA_PP_FARG)
#define KAGUYA_CALL_ARGS(N) KAGUYA_PP_REPEAT_ARG(N, KAGUYA_PUSH_ARG_DEF)

#define KAGUYA_PP_TEMPLATE(N) KAGUYA_PP_CAT(typename A,N)
#define KAGUYA_PUSH_ARG_DEF(N) KAGUYA_PP_CAT(a,N) 

		KAGUYA_PP_REPEAT_DEF(9, KAGUYA_OP_FN_DEF)
#undef KAGUYA_OP_FN_DEF
#undef KAGUYA_TEMPLATE_PARAMETER

#undef KAGUYA_CALL_ARGS
#undef KAGUYA_FUNCTION_ARGS_DEF
#undef KAGUYA_PUSH_ARG_DEF
#undef KAGUYA_PP_TEMPLATE
#undef KAGUYA_PP_FARG
#undef KAGUYA_CALL_DEF
#undef KAGUYA_OP_FN_DEF
#endif

	inline std::ostream& operator<<(std::ostream& os, const FunctionResults& res)
	{
		for (FunctionResults::const_iterator it = res.begin(); it != res.end(); ++it)
		{
			if (it != res.begin())
			{
				os << ",";
			}
			util::stackValueDump(os, res.state(), it.index());
		}

		return os;
	}


	template<>	struct lua_type_traits<FunctionResults> {
		static int push(lua_State* l, const FunctionResults& ref)
		{
			int size = 0;
			for (FunctionResults::const_iterator it = ref.cbegin(); it != ref.cend(); ++it)
			{
				size += it->push(l);
			}
			return size;
		}
	};

	template<>	struct lua_type_traits<FunctionResults::reference> {
		static int push(lua_State* l, const FunctionResults::reference& ref)
		{
			return ref.push(l);
		}
	};
	template <unsigned int I>
	FunctionResults::reference get(const FunctionResults& res) { return res.result_at(I); }

	template<>	struct lua_type_traits<LuaFunction> {
		typedef LuaFunction get_type;
		typedef LuaFunction push_type;

		static bool strictCheckType(lua_State* l, int index)
		{
			return lua_isfunction(l, index);
		}
		static bool checkType(lua_State* l, int index)
		{
			return lua_isfunction(l, index) || lua_isnil(l, index);
		}
		static LuaFunction get(lua_State* l, int index)
		{
			lua_pushvalue(l, index);
			return LuaFunction(l, StackTop());
		}
		static int push(lua_State* l, const LuaFunction& ref)
		{
			ref.push(l);
			return 1;
		}
	};
	template<>	struct lua_type_traits<const LuaFunction&> :lua_type_traits<LuaFunction> {};

	template<>	struct lua_type_traits<LuaThread> {
		typedef LuaThread get_type;
		typedef LuaThread push_type;

		static bool strictCheckType(lua_State* l, int index)
		{
			return lua_isthread(l, index);
		}
		static bool checkType(lua_State* l, int index)
		{
			return lua_isthread(l, index) || lua_isnil(l, index);
		}
		static LuaThread get(lua_State* l, int index)
		{
			lua_pushvalue(l, index);
			return LuaThread(l, StackTop());
		}
		static int push(lua_State* l, const LuaThread& ref)
		{
			ref.push(l);
			return 1;
		}
	};
	template<>	struct lua_type_traits<const LuaThread&> :lua_type_traits<LuaThread> {};



	/**
	* @brief table and function binder.
	* state["table"]->*"fun"() is table:fun() in Lua
	* @param arg... function args
	*/
	class mem_fun_binder
	{
	public:
		template<class T>
		mem_fun_binder(LuaRef self, T key) :self_(self), fun_(self_.getField(key))
		{
		}

#define KAGUYA_DELEGATE_LUAREF fun_
#define KAGUYA_DELEGATE_FIRST_ARG self_
#include "kaguya/delegate_to_luaref.inl"
#undef KAGUYA_DELEGATE_FIRST_ARG
#undef KAGUYA_DELEGATE_LUAREF

	private:
		LuaRef self_;//Table or Userdata
		LuaFunction fun_;
	};
	/*
	inline mem_fun_binder LuaRef::operator->*(const char* key)
	{
		return mem_fun_binder(*this, key);
	}*/
}
