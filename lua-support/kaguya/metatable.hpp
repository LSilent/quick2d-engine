// Copyright satoren
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <iostream>

#include "kaguya/config.hpp"
#include "kaguya/native_function.hpp"


#include "kaguya/lua_ref_function.hpp"

namespace kaguya
{
	template<class A1, class A2 = void
		, class A3 = void, class A4 = void, class A5 = void
		, class A6 = void, class A7 = void, class A8 = void,
	class A9 = void>struct MultipleBase {
	};


	template<typename class_type, typename base_class_type = void>
	struct ClassMetatable
	{

		struct DataHolderBase
		{
			virtual int push_to_lua(lua_State* data)const = 0;
			virtual ~DataHolderBase() {}
		};
		template<typename T>
		struct DataHolder :DataHolderBase
		{
			DataHolder(const T& d) :data(d) {}
			T data;
			virtual int push_to_lua(lua_State* state)const
			{
				return lua_type_traits<T>::push(state, data);
			}
		};
		template<int N>
		struct DataHolder<char[N]> :DataHolderBase {
			std::string data;
			DataHolder(const std::string& d) :data(d) {}
			virtual int push_to_lua(lua_State* state)const
			{
				return lua_type_traits<std::string>::push(state, data);
			}
		};
		template<int N>
		struct DataHolder<const char[N]> :DataHolderBase {
			std::string data;
			DataHolder(const std::string& d) :data(d) {}
			virtual int push_to_lua(lua_State* state)const
			{
				return lua_type_traits<std::string>::push(state, data);
			}
		};
#if KAGUYA_USE_CPP11
		template<typename T>
		struct MoveDataHolder :DataHolderBase
		{
			MoveDataHolder(T&& d) :data(std::move(d)) {}
			T data;
			virtual int push_to_lua(lua_State* state)const
			{
				return lua_type_traits<T>::push(state, std::move(data));
			}
		};
		template<int N>
		struct MoveDataHolder<char[N]> :DataHolderBase {
			std::string data;
			MoveDataHolder(const std::string& d) :data(d) {}
			virtual int push_to_lua(lua_State* state)const
			{
				return lua_type_traits<std::string>::push(state, data);
			}
		};
		template<int N>
		struct MoveDataHolder<const char[N]> :DataHolderBase {
			std::string data;
			MoveDataHolder(const std::string& d) :data(d) {}
			virtual int push_to_lua(lua_State* state)const
			{
				return lua_type_traits<std::string>::push(state, data);
			}
		};
#endif
		typedef standard::shared_ptr<DataHolderBase> DataHolderType;

		typedef std::vector<FunctorType> FunctorOverloadType;
		typedef std::map<std::string, FunctorOverloadType> FuncMapType;
		typedef std::map<std::string, FunctorType> PropMapType;

		typedef std::map<std::string, DataHolderType> MemberMapType;
		typedef std::map<std::string, std::string> CodeChunkMapType;


		ClassMetatable()
		{
			FunctorType dtor(&class_userdata::destructor<ObjectWrapperBase>);
			function_map_["__gc"].push_back(dtor);

			KAGUYA_STATIC_ASSERT(is_registerable<class_type>::value || !traits::is_std_vector<class_type>::value, "std::vector is binding to lua-table by default.If you wants register for std::vector yourself,"
				"please define KAGUYA_NO_STD_VECTOR_TO_TABLE");

			KAGUYA_STATIC_ASSERT(is_registerable<class_type>::value || !traits::is_std_map<class_type>::value, "std::map is binding to lua-table by default.If you wants register for std::map yourself,"
				"please define KAGUYA_NO_STD_MAP_TO_TABLE");

			//can not register push specialized class
			KAGUYA_STATIC_ASSERT(is_registerable<class_type>::value,
				"Can not register specialized of type conversion class. e.g. std::tuple");
		}

