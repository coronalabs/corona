// Copyright (c) 2013 GitHub, Inc. All rights reserved.
// Use of this source code is governed by MIT license that can be found in the
// LICENSE file.
//
// This file is modified from Rescle written by yoshio.okumura@gmail.com:
// http://code.google.com/p/rescle/

#ifndef VERSION_INFO_UPDATER
#define VERSION_INFO_UPDATER

#include <string>
#include <vector>
#include <map>

#include <windows.h>

#define RU_VS_COMMENTS          L"Comments"
#define RU_VS_COMPANY_NAME      L"CompanyName"
#define RU_VS_FILE_DESCRIPTION  L"FileDescription"
#define RU_VS_FILE_VERSION      L"FileVersion"
#define RU_VS_INTERNAL_NAME     L"InternalName"
#define RU_VS_LEGAL_COPYRIGHT   L"LegalCopyright"
#define RU_VS_LEGALLRADEMARKS   L"LegalTrademarks"
#define RU_VS_ORIGINAL_FILENAME L"OriginalFilename"
#define RU_VS_PRIVATE_BUILD     L"PrivateBuild"
#define RU_VS_PRODUCT_NAME      L"ProductName"
#define RU_VS_PRODUCT_VERSION   L"ProductVersion"
#define RU_VS_SPECIAL_BUILD     L"SpecialBuild"

namespace rescle {

class VersionStampValue {
 public:
  unsigned short wOffset;
  unsigned short wLength;
  unsigned short wKeyLength;
  unsigned short wType;
  std::wstring szKey;
  std::vector<char> Data;
  unsigned short GetLength(const bool& rounding = true) const;
};

class IconsValue {
 public:
  typedef struct _ICONENTRY {
    BYTE width;
    BYTE height;
    BYTE colorCount;
    BYTE reserved;
    WORD planes;
    WORD bitCount;
    DWORD bytesInRes;
    DWORD imageOffset;
  } ICONENTRY;

  typedef struct _ICONHEADER {
    WORD reserved;
    WORD type;
    WORD count;
    std::vector<ICONENTRY> entries;
  } ICONHEADER;

  ICONHEADER header;
  std::vector<std::vector<BYTE>> images;
  std::vector<BYTE> grpHeader;
};

class ResourceUpdater {
 public:
  typedef std::vector<std::wstring> StringValues;
  typedef std::map<UINT,StringValues> StringTable;
  typedef std::map<WORD,StringTable> StringTableMap;

  typedef std::vector<VersionStampValue> VersionStampValues;
  typedef std::map<UINT,VersionStampValues> VersionStampTable;
  typedef std::map<WORD,VersionStampTable> VersionStampMap;

  ResourceUpdater();
  ~ResourceUpdater();

  bool Load(const WCHAR* filename);
  bool Load(const char* filename);
  bool SetVersionString(const WORD& languageId, const WCHAR* name, const WCHAR* value);
  bool SetVersionString(const WCHAR* name, const WCHAR* value);
  bool SetProductVersion(const WORD& languageId, const UINT& id, const WORD v1, const WORD v2, const WORD v3, const WORD v4);
  bool SetProductVersion(const WORD v1, const WORD v2, const WORD v3, const WORD v4);
  bool SetFileVersion(const WORD& languageId, const UINT& id, const WORD v1, const WORD v2, const WORD v3, const WORD v4);
  bool SetFileVersion(const WORD v1, const WORD v2, const WORD v3, const WORD v4);
  bool ChangeString(const WORD& languageId, const UINT& id, const WCHAR* value);
  bool ChangeString(const UINT& id, const WCHAR* value);
  bool SetIcon(const WCHAR* path);
  bool Commit();

  static bool UpdateRaw(const WCHAR* filename, const WORD& languageId, const WCHAR* type, const UINT& id, const void* data, const size_t& dataSize, const bool& deleteOld);
  static bool GetResourcePointer(const HMODULE& hModule, const WORD& languageId, const int& id, const WCHAR* type, void*& data, size_t& dataSize);

private:
  bool Deserialize(const void* data, const size_t& dataSize, VersionStampValues& values);
  bool SerializeVersionInfo(VersionStampValues& values, std::vector<char>& out);
  bool SerializeStringTable(const StringValues& values, const UINT& blockId, std::vector<char>& out);

  static BOOL CALLBACK OnEnumIconNames(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam);

  // not thread-safe
  static BOOL CALLBACK OnEnumResourceName(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam);

  // not thread-safe
  static BOOL CALLBACK OnEnumResourceLanguage(HANDLE hModule, LPCTSTR lpszType, LPCTSTR lpszName, WORD wIDLanguage, LONG_PTR lParam);

  HMODULE hModule;
  std::wstring filename;
  VersionStampMap versionStampMap;
  StringTableMap stringTableMap;
  IconsValue icon;
  bool wasIconIdFound;
  UINT firstIconId;
};

class ScopedResourceUpdater {
 public:
  ScopedResourceUpdater(const WCHAR* filename, const bool& deleteOld);
  ~ScopedResourceUpdater();

  HANDLE Get() const;
  bool Commit();

 private:
  bool EndUpdate(const bool& doesCommit);

  HANDLE handle;
  bool commited;
};

}  // namespace rescle

#endif // VERSION_INFO_UPDATER
