/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#ifdef _WIN32

#include "platform_windows.h"
#include "types.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  srand(Platform::GetCurrentTimeMS() + Platform::GetProcessID());
  return TRUE;
}

int Platform::GetConfigInt(const char* name)
{
  char buf[1024];
  if (GetEnvironmentVariable(name, buf, ARRAY_SIZE(buf)) == 0)
  {
    return 0;
  }
  return atoi(buf);
}

bool Platform::GetConfigBool(const char* name)
{
  char buf[1024];
  if (GetEnvironmentVariable(name, buf, ARRAY_SIZE(buf)) == 0)
  {
    return false;
  }
  return atoi(buf) != 0 || _stricmp(buf, "true") == 0;
}

#endif