		LuaRef registerClass(lua_State* state)const
		{
			util::ScopedSavedStack save(state);
			if (class_userdata::newmetatable<class_type>(state))
			{
				LuaTable metatable(state, StackTop());
				metatable.push();
				registerMember(state);

				if (!traits::is_same<base_class_type, void>::value || !property_map_.empty())//if base class has property and derived class hasnt property. need property access metamethod
				{
					for (PropMapType::const_iterator it = property_map_.begin(); it != property_map_.end(); ++it)
					{
						int count = lua_type_traits<FunctorType>::push(state, it->second);
						if (count > 1)
						{
							lua_pop(state, count - 1);
							count = 1;
						}
						if (count == 1)
						{
							lua_setfield(state, -2, ("_prop_" + it->first).c_str());
						}
					}
					LuaFunction indexfun = kaguya::LuaFunction::loadstring(state, "local arg = {...};local metatable = arg[1];"
						"return function(table, index)"
//						" if type(table) == 'userdata' then "
						" local propfun = metatable['_prop_'..index];"
						" if propfun then return propfun(table) end "
//						" end "
						" return metatable[index]"
						" end")(metatable);

					metatable.setField("__index", indexfun);

					LuaFunction newindexfn = LuaFunction::loadstring(state, "local arg = {...};local metatable = arg[1];"
						" return function(table, index, value) "
						" if type(table) == 'userdata' then "
						" local propfun = metatable['_prop_'..index];"
						" if propfun then return propfun(table,value) end "
						" end "
						" rawset(table,index,value) "
						" end")(metatable);
					metatable.setField("__newindex", newindexfn);
				}
				else
				{
					metatable.setField("__index", metatable);
				}

				set_base_metatable(state, metatable, types::typetag<base_class_type>());

				return metatable;
			}
			else
			{
				except::OtherError(state, typeid(class_type*).name() + std::string(" is already registered"));
			}
			return LuaRef(state);
		}

#if KAGUYA_USE_CPP11
		template<typename... Args>
		ClassMetatable& addConstructor()
		{
			typedef typename nativefunction::constructor_signature_type<class_type, Args...> cons;
			function_map_["new"].push_back(cons());
			return *this;
		}
#else
#define KAGUYA_TEMPLATE_PARAMETER(N)
#define KAGUYA_ADD_CON_FN_DEF(N) \
	KAGUYA_TEMPLATE_PARAMETER(N)\
	inline ClassMetatable& addConstructor()\
	{\
		typedef typename nativefunction::constructor_signature_type<class_type KAGUYA_PP_TEMPLATE_ARG_REPEAT_CONCAT(N)> cons;\
		function_map_["new"].push_back(cons());\
		return *this;\
	}
		KAGUYA_ADD_CON_FN_DEF(0)
#undef KAGUYA_TEMPLATE_PARAMETER
#define KAGUYA_TEMPLATE_PARAMETER(N) template<KAGUYA_PP_TEMPLATE_DEF_REPEAT(N)>
			KAGUYA_PP_REPEAT_DEF(9, KAGUYA_ADD_CON_FN_DEF)
#undef KAGUYA_TEMPLATE_PARAMETER
#undef KAGUYA_ADD_CON_FN_DEF
#endif
			//variadic arguments constructor(receive const std::vector<LuaRef>&)
	//		template<>ClassMetatable& addConstructor(types::typetag<VariadicArgType>* )
			ClassMetatable& addConstructorVariadicArg()
		{
			typedef typename nativefunction::constructor_signature_type<class_type, VariadicArgType> cons;
			function_map_["new"].push_back(FunctorType(cons()));
			return *this;
		}

#if defined(_MSC_VER) && _MSC_VER <= 1800
		//deprecated  rename to addMemberFunction
			//can not write  Ret class_type::* f on MSC++2013
		template<typename Fun>
		ClassMetatable& addMember(const char* name, Fun f)
		{
			return addMemberFunction(name, f);
		}
		//can not write  Ret class_type::* f on MSC++2013
		template<typename Fun>
		ClassMetatable& addMemberFunction(const char* name, Fun f)
		{
			if (has_key(name, true))
			{
				//already registerd
				return *this;
			}
			addFunction(name, f);
			return *this;
		}
#else
		//deprecated  rename to addMemberFunction
		template<typename Ret>
		ClassMetatable& addMember(const char* name, Ret class_type::* f)
		{
			return addMemberFunction(name, f);
		}

