// Copyright satoren
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>
#include <cstring>
#include <typeinfo>
#include <algorithm>

#include "kaguya/config.hpp"
#include "kaguya/traits.hpp"
#include "kaguya/exception.hpp"

namespace kaguya
{
	namespace types
	{
		template<typename T>
		struct typetag {};
	}

#define KAGUYA_METATABLE_PREFIX "kaguya_object_type_"
#define KAGUYA_METATABLE_TYPE_NAME_KEY -212114

	template<typename T>
	inline const std::string& metatableName()
	{
		typedef typename traits::remove_cv<T>::type noncv_type;
		typedef typename traits::remove_pointer<noncv_type>::type noncvpointer_type;
		typedef typename traits::remove_const_and_reference<noncvpointer_type>::type noncvpointerref_type;

		static const std::string v = std::string(KAGUYA_METATABLE_PREFIX) + typeid(noncvpointerref_type*).name();

		return v;
	}
	template<typename T>
	const std::type_info& metatableType()
	{
		typedef typename traits::remove_cv<T>::type noncv_type;
		typedef typename traits::remove_pointer<noncv_type>::type noncvpointer_type;
		typedef typename traits::remove_const_and_reference<noncvpointer_type>::type noncvpointerref_type;
		return typeid(noncvpointerref_type*);
	}

	namespace class_userdata
	{
		template<typename T>bool get_metatable(lua_State* l)
		{
			luaL_getmetatable(l, metatableName<T>().c_str());
			return !lua_isnil(l, -1);
		}
		template<typename T>bool available_metatable(lua_State* l)
		{
			util::ScopedSavedStack save(l);
			return get_metatable<T>(l);
		}
		inline bool newmetatable(lua_State* l,const char* metatablename)
		{
			if (luaL_newmetatable(l, metatablename))
			{
#if LUA_VERSION_NUM < 503
				lua_pushstring(l, metatablename);
				lua_setfield(l, -2, "__name");
#endif
				lua_pushstring(l, metatablename);
				lua_rawseti(l, -2, KAGUYA_METATABLE_TYPE_NAME_KEY);
				return true;
			}
			return false;
		}
		template<typename T>bool newmetatable(lua_State* l)
		{
			return newmetatable(l, metatableName<T>().c_str());
		}
		template<typename T>void setmetatable(lua_State* l)
		{
			if (available_metatable<T>(l))
			{
				return luaL_setmetatable(l, metatableName<T>().c_str());
			}
			else
			{
				luaL_getmetatable(l, KAGUYA_METATABLE_PREFIX "unknown_type");

				if(lua_isnil(l, -1))
				{
					lua_pop(l,1);
					newmetatable(l, KAGUYA_METATABLE_PREFIX "unknown_type");
				}
				lua_setmetatable(l, -2);
			}
		}

		template<typename T>T* test_userdata(lua_State* l, int index)
		{
			return static_cast<T*>(luaL_testudata(l, index, metatableName<T>().c_str()));
		}

		template<typename T>inline void destructor(T* pointer)
		{
			if (pointer)
			{
				pointer->~T();
			}
		}
	}
	inline bool available_metatable(lua_State* l, const char* t)
	{
		util::ScopedSavedStack save(l);
		luaL_getmetatable(l, t);
		return !lua_isnil(l, -1);
	}
	template<typename T>
	bool available_metatable(lua_State* l, types::typetag<T> type = types::typetag<T>())
	{
		return class_userdata::available_metatable<T>(l);
	}

	struct ObjectWrapperBase
	{
		virtual bool is_native_type(const std::type_info& type) = 0;

		virtual const void* native_cget() = 0;
		virtual void* native_get() = 0;
		virtual const void* cget() = 0;
		virtual void* get() = 0;

		virtual const std::type_info& type() = 0;

//		virtual void addRef(lua_State* state, int index) {};

		ObjectWrapperBase() {}
		virtual ~ObjectWrapperBase() {}
	private:

		//noncopyable
		ObjectWrapperBase(const ObjectWrapperBase&);
		ObjectWrapperBase& operator=(const ObjectWrapperBase&);
	};

	template<class T>
	struct ObjectWrapper : ObjectWrapperBase
	{
#if KAGUYA_USE_CPP11
		template<class... Args>
		ObjectWrapper(Args&&... args) : object_(std::forward<Args>(args)...) {}
#else

		ObjectWrapper() : object_() {}
		template<class Arg1>
		ObjectWrapper(const Arg1& v1) : object_(v1) {}
		template<class Arg1, class Arg2>
		ObjectWrapper(const Arg1& v1, const Arg2& v2) : object_(v1, v2) {}
		template<class Arg1, class Arg2, class Arg3>
		ObjectWrapper(const Arg1& v1, const Arg2& v2, const Arg3& v3) : object_(v1, v2, v3) {}
		template<class Arg1, class Arg2, class Arg3, class Arg4>
		ObjectWrapper(const Arg1& v1, const Arg2& v2, const Arg3& v3, const Arg4& v4) : object_(v1, v2, v3, v4) {}
		template<class Arg1, class Arg2, class Arg3, class Arg4, class Arg5>
		ObjectWrapper(const Arg1& v1, const Arg2& v2, const Arg3& v3, const Arg4& v4, const Arg5& v5) : object_(v1, v2, v3, v4, v5) {}
		template<class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6>
		ObjectWrapper(const Arg1& v1, const Arg2& v2, const Arg3& v3, const Arg4& v4, const Arg5& v5, const Arg6& v6) : object_(v1, v2, v3, v4, v5, v6) {}
		template<class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7>
		ObjectWrapper(const Arg1& v1, const Arg2& v2, const Arg3& v3, const Arg4& v4, const Arg5& v5, const Arg6& v6, const Arg7& v7) : object_(v1, v2, v3, v4, v5, v6, v7) {}
		template<class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8>
		ObjectWrapper(const Arg1& v1, const Arg2& v2, const Arg3& v3, const Arg4& v4, const Arg5& v5, const Arg6& v6, const Arg7& v7, const Arg8& v8) : object_(v1, v2, v3, v4, v5, v6, v7, v8) {}
		template<class Arg1, class Arg2, class Arg3, class Arg4, class Arg5, class Arg6, class Arg7, class Arg8, class Arg9>
		ObjectWrapper(const Arg1& v1, const Arg2& v2, const Arg3& v3, const Arg4& v4, const Arg5& v5, const Arg6& v6, const Arg7& v7, const Arg8& v8, const Arg9& v9) : object_(v1, v2, v3, v4, v5, v6, v7, v8, v9) {}

#endif

		virtual bool is_native_type(const std::type_info& type)
		{
			return metatableType<T>() == type;
		}
		virtual const std::type_info& type()
		{
			return metatableType<T>();
		}

		virtual void* get()
		{
			return &object_;
		}
		virtual const void* cget()
		{
			return &object_;
		}
		virtual const void* native_cget() { return cget(); };
		virtual void* native_get() { return get(); };

	private:
		T object_;
	};


	struct ObjectSharedPointerWrapper : ObjectWrapperBase
	{

		template<typename T>
		ObjectSharedPointerWrapper(const standard::shared_ptr<T>& sptr) :object_(sptr), type_(metatableType<T>()),
			shared_ptr_type_(metatableType<standard::shared_ptr<T> >()) {}
#if KAGUYA_USE_RVALUE_REFERENCE
		template<typename T>
		ObjectSharedPointerWrapper(standard::shared_ptr<T>&& sptr) : object_(std::move(sptr)), type_(metatableType<T>()),
			shared_ptr_type_(metatableType<standard::shared_ptr<T> >()) {}
#endif
		virtual bool is_native_type(const std::type_info& type)
		{
			return shared_ptr_type_ == type;
		}
		virtual const std::type_info& type()
		{
			return type_;
		}
		virtual void* get()
		{
			return object_.get();
		}
		virtual const void* cget()
		{
			return object_.get();
		}
		virtual const void* native_cget() { return &object_; };
		virtual void* native_get() { return &object_; };

