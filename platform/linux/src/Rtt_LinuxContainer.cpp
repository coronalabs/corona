//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_LinuxContainer.h"
#include <stdio.h>
#include <math.h>

using namespace std;

membuf::membuf()
	: m_size(0)
	, m_capacity(0)
	, m_data(0)
	, m_start(0)
{
}

membuf::membuf(const void* data, int size)
	: m_size(0)
	, m_capacity(0)
	, m_data(0)
	, m_start(0)
{
	append(data, size);
}

membuf::membuf(const membuf& buf)
	: m_size(0)
	, m_capacity(0)
	, m_data(0)
	, m_start(0)
{
	append(buf);
}

membuf::membuf(const string& str)
	: m_size(0)
	, m_capacity(0)
	, m_data(0)
	, m_start(0)
{
	append(str);
}

membuf::~membuf()
{
	clear();
}

void membuf::operator=(const membuf& buf)
{
	clear();
	append(buf.data(), buf.size());
}

void membuf::clear()
{
	if (m_data)
		free(m_data);

	m_capacity = 0;
	m_data = NULL;
	m_size = 0;
	m_start = 0;
}

void membuf::append(const void* data, int datasize)
{
	if (datasize > 0)
	{
		int old_size = m_size + m_start;
		int new_size = old_size + datasize;

		// resize if need

		static const int BLOCKSIZE = 4096;
		int new_capacity = (new_size + BLOCKSIZE) & ~(BLOCKSIZE - 1); // +1 for ZERO at end

		if (m_data == NULL)
			m_data = (char*)malloc(new_capacity);
		else
		{
			if (new_capacity != m_capacity)
				m_data = (char*)realloc(m_data, new_capacity);
		}
		m_capacity = new_capacity;

		memcpy(m_data + old_size, data, datasize);
		m_size += datasize;
	}
}

void membuf::append(const membuf& buf)
{
	append(buf.data(), buf.size());
}

void membuf::append(const string& str)
{
	append(str.c_str(), (int)str.size());
}

void membuf::append(char ch)
{
	append(&ch, 1);
}

// delete len bytes from back
void membuf::erase(int len)
{
	if (len > 0)
	{
		if (len < m_size)
		{
			m_size -= len;
		}
		else
			clear();
	}
}

void membuf::remove(int len)
{
	if (len > 0)
	{
		if (len >= m_size)
			clear();
		else
		{
			m_size -= len;
			m_start += len;
		}
	}
}

const char* membuf::c_str() const
{
	if (m_data)
	{
		// ensure end of string
		m_data[m_size + m_start] = 0;
		return (const char*)data();
	}
	return "";
}

//
// as_value
//

const string& as_value::to_string() const
// Conversion to const string&.
{
	switch (m_type)
	{
	case STRING:
		// don't need to do anything
		break;

	case NIL:
		m_string = "nil";
		break;

	case UNDEFINED:
	{
		m_string = "";
		break;
	}

	case BOOLEAN:
		m_string = m_bool ? "true" : "false";
		break;

	case NUMBER:
		// @@ Moock says if value is a NAN, then result is "NaN"
		// INF goes to "Infinity"
		// -INF goes to "-Infinity"
		char buffer[50];
		snprintf(buffer, sizeof(buffer), int(m_number) == m_number ? "%.14g" : "%.02f", m_number);
		m_string = buffer;
		break;

	default:
		assert(0);
	}
	return m_string;
}

const char* as_value::c_str() const
{
	return to_string().c_str();
}

double	as_value::to_number() const
// Conversion to double.
{
	switch (m_type)
	{
	case STRING:
	{
		double val;
		char* tail = 0;
		val = strtod(m_string.c_str(), &tail);
		return (tail == m_string.c_str() || *tail != 0) ? NAN : val; // Failed conversion to Number ?
	}
	case NUMBER:
		return m_number;
	case BOOLEAN:
		return m_bool ? 1 : 0;
	default:
		return NAN;
	}
}

bool	as_value::to_bool() const
// Conversion to boolean.
{
	switch (m_type)
	{
	case STRING:

		if (m_string == "false" || m_string == "no" || m_string == "disable")
		{
			return false;
		}
		else if (m_string == "true" || m_string == "yes" || m_string == "enable")
		{
			return true;
		}
		return to_number() != 0.0;

	case NUMBER:
		return m_number != 0;

	case BOOLEAN:
		return m_bool;

	case UNDEFINED:
	case NIL:
		return false;

	default:
		assert(0);
	}
	return false;
}

void	as_value::operator=(const as_value& v)
{
	switch (v.m_type)
	{
	case UNDEFINED:
		set_undefined();
		break;

	case NIL:
		set_nil();
		break;

	case NUMBER:
		set_double(v.m_number);
		break;

	case BOOLEAN:
		set_bool(v.m_bool);
		break;

	case STRING:
		set_string(v.m_string);
		break;

	default:
		assert(0);
	}
}

bool	as_value::operator==(const as_value& v) const
// Return true if operands are equal.
{
	// types don't match
	if (m_type != v.m_type)
	{
		return false;
	}

	switch (m_type)
	{
	case UNDEFINED:
		return v.m_type == UNDEFINED;

	case NIL:
		return v.m_type == NIL;

	case STRING:
		return m_string == v.to_string();

	case NUMBER:
		return m_number == v.to_number();

	case BOOLEAN:
		return m_bool == v.to_bool();

	default:
		assert(0);
		return false;
	}
}


bool	as_value::operator!=(const as_value& v) const
// Return true if operands are not equal.
{
	return !(*this == v);
}

//
// ctor
//

as_value::as_value(float val)
	: m_type(NUMBER)
	, m_number(val)
{
}

as_value::as_value(int val)
	: m_type(NUMBER)
	, m_number(val)
{
}

as_value::as_value(unsigned int val)
	: m_type(NUMBER)
	, m_number(val)
{
}

as_value::as_value(int64_t val)
	: m_type(NUMBER)
	, m_number((double)val)
{
}

as_value::as_value(uint64_t val)
	: m_type(NUMBER)
	, m_number((double)val)
{
}

as_value::as_value(double val)
	: m_type(NUMBER)
	, m_number(val)
{
}

as_value::as_value(bool val)
	: m_type(BOOLEAN)
	, m_bool(val)
{
}

as_value::as_value(const char* str)
	: m_type(STRING)
	, m_string(str)
{
}

as_value::as_value(const string& str)
	: m_type(STRING)
	, m_string(str)
{
}

as_value::as_value()
	: m_type(UNDEFINED)
{
}

as_value::as_value(const as_value& v)
	: m_type(UNDEFINED)
{
	*this = v;
}

as_value::~as_value()
{
	set_undefined();
}

void	as_value::set_undefined()
{
	m_type = UNDEFINED;
}

void	as_value::set_nil()
{
	set_undefined();
	m_type = NIL;
}

void	as_value::set_double(double val)
{
	set_undefined();
	m_type = NUMBER;
	m_number = val;
}

void	as_value::set_bool(bool val)
{
	set_undefined();
	m_type = BOOLEAN;
	m_bool = val;
}

void	as_value::set_string(const string& str)
{
	set_undefined();
	m_type = STRING;
	m_string = str;
}

void	as_value::set_string(const char* str)
{
	set_undefined();
	m_type = STRING;
	m_string = str;
}

void	as_value::set_int(int val)
{
	set_undefined();
	m_type = NUMBER;
	m_number = val;
}