		template<typename Ret>
		ClassMetatable& addMemberFunction(const char* name, Ret class_type::* f)
		{
			if (has_key(name, true))
			{
				//already registerd
				return *this;
			}
			addFunction(name, f);
			return *this;
		}
#endif
		//add member property
		template<typename Ret>
		ClassMetatable& addProperty(const char* name, Ret class_type::* mem)
		{
			property_map_[name] = FunctorType(mem);
			return *this;
		}


		//deprecated rename to addStaticFunction
		template<typename Fun>
		ClassMetatable& addStaticMember(const char* name, Fun f)
		{
			return addStaticFunction(name, f);
		}
		template<typename Fun>
		ClassMetatable& addStaticFunction(const char* name, Fun f)
		{
			if (has_key(name, true))
			{
				//already registerd
				return *this;
			}
			addFunction(name, f);
			return *this;
		}

		//add field to 
		ClassMetatable& addCodeChunkResult(const char* name, const std::string& lua_code_chunk)
		{
			code_chunk_map_[name] = lua_code_chunk;
			return *this;
		}


		template<typename Data>
		ClassMetatable& addStaticField(const char* name, const Data& f)
		{
			if (has_key(name))
			{
				//already registerd
				return *this;
			}
			member_map_[name] = DataHolderType(new DataHolder<Data>(f));
			return *this;
		}

#if KAGUYA_USE_CPP11
		template<typename Data>
		ClassMetatable& addStaticField(const char* name, Data&& f)
		{
			if (has_key(name))
			{
				//already registerd
				return *this;
			}
			member_map_[name] = DataHolderType(new MoveDataHolder<Data>(std::move(f)));
			return *this;
		}
#endif
	private:
		template<typename T, typename F>
		static void* base_pointer_cast(void* from)
		{
			return static_cast<T*>(static_cast<F*>(from));
		}

		void set_base_metatable(lua_State* state, LuaTable& metatable, types::typetag<void>)const
		{
		}
		template<class Base>
		void set_base_metatable(lua_State* state, LuaTable& metatable, types::typetag<Base>)const
		{
			class_userdata::get_metatable<Base>(state);
			metatable.setMetatable(LuaTable(state, StackTop()));

			PointerConverter& pconverter = PointerConverter::get(state);
			pconverter.add_type_conversion<Base, class_type>();
		}

		void set_multiple_base(lua_State* state, LuaTable& metatable, const LuaTable& metabases)const
		{
			LuaTable newmeta(state, NewTable());

			LuaFunction indexfun = kaguya::LuaFunction::loadstring(state, "local arg = {...};local metabases = arg[1];"
				"return function(t, k)"
				" for i = 1,#metabases do "
				" local v = metabases[i][k] "
				" if v then "
				" t[k] = v "
				" return v end "
				" end"
				" end")(metabases);

			newmeta.setField("__index", indexfun);

			metatable.setMetatable(newmeta);
		
		}
#if KAGUYA_USE_CPP11

		template<typename Base>
		void metatables(lua_State* state, LuaTable& metabases, PointerConverter& pvtreg, types::typetag<MultipleBase<Base> >)const
		{
			class_userdata::get_metatable<Base>(state);
			metabases.setField(metabases.size() + 1, LuaTable(state, StackTop()));
			pvtreg.add_type_conversion<Base, class_type>();
		}
		template<typename Base, typename... Remain>
		void metatables(lua_State* state, LuaTable& metabases, PointerConverter& pvtreg, types::typetag<MultipleBase<Base, Remain...> >)const
		{
			class_userdata::get_metatable<Base>(state);
			metabases.setField(metabases.size() + 1, LuaTable(state, StackTop()));
			pvtreg.add_type_conversion<Base, class_type>();
			metatables(state, metabases, pvtreg, types::typetag<MultipleBase<Remain...> >());
		}