		const standard::shared_ptr<void> object()const { return object_; }
		const std::type_info& shared_ptr_type()const { return shared_ptr_type_; }
	private:
		standard::shared_ptr<void> object_;
		const std::type_info& type_;

		const std::type_info& shared_ptr_type_;
	};

	template<class T>
	struct ObjectSmartPointerWrapper : ObjectWrapperBase
	{
		ObjectSmartPointerWrapper(const T& sptr) :object_(sptr) {}
#if KAGUYA_USE_RVALUE_REFERENCE
		ObjectSmartPointerWrapper(T&& sptr) : object_(std::move(sptr)) {}
#endif
		virtual bool is_native_type(const std::type_info& type)
		{
			return metatableType<T>() == type;
		}
		virtual const std::type_info& type()
		{
			return metatableType<typename T::element_type>();
		}
		virtual void* get()
		{
			return object_.get();
		}
		virtual const void* cget()
		{
			return object_.get();
		}
		virtual const void* native_cget() { return &object_; };
		virtual void* native_get() { return &object_; };

	private:
		T object_;
	};

	template<class T>
	struct ObjectPointerWrapper : ObjectWrapperBase
	{
		ObjectPointerWrapper(T* ptr) :object_(ptr) {}

		virtual bool is_native_type(const std::type_info& type)
		{
			return metatableType<T>() == type;
		}
		virtual const std::type_info& type()
		{
			return metatableType<T>();
		}
		virtual void* get()
		{
			if (traits::is_const<T>::value)
			{
				return 0;
			}
			return const_cast<void*>(static_cast<const void*>(object_));
		}
		virtual const void* cget()
		{
			return object_;
		}
		virtual const void* native_cget() { return cget(); };
		virtual void* native_get() { return get(); };

		~ObjectPointerWrapper()
		{
//			for (std::vector<std::pair<lua_State*, int> >::iterator i = retain_ref_.begin(); i != retain_ref_.end(); ++i)
//			{
//				luaL_unref(i->first, LUA_REGISTRYINDEX, i->second);
//			}
		}

//		virtual void addRef(lua_State* state, int index) {
//			lua_pushvalue(state, index);
//			retain_ref_.push_back(std::pair<lua_State*, int>(state, luaL_ref(state, LUA_REGISTRYINDEX)));
//		};
	private:
		T* object_;
//		std::vector<std::pair<lua_State*, int> > retain_ref_;
	};

	//for internal use
	struct PointerConverter
	{
		template<typename T, typename F>
		static void* base_pointer_cast(void* from)
		{
			return static_cast<T*>(static_cast<F*>(from));
		}
		template<typename T, typename F>
		static standard::shared_ptr<void> base_shared_pointer_cast(const standard::shared_ptr<void>& from)
		{
			return standard::shared_ptr<T>(standard::static_pointer_cast<F>(from));
		}


		typedef void* (*convert_function_type)(void*);
		typedef standard::shared_ptr<void>(*shared_ptr_convert_function_type)(const standard::shared_ptr<void>&);
		typedef std::pair<std::string, std::string> convert_map_key;


		template<typename ToType, typename FromType>
		void add_type_conversion()
		{
			add_function(metatableType<ToType>(), metatableType<FromType>(), &base_pointer_cast<ToType, FromType>);
			add_function(metatableType<standard::shared_ptr<ToType> >(), metatableType<standard::shared_ptr<FromType> >(), &base_shared_pointer_cast<ToType, FromType>);
		}


