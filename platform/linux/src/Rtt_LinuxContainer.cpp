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
  // ensure ZERO at end of data
  if (m_data)
    m_data[m_size + m_start] = 0;

  return (const char*)data();
}