		template<typename... Bases>
		void set_base_metatable(lua_State* state, LuaTable& metatable, types::typetag<MultipleBase<Bases...> > metatypes)const
		{
			PointerConverter& pconverter = PointerConverter::get(state);
			LuaTable metabases(state, NewTable(sizeof...(Bases), 0));
			metatables(state, metabases, pconverter, metatypes);
			set_multiple_base(state, metatable, metabases);
		}

#else
#define KAGUYA_TEMPLATE_PARAMETER(N) template<KAGUYA_PP_TEMPLATE_DEF_REPEAT(N)>
#define KAGUYA_GET_BASE_METATABLE(N) class_userdata::get_metatable<KAGUYA_PP_CAT(A,N)>(state);\
		metabases.setField(metabases.size() + 1, LuaTable(state, StackTop())); \
		pconverter.add_type_conversion<KAGUYA_PP_CAT(A,N),class_type>();
#define KAGUYA_MULTIPLE_INHERITANCE_SETBASE_DEF(N) \
	KAGUYA_TEMPLATE_PARAMETER(N)\
		void set_base_metatable(lua_State* state, LuaTable& metatable, types::typetag<MultipleBase<KAGUYA_PP_TEMPLATE_ARG_REPEAT(N)> > metatypes)const\
		{\
			PointerConverter& pconverter = PointerConverter::get(state);\
			LuaTable metabases(state, NewTable(N, 0));\
			KAGUYA_PP_REPEAT(N, KAGUYA_GET_BASE_METATABLE); \
			set_multiple_base(state, metatable, metabases);\
		}\

		KAGUYA_PP_REPEAT_DEF(9, KAGUYA_MULTIPLE_INHERITANCE_SETBASE_DEF)
#undef KAGUYA_TEMPLATE_PARAMETER
#undef KAGUYA_MULTIPLE_INHERITANCE_SETBASE_DEF
#undef KAGUYA_GET_BASE_METATABLE
#undef KAGUYA_TYPE_CHECK_REP
#endif

			bool has_key(const std::string& key, bool exclude_function = false)
		{
			if (!exclude_function && function_map_.find(key) != function_map_.end())
			{
				return true;
			}
			if (member_map_.find(key) != member_map_.end())
			{
				return true;
			}
			return false;
		}
		void registerFunction(lua_State* state, const char* name, const FunctorOverloadType& func_array)const
		{
			int count = lua_type_traits<FunctorOverloadType>::push(state, func_array);
			if (count > 1)
			{
				lua_pop(state, count - 1);
				count = 1;
			}
			if (count == 1)
			{
				lua_setfield(state, -2, name);
			}
		}
		void registerField(lua_State* state, const char* name, const DataHolderType& value)const
		{
			int count = value->push_to_lua(state);
			if (count > 1)
			{
				lua_pop(state, count - 1);
				count = 1;
			}
			if (count == 1)
			{
				lua_setfield(state, -2, name);
			}
		}
		void registerCodeChunk(lua_State* state, const char* name, std::string value)const
		{
			util::ScopedSavedStack save(state);
			int status = luaL_loadstring(state, value.c_str());
			if (!except::checkErrorAndThrow(status, state)) { return; }
			status = lua_pcall_wrap(state, 0, 1);
			if (!except::checkErrorAndThrow(status, state)) { return; }
			lua_setfield(state, -2, name);
		}

		void registerMember(lua_State* state)const
		{
			for (typename FuncMapType::const_iterator it = function_map_.begin(); it != function_map_.end(); ++it)
			{
				registerFunction(state, it->first.c_str(), it->second);
			}
			for (typename MemberMapType::const_iterator it = member_map_.begin(); it != member_map_.end(); ++it)
			{
				registerField(state, it->first.c_str(), it->second);
			}
			for (typename CodeChunkMapType::const_iterator it = code_chunk_map_.begin(); it != code_chunk_map_.end(); ++it)
			{
				registerCodeChunk(state, it->first.c_str(), it->second);
			}
		}
		static bool is_metafield(const std::string& name)
		{
			return name.compare(0, 2, "__") == 0;
		}
		template<typename Fun>
		ClassMetatable& addFunction(const char* name, Fun f)
		{
			function_map_[name].push_back(FunctorType(f));
			return *this;
		}

		FuncMapType function_map_;
		PropMapType property_map_;
		MemberMapType member_map_;
		CodeChunkMapType code_chunk_map_;
	};
}