		template<typename TO>
		TO* get_pointer(ObjectWrapperBase* from)const
		{
			const std::type_info& to_type = metatableType<TO>();
			if (to_type == from->type())
			{
				return static_cast<TO*>(from->get());
			}
			std::map<convert_map_key, std::vector<convert_function_type> >::const_iterator match = function_map_.find(convert_map_key(to_type.name(), from->type().name()));
			if (match != function_map_.end())
			{
				return static_cast<TO*>(pcvt_list_apply(from->get(), match->second));
			}
			return 0;

		}
		template<typename TO>
		const TO* get_const_pointer(ObjectWrapperBase* from)const
		{
			const std::type_info& to_type = metatableType<TO>();
			if (to_type == from->type())
			{
				return static_cast<const TO*>(from->cget());
			}
			std::map<convert_map_key, std::vector<convert_function_type> >::const_iterator match = function_map_.find(convert_map_key(to_type.name(), from->type().name()));
			if (match != function_map_.end())
			{
				return static_cast<const TO*>(pcvt_list_apply(const_cast<void*>(from->cget()), match->second));
			}
			return 0;
		}

		template<typename TO>
		standard::shared_ptr<TO> get_shared_pointer(ObjectWrapperBase* from)const
		{
			const std::type_info& to_type = metatableType<standard::shared_ptr<TO> >();
			ObjectSharedPointerWrapper* ptr = dynamic_cast<ObjectSharedPointerWrapper*>(from);
			if (!ptr) {
				return standard::shared_ptr<TO>();
			}
			if (to_type == from->type())
			{
				return standard::static_pointer_cast<TO>(ptr->object());
			}
			std::map<convert_map_key, std::vector<shared_ptr_convert_function_type> >::const_iterator match = shared_ptr_function_map_.find(convert_map_key(to_type.name(), ptr->shared_ptr_type().name()));
			if (match != shared_ptr_function_map_.end())
			{
				return standard::static_pointer_cast<TO>(pcvt_list_apply(ptr->object(), match->second));
			}
			return standard::shared_ptr<TO>();
		}
		standard::shared_ptr<void> get_shared_pointer(ObjectWrapperBase* from)const
		{
			ObjectSharedPointerWrapper* ptr = dynamic_cast<ObjectSharedPointerWrapper*>(from);
			if (ptr) {
				return ptr->object();
			}
			return standard::shared_ptr<void>();
		}


		template<class T>
		T* get_pointer(ObjectWrapperBase* from, types::typetag<T> tag)
		{
			return get_pointer<T>(from);
		}
		template<class T>
		const T* get_pointer(ObjectWrapperBase* from, types::typetag<const T> tag)
		{
			return get_const_pointer<T>(from);
		}
		template<class T>
		standard::shared_ptr<T> get_pointer(ObjectWrapperBase* from, types::typetag<standard::shared_ptr<T> > tag)
		{
			return get_shared_pointer<T>(from);
		}


		static int deleter(lua_State *state)
		{
			PointerConverter* ptr = (PointerConverter*)lua_touserdata(state, 1);
			ptr->~PointerConverter();
			return 0;
		}

