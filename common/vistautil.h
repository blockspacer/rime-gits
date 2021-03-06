/*
  Copyright 2014 Google Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#ifndef GOOPY_COMMON_VISTAUTIL_H__
#define GOOPY_COMMON_VISTAUTIL_H__

// These are headers you would need if not using tollbar's precompiled headers
#include <windows.h>
#include <tchar.h>
#include <accctrl.h>
#include <Aclapi.h>
#include <Sddl.h>
#include <WinNT.h>
#include <atlsecurity.h>
#include "vista_winnt.h" // additional stuff for vista

namespace vista_util {

// This is fast, since it caches the answer after first run
bool IsVistaOrLater();

// Does the user have the reg key for disabling UAC policy at startup set?
// (Might be better to detect if UAC is disabled now, as opposed to inferring
//  this from the startup flag)
bool IsUACDisabled();

// Determine the mandatory level of a process
//   processID, the process to query, or (0) to use the current process
//   On Vista, level should alwys be filled in with either
//     MandatoryLevelLow (IE)
//     MandatoryLevelMedium(user), or 
//     MandatoryLevelHigh( Elevated Admin)
//   On error, level remains unchanged
HRESULT GetProcessIntegrityLevel(DWORD processID, MANDATORY_LEVEL* level);

// Elevated processes need to be careful how they launch child processes
// to avoid having them inherit too many credentials or not being able to
// elevate their own IE processes normally.  Microsoft's advice from 
// http://msdn.microsoft.com/library/en-us/ietechcol/dnwebgen/protectedmode.asp
// will launch a low integrity IE, but that IE cannot elevate properly since
// it was running from the wrong token. The best method I can gather is to find
// an existing process on the machine running at normal user rights, and launch
// this process impersonating that token rather than trying to adjust token
// privileges of the elevated token.  TODO(tw): Implemmt and test this
HRESULT CreateProcessAsNormalUserFromElevatedAdmin(const TCHAR *commandline,
    STARTUPINFO* startup_info, PROCESS_INFORMATION* process_info);

// Starts a new elevated process. file_path specifies the program to be run.
// If exit_code is not null, the function waits until the spawned process has 
// completed. The exit code of the process is returned therein.
// If exit_code is null, the function will return after spawning the program
// and will not wait for completion.
// show_window is one of the SW_* constants to specify howw the windows is 
// opened.
HRESULT RunElevated(const TCHAR* file_path, const TCHAR* parameters,
    HWND window, int show_window, DWORD* exit_code);

// If there is no specific integrity level defined, return S_FALSE (1) and set
// level to MandatoryLevelMedium (the vista default)
HRESULT GetFileOrFolderIntegrityLevel(const TCHAR* file,
    MANDATORY_LEVEL* level, bool* and_children);

// A level of MandatoryLevelUntrusted (0) will remove the integrity level for
// this file and all children
HRESULT SetFileOrFolderIntegrityLevel(const TCHAR* file,
    MANDATORY_LEVEL level, bool and_children);

// If there is no specific integrity level defined, return S_FALSE (1) and set
// level to MandatoryLevelMedium (the vista default)
// root must be one of the 4 pre-defined roots: HKLM, HKCU, HKCR, HCU
HRESULT GetRegKeyIntegrityLevel(HKEY root, const TCHAR* subkey,
    MANDATORY_LEVEL* level, bool* and_children);

// A level of MandatoryLevelUntrusted (0) will remove the integrity label
// root must be one of the 4 pre-defined roots: HKLM, HKCU, HKCR, HCU
HRESULT SetRegKeyIntegrityLevel(HKEY root, const TCHAR* subkey,
    MANDATORY_LEVEL level, bool and_children);

// Gets a SECURITY_ATTRIBUTES object with a NULL acl.
// Used to lower security limits.
void GetSecurityAttributesWithEmptyAcl(SECURITY_ATTRIBUTES* attributes,
                                       SECURITY_DESCRIPTOR* descriptor,
                                       bool inherit_handle);

// Creates a security descriptor that can be used to make an object accessible
// from all integrity levels. In the case of non-vista, CSecurityDesc will
// be left alone, which results in the default security descriptor.
class LowIntegritySecurityDesc : public CSecurityDesc {
 public:
  // Mask will be added as an allowed ACE of the DACL.
  // For example, use MUTEX_ALL_ACCESS for shared mutexes.
  explicit LowIntegritySecurityDesc(ACCESS_MASK mask);
  bool is_valid() { return is_valid_; }

 private:
  bool is_valid_;
};


}; // namespace vista_util


#endif // #ifndef GOOPY_COMMON_VISTAUTIL_H__
