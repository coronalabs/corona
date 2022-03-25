//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

// Generic C++ containers.  Problem: STL is murder on compile times,
// and is hard to debug.  These are substitutes that compile much
// faster and are somewhat easier to debug.  Not as featureful,
// efficient or hammered-on as STL though.  You can use STL implementations if you want

#ifndef CONTAINER_H
#define CONTAINER_H

#include <stdlib.h>
#include <string.h>	// for strcmp and friends
#include <new>	// for placement new
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <string>

// A smart (strong) pointer asserts that the pointed-to object will
// not go away as long as the strong pointer is valid.  "Owners" of an
// object should keep strong pointers; other objects should use a
// strong pointer temporarily while they are actively using the
// object, to prevent the object from being deleted.
template<class T>
class smart_ptr
{
public:
	smart_ptr(T *ptr)	:
		m_ptr(ptr)
	{
		if (m_ptr)
		{
			m_ptr->add_ref();
		}
	}

	smart_ptr() : m_ptr(NULL) {}
	smart_ptr(const smart_ptr<T>& s) :
		m_ptr(s.m_ptr)
	{
		if (m_ptr)
		{
			m_ptr->add_ref();
		}
	}

	~smart_ptr()
	{
		if (m_ptr)
		{
			m_ptr->drop_ref();
		}
	}

	//	operator bool() const { return m_ptr != NULL; }
	void	operator=(const smart_ptr<T> &s) { set_ref(s.m_ptr); }
	void	operator=(T* ptr) { set_ref(ptr); }
	//	void	operator=(const weak_ptr<T>& w);
	T *operator->() const { /*assert(m_ptr);*/ return m_ptr; }
	T *get() const { return m_ptr; }
	T& operator*() const { return *(T*) m_ptr; }
	operator T*() const { return m_ptr; }
	bool	operator==(const smart_ptr<T> &p) const { return m_ptr == p.m_ptr; }
	bool	operator!=(const smart_ptr<T> &p) const { return m_ptr != p.m_ptr; }
	bool	operator==(T *p) const { return m_ptr == p; }
	bool	operator!=(T *p) const { return m_ptr != p; }

	// Provide work-alikes for static_cast, dynamic_cast, implicit up-cast?  ("gentle_cast" a la ajb?)

private:
	void	set_ref(T *ptr)
	{
		if (ptr != m_ptr)
		{
			if (m_ptr)
			{
				m_ptr->drop_ref();
			}
			m_ptr = ptr;

			if (m_ptr)
			{
				m_ptr->add_ref();
			}
		}
	}

	//	friend weak_ptr;

	T *m_ptr;
};


// Helper for making objects that can have weak_ptr's.
class weak_proxy
{
public:
	weak_proxy()
		:
		m_ref_count(0),
		m_alive(true)
	{
	}

	// weak_ptr's call this to determine if their pointer is valid or not.
	bool	is_alive() const { return m_alive; }

	// Only the actual object should call this.
	void	notify_object_died() { m_alive = false; }

	void	add_ref()
	{
//		assert(m_ref_count >= 0);
		m_ref_count++;
	}
	void	drop_ref()
	{
//		assert(m_ref_count > 0);

		m_ref_count--;
		if (m_ref_count == 0)
		{
			// Now we die.
			delete this;
		}
	}

private:
	// Don't use these.
	weak_proxy(const weak_proxy &w) { /*assert(0);*/ }
	void	operator=(const weak_proxy &w) { /*assert(0); */}

	int m_ref_count;
	bool	m_alive;
};


// A weak pointer points at an object, but the object may be deleted
// at any time, in which case the weak pointer automatically becomes
// NULL.  The only way to use a weak pointer is by converting it to a
// strong pointer (i.e. for temporary use).
//
// The class pointed to must have a "weak_proxy* get_weak_proxy()" method.
//
// Usage idiom:
//
// if (smart_ptr<my_type> ptr = m_weak_ptr_to_my_type) { ... use ptr->whatever() safely in here ... }

template<class T>
class weak_ptr
{
public:
	weak_ptr()
		:
		m_ptr(0)
	{
	}

	weak_ptr(T *ptr)
		:
		m_ptr(0)
	{
		operator=(ptr);
	}

	weak_ptr(const smart_ptr<T> &ptr)
	{
		operator=(ptr.get());
	}

	// Default constructor and assignment from weak_ptr<T> are OK.

	void	operator=(T *ptr)
	{
		m_ptr = ptr;

		if (m_ptr)
		{
			m_proxy = m_ptr->get_weak_proxy();
			//			assert(m_proxy != NULL);
			//			assert(m_proxy->is_alive());
		}
		else
		{
			m_proxy = NULL;
		}
	}

	void	operator=(const smart_ptr<T> &ptr) { operator=(ptr.get()); }

	bool	operator==(const smart_ptr<T> &ptr) const
	{
		check_proxy();
		return m_ptr == ptr.get();
	}

	bool	operator!=(const smart_ptr<T> &ptr) const
	{
		check_proxy();
		return m_ptr != ptr.get();
	}

	bool	operator==(T *ptr) const
	{
		check_proxy();
		return m_ptr == ptr;
	}

	bool	operator!=(T *ptr) const
	{
		check_proxy();
		return m_ptr != ptr;
	}

	T *operator->() const
	{
		check_proxy();
		assert(m_ptr);
		return m_ptr;
	}

	T *get() const
	{
		check_proxy();
		return m_ptr;
	}

