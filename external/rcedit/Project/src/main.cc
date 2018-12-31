// Copyright (c) 2013 GitHub, Inc. All rights reserved.
// Use of this source code is governed by MIT license that can be found in the
// LICENSE file.

#include <string.h>
#include <stdlib.h>

#include "rescle.h"

bool print_error(const char* message) {
  fprintf(stderr, "Fatal error: %s\n", message);
  return 1;
}

bool parse_version_string(const wchar_t* str, int *v1, int *v2, int *v3, int *v4) {
  *v1 = *v2 = *v3 = *v4 = 0;
  if (swscanf_s(str, L"%d.%d.%d.%d", v1, v2, v3, v4) == 4)
    return true;
  if (swscanf_s(str, L"%d.%d.%d", v1, v2, v3) == 3)
    return true;
  if (swscanf_s(str, L"%d.%d", v1, v2) == 2)
    return true;
  if (swscanf_s(str, L"%d", v1) == 1)
    return true;

  return false;
}

int wmain(int argc, const wchar_t* argv[]) {
  bool loaded = false;
  rescle::ResourceUpdater updater;

  for (int i = 1; i < argc; ++i) {
    if (wcscmp(argv[i], L"--set-version-string") == 0 ||
        wcscmp(argv[i], L"-svs") == 0) {
      if (argc - i < 3)
        return print_error("--set-version-string requires 'Key' and 'Value'");

      const wchar_t* key = argv[++i];
      const wchar_t* value = argv[++i];
      if (!updater.SetVersionString(key, value))
        return print_error("Unable to change version string");

    } else if (wcscmp(argv[i], L"--set-file-version") == 0 ||
               wcscmp(argv[i], L"-sfv") == 0) {
      if (argc - i < 2)
        return print_error("--set-file-version requires a version string");

      int v1, v2, v3, v4;
      if (!parse_version_string(argv[++i], &v1, &v2, &v3, &v4))
        return print_error("Unable to parse version string");

      if (!updater.SetFileVersion(v1, v2, v3, v4))
        return print_error("Unable to change file version");

      if (!updater.SetVersionString(L"FileVersion", argv[i]))
        return print_error("Unable to change FileVersion string");

    } else if (wcscmp(argv[i], L"--set-product-version") == 0 ||
               wcscmp(argv[i], L"-spv") == 0) {
      if (argc - i < 2)
        return print_error("--set-product-version requires a version string");

      int v1, v2, v3, v4;
      if (!parse_version_string(argv[++i], &v1, &v2, &v3, &v4))
        return print_error("Unable to parse version string");

      if (!updater.SetProductVersion(v1, v2, v3, v4))
        return print_error("Unable to change file version");

      if (!updater.SetVersionString(L"ProductVersion", argv[i]))
        return print_error("Unable to change ProductVersion string");

    } else if (wcscmp(argv[i], L"--set-icon") == 0 ||
               wcscmp(argv[i], L"-si") == 0) {
      if (argc - i < 2)
        return print_error("--set-icon requires path to the icon");

      if (!updater.SetIcon(argv[++i]))
        return print_error("Unable to set icon");

    } else {
      if (loaded)
        return print_error("Unexpected trailing arguments");

      loaded = true;
      if (!updater.Load(argv[i]))
        return print_error("Unable to load file");

    }
  }

  if (!loaded)
    return print_error("You should specify a exe/dll file");

  if (!updater.Commit())
    return print_error("Unable to commit changes");

  return 0;
}
