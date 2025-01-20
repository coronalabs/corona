//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LogEntry.h"
#include <unordered_map>
#include <vector>
#include <Windows.h>


class LogEntryCollection final
{
	private:
		LogEntryCollection(const LogEntryCollection& value);

	public:
		LogEntryCollection();
		virtual ~LogEntryCollection();

		LogEntry* Add();
		void Clear();
		bool RemoveByIndex(int index);
		bool RemoveByIndexRange(int index, int count);
		int GetCount() const;
		LogEntry* GetByIndex(int index) const;
		LogEntry* GetById(DWORD integerId) const;
		int GetIndexById(DWORD integerId) const;
		bool ContainsId(DWORD integerId) const;

	private:
		const LogEntryCollection& operator=(const LogEntryCollection& value) {}

		std::vector<LogEntry*> fEntryPointers;
		std::unordered_map<DWORD, LogEntry*> fEntryIdToPointerMap;
		DWORD fNextIntegerId;
};
