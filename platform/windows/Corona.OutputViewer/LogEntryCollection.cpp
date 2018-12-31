//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LogEntryCollection.h"
#include <exception>


#pragma region Constructors/Destructors
LogEntryCollection::LogEntryCollection()
:	fNextIntegerId(1)
{
	fEntryPointers.reserve(32768);
	fEntryIdToPointerMap.reserve(32768);
}

LogEntryCollection::LogEntryCollection(const LogEntryCollection& value)
{
	// Copy constructor is not supported.
	throw std::exception();
}

LogEntryCollection::~LogEntryCollection()
{
	Clear();
}

#pragma endregion


#pragma region Public Methods
LogEntry* LogEntryCollection::Add()
{
	// Generate a unique ID for the log entry we're about to create below.
	DWORD integerId;
	for (integerId = fNextIntegerId; ContainsId(integerId); integerId++);
	fNextIntegerId = integerId + 1;

	// Create a new log entry object and add it to the collection.
	auto entryPointer = new LogEntry(integerId);
	fEntryPointers.push_back(entryPointer);
	fEntryIdToPointerMap.insert(std::pair<DWORD, LogEntry*>(entryPointer->GetIntegerId(), entryPointer));

	// Return the newly created log entry object.
	// The caller is expected to configure it from here.
	return entryPointer;
}

void LogEntryCollection::Clear()
{
	for (auto&& entryPointer : fEntryPointers)
	{
		if (entryPointer)
		{
			delete entryPointer;
		}
	}
	fEntryPointers.clear();
	fEntryIdToPointerMap.clear();
}

bool LogEntryCollection::RemoveByIndex(int index)
{
	// Validate.
	if ((index < 0) || (index >= (int)fEntryPointers.size()))
	{
		return false;
	}

	// Remove the indexed entry from the collections.
	auto vectorIterator = fEntryPointers.begin() + index;
	auto entryPointer = *vectorIterator;
	if (entryPointer)
	{
		auto mapIterator = fEntryIdToPointerMap.find(entryPointer->GetIntegerId());
		if (mapIterator != fEntryIdToPointerMap.end())
		{
			fEntryIdToPointerMap.erase(mapIterator);
		}
		delete entryPointer;
	}
	fEntryPointers.erase(vectorIterator);
	return true;
}

bool LogEntryCollection::RemoveByIndexRange(int index, int count)
{
	// Validate.
	if ((index < 0) || (index >= (int)fEntryPointers.size()) || (count < 1))
	{
		return false;
	}

	// Reduce the given remove count to the bounds of the collection in case it exceeds it.
	if ((index + count) > (int)fEntryPointers.size())
	{
		count = (int)fEntryPointers.size() - index;
	}

	// Delete all entries within the given range.
	auto vectorIterator = fEntryPointers.begin() + index;
	for (int entriesDeleted = 0; entriesDeleted < count; entriesDeleted++)
	{
		auto entryPointer = *(vectorIterator + entriesDeleted);
		auto mapIterator = fEntryIdToPointerMap.find(entryPointer->GetIntegerId());
		if (mapIterator != fEntryIdToPointerMap.end())
		{
			fEntryIdToPointerMap.erase(mapIterator);
		}
		delete entryPointer;
	}
	fEntryPointers.erase(vectorIterator, vectorIterator + count);
	return true;
}

int LogEntryCollection::GetCount() const
{
	return (int)fEntryPointers.size();
}

LogEntry* LogEntryCollection::GetByIndex(int index) const
{
	// Validate.
	if ((index < 0) || (index >= (int)fEntryPointers.size()))
	{
		return false;
	}

	// Fetch the entry by its zero based index.
	return fEntryPointers.at(index);
}

LogEntry* LogEntryCollection::GetById(DWORD integerId) const
{
	auto iterator = fEntryIdToPointerMap.find(integerId);
	if (iterator != fEntryIdToPointerMap.end())
	{
		return (*iterator).second;
	}
	return nullptr;
}

int LogEntryCollection::GetIndexById(DWORD integerId) const
{
	unsigned int count = fEntryPointers.size();
	if (count > 0)
	{
		auto nextEntryPointer = fEntryPointers.front();
		for (unsigned int index = 0; index < count; ++index, ++nextEntryPointer)
		{
			if (nextEntryPointer->GetIntegerId() == integerId)
			{
				return (int)index;
			}
		}
	}
	return -1;
}

bool LogEntryCollection::ContainsId(DWORD integerId) const
{
	return (GetById(integerId) != nullptr);
}

#pragma endregion