	// Conversion to smart_ptr.
	operator smart_ptr<T>()
	{
		check_proxy();
		return smart_ptr<T>(m_ptr);
	}

	bool	operator==(T *ptr) { check_proxy(); return m_ptr == ptr; }
	bool	operator==(const smart_ptr<T> &ptr) { check_proxy(); return m_ptr == ptr.get(); }

	// for hash< weak_ptr<...>, ...>
	bool	operator==(const weak_ptr<T> &ptr) const
	{
		check_proxy();
		ptr.check_proxy();
		return m_ptr == ptr.m_ptr;
	}

private:

	void check_proxy() const // Set m_ptr to NULL if the object died.
	{
		if (m_ptr)
		{
			assert(m_proxy != NULL);

			if (m_proxy->is_alive() == false)
			{
				// Underlying object went away.
				m_proxy = NULL;
				m_ptr = NULL;
			}
		}
	}

	mutable smart_ptr<weak_proxy> m_proxy;
	mutable T *m_ptr;
};

// For stuff that's tricky to keep track of w/r/t ownership & cleanup.
struct ref_counted
{
	ref_counted() :
		m_ref_count(0),
		m_weak_proxy(0)
	{
	}

	virtual ~ref_counted()
	{
		//assert(m_ref_count == 0);

		if (m_weak_proxy)
		{
			m_weak_proxy->notify_object_died();
			m_weak_proxy->drop_ref();
		}
	}

	void add_ref() const
	{
		//	assert(m_ref_count >= 0);
		m_ref_count++;
	}

	void	 drop_ref()
	{
		//assert(m_ref_count > 0);
		m_ref_count--;

		if (m_ref_count == 0)
		{
			// Delete me!
			delete this;
		}
	}

	int get_ref_count() const { return m_ref_count; }
	weak_proxy *get_weak_proxy() const
	{
		// By rights, somebody should be holding a ref to us.
		// Vitaly: Sometimes it not so, for example in the constructor of character
		// where this->ref_counted == 0fadd_frame_lab
		//assert(m_ref_count > 0);

		if (m_weak_proxy == NULL)
		{
			m_weak_proxy = new weak_proxy;
			m_weak_proxy->add_ref();
		}

		return m_weak_proxy;
	}


private:
	mutable int	m_ref_count;
	mutable weak_proxy *m_weak_proxy;
};

typedef signed char	Sint8;
typedef unsigned char	Uint8;
typedef unsigned short Uint16;
typedef signed short Sint16;
typedef unsigned short uint16;
typedef signed short sint16;
typedef unsigned int Uint32;
typedef signed int Sint32;
typedef unsigned int uint32;
typedef signed int sint32;

inline int fchop( float f ) { return (int) f; }	// replace w/ inline asm if desired
inline int frnd(float f) { return fchop(f + 0.5f); }	// replace with inline asm if desired

struct membuf
{
	membuf();
	membuf(const void* data, int size);
	membuf(const membuf& buf);
	membuf(const std::string& str);
	~membuf();

	inline int size() const { return m_size; }
	inline const void* data() const { return m_data + m_start; }
	inline void* data() { return m_data + m_start; }
	inline bool empty() const { return m_size == 0; }
	const char* c_str() const;

	void clear();

	void append(char ch);
	void append(const void* data, int size);
	void append(const membuf& buf);
	void append(const std::string& str);  // We do not append the terminating '\0'.

	void remove(int size);
	void erase(int size);
	void	operator=(const membuf& buf);

private:

	int m_size;
	int m_capacity;
	char* m_data;
	int m_start;
};

//
// variant
//
struct as_value
{
private:

	enum type
	{
		UNDEFINED,
		BOOLEAN,
		NUMBER,
		STRING,
		NIL,
		//OBJECT
	};
	type	m_type;

	mutable std::string	m_string;
	union
	{
		double m_number;
		bool m_bool;
		//as_object* m_object;
	};


public:

	// constructors
	as_value(float val);
	as_value(int val);
	as_value(unsigned int val);
	as_value(int64_t val);
	as_value(uint64_t val);
	as_value(double val);
	as_value(bool val);
	as_value(const char* str);
	as_value(const std::string& str);
	as_value();
	as_value(const as_value& v);
	//as_value(as_object* val);

	~as_value();

	const std::string& to_string() const;
	const char* c_str() const;
	double	to_number() const;
	int	to_int() const { return (int)to_number(); };
	float	to_float() const { return (float)to_number(); };
	bool	to_bool() const;
	//as_object* to_object() const;

	// These set_*()'s are more type-safe; should be used
	// in preference to generic overloaded set().  You are
	// more likely to get a warning/error if misused.
	void	set_undefined();
	void	set_nil();
	void	set_double(double val);
	//void	set_object(as_object* val);
	void	set_bool(bool val);
	void	set_string(const std::string& str);
	void	set_string(const char* str);
	void	set_int(int val);

	void	operator=(const as_value& v);
	bool	operator==(const as_value& v) const;
	bool	operator!=(const as_value& v) const;
	void	operator+=(double v) { set_double(to_number() + v); }

	inline bool is_string() const { return m_type == STRING; }
	inline bool is_number() const { return m_type == NUMBER; }
	inline bool is_bool() const { return m_type == BOOLEAN; }
	inline bool is_undefined() const { return m_type == UNDEFINED; }
	inline bool is_nil() const { return m_type == NIL; }
	//inline bool is_object() const { return m_type == OBJECT && m_object != NULL; }

};

#endif // CONTAINER_H