		static PointerConverter& get(lua_State* state)
		{
			static char kaguya_ptrcvt_key_ptr;
			util::ScopedSavedStack save(state);
			lua_pushlightuserdata(state, &kaguya_ptrcvt_key_ptr);
			lua_gettable(state, LUA_REGISTRYINDEX);
			if (lua_isuserdata(state, -1))
			{
				return *static_cast<PointerConverter*>(lua_touserdata(state, -1));
			}
			else
			{
				void* ptr = lua_newuserdata(state, sizeof(PointerConverter));//dummy data for gc call
				if (!ptr) { throw std::runtime_error("critical error. maybe failed memory allocation"); }//critical error
				PointerConverter* converter = new(ptr) PointerConverter();
				if (!converter) { throw std::runtime_error("critical error. maybe failed memory allocation"); }//critical error

				lua_createtable(state, 0, 0);
				lua_pushcclosure(state, &deleter, 0);
				lua_setfield(state, -2, "__gc");
				lua_pushvalue(state, -1);
				lua_setfield(state, -2, "__index");
				lua_setmetatable(state, -2);//set to userdata
				lua_pushlightuserdata(state, &kaguya_ptrcvt_key_ptr);
				lua_pushvalue(state, -2);
				lua_settable(state, LUA_REGISTRYINDEX);
				return *converter;
			}
		}
	private:
		void add_function(const std::type_info& to_type, const std::type_info& from_type, convert_function_type f)
		{
			std::map<convert_map_key, std::vector<convert_function_type> > add_map;
			for (std::map<convert_map_key, std::vector<convert_function_type> >::iterator it = function_map_.begin();
			it != function_map_.end(); ++it)
			{
//				if (it->first.first == from_type.name())
//				{
//					std::vector<convert_function_type> newlist = it->second;
//					newlist.push_back(f);
//					add_map[convert_map_key(to_type.name(), it->first.second)] = newlist;
//				}

				if (it->first.second == to_type.name())
				{
					std::vector<convert_function_type> newlist;
					newlist.push_back(f);
					newlist.insert(newlist.end(), it->second.begin(), it->second.end());
					add_map[convert_map_key(it->first.first, from_type.name())] = newlist;
				}
			}
			function_map_.insert(add_map.begin(), add_map.end());

			std::vector<convert_function_type> flist; flist.push_back(f);
			function_map_[convert_map_key(to_type.name(), from_type.name())] = flist;
		}
		void add_function(const std::type_info& to_type, const std::type_info& from_type, shared_ptr_convert_function_type f)
		{
			std::map<convert_map_key, std::vector<shared_ptr_convert_function_type> > add_map;
			for (std::map<convert_map_key, std::vector<shared_ptr_convert_function_type> >::iterator it = shared_ptr_function_map_.begin();
			it != shared_ptr_function_map_.end(); ++it)
			{
//				if (it->first.first == from_type.name())
//				{
//					std::vector<shared_ptr_convert_function_type> newlist = it->second;
//					newlist.push_back(f);
//					add_map[convert_map_key(to_type.name(), it->first.second)] = newlist;
//				}

				if (it->first.second == to_type.name())
				{
					std::vector<shared_ptr_convert_function_type> newlist;
					newlist.push_back(f);
					newlist.insert(newlist.end(), it->second.begin(), it->second.end());
					add_map[convert_map_key(it->first.first, from_type.name())] = newlist;
				}
			}
			shared_ptr_function_map_.insert(add_map.begin(), add_map.end());

			std::vector<shared_ptr_convert_function_type> flist; flist.push_back(f);
			shared_ptr_function_map_[convert_map_key(to_type.name(), from_type.name())] = flist;
		}

		void* pcvt_list_apply(void* ptr, const std::vector<convert_function_type>& flist)const
		{
			for (std::vector<convert_function_type>::const_iterator i = flist.begin(); i != flist.end(); ++i)
			{
				ptr = (*i)(ptr);
			}
			return ptr;
		}
		standard::shared_ptr<void> pcvt_list_apply(standard::shared_ptr<void> ptr, const std::vector<shared_ptr_convert_function_type>& flist)const
		{
			for (std::vector<shared_ptr_convert_function_type>::const_iterator i = flist.begin(); i != flist.end(); ++i)
			{

#if KAGUYA_USE_CPP11
				ptr = (*i)(std::move(ptr));
#else
				ptr = (*i)(ptr);
#endif
			}
			return ptr;
		}


		PointerConverter() {}

		std::map<convert_map_key, std::vector<convert_function_type> > function_map_;
		std::map<convert_map_key, std::vector<shared_ptr_convert_function_type> > shared_ptr_function_map_;



		PointerConverter(PointerConverter&);
		PointerConverter& operator=(PointerConverter&);
	};

	namespace detail
	{
		inline bool object_wrapper_type_check(lua_State* l, int index)
		{
			if (lua_getmetatable(l, index))
			{
				lua_rawgeti(l, -1, KAGUYA_METATABLE_TYPE_NAME_KEY);
				const char* metatable_name = lua_tostring(l, -1);

				lua_pop(l, 2);
				return metatable_name != 0;
			}
			return false;
		}
	}

	inline ObjectWrapperBase* object_wrapper(lua_State* l, int index)
	{
		if (detail::object_wrapper_type_check(l, index))
		{
			ObjectWrapperBase* ptr = static_cast<ObjectWrapperBase*>(lua_touserdata(l, index));
			return ptr;
		}
		return 0;
	}

	template<typename RequireType>
	inline ObjectWrapperBase* object_wrapper(lua_State* l, int index,bool convert = true)
	{
		if (detail::object_wrapper_type_check(l, index))
		{
			ObjectWrapperBase* ptr = static_cast<ObjectWrapperBase*>(lua_touserdata(l, index));

			if (ptr->is_native_type(metatableType<RequireType>()))
			{
				return ptr;
			}
			else if (ptr->type() == metatableType<RequireType>())
			{
				return ptr;
			}
			else if(convert)
			{
				PointerConverter& pcvt = PointerConverter::get(l);
				return pcvt.get_pointer(ptr, types::typetag<RequireType>()) ? ptr : 0;
			}
			return 0;
		}
		return 0;
	}

	template<class T>
	T* get_pointer(lua_State* l, int index, types::typetag<T> tag)
	{
		int type = lua_type(l, index);

		if (type == LUA_TLIGHTUSERDATA)
		{
			return (T*)lua_topointer(l, index);
		}
		else if (type != LUA_TUSERDATA)
		{
			return 0;
		}
		else
		{
			ObjectWrapperBase* objwrapper = object_wrapper(l, index);
			if (objwrapper)
			{
				if (static_cast<ObjectWrapperBase*>(objwrapper)->is_native_type(metatableType<T>()))
				{
					return static_cast<T*>(objwrapper->native_get());
				}
				else if (objwrapper->type() == metatableType<T>())
				{
					return static_cast<T*>(objwrapper->get());
				}
				else
				{
					PointerConverter& pcvt = PointerConverter::get(l);
					return pcvt.get_pointer<T>(objwrapper);
				}
			}
		}
		return 0;
	}
	template<class T>
	const T* get_const_pointer(lua_State* l, int index, types::typetag<T> tag)
	{
		int type = lua_type(l, index);

		if (type == LUA_TLIGHTUSERDATA)
		{
			return (T*)lua_topointer(l, index);
		}
		else if (type != LUA_TUSERDATA)
		{
			return 0;
		}
		else
		{
			ObjectWrapperBase* objwrapper = object_wrapper(l, index);
			if (objwrapper)
			{
				if (objwrapper->is_native_type(metatableType<T>()))
				{
					return static_cast<const T*>(objwrapper->native_cget());
				}
				else if (objwrapper->type() == metatableType<T>())
				{
					return static_cast<const T*>(objwrapper->cget());
				}
				else
				{
					PointerConverter& pcvt = PointerConverter::get(l);
					return pcvt.get_const_pointer<T>(objwrapper);
				}
			}
		}
		return 0;
	}



	template<class T>
	standard::shared_ptr<T> get_shared_pointer(lua_State* l, int index, types::typetag<T> tag)
	{

		ObjectWrapperBase* objwrapper = object_wrapper(l, index);
		if (objwrapper)
		{
			if (static_cast<ObjectWrapperBase*>(objwrapper)->is_native_type(metatableType<standard::shared_ptr<T> >()))
			{
				standard::shared_ptr<T>* ptr = static_cast<standard::shared_ptr<T>*>(objwrapper->native_get());
				if (ptr) { return *ptr; }
			}
			else
			{
				PointerConverter& pcvt = PointerConverter::get(l);
				return pcvt.get_shared_pointer<T>(objwrapper);
			}
		}
		return standard::shared_ptr<T>();
	}
}
