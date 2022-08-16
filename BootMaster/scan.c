/*
 * BootMaster/scan.c
 * Code related to scanning for boot loaders
 *
 * Copyright (c) 2006-2010 Christoph Pfisterer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Christoph Pfisterer nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Modifications copyright (c) 2012-2021 Roderick W. Smith
 *
 * Modifications distributed under the terms of the GNU General Public
 * License (GPL) version 3 (GPLv3), or (at your option) any later version.
 */
/*
 * Modified for RefindPlus
 * Copyright (c) 2020-2022 Dayo Akanji (sf.net/u/dakanji/profile)
 *
 * Modifications distributed under the preceding terms.
 */

#include "global.h"
#include "config.h"
#include "screenmgt.h"
#include "lib.h"
#include "icns.h"
#include "menu.h"
#include "mok.h"
#include "apple.h"
#include "mystrings.h"
#include "security_policy.h"
#include "driver_support.h"
#include "launch_efi.h"
#include "launch_legacy.h"
#include "linux.h"
#include "scan.h"
#include "install.h"
#include "../include/refit_call_wrapper.h"


//
// constants

#if defined (EFIX64)
    #define SHELL_NAMES L"\\EFI\\BOOT\\tools\\x64_Shell.efi,\\EFI\\BOOT\\tools\\Shell_x64.efi,\\EFI\\BOOT\\tools\\Shell.efi,\\EFI\\BOOT\\tools_x64\\x64_Shell.efi,\\EFI\\BOOT\\tools_x64\\Shell_x64.efi,\\EFI\\BOOT\\tools_x64\\Shell.efi,\\EFI\\tools_x64\\x64_Shell.efi,\\EFI\\tools_x64\\shell_x64.efi,\\EFI\\tools_x64\\shell.efi,\\EFI\\tools\\x64_Shell.efi,\\EFI\\tools\\shell_x64.efi,\\EFI\\tools\\shell.efi,\\EFI\\x64_Shell.efi,\\EFI\\shell_x64.efi,\\EFI\\shell.efi,\\x64_Shell.efi,\\shell_x64.efi,\\shell.efi"
    #define GPTSYNC_NAMES L"\\EFI\\BOOT\\tools\\x64_gptsync.efi,\\EFI\\BOOT\\tools\\gptsync_x64.efi,\\EFI\\BOOT\\tools\\gptsync.efi,\\EFI\\BOOT\\tools_x64\\x64_gptsync.efi,\\EFI\\BOOT\\tools_x64\\gptsync_x64.efi,\\EFI\\BOOT\\tools_x64\\gptsync.efi,\\EFI\\tools_x64\\gptsync.efi,\\EFI\\tools_x64\\gptsync_x64.efi,\\EFI\\tools\\gptsync.efi,\\EFI\\tools\\gptsync_x64.efi,\\EFI\\gptsync.efi,\\EFI\\gptsync_x64.efi,\\gptsync.efi,\\gptsync_x64.efi"
    #define GDISK_NAMES L"\\EFI\\BOOT\\tools\\x64_gdisk.efi,\\EFI\\BOOT\\tools\\gdisk_x64.efi,\\EFI\\BOOT\\tools\\gdisk.efi,\\EFI\\BOOT\\tools_x64\\x64_gdisk.efi,\\EFI\\BOOT\\tools_x64\\gdisk_x64.efi,\\EFI\\BOOT\\tools_x64\\gdisk.efi,\\EFI\\tools_x64\\gdisk.efi,\\EFI\\tools_x64\\gdisk_x64.efi,\\EFI\\tools\\gdisk.efi,\\EFI\\tools\\gdisk_x64.efi,\\EFI\\gdisk.efi,\\EFI\\gdisk_x64.efi,\\gdisk.efi,\\gdisk_x64.efi"
    #define NETBOOT_NAMES L"\\EFI\\BOOT\\tools\\x64_ipxe.efi,\\EFI\\BOOT\\tools\\ipxe_x64.efi,\\EFI\\BOOT\\tools\\ipxe.efi,\\EFI\\BOOT\\tools_x64\\x64_ipxe.efi,\\EFI\\BOOT\\tools_x64\\ipxe_x64.efi,\\EFI\\BOOT\\tools_x64\\ipxe.efi,\\EFI\\tools_x64\\ipxe.efi,\\EFI\\tools_x64\\ipxe_x64.efi,\\EFI\\tools\\ipxe.efi,\\EFI\\tools\\ipxe_x64.efi,\\EFI\\ipxe.efi,\\EFI\\ipxe_x64.efi,\\ipxe.efi,\\ipxe_x64.efi"
    #define MEMTEST_NAMES L"x64_memtest.efi,memtest.efi,memtest_x64.efi,x64_memtest86.efi,memtest86.efi,memtest86_x64.efi"
    #define FALLBACK_FULLNAME       L"EFI\\BOOT\\bootx64.efi"
    #define FALLBACK_BASENAME       L"bootx64.efi"
#elif defined (EFI32)
    #define SHELL_NAMES L"\\EFI\\BOOT\\tools\\ia32_Shell.efi,\\EFI\\BOOT\\tools\\Shell_ia32.efi,\\EFI\\BOOT\\tools\\Shell.efi,\\EFI\\BOOT\\tools_ia32\\ia32_Shell.efi,\\EFI\\BOOT\\tools_ia32\\Shell_ia32.efi,\\EFI\\BOOT\\tools_ia32\\Shell.efi,\\EFI\\tools_ia32\\ia32_Shell.efi,\\EFI\\tools_ia32\\shell_ia32.efi,\\EFI\\tools_ia32\\shell.efi,\\EFI\\tools\\ia32_Shell.efi,\\EFI\\tools\\shell_ia32.efi,\\EFI\\tools\\shell.efi,\\EFI\\ia32_Shell.efi,\\EFI\\shell_ia32.efi,\\EFI\\shell.efi,\\ia32_Shell.efi,\\shell_ia32.efi,\\shell.efi"
    #define GPTSYNC_NAMES L"\\EFI\\BOOT\\tools\\ia32_gptsync.efi,\\EFI\\BOOT\\tools\\gptsync_ia32.efi,\\EFI\\BOOT\\tools\\gptsync.efi,\\EFI\\BOOT\\tools_ia32\\ia32_gptsync.efi,\\EFI\\BOOT\\tools_ia32\\gptsync_ia32.efi,\\EFI\\BOOT\\tools_ia32\\gptsync.efi,\\EFI\\tools_ia32\\gptsync.efi,\\EFI\\tools_ia32\\gptsync_ia32.efi,\\EFI\\tools\\gptsync.efi,\\EFI\\tools\\gptsync_ia32.efi,\\EFI\\gptsync.efi,\\EFI\\gptsync_ia32.efi,\\gptsync.efi,\\gptsync_ia32.efi"
    #define GDISK_NAMES L"\\EFI\\BOOT\\tools\\ia32_gdisk.efi,\\EFI\\BOOT\\tools\\gdisk_ia32.efi,\\EFI\\BOOT\\tools\\gdisk.efi,\\EFI\\BOOT\\tools_ia32\\ia32_gdisk.efi,\\EFI\\BOOT\\tools_ia32\\gdisk_ia32.efi,\\EFI\\BOOT\\tools_ia32\\gdisk.efi,\\EFI\\tools_ia32\\gdisk.efi,\\EFI\\tools_ia32\\gdisk_ia32.efi,\\EFI\\tools\\gdisk.efi,\\EFI\\tools\\gdisk_ia32.efi,\\EFI\\gdisk.efi,\\EFI\\gdisk_ia32.efi,\\gdisk.efi,\\gdisk_ia32.efi"
    #define NETBOOT_NAMES L"\\EFI\\BOOT\\tools\\ia32_ipxe.efi,\\EFI\\BOOT\\tools\\ipxe_ia32.efi,\\EFI\\BOOT\\tools\\ipxe.efi,\\EFI\\BOOT\\tools_ia32\\ia32_ipxe.efi,\\EFI\\BOOT\\tools_ia32\\ipxe_ia32.efi,\\EFI\\BOOT\\tools_ia32\\ipxe.efi,\\EFI\\tools_ia32\\ipxe.efi,\\EFI\\tools_ia32\\ipxe_ia32.efi,\\EFI\\tools\\ipxe.efi,\\EFI\\tools\\ipxe_ia32.efi,\\EFI\\ipxe.efi,\\EFI\\ipxe_ia32.efi,\\ipxe.efi,\\ipxe_ia32.efi"
    #define MEMTEST_NAMES L"ia32_memtest.efi,memtest.efi,memtest_ia32.efi,ia32_memtest86.efi,memtest86.efi,memtest86_ia32.efi"
    #define FALLBACK_FULLNAME       L"EFI\\BOOT\\bootia32.efi"
    #define FALLBACK_BASENAME       L"bootia32.efi"
#elif defined (EFIAARCH64)
    #define SHELL_NAMES L"\\EFI\\BOOT\\tools\\aa64_Shell.efi,\\EFI\\BOOT\\tools\\Shell_aa64.efi,\\EFI\\BOOT\\tools\\Shell.efi,\\EFI\\BOOT\\tools_aa64\\aa64_Shell.efi,\\EFI\\BOOT\\tools_aa64\\Shell_aa64.efi,\\EFI\\BOOT\\tools_aa64\\Shell.efi,\\EFI\\tools_aa64\\aa64_Shell.efi,\\EFI\\tools_aa64\\shell_aa64.efi,\\EFI\\tools_aa64\\shell.efi,\\EFI\\tools\\aa64_Shell.efi,\\EFI\\tools\\shell_aa64.efi,\\EFI\\tools\\shell.efi,\\EFI\\aa64_Shell.efi,\\EFI\\shell_aa64.efi,\\EFI\\shell.efi,\\aa64_Shell.efi,\\shell_aa64.efi,\\shell.efi"
    #define GPTSYNC_NAMES L"\\EFI\\BOOT\\tools\\aa64_gptsync.efi,\\EFI\\BOOT\\tools\\gptsync_aa64.efi,\\EFI\\BOOT\\tools\\gptsync.efi,\\EFI\\BOOT\\tools_aa64\\aa64_gptsync.efi,\\EFI\\BOOT\\tools_aa64\\gptsync_aa64.efi,\\EFI\\BOOT\\tools_aa64\\gptsync.efi,\\EFI\\tools_aa64\\gptsync.efi,\\EFI\\tools_aa64\\gptsync_aa64.efi,\\EFI\\tools\\gptsync.efi,\\EFI\\tools\\gptsync_aa64.efi,\\EFI\\gptsync.efi,\\EFI\\gptsync_aa64.efi,\\gptsync.efi,\\gptsync_aa64.efi"
    #define GDISK_NAMES L"\\EFI\\BOOT\\tools\\aa64_gdisk.efi,\\EFI\\BOOT\\tools\\gdisk_aa64.efi,\\EFI\\BOOT\\tools\\gdisk.efi,\\EFI\\BOOT\\tools_aa64\\aa64_gdisk.efi,\\EFI\\BOOT\\tools_aa64\\gdisk_aa64.efi,\\EFI\\BOOT\\tools_aa64\\gdisk.efi,\\EFI\\tools_aa64\\gdisk.efi,\\EFI\\tools_aa64\\gdisk_aa64.efi,\\EFI\\tools\\gdisk.efi,\\EFI\\tools\\gdisk_aa64.efi,\\EFI\\gdisk.efi,\\EFI\\gdisk_aa64.efi,\\gdisk.efi,\\gdisk_aa64.efi"
    #define NETBOOT_NAMES L"\\EFI\\BOOT\\tools\\aa64_ipxe.efi,\\EFI\\BOOT\\tools\\ipxe_aa64.efi,\\EFI\\BOOT\\tools\\ipxe.efi,\\EFI\\BOOT\\tools_aa64\\aa64_ipxe.efi,\\EFI\\BOOT\\tools_aa64\\ipxe_aa64.efi,\\EFI\\BOOT\\tools_aa64\\ipxe.efi,\\EFI\\tools_aa64\\ipxe.efi,\\EFI\\tools_aa64\\ipxe_aa64.efi,\\EFI\\tools\\ipxe.efi,\\EFI\\tools\\ipxe_aa64.efi,\\EFI\\ipxe.efi,\\EFI\\ipxe_aa64.efi,\\ipxe.efi,\\ipxe_aa64.efi"
    #define MEMTEST_NAMES L"aa64_memtest.efi,memtest.efi,memtest_aa64.efi,aa64_memtest86.efi,memtest86.efi,memtest86_aa64.efi"
    #define FALLBACK_FULLNAME       L"EFI\\BOOT\\bootaa64.efi"
    #define FALLBACK_BASENAME       L"bootaa64.efi"
#else
    #define SHELL_NAMES             L"\\EFI\\tools\\shell.efi,\\shell.efi"
    #define GPTSYNC_NAMES           L"\\EFI\\tools\\gptsync.efi"
    #define GDISK_NAMES             L"\\EFI\\tools\\gdisk.efi"
    #define NETBOOT_NAMES           L"\\EFI\\tools\\ipxe.efi"
    #define MEMTEST_NAMES           L"\\memtest.efi,\\memtest86.efi"
    #define DRIVER_DIRS             L"drivers"
    #define FALLBACK_FULLNAME       L"EFI\\BOOT\\boot.efi" // Not really correct
    #define FALLBACK_BASENAME       L"boot.efi"            // Not really correct
#endif

#define IPXE_DISCOVER_NAME      L"\\efi\\tools\\ipxe_discover.efi"
#define IPXE_NAME               L"\\efi\\tools\\ipxe.efi"

// Patterns that identify Linux kernels. Added to the loader match pattern when the
// scan_all_linux_kernels option is set in the configuration file. Causes kernels WITHOUT
// a ".efi" extension to be found when scanning for boot loaders.
#define LINUX_MATCH_PATTERNS    L"vmlinuz*,bzImage*,kernel*"

EFI_GUID GlobalGuid = EFI_GLOBAL_VARIABLE;

extern EFI_GUID GuidAPFS;

#if REFIT_DEBUG > 0
static CHAR16  *Spacer   = L"                ";

BOOLEAN  LogNewLine      = FALSE;
#endif

BOOLEAN  ScanningLoaders = FALSE;
BOOLEAN  FirstLoaderScan = FALSE;

// Structure used to hold boot loader filenames and time stamps in
// a linked list; used to sort entries within a directory.
struct
LOADER_LIST {
    CHAR16              *FileName;
    EFI_TIME             TimeStamp;
    struct LOADER_LIST  *NextEntry;
};

//
// misc functions
//

// Creates a copy of a menu screen.
// Returns a pointer to the copy of the menu screen.
REFIT_MENU_SCREEN * CopyMenuScreen (
    REFIT_MENU_SCREEN *Entry
) {
    UINTN              i;
    REFIT_MENU_SCREEN *NewEntry;

    NewEntry = AllocateZeroPool (sizeof (REFIT_MENU_SCREEN));

    if ((Entry != NULL) && (NewEntry != NULL)) {
        NewEntry->Title = (Entry->Title) ? StrDuplicate (Entry->Title) : NULL;

        if (Entry->TitleImage != NULL) {
            NewEntry->TitleImage = egCopyImage (Entry->TitleImage);
        }

        NewEntry->InfoLineCount = Entry->InfoLineCount;
        NewEntry->InfoLines = (CHAR16 **) AllocateZeroPool (
            Entry->InfoLineCount * (sizeof (CHAR16 *))
        );
        for (i = 0; i < Entry->InfoLineCount && NewEntry->InfoLines; i++) {
            NewEntry->InfoLines[i] = (Entry->InfoLines[i])
                ? StrDuplicate (Entry->InfoLines[i]) : NULL;
        } // for

        NewEntry->EntryCount = Entry->EntryCount;
        NewEntry->Entries = (REFIT_MENU_ENTRY **) AllocateZeroPool (
            Entry->EntryCount * (sizeof (REFIT_MENU_ENTRY *))
        );
        for (i = 0; i < Entry->EntryCount && NewEntry->Entries; i++) {
            AddMenuEntryCopy (NewEntry, Entry->Entries[i]);
        } // for

        NewEntry->TimeoutSeconds =  Entry->TimeoutSeconds;
        NewEntry->TimeoutText    = (Entry->TimeoutText)
            ? StrDuplicate (Entry->TimeoutText) : NULL;

        NewEntry->Hint1 = (Entry->Hint1) ? StrDuplicate (Entry->Hint1) : NULL;
        NewEntry->Hint2 = (Entry->Hint2) ? StrDuplicate (Entry->Hint2) : NULL;
    }

    return (NewEntry);
} // REFIT_MENU_SCREEN * CopyMenuScreen()

// Creates a copy of a menu entry. Intended to enable moving a stack-based
// menu entry (such as the ones for the "reboot" and "exit" functions) to
// to the heap. This enables easier deletion of the whole set of menu
// entries when re-scanning.
// Returns a pointer to the copy of the menu entry.
REFIT_MENU_ENTRY * CopyMenuEntry (
    REFIT_MENU_ENTRY *Entry
) {
    REFIT_MENU_ENTRY *NewEntry = NULL;

    NewEntry = AllocateZeroPool (sizeof (REFIT_MENU_ENTRY));
    if ((Entry != NULL) && (NewEntry != NULL)) {
        NewEntry->Tag            =  Entry->Tag;
        NewEntry->Row            =  Entry->Row;
        NewEntry->ShortcutDigit  =  Entry->ShortcutDigit;
        NewEntry->ShortcutLetter =  Entry->ShortcutLetter;
        NewEntry->Title          = (Entry->Title      != NULL) ? StrDuplicate (Entry->Title)       : NULL;
        NewEntry->Image          = (Entry->Image      != NULL) ? egCopyImage (Entry->Image)        : NULL;
        NewEntry->BadgeImage     = (Entry->BadgeImage != NULL) ? egCopyImage (Entry->BadgeImage)   : NULL;
        NewEntry->SubScreen      = (Entry->SubScreen  != NULL) ? CopyMenuScreen (Entry->SubScreen) : NULL;
    }
    return (NewEntry);
} // REFIT_MENU_ENTRY * CopyMenuEntry()

// Creates a new LOADER_ENTRY data structure and populates it with
// default values from the specified Entry, or NULL values if Entry
// is unspecified (NULL).
// Returns a pointer to the new data structure, or NULL if it
// couldn't be allocated
LOADER_ENTRY * InitializeLoaderEntry (
    IN LOADER_ENTRY *Entry
) {
    LOADER_ENTRY *NewEntry = NULL;

    NewEntry = AllocateZeroPool (sizeof (LOADER_ENTRY));
    if (NewEntry != NULL) {
        NewEntry->Enabled         = TRUE;
        NewEntry->UseGraphicsMode = FALSE;
        NewEntry->EfiLoaderPath   = NULL;
        NewEntry->me.Title        = NULL;
        NewEntry->me.Tag          = TAG_LOADER;
        NewEntry->OSType          = 0;
        NewEntry->EfiBootNum      = 0;
        if (Entry != NULL) {
            NewEntry->EfiBootNum      =  Entry->EfiBootNum;
            NewEntry->UseGraphicsMode =  Entry->UseGraphicsMode;
            NewEntry->Volume          = (Entry->Volume)        ? CopyVolume (Entry->Volume)                 : NULL;
            NewEntry->LoaderPath      = (Entry->LoaderPath)    ? StrDuplicate (Entry->LoaderPath)           : NULL;
            NewEntry->LoadOptions     = (Entry->LoadOptions)   ? StrDuplicate (Entry->LoadOptions)          : NULL;
            NewEntry->InitrdPath      = (Entry->InitrdPath)    ? StrDuplicate (Entry->InitrdPath)           : NULL;
            NewEntry->EfiLoaderPath   = (Entry->EfiLoaderPath) ? DuplicateDevicePath (Entry->EfiLoaderPath) : NULL;
        }
    }

    return (NewEntry);
} // LOADER_ENTRY *InitializeLoaderEntry()

// Prepare a REFIT_MENU_SCREEN data structure for a subscreen entry. This sets up
// the default entry that launches the boot loader using the same options as the
// main Entry does. Subsequent options can be added by the calling function.
// If a subscreen already exists in the Entry that is passed to this function,
// it is left unchanged and a pointer to it is returned.
// Returns a pointer to the new subscreen data structure, or NULL if there
// were problems allocating memory.
REFIT_MENU_SCREEN * InitializeSubScreen (
    IN LOADER_ENTRY *Entry
) {
    CHAR16              *FileName;
    CHAR16              *MainOptions = NULL;
    CHAR16              *DisplayName = NULL;
    REFIT_MENU_SCREEN   *SubScreen   = NULL;
    LOADER_ENTRY        *SubEntry;

    FileName = Basename (Entry->LoaderPath);
    if (Entry->me.SubScreen) {
        // Existing subscreen ... Early Return ... Add new entry later.
        return CopyMenuScreen (Entry->me.SubScreen);
    }

    // No subscreen yet ... Initialize default entry
    SubScreen = AllocateZeroPool (sizeof (REFIT_MENU_SCREEN));
    if (SubScreen == NULL) {
        // Early Return
        return NULL;
    }

    if (GlobalConfig.SyncAPFS) {
        EFI_STATUS                 Status;
        APPLE_APFS_VOLUME_ROLE VolumeRole = 0;

        // DA-TAG: Limit to TianoCore
        #ifndef __MAKEWITH_TIANO
        Status = EFI_NOT_STARTED;
        #else
        Status = RP_GetApfsVolumeInfo (
            Entry->Volume->DeviceHandle,
            NULL, NULL,
            &VolumeRole
        );
        #endif

        if (!EFI_ERROR(Status)) {
            if (VolumeRole == APPLE_APFS_VOLUME_ROLE_PREBOOT) {
                DisplayName = GetVolumeGroupName (Entry->LoaderPath, Entry->Volume);
            }
        }
    }

    SubScreen->Title = PoolPrint (
        L"Boot Options for %s on %s",
        (Entry->Title != NULL) ? Entry->Title  : FileName,
        (DisplayName) ? DisplayName : Entry->Volume->VolName
    );

    #if REFIT_DEBUG > 0
    ALT_LOG(1, LOG_THREE_STAR_MID, L"Build Subscreen:- '%s'", SubScreen->Title);
    #endif

    SubScreen->TitleImage = egCopyImage (Entry->me.Image);

    // Default entry
    SubEntry = InitializeLoaderEntry (Entry);
    if (SubEntry != NULL) {
        #if REFIT_DEBUG > 0
        ALT_LOG(1, LOG_LINE_NORMAL, L"Setting Entries for '%s'", SubScreen->Title);
        #endif

        CHAR16 *NameOS = L"";
        // DA-TAG: Maintain space after NameOS
        if (Entry->OSType == 'M') {
            NameOS = L"MacOS ";
        }
        else if (Entry->OSType == 'L') {
            NameOS = L"Linux ";
        }
        else if (Entry->OSType == 'W') {
            if (FoundSubStr (SubScreen->Title, L"UEFI")) {
                NameOS = L"Windows (UEFI) ";
            }
            else if (FoundSubStr (SubScreen->Title, L"Legacy")) {
                NameOS = L"Windows (Legacy) ";
            }
            else {
                NameOS = L"Windows ";
            }
        }
        else if (Entry->OSType == 'R') {
            NameOS = L"rEFit Variant ";
        }
        else if (Entry->OSType == 'G') {
            NameOS = L"Grub ";
        }
        else if (Entry->OSType == 'X') {
            NameOS = L"XoM ";
        }
        else if (Entry->OSType == 'E') {
            NameOS = L"Elilo ";
        }
        else if (FoundSubStr (SubScreen->Title, L"OpenCore")) {
            NameOS = L"OpenCore ";
        }
        else if (FoundSubStr (SubScreen->Title, L"Clover")) {
            NameOS = L"Clover ";
        }
        SubEntry->me.Title    = PoolPrint (L"Boot %swith Default Options", NameOS);
        MainOptions           = StrDuplicate (SubEntry->LoadOptions);
        SubEntry->LoadOptions = AddInitrdToOptions (MainOptions, SubEntry->InitrdPath);
        MY_FREE_POOL(MainOptions);
        AddMenuEntry (SubScreen, (REFIT_MENU_ENTRY *) SubEntry);
    }

    SubScreen->Hint1 = StrDuplicate (SUBSCREEN_HINT1);
    SubScreen->Hint2 = (GlobalConfig.HideUIFlags & HIDEUI_FLAG_EDITOR)
        ? StrDuplicate (SUBSCREEN_HINT2_NO_EDITOR)
        : StrDuplicate (SUBSCREEN_HINT2);

    MY_FREE_POOL(FileName);
    MY_FREE_POOL(DisplayName);

    return SubScreen;
} // REFIT_MENU_SCREEN *InitializeSubScreen()

VOID GenerateSubScreen (
    IN OUT LOADER_ENTRY *Entry,
    IN     REFIT_VOLUME *Volume,
    IN     BOOLEAN       GenerateReturn
) {
    EFI_STATUS          Status;
    REFIT_MENU_SCREEN  *SubScreen;
    REFIT_VOLUME       *DiagnosticsVolume;
    LOADER_ENTRY       *SubEntry;
    REFIT_FILE         *File;
    BOOLEAN             UseSystemVolume;
    CHAR16             *InitrdName;
    CHAR16             *KernelVersion = NULL;
    CHAR16            **TokenList;
    UINTN               TokenCount;
    UINTN               i;

    #if REFIT_DEBUG > 1
    CHAR16 *FuncTag = L"GenerateSubScreen";
    #endif

    BREAD_CRUMB(L"In %s ... A - START MAIN", FuncTag);

    // Create the submenu
    if (StrLen (Entry->Title) == 0) {
        MY_FREE_POOL(Entry->Title);
    }

    SubScreen = InitializeSubScreen (Entry);

    // InitializeSubScreen cannot return NULL but guard against this regardless
    if (SubScreen != NULL) {
        // Loader specific submenu entries
        if (Entry->OSType == 'M') {          // Entries for MacOS
            LOG_SEP(L"X");
            BREAD_CRUMB(L"In %s ... 1 - START for OSType M", FuncTag);
#if defined (EFIX64)
            SubEntry = InitializeLoaderEntry (Entry);
            if (SubEntry != NULL) {
                SubEntry->me.Title        = StrDuplicate (L"Boot MacOS with a 64-bit Kernel");
                SubEntry->LoadOptions     = StrDuplicate (L"arch=x86_64");
                SubEntry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_OSX;
                AddMenuEntry (SubScreen, (REFIT_MENU_ENTRY *) SubEntry);
            }

            SubEntry = InitializeLoaderEntry (Entry);
            if (SubEntry != NULL) {
                SubEntry->me.Title        = StrDuplicate (L"Boot MacOS with a 32-bit Kernel");
                SubEntry->LoadOptions     = StrDuplicate (L"arch=i386");
                SubEntry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_OSX;
                AddMenuEntry (SubScreen, (REFIT_MENU_ENTRY *) SubEntry);
            }
#endif

            SubEntry = InitializeLoaderEntry (Entry);
            if (SubEntry != NULL) {
                SubEntry->me.Title        = StrDuplicate (L"Boot MacOS in Verbose Mode");
                SubEntry->LoadOptions     = StrDuplicate (L"-v");
                SubEntry->UseGraphicsMode = FALSE;
                AddMenuEntry (SubScreen, (REFIT_MENU_ENTRY *) SubEntry);
            }

#if defined (EFIX64)
            SubEntry = InitializeLoaderEntry (Entry);
            if (SubEntry != NULL) {
                SubEntry->me.Title        = StrDuplicate (L"Boot MacOS in Verbose Mode (64-bit)");
                SubEntry->LoadOptions     = StrDuplicate (L"-v arch=x86_64");
                SubEntry->UseGraphicsMode = FALSE;
                AddMenuEntry (SubScreen, (REFIT_MENU_ENTRY *) SubEntry);
            }
            SubEntry = InitializeLoaderEntry (Entry);
            if (SubEntry != NULL) {
                SubEntry->me.Title        = StrDuplicate (L"Boot MacOS in Verbose Mode (32-bit)");
                SubEntry->LoadOptions     = StrDuplicate (L"-v arch=i386");
                SubEntry->UseGraphicsMode = FALSE;
                AddMenuEntry (SubScreen, (REFIT_MENU_ENTRY *) SubEntry);
            }
#endif

            if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_SAFEMODE)) {
                SubEntry = InitializeLoaderEntry (Entry);
                if (SubEntry != NULL) {
                    SubEntry->me.Title        = StrDuplicate (L"Boot MacOS in Safe Mode (Quiet)");
                    SubEntry->LoadOptions     = StrDuplicate (L"-x");
                    SubEntry->UseGraphicsMode = FALSE;
                    AddMenuEntry (SubScreen, (REFIT_MENU_ENTRY *) SubEntry);
                }
                SubEntry = InitializeLoaderEntry (Entry);
                if (SubEntry != NULL) {
                    SubEntry->me.Title        = StrDuplicate (L"Boot MacOS in Safe Mode (Verbose)");
                    SubEntry->LoadOptions     = StrDuplicate (L"-v -x");
                    SubEntry->UseGraphicsMode = FALSE;
                    AddMenuEntry (SubScreen, (REFIT_MENU_ENTRY *) SubEntry);
                }
            } // Safe mode allowed

            if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_SINGLEUSER)) {
                SubEntry = InitializeLoaderEntry (Entry);
                if (SubEntry != NULL) {
                    SubEntry->me.Title        = StrDuplicate (L"Boot MacOS in Single User Mode (Quiet)");
                    SubEntry->LoadOptions     = StrDuplicate (L"-s");
                    SubEntry->UseGraphicsMode = FALSE;
                    AddMenuEntry (SubScreen, (REFIT_MENU_ENTRY *) SubEntry);
                }
                SubEntry = InitializeLoaderEntry (Entry);
                if (SubEntry != NULL) {
                    SubEntry->me.Title        = StrDuplicate (L"Boot MacOS in Single User Mode (Verbose)");
                    SubEntry->LoadOptions     = StrDuplicate (L"-v -s");
                    SubEntry->UseGraphicsMode = FALSE;
                    AddMenuEntry (SubScreen, (REFIT_MENU_ENTRY *) SubEntry);
                }
            } // single-user mode allowed

            // Check for Apple hardware diagnostics
            if (!(GlobalConfig.HideUIFlags & HIDEUI_FLAG_HWTEST)) {
                UseSystemVolume = FALSE;
                // DA-TAG: Investigate This
                //         Is SingleAPFS really needed?
                //         Play safe and add it for now
                if (SingleAPFS) {
                    if (GlobalConfig.SyncAPFS && (Volume->FSType == FS_TYPE_APFS)) {
                        APPLE_APFS_VOLUME_ROLE VolumeRole = 0;

                        // DA-TAG: Limit to TianoCore
                        #ifndef __MAKEWITH_TIANO
                        Status = EFI_NOT_STARTED;
                        #else
                        Status = RP_GetApfsVolumeInfo (
                            Volume->DeviceHandle,
                            NULL, NULL,
                            &VolumeRole
                        );
                        #endif

                        if (!EFI_ERROR(Status)) {
                            if (VolumeRole == APPLE_APFS_VOLUME_ROLE_PREBOOT) {
                                for (i = 0; i < SystemVolumesCount; i++) {
                                    if (GuidsAreEqual (&(SystemVolumes[i]->PartGuid), &(Volume->PartGuid))) {
                                        UseSystemVolume = TRUE;
                                        break;
                                    }
                                } // for
                            }
                        }
                    }
                }

                DiagnosticsVolume = (UseSystemVolume) ? SystemVolumes[i] : Volume;
                if (FileExists (DiagnosticsVolume->RootDir, MACOSX_DIAGNOSTICS)) {
                    SubEntry = InitializeLoaderEntry (Entry);
                    if (SubEntry != NULL) {
                        MY_FREE_POOL(SubEntry->LoaderPath);
                        SubEntry->Volume          = CopyVolume (DiagnosticsVolume);
                        SubEntry->me.Title        = StrDuplicate (L"Run Apple Hardware Test");
                        SubEntry->LoaderPath      = StrDuplicate (MACOSX_DIAGNOSTICS);
                        SubEntry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_OSX;
                        AddMenuEntry (SubScreen, (REFIT_MENU_ENTRY *) SubEntry);
                    }
                }
            } // if !(GlobalConfig.HideUIFlags & HIDEUI_FLAG_HWTEST)
            BREAD_CRUMB(L"In %s ... 2 END for OSType M", FuncTag);
        }
        else if (Entry->OSType == 'L') {   // Entries for Linux kernels with EFI stub loaders
            LOG_SEP(L"X");
            BREAD_CRUMB(L"In %s ... 1 - START for OSType L", FuncTag);
            File = ReadLinuxOptionsFile (Entry->LoaderPath, Volume);

            BREAD_CRUMB(L"In %s ... 2", FuncTag);
            if (File) {
                BREAD_CRUMB(L"In %s ... 2a 1", FuncTag);
                InitrdName    = FindInitrd (Entry->LoaderPath, Volume);

                BREAD_CRUMB(L"In %s ... 2a 2", FuncTag);
                TokenCount    = ReadTokenLine (File, &TokenList);

                BREAD_CRUMB(L"In %s ... 2a 3", FuncTag);
                KernelVersion = FindNumbers (Entry->LoaderPath);

                BREAD_CRUMB(L"In %s ... 2a 4", FuncTag);
                ReplaceSubstring (&(TokenList[1]), KERNEL_VERSION, KernelVersion);

                BREAD_CRUMB(L"In %s ... 2a 5", FuncTag);
                // First entry requires special processing, since it was initially set
                // up with a default title but correct options by InitializeSubScreen(),
                // earlier.
                if ((TokenCount > 1) && (SubScreen->Entries != NULL) && (SubScreen->Entries[0] != NULL)) {
                    BREAD_CRUMB(L"In %s ... 2a 5a 1", FuncTag);
                    MY_FREE_POOL(SubScreen->Entries[0]->Title);
                    SubScreen->Entries[0]->Title = TokenList[0]
                        ? StrDuplicate (TokenList[0])
                        : StrDuplicate (L"Boot Linux");
                }

                BREAD_CRUMB(L"In %s ... 2a 6", FuncTag);
                FreeTokenLine (&TokenList, &TokenCount);

                BREAD_CRUMB(L"In %s ... 2a 7", FuncTag);
                while ((TokenCount = ReadTokenLine (File, &TokenList)) > 1) {
                    LOG_SEP(L"X");
                    BREAD_CRUMB(L"In %s ... 2a 7a 1 START WHILE LOOP", FuncTag);
                    ReplaceSubstring (&(TokenList[1]), KERNEL_VERSION, KernelVersion);

                    BREAD_CRUMB(L"In %s ... 2a 7a 2", FuncTag);
                    SubEntry = InitializeLoaderEntry (Entry);

                    BREAD_CRUMB(L"In %s ... 2a 7a 3", FuncTag);
                    if (SubEntry != NULL) {
                        BREAD_CRUMB(L"In %s ... 2a 7a 3a 1", FuncTag);
                        SubEntry->me.Title = TokenList[0]
                            ? StrDuplicate (TokenList[0])
                            : StrDuplicate (L"Boot Linux");

                        BREAD_CRUMB(L"In %s ... 2a 7a 3a 2", FuncTag);
                        MY_FREE_POOL(SubEntry->LoadOptions);

                        BREAD_CRUMB(L"In %s ... 2a 7a 3a 3", FuncTag);
                        SubEntry->LoadOptions = AddInitrdToOptions (TokenList[1], InitrdName);

                        BREAD_CRUMB(L"In %s ... 2a 7a 3a 4", FuncTag);
                        SubEntry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_LINUX;

                        BREAD_CRUMB(L"In %s ... 2a 7a 3a 5", FuncTag);
                        AddMenuEntry (SubScreen, (REFIT_MENU_ENTRY *) SubEntry);
                    }

                    BREAD_CRUMB(L"In %s ... 2a 7a 4", FuncTag);
                    FreeTokenLine (&TokenList, &TokenCount);

                    BREAD_CRUMB(L"In %s ... 2a 7a 5 END WHILE LOOP", FuncTag);
                    LOG_SEP(L"X");
                } // while
                FreeTokenLine (&TokenList, &TokenCount);

                BREAD_CRUMB(L"In %s ... 2a 8", FuncTag);
                MY_FREE_POOL(KernelVersion);
                MY_FREE_POOL(InitrdName);
                MY_FREE_FILE(File);
            } // if File

            BREAD_CRUMB(L"In %s ... 3 END for OSType L", FuncTag);
        }
        else if (Entry->OSType == 'E') {   // Entries for ELILO
            LOG_SEP(L"X");
            BREAD_CRUMB(L"In %s ... 1 - START for OSType E", FuncTag);
            SubEntry = InitializeLoaderEntry (Entry);
            if (SubEntry != NULL) {
                SubEntry->me.Title        = StrDuplicate (L"Run ELILO in interactive mode");
                SubEntry->LoadOptions     = StrDuplicate (L"-p");
                SubEntry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_ELILO;
                AddMenuEntry (SubScreen, (REFIT_MENU_ENTRY *) SubEntry);
            }

            SubEntry = InitializeLoaderEntry (Entry);
            if (SubEntry != NULL) {
                SubEntry->me.Title        = StrDuplicate (L"Boot Linux for a 17\" iMac or a 15\" MacBook Pro (*)");
                SubEntry->LoadOptions     = StrDuplicate (L"-d 0 i17");
                SubEntry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_ELILO;
                AddMenuEntry (SubScreen, (REFIT_MENU_ENTRY *) SubEntry);
            }

            SubEntry = InitializeLoaderEntry (Entry);
            if (SubEntry != NULL) {
                SubEntry->me.Title        = StrDuplicate (L"Boot Linux for a 20\" iMac (*)");
                SubEntry->LoadOptions     = StrDuplicate (L"-d 0 i20");
                SubEntry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_ELILO;
                AddMenuEntry (SubScreen, (REFIT_MENU_ENTRY *) SubEntry);
            }

            SubEntry = InitializeLoaderEntry (Entry);
            if (SubEntry != NULL) {
                SubEntry->me.Title        = StrDuplicate (L"Boot Linux for a Mac Mini (*)");
                SubEntry->LoadOptions     = StrDuplicate (L"-d 0 mini");
                SubEntry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_ELILO;
                AddMenuEntry (SubScreen, (REFIT_MENU_ENTRY *) SubEntry);
            }

            AddMenuInfoLine (SubScreen, L"NOTE: This is an example. Entries");
            AddMenuInfoLine (SubScreen, L"marked with (*) may not work.");

            BREAD_CRUMB(L"In %s ... 2 END for OSType E", FuncTag);
        }
        else if (Entry->OSType == 'X') {   // Entries for xom.efi
            LOG_SEP(L"X");
            BREAD_CRUMB(L"In %s ... 1 - START for OSType X", FuncTag);
            // Skip the built-in selection and boot from hard disk only by default
            Entry->LoadOptions = L"-s -h";

            SubEntry = InitializeLoaderEntry (Entry);
            if (SubEntry != NULL) {
                SubEntry->me.Title        = StrDuplicate (L"Boot Windows from Hard Disk");
                SubEntry->LoadOptions     = StrDuplicate (L"-s -h");
                SubEntry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_WINDOWS;
                AddMenuEntry (SubScreen, (REFIT_MENU_ENTRY *) SubEntry);
            }

            SubEntry = InitializeLoaderEntry (Entry);
            if (SubEntry != NULL) {
                SubEntry->me.Title        = StrDuplicate (L"Boot Windows from CD-ROM");
                SubEntry->LoadOptions     = StrDuplicate (L"-s -c");
                SubEntry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_WINDOWS;
                AddMenuEntry (SubScreen, (REFIT_MENU_ENTRY *) SubEntry);
            }

            SubEntry = InitializeLoaderEntry (Entry);
            if (SubEntry != NULL) {
                SubEntry->me.Title        = StrDuplicate (L"Run XOM in text mode");
                SubEntry->LoadOptions     = StrDuplicate (L"-v");
                SubEntry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_WINDOWS;
                AddMenuEntry (SubScreen, (REFIT_MENU_ENTRY *) SubEntry);
            }
            BREAD_CRUMB(L"In %s ... 2 END for OSType X", FuncTag);
        } // Entries for xom.efi

        LOG_SEP(L"X");
        BREAD_CRUMB(L"In %s ... Z 1 - START", FuncTag);
        if (GenerateReturn) {
            BREAD_CRUMB(L"In %s ... Z 1a 1", FuncTag);
            BOOLEAN RetVal = GetReturnMenuEntry (&SubScreen);

            BREAD_CRUMB(L"In %s ... Z 1a 2", FuncTag);
            if (!RetVal) {
                BREAD_CRUMB(L"In %s ... Z 1a 2a 1", FuncTag);
                FreeMenuScreen (&SubScreen);
                BREAD_CRUMB(L"In %s ... Z 1a 2a 2", FuncTag);
            }
        }
        Entry->me.SubScreen = SubScreen;

        BREAD_CRUMB(L"In %s ... Z 2 END MAIN - VOID", FuncTag);
        LOG_SEP(L"X");
    }
} // VOID GenerateSubScreen()

// Sets a few defaults for a loader entry -- mainly the icon, but also the OS type
// code and shortcut letter. For Linux EFI stub loaders, also sets kernel options
// that will (with luck) work fairly automatically.
VOID SetLoaderDefaults (
    LOADER_ENTRY *Entry,
    CHAR16       *LoaderPath,
    REFIT_VOLUME *Volume
) {
    CHAR16  *PathOnly;
    CHAR16  *NameClues;
    CHAR16  *OSIconName     = NULL;
    CHAR16   ShortcutLetter = 0;
    BOOLEAN  MergeFsName    = TRUE;

    #if REFIT_DEBUG > 1
    CHAR16 *FuncTag = L"SetLoaderDefaults";

    ALT_LOG(1, LOG_LINE_NORMAL,
        L"Getting Default Setting for Loader:- '%s'",
        (Entry->me.Title) ? Entry->me.Title : Entry->Title
    );
    #endif

    LOG_SEP(L"X");
    BREAD_CRUMB(L"In %s ... 1 - START", FuncTag);
    NameClues = Basename (LoaderPath);

    BREAD_CRUMB(L"In %s ... 2", FuncTag);
    PathOnly  = FindPath (LoaderPath);

    BREAD_CRUMB(L"In %s ... 3", FuncTag);
    if (!AllowGraphicsMode) {
        BREAD_CRUMB(L"In %s ... 3a 1", FuncTag);
        #if REFIT_DEBUG > 0
        ALT_LOG(1, LOG_THREE_STAR_MID,
            L"In SetLoaderDefaults ... Skipped Loading Icon in %s Mode",
            (GlobalConfig.DirectBoot)
                ? L"DirectBoot"
                : L"Text Screen"
        );
        #endif
    }
    else {
        BREAD_CRUMB(L"In %s ... 3b 1", FuncTag);
        if (Volume->DiskKind == DISK_KIND_NET) {
            BREAD_CRUMB(L"In %s ... 3b 1a 1", FuncTag);
            MergeStrings (&NameClues, Entry->me.Title, L' ');
        }
        else {
            BREAD_CRUMB(L"In %s ... 3b 1a 2a 1", FuncTag);
            if (!Entry->me.Image &&
                !GlobalConfig.HiddenIconsIgnore &&
                GlobalConfig.HiddenIconsPrefer
            ) {
                BREAD_CRUMB(L"In %s ... 3b 1a 2a 1a 1", FuncTag);
                #if REFIT_DEBUG > 0
                ALT_LOG(1, LOG_LINE_NORMAL, L"Checking for '.VolumeIcon' Image");
                #endif

                // use a ".VolumeIcon" image icon for the loader
                // Takes precedence all over options
                Entry->me.Image = egCopyImage (Volume->VolIconImage);
            }

            BREAD_CRUMB(L"In %s ... 3b 1a 2a 2", FuncTag);
            if (!Entry->me.Image) {
                BREAD_CRUMB(L"In %s ... 3b 1a 2a 2a 1", FuncTag);
                #if REFIT_DEBUG > 0
                if (!GlobalConfig.HiddenIconsIgnore &&
                    GlobalConfig.HiddenIconsPrefer
                ) {
                    ALT_LOG(1, LOG_LINE_NORMAL, L"Could Not Find '.VolumeIcon' Image");
                }
                #endif

                BREAD_CRUMB(L"In %s ... 3b 1a 2a 2a 2", FuncTag);
                BOOLEAN MacFlag = FALSE;
                if (LoaderPath &&
                    FoundSubStr (LoaderPath, L"System\\Library\\CoreServices")
                ) {
                    BREAD_CRUMB(L"In %s ... 3b 1a 2a 2a 2a 1", FuncTag);
                    MacFlag = TRUE;
                }

                BREAD_CRUMB(L"In %s ... 3b 1a 2a 2a 3", FuncTag);
                if (!GlobalConfig.SyncAPFS || !MacFlag) {
                    BREAD_CRUMB(L"In %s ... 3b 1a 2a 2a 3a 1", FuncTag);
                    CHAR16 *NoExtension = StripEfiExtension (NameClues);
                    if (NoExtension != NULL) {
                        // locate a custom icon for the loader
                        // Anything found here takes precedence over the "hints" in the OSIconName variable
                        #if REFIT_DEBUG > 0
                        ALT_LOG(1, LOG_LINE_NORMAL, L"Search for Icon in Bootloader Directory");
                        #endif

                        BREAD_CRUMB(L"In %s ... 3b 1a 2a 2a 3a 1a 1", FuncTag);
                        if (!Entry->me.Image) {
                            BREAD_CRUMB(L"In %s ... 3b 1a 2a 2a 3a 1a 1a 1", FuncTag);
                            Entry->me.Image = egLoadIconAnyType (
                                Volume->RootDir,
                                PathOnly,
                                NoExtension,
                                GlobalConfig.IconSizes[ICON_SIZE_BIG]
                            );
                        }

                        BREAD_CRUMB(L"In %s ... 3b 1a 2a 2a 3a 1a 2", FuncTag);
                        if (!Entry->me.Image &&
                            !GlobalConfig.HiddenIconsIgnore
                        ) {
                            BREAD_CRUMB(L"In %s ... 3b 1a 2a 2a 3a 1a 2a 1", FuncTag);
                            Entry->me.Image = egCopyImage (Volume->VolIconImage);
                        }

                        BREAD_CRUMB(L"In %s ... 3b 1a 2a 2a 3a 1a 3", FuncTag);
                        MY_FREE_POOL(NoExtension);
                    }
                    BREAD_CRUMB(L"In %s ... 3b 1a 2a 2a 3a 2", FuncTag);
                }
                BREAD_CRUMB(L"In %s ... 3b 1a 2a 2a 4", FuncTag);
            }

            BREAD_CRUMB(L"In %s ... 3b 1a 2a 3", FuncTag);
            // Begin creating icon "hints" by using last part of directory path leading
            // to the loader
            #if REFIT_DEBUG > 0
            if (Entry->me.Image == NULL) {
                ALT_LOG(1, LOG_THREE_STAR_MID,
                    L"Creating Icon Hint from Loader Path:- '%s'",
                    LoaderPath
                );
            }
            #endif

            BREAD_CRUMB(L"In %s ... 3b 1a 2a 4", FuncTag);
            CHAR16 *Temp = FindLastDirName (LoaderPath);

            BREAD_CRUMB(L"In %s ... 3b 1a 2a 5", FuncTag);
            MergeStrings (&OSIconName, Temp, L',');

            BREAD_CRUMB(L"In %s ... 3b 1a 2a 6", FuncTag);
            MY_FREE_POOL(Temp);

            BREAD_CRUMB(L"In %s ... 3b 1a 2a 7", FuncTag);
            if (OSIconName != NULL) {
                BREAD_CRUMB(L"In %s ... 3b 1a 2a 7a 1", FuncTag);
                ShortcutLetter = OSIconName[0];
            }

            BREAD_CRUMB(L"In %s ... 3b 1a 2a 8", FuncTag);
            // Add every "word" in the filesystem and partition names, delimited by
            // spaces, dashes (-), underscores (_), or colons (:), to the list of
            // hints to be used in searching for OS icons.
            if (Volume->FsName && (Volume->FsName[0] != L'\0')) {
                BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 1", FuncTag);
                if (FoundSubStr (Volume->FsName, L"PreBoot") && GlobalConfig.SyncAPFS) {
                    BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 1a 1", FuncTag);
                    MergeFsName = FALSE;
                }

                BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 2", FuncTag);
                if (MergeFsName) {
                    BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 2a 1", FuncTag);
                    #if REFIT_DEBUG > 0
                    if (Entry->me.Image == NULL) {
                        ALT_LOG(1, LOG_LINE_NORMAL,
                            L"Merge Hints Based on Filesystem Name:- '%s'",
                            Volume->FsName
                        );
                    }
                    #endif

                    BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 2a 2", FuncTag);
                    MergeUniqueWords (&OSIconName, Volume->FsName, L',');
                }
                else {
                    BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 2b 1", FuncTag);
                    if (Volume->VolName && (Volume->VolName[0] != L'\0')) {
                        BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 2b 1a 1", FuncTag);
                        CHAR16 *DisplayName = NULL;

                        if (GlobalConfig.SyncAPFS) {
                            BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 2b 1a 1a 1", FuncTag);
                            EFI_STATUS                 Status;
                            APPLE_APFS_VOLUME_ROLE VolumeRole = 0;

                            // DA-TAG: Limit to TianoCore
                            #ifndef __MAKEWITH_TIANO
                            Status = EFI_NOT_STARTED;
                            #else
                            Status = RP_GetApfsVolumeInfo (
                                Volume->DeviceHandle,
                                NULL, NULL,
                                &VolumeRole
                            );
                            #endif

                            BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 2b 1a 1a 2", FuncTag);
                            if (!EFI_ERROR(Status)) {
                                BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 2b 1a 1a 2a 1", FuncTag);
                                if (VolumeRole == APPLE_APFS_VOLUME_ROLE_PREBOOT) {
                                    BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 2b 1a 1a 2a 1a 1", FuncTag);
                                    DisplayName = GetVolumeGroupName (Entry->LoaderPath, Volume);
                                }
                                BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 2b 1a 1a 2a 2", FuncTag);
                            }

                            BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 2b 1a 1a 3", FuncTag);
                        } // if GlobalConfig.SyncAPFS

                        BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 2b 1a 2", FuncTag);
                        CHAR16 *TargetName = (DisplayName)
                            ? StrDuplicate (DisplayName)
                            : StrDuplicate (Volume->VolName);

                        #if REFIT_DEBUG > 0
                        BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 2b 1a 3", FuncTag);
                        if (Entry->me.Image == NULL) {
                            BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 2b 3a 1", FuncTag);
                            ALT_LOG(1, LOG_LINE_NORMAL,
                                L"Merge Hints Based on Volume Name:- '%s'",
                                TargetName
                            );
                        }
                        #endif

                        BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 2b 1a 4", FuncTag);
                        MergeUniqueWords (&OSIconName, TargetName, L',');

                        BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 2b 1a 5", FuncTag);
                        MY_FREE_POOL(DisplayName);
                        MY_FREE_POOL(TargetName);
                    } // if Volume->VolName

                    BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 2b 2", FuncTag);
                } // if/else MergeFsName

                BREAD_CRUMB(L"In %s ... 3b 1a 2a 8a 3", FuncTag);
            } // if Volume->FsName

            BREAD_CRUMB(L"In %s ... 3b 1a 2a 9", FuncTag);
            if (Volume->PartName &&
                Volume->PartName[0] != L'\0'
            ) {
                #if REFIT_DEBUG > 0
                if (Entry->me.Image == NULL) {
                    ALT_LOG(1, LOG_LINE_NORMAL,
                        L"Merge Hints Based on Partition Name:- '%s'",
                        Volume->PartName
                    );
                }
                #endif

                BREAD_CRUMB(L"In %s ... 3b 1a 2a 9a 1", FuncTag);
                MergeUniqueWords (&OSIconName, Volume->PartName, L',');
            }
            BREAD_CRUMB(L"In %s ... 3b 1a 3", FuncTag);
        } // if/else Volume->DiskKind == DISK_KIND_NET
        BREAD_CRUMB(L"In %s ... 3b 2", FuncTag);
    }

    BREAD_CRUMB(L"In %s ... 4", FuncTag);
    BOOLEAN GetImage = (AllowGraphicsMode && !Entry->me.Image);

    // Detect specific loaders
    BREAD_CRUMB(L"In %s ... 5", FuncTag);
    if (FoundSubStr (NameClues, L"bzImage") ||
        FoundSubStr (NameClues, L"vmlinuz") ||
        FoundSubStr (NameClues, L"kernel")
    ) {
        BREAD_CRUMB(L"In %s ... 5a 1", FuncTag);
        if (Volume->DiskKind != DISK_KIND_NET) {
            BREAD_CRUMB(L"In %s ... 5a 1a 1", FuncTag);
            GuessLinuxDistribution (&OSIconName, Volume, LoaderPath);

            BREAD_CRUMB(L"In %s ... 5a 1a 2", FuncTag);
            Entry->LoadOptions = GetMainLinuxOptions (LoaderPath, Volume);
        }

        BREAD_CRUMB(L"In %s ... 5a 2", FuncTag);
        if (GetImage) MergeStrings (&OSIconName, L"linux", L',');
        Entry->OSType = 'L';

        BREAD_CRUMB(L"In %s ... 5a 3", FuncTag);
        if (ShortcutLetter == 0) {
            BREAD_CRUMB(L"In %s ... 5a 3a 1", FuncTag);
            ShortcutLetter = 'L';
        }

        BREAD_CRUMB(L"In %s ... 5a 4", FuncTag);
        Entry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_LINUX;
    }
    else if (FoundSubStr (LoaderPath, L"refindplus")) {
        BREAD_CRUMB(L"In %s ... 5b 1", FuncTag);
        if (GetImage) MergeStrings (&OSIconName, L"refindplus", L',');

        BREAD_CRUMB(L"In %s ... 5b 2", FuncTag);
        Entry->OSType = 'R';
        ShortcutLetter = 'R';
    }
    else if (FoundSubStr (LoaderPath, L"refind")) {
        BREAD_CRUMB(L"In %s ... 5c 1 - A", FuncTag);
        if (GetImage) MergeStrings (&OSIconName, L"refind", L',');

        BREAD_CRUMB(L"In %s ... 5c 2 - A", FuncTag);
        Entry->OSType = 'R';
        ShortcutLetter = 'R';
    }
    else if (FoundSubStr (LoaderPath, L"refit")) {
        BREAD_CRUMB(L"In %s ... 5c 1 - B", FuncTag);
        if (GetImage) MergeStrings (&OSIconName, L"refit", L',');

        BREAD_CRUMB(L"In %s ... 5c 2 - B", FuncTag);
        Entry->OSType = 'R';
        ShortcutLetter = 'R';
    }
    else if (FoundSubStr (LoaderPath, MACOSX_LOADER_PATH)) {
        BREAD_CRUMB(L"In %s ... 5d 1", FuncTag);
        if (FileExists (Volume->RootDir, L"EFI\\refindplus\\config.conf") ||
            FileExists (Volume->RootDir, L"EFI\\refindplus\\refind.conf") ||
            FileExists (Volume->RootDir, L"EFI\\refind\\config.conf")     ||
            FileExists (Volume->RootDir, L"EFI\\refind\\refind.conf")
        ) {
            BREAD_CRUMB(L"In %s ... 5d 1a 1", FuncTag);
            if (GetImage) MergeStrings (&OSIconName, L"refind", L',');

            BREAD_CRUMB(L"In %s ... 5d 1a 2", FuncTag);
            Entry->OSType = 'R';
            ShortcutLetter = 'R';
        }
        else {
            BREAD_CRUMB(L"In %s ... 5d 1b 1", FuncTag);
            if (GetImage) MergeStrings (&OSIconName, L"mac", L',');

            BREAD_CRUMB(L"In %s ... 5d 1b 2", FuncTag);
            Entry->OSType = 'M';
            ShortcutLetter = 'M';
            Entry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_OSX;
        }
        BREAD_CRUMB(L"In %s ... 5d 2", FuncTag);
    }
    else if (MyStriCmp (NameClues, L"diags.efi")) {
        BREAD_CRUMB(L"In %s ... 5e 1", FuncTag);
        if (GetImage) MergeStrings (&OSIconName, L"hwtest", L',');
    }
    else if (MyStriCmp (NameClues, L"e.efi") ||
        MyStriCmp (NameClues, L"elilo.efi")  ||
        FoundSubStr (NameClues, L"elilo")
    ) {
        BREAD_CRUMB(L"In %s ... 5f 1", FuncTag);
        if (GetImage) MergeStrings (&OSIconName, L"elilo,linux", L',');
        Entry->OSType = 'E';

        BREAD_CRUMB(L"In %s ... 5f 2", FuncTag);
        if (ShortcutLetter == 0) {
            BREAD_CRUMB(L"In %s ... 5f 2a 1", FuncTag);
            ShortcutLetter = 'L';
        }

        BREAD_CRUMB(L"In %s ... 5f 3", FuncTag);
        Entry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_ELILO;
    }
    else if (FoundSubStr (NameClues, L"grub")) {
        BREAD_CRUMB(L"In %s ... 5g 1", FuncTag);
        if (GetImage) MergeStrings (&OSIconName, L"grub,linux", L',');

        BREAD_CRUMB(L"In %s ... 5g 2", FuncTag);
        Entry->OSType = 'G';
        ShortcutLetter = 'G';
        Entry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_GRUB;
    }
    else if (MyStriCmp (NameClues, L"cdboot.efi") ||
        MyStriCmp (NameClues, L"bootmgr.efi")  ||
        MyStriCmp (NameClues, L"bootmgfw.efi") ||
        MyStriCmp (NameClues, L"bkpbootmgfw.efi")
    ) {
        BREAD_CRUMB(L"In %s ... 5h 1", FuncTag);
        if (GetImage) MergeStrings (&OSIconName, L"win8", L',');

        BREAD_CRUMB(L"In %s ... 5h 2", FuncTag);
        Entry->OSType = 'W';
        ShortcutLetter = 'W';
        Entry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_WINDOWS;
    }
    else if (MyStriCmp (NameClues, L"xom.efi")) {
        BREAD_CRUMB(L"In %s ... 5i 1", FuncTag);
        if (GetImage) MergeStrings (&OSIconName, L"xom,win,win8", L',');

        BREAD_CRUMB(L"In %s ... 5i 2", FuncTag);
        Entry->OSType = 'X';
        ShortcutLetter = 'W';
        Entry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_WINDOWS;
    }
    else if (MyStriCmp (NameClues, L"opencore")) {
        BREAD_CRUMB(L"In %s ... 5j 1", FuncTag);
        if (GetImage) MergeStrings (&OSIconName, L"opencore", L',');

        BREAD_CRUMB(L"In %s ... 5j 2", FuncTag);
        Entry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_OPENCORE;
    }
    else if (MyStriCmp (NameClues, L"clover")) {
        BREAD_CRUMB(L"In %s ... 5k 1", FuncTag);
        if (GetImage) MergeStrings (&OSIconName, L"clover", L',');

        BREAD_CRUMB(L"In %s ... 5k 2", FuncTag);
        Entry->UseGraphicsMode = GlobalConfig.GraphicsFor & GRAPHICS_FOR_CLOVER;
    }
    else if (FoundSubStr (NameClues, L"ipxe")) {
        BREAD_CRUMB(L"In %s ... 5l 1", FuncTag);
        if (GetImage) MergeStrings (&OSIconName, L"network", L',');

        BREAD_CRUMB(L"In %s ... 5l 2", FuncTag);
        Entry->OSType = 'N';
        ShortcutLetter = 'N';
    }

    BREAD_CRUMB(L"In %s ... 6", FuncTag);
    if ((ShortcutLetter >= 'a') && (ShortcutLetter <= 'z')) {
        BREAD_CRUMB(L"In %s ... 6a 1", FuncTag);
        ShortcutLetter = ShortcutLetter - 'a' + 'A'; // convert lowercase to uppercase
    }

    BREAD_CRUMB(L"In %s ... 7", FuncTag);
    Entry->me.ShortcutLetter = ShortcutLetter;

    BREAD_CRUMB(L"In %s ... 8", FuncTag);
    if (GetImage) {
        #if REFIT_DEBUG > 0
        ALT_LOG(1, LOG_LINE_NORMAL,
            L"Trying to Locate an Icon Based on Hints:- '%s'",
            OSIconName
        );
        #endif

        BREAD_CRUMB(L"In %s ... 8a 1", FuncTag);
        Entry->me.Image = LoadOSIcon (OSIconName, L"unknown", FALSE);
    }

    BREAD_CRUMB(L"In %s ... 9", FuncTag);
    MY_FREE_POOL(PathOnly);
    MY_FREE_POOL(OSIconName);
    MY_FREE_POOL(NameClues);

    BREAD_CRUMB(L"In %s ... 10 - END:- VOID", FuncTag);
    LOG_SEP(L"X");
} // VOID SetLoaderDefaults()

CHAR16 * GetVolumeGroupName (
    IN CHAR16       *LoaderPath,
    IN REFIT_VOLUME *Volume
) {
    UINTN   i;
    CHAR16 *VolumeGroupName = NULL;

    if (!GlobalConfig.SyncAPFS) {
        // Early Return
        return NULL;
    }

    if (SingleAPFS) {
        for (i = 0; i < SystemVolumesCount; i++) {
            if (GuidsAreEqual (&(SystemVolumes[i]->PartGuid), &(Volume->PartGuid))) {
                VolumeGroupName = StrDuplicate (SystemVolumes[i]->VolName);
                break;
            }
        } // for
    }

    if (!VolumeGroupName) {
        for (i = 0; i < SystemVolumesCount; i++) {
            if (
                FoundSubStr (
                    LoaderPath,
                    GuidAsString (&(SystemVolumes[i]->VolUuid))
                )
            ) {
                VolumeGroupName = StrDuplicate (SystemVolumes[i]->VolName);
                break;
            }
        } // for
    }

    if (!VolumeGroupName) {
        for (i = 0; i < DataVolumesCount; i++) {
            if (
                FoundSubStr (
                    LoaderPath,
                    GuidAsString (&(DataVolumes[i]->VolUuid))
                )
            ) {
                VolumeGroupName = StrDuplicate (DataVolumes[i]->VolName);
                break;
            }
        } // for
    }

    return VolumeGroupName;
} // CHAR16 * GetVolumeGroupName()

// Add an NVRAM-based EFI boot loader entry to the menu.
static
LOADER_ENTRY * AddEfiLoaderEntry (
    IN EFI_DEVICE_PATH *EfiLoaderPath,
    IN CHAR16          *LoaderTitle,
    IN UINT16           EfiBootNum,
    IN UINTN            Row,
    IN EG_IMAGE        *Icon
) {
    CHAR16        *TempStr;
    CHAR16        *FullTitle  = NULL;
    CHAR16        *OSIconName = NULL;
    LOADER_ENTRY  *Entry;

    Entry = InitializeLoaderEntry (NULL);
    if (Entry) {
        Entry->DiscoveryType = DISCOVERY_TYPE_AUTO;

        if (LoaderTitle) {
            FullTitle = PoolPrint (L"Reboot to %s", LoaderTitle);
        }

        Entry->me.Row        = Row;
        Entry->me.Tag        = TAG_FIRMWARE_LOADER;
        Entry->me.Title      = StrDuplicate ((FullTitle) ? FullTitle : L"Unknown");
        Entry->Title         = StrDuplicate ((LoaderTitle) ? LoaderTitle : L"Unknown"); // without "Reboot to"
        Entry->EfiLoaderPath = DuplicateDevicePath (EfiLoaderPath);
        TempStr              = DevicePathToStr (EfiLoaderPath);

        #if REFIT_DEBUG > 0
        ALT_LOG(1, LOG_LINE_NORMAL, L"UEFI Loader Path:- '%s'", TempStr);
        #endif

        MY_FREE_POOL(TempStr);

        Entry->EfiBootNum = EfiBootNum;

        MergeUniqueWords (&OSIconName, Entry->me.Title, L',');
        MergeUniqueStrings (&OSIconName, L"Unknown", L',');

        if (Icon) {
            Entry->me.Image = egCopyImage (Icon);
        }
        else {
            Entry->me.Image = LoadOSIcon (OSIconName, NULL, FALSE);
        }

        if (Row == 0) {
            Entry->me.BadgeImage = BuiltinIcon (BUILTIN_ICON_VOL_EFI);
        }
        else {
            Entry->me.BadgeImage = NULL;
        }

        MY_FREE_POOL(OSIconName);

        Entry->Volume      = NULL;
        Entry->LoaderPath  = NULL;
        Entry->LoadOptions = NULL;
        Entry->InitrdPath  = NULL;
        Entry->Enabled     = TRUE;

        AddMenuEntry (MainMenu, (REFIT_MENU_ENTRY *) Entry);
    } // if Entry


    return Entry;
} // LOADER_ENTRY * AddEfiLoaderEntry()


// Add a specified EFI boot loader to the list,
// using automatic settings for icons, options, etc.
static
LOADER_ENTRY * AddLoaderEntry (
    IN CHAR16       *LoaderPath,
    IN CHAR16       *LoaderTitle,
    IN REFIT_VOLUME *Volume,
    IN BOOLEAN       SubScreenReturn
) {
    EFI_STATUS     Status;
    LOADER_ENTRY  *Entry;
    CHAR16        *TitleEntry  = NULL;
    CHAR16        *DisplayName = NULL;

    if (Volume == NULL) {
        // Early Return
        return NULL;
    }

    if (Volume->FSType == FS_TYPE_NTFS) {
        if (MyStriCmp (Volume->VolName, L"System Reserved") ||
            MyStriCmp (Volume->VolName, L"Basic Data Partition") ||
            MyStriCmp (Volume->VolName, L"Microsoft Reserved Partition")
        ) {
            // Early Return on Windows Support Volume
            return NULL;
        }
    }

    APPLE_APFS_VOLUME_ROLE VolumeRole = 0;

    // DA-TAG: Limit to TianoCore
    #ifndef __MAKEWITH_TIANO
    Status = EFI_NOT_STARTED;
    #else
    Status = RP_GetApfsVolumeInfo (
        Volume->DeviceHandle,
        NULL, NULL,
        &VolumeRole
    );
    #endif

    if (!EFI_ERROR(Status)) {
        if (VolumeRole != APPLE_APFS_VOLUME_ROLE_SYSTEM  &&
            VolumeRole != APPLE_APFS_VOLUME_ROLE_PREBOOT &&
            VolumeRole != APPLE_APFS_VOLUME_ROLE_UNDEFINED
        ) {
            // Early Return on APFS Support Volume
            return NULL;
        }

        if (GlobalConfig.SyncAPFS) {
            if (VolumeRole == APPLE_APFS_VOLUME_ROLE_PREBOOT) {
                DisplayName = GetVolumeGroupName (LoaderPath, Volume);

                if (!DisplayName) {
                    // Do not display this PreBoot Volume Menu Entry
                    return NULL;
                }

                BOOLEAN FoundPreBootName = FoundSubStr (DisplayName, L"PreBoot");
                if (FoundPreBootName) {
                    MY_FREE_POOL(DisplayName);
                }
                else {
                    MY_FREE_POOL(Volume->VolName);
                    Volume->VolName = PoolPrint (L"PreBoot - %s", DisplayName);
                }
            }
        }
    }

    CleanUpPathNameSlashes (LoaderPath);
    Entry = InitializeLoaderEntry (NULL);
    if (Entry == NULL) {
        // Early Return
        return NULL;
    }

    Entry->DiscoveryType = DISCOVERY_TYPE_AUTO;
    TitleEntry   = (LoaderTitle) ? LoaderTitle : LoaderPath;
    Entry->Title = StrDuplicate (TitleEntry);

    #if REFIT_DEBUG > 0
    if (DisplayName) {
        ALT_LOG(1, LOG_THREE_STAR_MID, L"Synced PreBoot:- '%s'", DisplayName);
    }
    ALT_LOG(1, LOG_LINE_NORMAL, L"Add Loader Entry:- '%s'", Entry->Title);
    ALT_LOG(1, LOG_LINE_NORMAL, L"UEFI Loader File:- '%s'", LoaderPath);
    #endif

    Entry->me.Title = (DisplayName || Volume->VolName)
        ? PoolPrint (
            L"Boot %s from %s",
            (LoaderTitle) ? LoaderTitle : LoaderPath,
            (DisplayName) ? DisplayName : Volume->VolName
        )
        : PoolPrint (
            L"Boot %s",
            (LoaderTitle) ? LoaderTitle : LoaderPath
        );
    Entry->me.Row = 0;
    Entry->me.BadgeImage = egCopyImage (Volume->VolBadgeImage);

    if ((LoaderPath != NULL) && (LoaderPath[0] != L'\\')) {
        Entry->LoaderPath = StrDuplicate (L"\\");
    }
    else {
        Entry->LoaderPath = NULL;
    }

    MergeStrings (&(Entry->LoaderPath), LoaderPath, 0);
    Entry->Volume = CopyVolume (Volume);
    SetLoaderDefaults (Entry, LoaderPath, Volume);
    GenerateSubScreen (Entry, Volume, SubScreenReturn);
    AddMenuEntry (MainMenu, (REFIT_MENU_ENTRY *) Entry);

    #if REFIT_DEBUG > 0
    LOG_MSG(
        "%s  - Found '%s' on '%s'",
        OffsetNext,
        TitleEntry,
        (DisplayName)
            ? DisplayName
            : (Volume->VolName)
                ? Volume->VolName
                : Entry->LoaderPath
    );

    ALT_LOG(1, LOG_THREE_STAR_END, L"Successfully Created Menu Entry for %s", Entry->Title);
    #endif

    MY_FREE_POOL(DisplayName);

    return Entry;
} // LOADER_ENTRY * AddLoaderEntry()

// Returns -1 if (Time1 < Time2), +1 if (Time1 > Time2), or 0 if
// (Time1 == Time2). Precision is only to the nearest second; since
// this is used for sorting boot loader entries, differences smaller
// than this are likely to be meaningless (and unlikely!).
static
INTN TimeComp (
    IN EFI_TIME *Time1,
    IN EFI_TIME *Time2
) {
    INT64 Time1InSeconds, Time2InSeconds;

    // Following values are overestimates; I'm assuming 31 days in every month.
    // This is fine for the purpose of this function, which is limited
    Time1InSeconds = (INT64) (Time1->Second)
        + ((INT64) (Time1->Minute)      * 60)
        + ((INT64) (Time1->Hour)        * 3600)
        + ((INT64) (Time1->Day)         * 86400)
        + ((INT64) (Time1->Month)       * 2678400)
        + ((INT64) (Time1->Year - 1998) * 32140800);

    Time2InSeconds = (INT64) (Time2->Second)
        + ((INT64) (Time2->Minute)      * 60)
        + ((INT64) (Time2->Hour)        * 3600)
        + ((INT64) (Time2->Day)         * 86400)
        + ((INT64) (Time2->Month)       * 2678400)
        + ((INT64) (Time2->Year - 1998) * 32140800);

    if (Time1InSeconds < Time2InSeconds) {
        return (-1);
    }
    else if (Time1InSeconds > Time2InSeconds) {
        return (1);
    }

    return 0;
} // INTN TimeComp()

// Adds a loader list element, keeping it sorted by date. EXCEPTION: Fedora's rescue
// kernel, which begins with "vmlinuz-0-rescue," should not be at the top of the list,
// since that will make it the default if kernel folding is enabled, so float it to
// the end.
// Returns the new first element (the one with the most recent date).
static struct
LOADER_LIST * AddLoaderListEntry (
    struct LOADER_LIST *LoaderList,
    struct LOADER_LIST *NewEntry
) {
    struct LOADER_LIST *LatestEntry  = NULL;
    struct LOADER_LIST *CurrentEntry = NULL;
    struct LOADER_LIST *PrevEntry    = NULL;
    BOOLEAN LinuxRescue              = FALSE;

    if (LoaderList == NULL) {
        LatestEntry = NewEntry;
    }
    else {
        LatestEntry = CurrentEntry = LoaderList;
        if (StriSubCmp (L"vmlinuz-0-rescue", NewEntry->FileName)) {
            LinuxRescue = TRUE;
        }

        while ((CurrentEntry != NULL) &&
            !StriSubCmp (L"vmlinuz-0-rescue", CurrentEntry->FileName) &&
            (LinuxRescue || (TimeComp (&(NewEntry->TimeStamp), &(CurrentEntry->TimeStamp)) < 0))
        ) {
            PrevEntry   = CurrentEntry;
            CurrentEntry = CurrentEntry->NextEntry;
        } // while

        NewEntry->NextEntry = CurrentEntry;

        if (PrevEntry == NULL) {
            LatestEntry = NewEntry;
        }
        else {
            PrevEntry->NextEntry = NewEntry;
        }
    }

    return (LatestEntry);
} // static VOID AddLoaderListEntry()

// Delete the LOADER_LIST linked list
static
VOID CleanUpLoaderList (
    struct LOADER_LIST *LoaderList
) {
    struct LOADER_LIST *Temp;

    while (LoaderList != NULL) {
        Temp = LoaderList;
        LoaderList = LoaderList->NextEntry;
        MY_FREE_POOL(Temp->FileName);
        MY_FREE_POOL(Temp);
    } // while
} // static VOID CleanUpLoaderList()

// Returns FALSE if the specified file/volume matches the GlobalConfig.DontScanDirs
// or GlobalConfig.DontScanVolumes specification, or if Path points to a volume
// other than the one specified by Volume, or if the specified path is SelfDir.
// Returns TRUE if none of these conditions is met -- that is, if the path is
// eligible for scanning.
BOOLEAN ShouldScan (
    REFIT_VOLUME *Volume,
    CHAR16       *Path
) {
    UINTN    i           = 0;
    CHAR16  *VolName     = NULL;
    CHAR16  *VolGuid     = NULL;
    CHAR16  *PathCopy    = NULL;
    CHAR16  *DontScanDir = NULL;
    BOOLEAN  ScanIt      = TRUE;

    if (MyStriCmp (Path, SelfDirPath)
        && (Volume->DeviceHandle == SelfVolume->DeviceHandle)
    ) {
        return FALSE;
    }

    EFI_STATUS                     Status;
    CHAR16           *TmpVolNameA  = NULL;
    CHAR16           *TmpVolNameB  = NULL;
    APPLE_APFS_VOLUME_ROLE VolumeRole = 0;

    // DA-TAG: Limit to TianoCore
    #ifndef __MAKEWITH_TIANO
    Status = EFI_NOT_STARTED;
    #else
    Status = RP_GetApfsVolumeInfo (
        Volume->DeviceHandle,
        NULL, NULL,
        &VolumeRole
    );
    #endif

    if (!EFI_ERROR(Status)) {
        if (VolumeRole != APPLE_APFS_VOLUME_ROLE_SYSTEM  &&
            VolumeRole != APPLE_APFS_VOLUME_ROLE_PREBOOT &&
            VolumeRole != APPLE_APFS_VOLUME_ROLE_UNDEFINED
        ) {
            // Early Return on APFS Support Volume
            return FALSE;
        }

        if (GlobalConfig.SyncAPFS) {
            TmpVolNameB = PoolPrint (L"%s - DATA", Volume->VolName);
            if (IsIn (TmpVolNameB, GlobalConfig.DontScanVolumes)) {
                ScanIt = FALSE;
            }
            MY_FREE_POOL(TmpVolNameB);
            if (!ScanIt) return FALSE;

            TmpVolNameA = SanitiseString (Volume->VolName);
            TmpVolNameB = PoolPrint (L"%s - DATA", TmpVolNameA);
            if (IsIn (TmpVolNameB, GlobalConfig.DontScanVolumes)) {
                ScanIt = FALSE;
            }
            MY_FREE_POOL(TmpVolNameA);
            MY_FREE_POOL(TmpVolNameB);
            if (!ScanIt) return FALSE;
        }
    }

    VolGuid = GuidAsString (&(Volume->PartGuid));
    if (IsIn (VolGuid, GlobalConfig.DontScanVolumes)
        || IsIn (Volume->FsName, GlobalConfig.DontScanVolumes)
        || IsIn (Volume->VolName, GlobalConfig.DontScanVolumes)
        || IsIn (Volume->PartName, GlobalConfig.DontScanVolumes)
    ) {
        ScanIt = FALSE;
    }
    MY_FREE_POOL(VolGuid);
    if (!ScanIt) return FALSE;

    // See if Path includes an explicit volume declaration that is NOT Volume.
    PathCopy = StrDuplicate (Path);
    if (SplitVolumeAndFilename (&PathCopy, &VolName)) {
        if (VolName) {
            if (!MyStriCmp (VolName, Volume->FsName)
                || !MyStriCmp(VolName, Volume->PartName)
            ) {
                ScanIt = FALSE;
            }
        }
    }
    MY_FREE_POOL(PathCopy);
    MY_FREE_POOL(VolName);

    // See if Volume is in GlobalConfig.DontScanDirs.
    while (ScanIt &&
        (DontScanDir = FindCommaDelimited (GlobalConfig.DontScanDirs, i++))
    ) {
        SplitVolumeAndFilename (&DontScanDir, &VolName);
        CleanUpPathNameSlashes (DontScanDir);
        if (VolName != NULL) {
            if (VolumeMatchesDescription (Volume, VolName)
                && MyStriCmp (DontScanDir, Path)
            ) {
                ScanIt = FALSE;
            }
        }
        else {
            if (MyStriCmp (DontScanDir, Path)) {
                ScanIt = FALSE;
            }
        }

        MY_FREE_POOL(DontScanDir);
        MY_FREE_POOL(VolName);
    } // while

    return ScanIt;
} // BOOLEAN ShouldScan()

// Returns TRUE if the file is byte-for-byte identical with the fallback file
// on the volume AND if the file is not itself the fallback file; returns
// FALSE if the file is not identical to the fallback file OR if the file
// IS the fallback file. Intended for use in excluding the fallback boot
// loader when it is a duplicate of another boot loader.
static
BOOLEAN DuplicatesFallback (
    IN REFIT_VOLUME *Volume,
    IN CHAR16       *FileName
) {
    EFI_STATUS       Status;
    EFI_FILE_HANDLE  FileHandle;
    EFI_FILE_HANDLE  FallbackHandle;
    EFI_FILE_INFO   *FileInfo;
    EFI_FILE_INFO   *FallbackInfo;
    CHAR8           *FileContents;
    CHAR8           *FallbackContents;
    UINTN            FileSize     = 0;
    UINTN            FallbackSize = 0;
    BOOLEAN          AreIdentical = FALSE;

    if (!FileExists (Volume->RootDir, FileName) ||
        !FileExists (Volume->RootDir, FALLBACK_FULLNAME)
    ) {
        return FALSE;
    }

    CleanUpPathNameSlashes (FileName);

    if (MyStriCmp (FileName, FALLBACK_FULLNAME)) {
        // Identical filenames ... so not a duplicate
        return FALSE;
    }

    Status = REFIT_CALL_5_WRAPPER(
        Volume->RootDir->Open, Volume->RootDir,
        &FileHandle, FileName,
        EFI_FILE_MODE_READ, 0
    );
    if (Status != EFI_SUCCESS) {
        return FALSE;
    }

    FileInfo = LibFileInfo (FileHandle);
    FileSize = FileInfo->FileSize;
    MY_FREE_POOL(FileInfo);

    Status = REFIT_CALL_5_WRAPPER(
        Volume->RootDir->Open, Volume->RootDir,
        &FallbackHandle, FALLBACK_FULLNAME,
        EFI_FILE_MODE_READ, 0
    );
    if (Status != EFI_SUCCESS) {
        REFIT_CALL_1_WRAPPER(FileHandle->Close, FileHandle);
        return FALSE;
    }

    FallbackInfo = LibFileInfo (FallbackHandle);
    FallbackSize = FallbackInfo->FileSize;
    MY_FREE_POOL(FallbackInfo);

    if (FallbackSize == FileSize) { // could be identical; do full check.
        FileContents = AllocatePool (FileSize);
        FallbackContents = AllocatePool (FallbackSize);
        if (FileContents && FallbackContents) {
            Status = REFIT_CALL_3_WRAPPER(
                FileHandle->Read, FileHandle,
                &FileSize, FileContents
            );
            if (Status == EFI_SUCCESS) {
                Status = REFIT_CALL_3_WRAPPER(
                    FallbackHandle->Read, FallbackHandle,
                    &FallbackSize, FallbackContents
                );
            }
            if (Status == EFI_SUCCESS) {
                AreIdentical = (CompareMem (
                    FileContents,
                    FallbackContents,
                    FileSize
                ) == 0);
            }
        }

        MY_FREE_POOL(FileContents);
        MY_FREE_POOL(FallbackContents);
    }

    // BUG ALERT: Some systems (e.g., DUET, some Macs with large displays) crash if the
    // following two calls are reversed.
    REFIT_CALL_1_WRAPPER(FileHandle->Close, FallbackHandle);
    REFIT_CALL_1_WRAPPER(FileHandle->Close, FileHandle);

    return AreIdentical;
} // BOOLEAN DuplicatesFallback()

// Returns FALSE if two measures of file size are identical for a single file,
// TRUE if not or if the file cannot be opened and the other measure is non-0.
// Despite the function's name, this is not an actual direct test of symbolic
// link status, since EFI does not officially support symlinks. It does seem
// to be a reliable indicator, though. (OTOH, some disk errors might cause a
// file to fail to open, which would return a false positive -- but as this
// function is used to exclude symbolic links from the list of boot loaders,
// that would be fine, as such boot loaders would not work anyway.)
//
// NB: *FullName MUST be properly cleaned up (using 'CleanUpPathNameSlashes')
static
BOOLEAN IsSymbolicLink (
    REFIT_VOLUME  *Volume,
    CHAR16        *FullName,
    EFI_FILE_INFO *DirEntry
) {
    EFI_FILE_HANDLE  FileHandle;
    EFI_FILE_INFO   *FileInfo = NULL;
    EFI_STATUS       Status;
    UINTN            FileSize2 = 0;

    Status = REFIT_CALL_5_WRAPPER(
        Volume->RootDir->Open, Volume->RootDir,
        &FileHandle, FullName,
        EFI_FILE_MODE_READ, 0
    );
    if (Status == EFI_SUCCESS) {
        FileInfo = LibFileInfo (FileHandle);
        if (FileInfo != NULL) {
            FileSize2 = FileInfo->FileSize;
        }
    }

    MY_FREE_POOL(FileInfo);

    return (DirEntry->FileSize != FileSize2);
} // BOOLEAN IsSymbolicLink()

// Scan an individual directory for EFI boot loader files and, if found,
// add them to the list. Exception: Ignores FALLBACK_FULLNAME, which is picked
// up in ScanEfiFiles(). Sorts the entries within the loader directory so that
// the most recent one appears first in the list.
// Returns TRUE if a duplicate for FALLBACK_FILENAME was found, FALSE if not.
static
BOOLEAN ScanLoaderDir (
    IN REFIT_VOLUME *Volume,
    IN CHAR16       *Path,
    IN CHAR16       *Pattern
) {
    EFI_STATUS               Status;
    REFIT_DIR_ITER           DirIter;
    EFI_FILE_INFO           *DirEntry;
    CHAR16                  *Message;
    CHAR16                  *Extension;
    CHAR16                  *FullName;
    struct LOADER_LIST      *NewLoader;
    struct LOADER_LIST      *LoaderList  = NULL;
    LOADER_ENTRY            *FirstKernel = NULL;
    LOADER_ENTRY            *LatestEntry = NULL;
    BOOLEAN                  FoundFallbackDuplicate = FALSE, IsLinux, InSelfPath;

    #if REFIT_DEBUG > 0
    ALT_LOG(1, LOG_LINE_NORMAL, L"Scanning for '%s' in '%s'", Pattern, Path);
    #endif

    InSelfPath = MyStriCmp (Path, SelfDirPath);

    if ((!SelfDirPath || !Path ||
        (InSelfPath && (Volume->DeviceHandle != SelfVolume->DeviceHandle)) ||
        (!InSelfPath)) && (ShouldScan (Volume, Path))
    ) {
        // Look through contents of the directory
        DirIterOpen (Volume->RootDir, Path, &DirIter);

        while (DirIterNext (&DirIter, 2, Pattern, &DirEntry)) {
            Extension = FindExtension (DirEntry->FileName);
            FullName  = StrDuplicate (Path);

            MergeStrings (&FullName, DirEntry->FileName, L'\\');
            CleanUpPathNameSlashes (FullName);

            // IsSymbolicLink (Volume, FullName, DirEntry) = is symbolic link
            // HasSignedCounterpart (Volume, FullName) = file with same name plus ".efi.signed" is present
            if (DirEntry->FileName[0] == '.' ||
                MyStriCmp (Extension, L".icns") ||
                MyStriCmp (Extension, L".png") ||
                (MyStriCmp (DirEntry->FileName, FALLBACK_BASENAME) &&
                (MyStriCmp (Path, L"EFI\\BOOT"))) ||
                FilenameIn (Volume, Path, DirEntry->FileName, SHELL_NAMES) ||
                IsSymbolicLink (Volume, FullName, DirEntry) ||
                HasSignedCounterpart (Volume, FullName) ||
                FilenameIn (Volume, Path, DirEntry->FileName, GlobalConfig.DontScanFiles) ||
                !IsValidLoader (Volume->RootDir, FullName)
            ) {
                // Skip This Entry
                continue;
            }

            NewLoader = AllocateZeroPool (sizeof (struct LOADER_LIST));
            if (NewLoader != NULL) {
                NewLoader->FileName  = StrDuplicate (FullName);
                NewLoader->TimeStamp = DirEntry->ModificationTime;
                LoaderList           = AddLoaderListEntry (LoaderList, NewLoader);

                if (DuplicatesFallback (Volume, FullName)) {
                    FoundFallbackDuplicate = TRUE;
                }
            }

            MY_FREE_POOL(Extension);
            MY_FREE_POOL(FullName);
        } // while

        if (LoaderList != NULL) {
            IsLinux   = FALSE;
            NewLoader = LoaderList;

            while (NewLoader != NULL) {
                IsLinux = (
                    StriSubCmp (L"bzImage", NewLoader->FileName) ||
                    StriSubCmp (L"vmlinuz", NewLoader->FileName) ||
                    StriSubCmp (L"kernel", NewLoader->FileName)
                );

                if ((FirstKernel != NULL) && IsLinux && GlobalConfig.FoldLinuxKernels) {
                    AddKernelToSubmenu (FirstKernel, NewLoader->FileName, Volume);
                }
                else {
                    LatestEntry = AddLoaderEntry (
                        NewLoader->FileName,
                        NULL, Volume,
                        !(IsLinux && GlobalConfig.FoldLinuxKernels)
                    );
                    if (IsLinux && (FirstKernel == NULL)) {
                        FirstKernel = LatestEntry;
                    }
                }

                NewLoader = NewLoader->NextEntry;
            } // while

            if (FirstKernel != NULL && IsLinux && GlobalConfig.FoldLinuxKernels) {
                #if REFIT_DEBUG > 0
                ALT_LOG(1, LOG_LINE_NORMAL, L"Adding 'Return' Entry to Folded Linux Kernels");
                #endif

                BOOLEAN RetVal = GetReturnMenuEntry (&FirstKernel->me.SubScreen);
                if (!RetVal) {
                    FreeMenuScreen (&FirstKernel->me.SubScreen);
                }
            }

            CleanUpLoaderList (LoaderList);
        }

        Status = DirIterClose (&DirIter);
        // NOTE: EFI_INVALID_PARAMETER really is an error that should be reported;
        // but reports have been received from users that get this error occasionally
        // but nothing wrong has been found or the problem reproduced. It is therefore
        // being put down to buggy EFI implementations and that particular error igored.
        if ((Status != EFI_NOT_FOUND) && (Status != EFI_INVALID_PARAMETER)) {
            if (Path) {
                Message = PoolPrint (
                    L"While Scanning the '%s' Directory on '%s'",
                    Path, Volume->VolName
                );
            }
            else {
                Message = PoolPrint (
                    L"While Scanning the Root Directory on '%s'",
                    Volume->VolName
                );
            }

            CheckError (Status, Message);
            MY_FREE_POOL(Message);
        }
    } // if !SelfDirPath

    return FoundFallbackDuplicate;
} // static VOID ScanLoaderDir()

// Run the IPXE_DISCOVER_NAME program, which obtains the IP address
// of the boot server and the name of the boot file it delivers.
static
CHAR16 * RuniPXEDiscover (
    EFI_HANDLE Volume
) {
    EFI_STATUS        Status;
    EFI_DEVICE_PATH  *FilePath;
    EFI_HANDLE        iPXEHandle;
    CHAR16           *boot_info = NULL;
    UINTN             boot_info_size = 0;

    FilePath = FileDevicePath (Volume, IPXE_DISCOVER_NAME);
    Status = REFIT_CALL_6_WRAPPER(
        gBS->LoadImage, FALSE,
        SelfImageHandle, FilePath,
        NULL, 0, &iPXEHandle
    );
    if (Status != 0) {
        return NULL;
    }

    REFIT_CALL_3_WRAPPER(
        gBS->StartImage, iPXEHandle,
        &boot_info_size, &boot_info
    );

    return boot_info;
} // RuniPXEDiscover()

// Scan for network (PXE) boot servers. This function relies on the presence
// of the IPXE_DISCOVER_NAME and IPXE_NAME program files on the volume from
// which RefindPlus launched. As of December 6, 2014, these tools are not
// entirely reliable. See BUILDING.txt for information on building them.
static
VOID ScanNetboot (VOID) {
    CHAR16        *iPXEFileName = IPXE_NAME;
    CHAR16        *Location;
    REFIT_VOLUME  *NetVolume;

    #if REFIT_DEBUG > 0
    ALT_LOG(1, LOG_LINE_NORMAL, L"Scanning for iPXE boot options");
    #endif

    if (FileExists (SelfVolume->RootDir, IPXE_NAME) &&
        FileExists (SelfVolume->RootDir, IPXE_DISCOVER_NAME) &&
        IsValidLoader (SelfVolume->RootDir, IPXE_DISCOVER_NAME) &&
        IsValidLoader (SelfVolume->RootDir, IPXE_NAME)
    ) {
        Location = RuniPXEDiscover (SelfVolume->DeviceHandle);
        if (Location != NULL && FileExists (SelfVolume->RootDir, iPXEFileName)) {
            NetVolume = CopyVolume (SelfVolume);
            if (NetVolume) {
                NetVolume->DiskKind = DISK_KIND_NET;

                MY_FREE_IMAGE(NetVolume->VolBadgeImage);
                NetVolume->VolBadgeImage = BuiltinIcon (BUILTIN_ICON_VOL_NET);

                MY_FREE_POOL(NetVolume->PartName);
                MY_FREE_POOL(NetVolume->VolName);
                MY_FREE_POOL(NetVolume->FsName);

                AddLoaderEntry (iPXEFileName, Location, NetVolume, TRUE);

                FreeVolume (&NetVolume);
            }
        }
    }
} // VOID ScanNetboot()


// Adds *FullFileName as a MacOS loader, if it exists.
// Returns TRUE if the fallback loader is NOT a duplicate of this one,
// FALSE if it IS a duplicate.
static
BOOLEAN ScanMacOsLoader (
    REFIT_VOLUME *Volume,
    CHAR16       *FullFileName
) {
    BOOLEAN   AddThisEntry       = TRUE;
    BOOLEAN   ScanFallbackLoader = TRUE;
    CHAR16   *VolName            = NULL;
    CHAR16   *PathName           = NULL;
    CHAR16   *FileName           = NULL;
    UINTN     i;

    SplitPathName (FullFileName, &VolName, &PathName, &FileName);
    if (FileExists (Volume->RootDir, FullFileName) &&
        !FilenameIn (Volume, PathName, L"boot.efi", GlobalConfig.DontScanFiles)
    ) {
        if (FileExists (Volume->RootDir, L"EFI\\refind\\config.conf") ||
            FileExists (Volume->RootDir, L"EFI\\refind\\refind.conf")
        ) {
            AddLoaderEntry (FullFileName, L"RefindPlus", Volume, TRUE);
        }
        else {
            if (GlobalConfig.SyncAPFS) {
                for (i = 0; i < SystemVolumesCount; i++) {
                    if (GuidsAreEqual (&(SystemVolumes[i]->VolUuid), &(Volume->VolUuid))) {
                        AddThisEntry = FALSE;
                        break;
                    }
                }
            }

            if (AddThisEntry) {
                AddLoaderEntry (FullFileName, L"MacOS", Volume, TRUE);
            }
        }

        if (DuplicatesFallback (Volume, FullFileName)) {
            ScanFallbackLoader = FALSE;
        }
    }

    MY_FREE_POOL(VolName);
    MY_FREE_POOL(PathName);
    MY_FREE_POOL(FileName);

    return ScanFallbackLoader;
} // VOID ScanMacOsLoader()

static
VOID ScanEfiFiles (
    REFIT_VOLUME *Volume
) {
    EFI_STATUS        Status;
    REFIT_DIR_ITER    EfiDirIter;
    EFI_FILE_INFO    *EfiDirEntry;
    UINTN             i, Length;
    CHAR16           *Temp;
    CHAR16           *FileName;
    CHAR16           *SelfPath;
    CHAR16           *MatchPatterns;
    CHAR16           *VolName            = NULL;
    CHAR16           *Directory          = NULL;
    BOOLEAN           ScanFallbackLoader = TRUE;
    BOOLEAN           FoundBRBackup      = FALSE;

    #if REFIT_DEBUG > 0
    UINTN    LogLineType;
    #endif

    if (!Volume ||
        !Volume->RootDir ||
        !Volume->VolName ||
        !Volume->IsReadable
    ) {
        // Early Return on Invalid Volume
        return;
    }

    if (Volume->FSType == FS_TYPE_NTFS) {
        if (MyStriCmp (Volume->VolName, L"System Reserved") ||
            MyStriCmp (Volume->VolName, L"Basic Data Partition") ||
            MyStriCmp (Volume->VolName, L"Microsoft Reserved Partition")
        ) {
            // Early Return on Windows Support Volume
            return;
        }
    }

    APPLE_APFS_VOLUME_ROLE VolumeRole = 0;

    // DA-TAG: Limit to TianoCore
    #ifndef __MAKEWITH_TIANO
    Status = EFI_NOT_STARTED;
    #else
    Status = RP_GetApfsVolumeInfo (
        Volume->DeviceHandle,
        NULL, NULL,
        &VolumeRole
    );
    #endif

    if (!EFI_ERROR(Status)) {
        if (VolumeRole != APPLE_APFS_VOLUME_ROLE_SYSTEM  &&
            VolumeRole != APPLE_APFS_VOLUME_ROLE_PREBOOT &&
            VolumeRole != APPLE_APFS_VOLUME_ROLE_UNDEFINED
        ) {
            // Early Return on APFS Support Volume
            return;
        }
    }

    if (GlobalConfig.SyncAPFS) {
        if (!EFI_ERROR(Status) && SingleAPFS &&
            (
                VolumeRole == APPLE_APFS_VOLUME_ROLE_SYSTEM  ||
                VolumeRole == APPLE_APFS_VOLUME_ROLE_PREBOOT ||
                VolumeRole == APPLE_APFS_VOLUME_ROLE_UNDEFINED
            )
        ) {
            for (i = 0; i < SkipApfsVolumesCount; i++) {
                if (GuidsAreEqual (&(SkipApfsVolumes[i]->PartGuid), &(Volume->PartGuid))) {
                    // Early Return on Skipped APFS Volume
                    return;
                }
            }
        }

        for (i = 0; i < SystemVolumesCount; i++) {
            if (GuidsAreEqual (&(SystemVolumes[i]->VolUuid), &(Volume->VolUuid))) {
                // Early Return on ReMapped APFS System Volume
                return;
            }
        }
    }

    #if REFIT_DEBUG > 0
    /* Exception for LOG_THREE_STAR_SEP */
    LogLineType = (FirstLoaderScan) ? LOG_THREE_STAR_MID : LOG_THREE_STAR_SEP;
    ALT_LOG(1, LogLineType,
        L"Scanning Volume '%s' for UEFI Loaders",
        Volume->VolName ? Volume->VolName : L"** No Name **"
    );
    #endif

    FirstLoaderScan = FALSE;

    MatchPatterns = StrDuplicate (LOADER_MATCH_PATTERNS);
    if (GlobalConfig.ScanAllLinux) {
        MergeStrings (&MatchPatterns, LINUX_MATCH_PATTERNS, L',');
    }

    // Check for MacOS boot loader
    if (ShouldScan (Volume, MACOSX_LOADER_DIR)) {
        FileName = StrDuplicate (MACOSX_LOADER_PATH);
        ScanFallbackLoader &= ScanMacOsLoader (Volume, FileName);
        MY_FREE_POOL(FileName);
        DirIterOpen (Volume->RootDir, L"\\", &EfiDirIter);

        while (DirIterNext (&EfiDirIter, 1, NULL, &EfiDirEntry)) {
            if (IsGuid (EfiDirEntry->FileName)) {
                FileName = PoolPrint (L"%s\\%s", EfiDirEntry->FileName, MACOSX_LOADER_PATH);
                ScanFallbackLoader &= ScanMacOsLoader (Volume, FileName);
                MY_FREE_POOL(FileName);
                FileName = PoolPrint (L"%s\\%s", EfiDirEntry->FileName, L"boot.efi");

                if (Volume->FSType != FS_TYPE_APFS) {
                    if (!StriSubCmp (FileName, GlobalConfig.MacOSRecoveryFiles)) {
                        MergeStrings (&GlobalConfig.MacOSRecoveryFiles, FileName, L',');
                    }
                }

                MY_FREE_POOL(FileName);
            }
        } // while
        DirIterClose (&EfiDirIter);

        // check for XOM
        FileName = StrDuplicate (L"System\\Library\\CoreServices\\xom.efi");
        if (FileExists (Volume->RootDir, FileName) &&
            !FilenameIn (Volume, MACOSX_LOADER_DIR, L"xom.efi", GlobalConfig.DontScanFiles)
        ) {
            AddLoaderEntry (FileName, L"Windows XP (XoM)", Volume, TRUE);
            if (DuplicatesFallback (Volume, FileName)) {
                ScanFallbackLoader = FALSE;
            }
        }

        MY_FREE_POOL(FileName);
    } // if ShouldScan

    // check for Microsoft boot loader/menu
    if (ShouldScan (Volume, L"EFI\\Microsoft\\Boot")) {
        FileName = StrDuplicate (L"EFI\\Microsoft\\Boot\\bkpbootmgfw.efi");
        if (FileExists (Volume->RootDir, FileName) &&
            !FilenameIn (
                Volume,
                L"EFI\\Microsoft\\Boot",
                L"bkpbootmgfw.efi",
                GlobalConfig.DontScanFiles
            )
        ) {
            // Boot Repair Backup
            AddLoaderEntry (FileName, L"UEFI Windows (BRBackup)", Volume, TRUE);
            FoundBRBackup = TRUE;
            if (DuplicatesFallback (Volume, FileName)) {
                ScanFallbackLoader = FALSE;
            }
        }

        MY_FREE_POOL(FileName);

        FileName = StrDuplicate (L"EFI\\Microsoft\\Boot\\bootmgfw.efi");
        if (FileExists (Volume->RootDir, FileName) &&
            !FilenameIn (
                Volume,
                L"EFI\\Microsoft\\Boot",
                L"bootmgfw.efi",
                GlobalConfig.DontScanFiles
            )
        ) {
            AddLoaderEntry (
                FileName,
                (FoundBRBackup) ? L"Assumed UEFI Windows (Potentially GRUB)" : L"Windows (UEFI)",
                Volume, TRUE
            );

            if (DuplicatesFallback (Volume, FileName)) {
                ScanFallbackLoader = FALSE;
            }
        }

        MY_FREE_POOL(FileName);
    } // if ShouldScan

    // Scan the root directory for EFI executables
    if (ScanLoaderDir (Volume, L"\\", MatchPatterns)) {
        ScanFallbackLoader = FALSE;
    }

    // Scan subdirectories of the EFI directory (as per the standard)
    DirIterOpen (Volume->RootDir, L"EFI", &EfiDirIter);
    while (DirIterNext (&EfiDirIter, 1, NULL, &EfiDirEntry)) {

        if (MyStriCmp (EfiDirEntry->FileName, L"tools") ||
            EfiDirEntry->FileName[0] == '.'
        ) {
            // Skip this ... Does not contain boot loaders or is scanned later
            continue;
        }

        FileName = PoolPrint (L"EFI\\%s", EfiDirEntry->FileName);
        if (ScanLoaderDir (Volume, FileName, MatchPatterns)) {
            ScanFallbackLoader = FALSE;
        }

        MY_FREE_POOL(FileName);
    } // while

    Status = DirIterClose (&EfiDirIter);
    if ((Status != EFI_NOT_FOUND) && (Status != EFI_INVALID_PARAMETER)) {
        Temp = PoolPrint (
            L"While Scanning the EFI System Partition on '%s'",
            Volume->VolName
        );
        CheckError (Status, Temp);
        MY_FREE_POOL(Temp);
    }

    // Scan user-specified (or additional default) directories.
    i = 0;
    while ((Directory = FindCommaDelimited (GlobalConfig.AlsoScan, i++)) != NULL) {
        if (ShouldScan (Volume, Directory)) {
            SplitVolumeAndFilename (&Directory, &VolName);
            CleanUpPathNameSlashes (Directory);

            Length = StrLen (Directory);
            if ((Length > 0) && ScanLoaderDir (Volume, Directory, MatchPatterns)) {
                ScanFallbackLoader = FALSE;
            }

            MY_FREE_POOL(VolName);
        }

        MY_FREE_POOL(Directory);
    } // while

    // Do not scan the fallback loader if it is on the same volume and a duplicate of RefindPlus itself.
    SelfPath = DevicePathToStr (SelfLoadedImage->FilePath);
    CleanUpPathNameSlashes (SelfPath);

    if ((Volume->DeviceHandle == SelfLoadedImage->DeviceHandle) &&
        DuplicatesFallback (Volume, SelfPath)
    ) {
        ScanFallbackLoader = FALSE;
    }
    MY_FREE_POOL(SelfPath);

    // Create an entry for fallback loaders
    if (ScanFallbackLoader &&
        FileExists (Volume->RootDir, FALLBACK_FULLNAME) &&
        ShouldScan (Volume, L"EFI\\BOOT") &&
        !FilenameIn (Volume, L"EFI\\BOOT", FALLBACK_BASENAME, GlobalConfig.DontScanFiles)
    ) {
        AddLoaderEntry (FALLBACK_FULLNAME, L"Fallback Loader", Volume, TRUE);
    }

    MY_FREE_POOL(MatchPatterns);
} // static VOID ScanEfiFiles()

// Scan internal disks for valid EFI boot loaders.
static
VOID ScanInternal (VOID) {
    UINTN VolumeIndex;

    #if REFIT_DEBUG > 0
    ALT_LOG(1, LOG_LINE_THIN_SEP, L"Scan for Internal Disk Volumes with Mode:- 'UEFI'");
    #endif

    FirstLoaderScan = TRUE;
    for (VolumeIndex = 0; VolumeIndex < VolumesCount; VolumeIndex++) {
        if (Volumes[VolumeIndex]->DiskKind == DISK_KIND_INTERNAL) {
            ScanEfiFiles (Volumes[VolumeIndex]);
        }
    } // for

    FirstLoaderScan = FALSE;
} // static VOID ScanInternal()

// Scan external disks for valid EFI boot loaders.
static
VOID ScanExternal (VOID) {
    UINTN VolumeIndex;

    #if REFIT_DEBUG > 0
    ALT_LOG(1, LOG_LINE_THIN_SEP, L"Scan for External Disk Volumes with Mode:- 'UEFI'");
    #endif

    FirstLoaderScan = TRUE;
    for (VolumeIndex = 0; VolumeIndex < VolumesCount; VolumeIndex++) {
        if (Volumes[VolumeIndex]->DiskKind == DISK_KIND_EXTERNAL) {
            ScanEfiFiles (Volumes[VolumeIndex]);
        }
    } // for

    FirstLoaderScan = FALSE;

} // static VOID ScanExternal()

// Scan internal disks for valid EFI boot loaders.
static
VOID ScanOptical (VOID) {
    UINTN VolumeIndex;

    #if REFIT_DEBUG > 0
    ALT_LOG(1, LOG_LINE_THIN_SEP, L"Scan for Optical Discs with Mode:- 'UEFI'");
    #endif

    FirstLoaderScan = TRUE;
    for (VolumeIndex = 0; VolumeIndex < VolumesCount; VolumeIndex++) {
        if (Volumes[VolumeIndex]->DiskKind == DISK_KIND_OPTICAL) {
            ScanEfiFiles (Volumes[VolumeIndex]);
        }
    } // for
    FirstLoaderScan = FALSE;

} // static VOID ScanOptical()

// Scan options stored in UEFI firmware's boot list. Adds discovered and allowed
// items to the specified Row.
// If MatchThis != NULL, only adds items with labels containing any element of
// the MatchThis comma-delimited string; otherwise, searches for anything that
// does not match GlobalConfig.DontScanFirmware or the contents of the
// HiddenFirmware UEFI variable.
// If Icon != NULL, uses the specified icon; otherwise tries to find one to
// match the label.
static
VOID ScanFirmwareDefined (
    IN UINTN     Row,
    IN CHAR16   *MatchThis,
    IN EG_IMAGE *Icon
) {
    BOOT_ENTRY_LIST *BootEntries;
    BOOT_ENTRY_LIST *CurrentEntry;
    BOOLEAN          FoundItem   = FALSE;
    BOOLEAN          ScanIt      = TRUE;
    CHAR16          *OneElement  = NULL;
    CHAR16          *DontScanFirmware;
    UINTN            i;

    #if REFIT_DEBUG > 0
    if (Row == 0) {
        ALT_LOG(1, LOG_LINE_NORMAL,
            L"Scanning for Firmware Defined Boot Options"
        );
    }
    #endif

    DontScanFirmware = ReadHiddenTags (L"HiddenFirmware");

    #if REFIT_DEBUG > 0
    if (GlobalConfig.DontScanFirmware != NULL) {
        ALT_LOG(1, LOG_LINE_NORMAL,
            L"GlobalConfig.DontScanFirmware:- '%s'",
            GlobalConfig.DontScanFirmware
        );
    }
    if (DontScanFirmware != NULL) {
        ALT_LOG(1, LOG_LINE_NORMAL,
            L"Firmware Hidden Tags:- '%s'",
            DontScanFirmware
        );
    }
    #endif

    if (GlobalConfig.DontScanFirmware && *GlobalConfig.DontScanFirmware) {
        // Only merge GlobalConfig.DontScanFirmware if not empty
        MergeStrings (&DontScanFirmware, GlobalConfig.DontScanFirmware, L',');
    }

    if (Row == 0) {
        #if REFIT_DEBUG > 0
        ALT_LOG(1, LOG_THREE_STAR_MID, L"NB: Excluding UEFI Shell from Scan");
        #endif

        MergeStrings(&DontScanFirmware, L"shell", L',');
    }

    #if REFIT_DEBUG > 0
    if (DontScanFirmware != NULL) {
        ALT_LOG(1, LOG_LINE_NORMAL,
            L"Merged Firmware Scan Exclusion List:- '%s'",
            DontScanFirmware
        );
    }
    #endif

    BootEntries  = FindBootOrderEntries();
    CurrentEntry = BootEntries;

    while (CurrentEntry != NULL) {
        if (MatchThis) {
            ScanIt = FALSE;
            i = 0;
            while (!ScanIt && (OneElement = FindCommaDelimited (MatchThis, i++))) {
                if (StriSubCmp (OneElement, CurrentEntry->BootEntry.Label) &&
                    !IsInSubstring (CurrentEntry->BootEntry.Label, DontScanFirmware)
                ) {
                    ScanIt = TRUE;
                }
                MY_FREE_POOL(OneElement);
            } // while
        }
        else {
            if (IsInSubstring (CurrentEntry->BootEntry.Label, DontScanFirmware)) {
                ScanIt = FALSE;
            }
        }

        if (ScanIt) {
            #if REFIT_DEBUG > 0
            ALT_LOG(1, LOG_LINE_NORMAL,
                L"Adding UEFI Loader Entry for '%s'",
                CurrentEntry->BootEntry.Label
            );
            #endif

            FoundItem = TRUE;
            AddEfiLoaderEntry (
                CurrentEntry->BootEntry.DevPath,
                CurrentEntry->BootEntry.Label,
                CurrentEntry->BootEntry.BootNum,
                Row,
                Icon
            );
        }

        CurrentEntry = CurrentEntry->NextBootEntry;

        // Assume the next item is to be scanned
        ScanIt = TRUE;
    } // while

    MY_FREE_POOL(DontScanFirmware);
    DeleteBootOrderEntries (BootEntries);

    #if REFIT_DEBUG > 0
    ALT_LOG(1, LOG_LINE_NORMAL,
        L"Evaluate Firmware Defined Options ... Relevant Options:- '%s'",
        (FoundItem) ? L"Added" : L"None"
    );
    #endif
} // static VOID ScanFirmwareDefined()

// default volume badge icon based on disk kind
EG_IMAGE * GetDiskBadge (IN UINTN DiskType) {
    EG_IMAGE * Badge = NULL;

    if (GlobalConfig.HideUIFlags & HIDEUI_FLAG_BADGES) {
        #if REFIT_DEBUG > 0
        ALT_LOG(1, LOG_THREE_STAR_MID,
            L"Skipped ... Config Setting is Active:- 'HideUI Badges'"
        );
        #endif

        return NULL;
    }

    switch (DiskType) {
        case BBS_HARDDISK: Badge = BuiltinIcon (BUILTIN_ICON_VOL_INTERNAL); break;
        case BBS_USB:      Badge = BuiltinIcon (BUILTIN_ICON_VOL_EXTERNAL); break;
        case BBS_CDROM:    Badge = BuiltinIcon (BUILTIN_ICON_VOL_OPTICAL);  break;
    } // switch

    return Badge;
} // EG_IMAGE * GetDiskBadge()

//
// pre-boot tool functions
//

static
LOADER_ENTRY * AddToolEntry (
    IN REFIT_VOLUME *Volume,
    IN CHAR16       *LoaderPath,
    IN CHAR16       *LoaderTitle,
    IN EG_IMAGE     *Image,
    IN CHAR16        ShortcutLetter,
    IN BOOLEAN       UseGraphicsMode
) {
    LOADER_ENTRY *Entry;

    Entry = AllocateZeroPool (sizeof (LOADER_ENTRY));
    Entry->me.Title          = (LoaderTitle) ? StrDuplicate (LoaderTitle) : StrDuplicate (L"Unknown Tool");
    Entry->me.Tag            = TAG_TOOL;
    Entry->me.Row            = 1;
    Entry->me.ShortcutLetter = ShortcutLetter;
    Entry->me.Image          = egCopyImage (Image);
    Entry->LoaderPath        = (LoaderPath) ? StrDuplicate (LoaderPath) : NULL;
    Entry->Volume            = CopyVolume (Volume);
    Entry->UseGraphicsMode   = UseGraphicsMode;

    AddMenuEntry (MainMenu, (REFIT_MENU_ENTRY *) Entry);

    return Entry;
} // static LOADER_ENTRY * AddToolEntry()

// Locates boot loaders.
// NOTE: This assumes that GlobalConfig.LegacyType is correctly set.
VOID ScanForBootloaders (VOID) {
    UINTN     i;
    BOOLEAN   DeleteItem;
    BOOLEAN   ScanForLegacy       = FALSE;
    BOOLEAN   AmendedDontScan     = FALSE;
    CHAR16   *HiddenTags          = NULL;
    CHAR16   *HiddenLegacy        = NULL;
    CHAR16   *DontScanItem        = NULL;
    CHAR16   *OrigDontScanDirs    = NULL;
    CHAR16   *OrigDontScanFiles   = NULL;
    CHAR16   *OrigDontScanVolumes = NULL;
    CHAR16    ShortCutKey;

    #if REFIT_DEBUG > 0
    CHAR16  *MsgStr = NULL;
    #endif

    ScanningLoaders = TRUE;

    #if REFIT_DEBUG > 0
    ALT_LOG(1, LOG_BLANK_LINE_SEP, L"X");
    MsgStr = StrDuplicate (L"S E E K   U E F I   L O A D E R S");
    ALT_LOG(1, LOG_LINE_SEPARATOR, L"%s", MsgStr);
    LOG_MSG("%s", MsgStr);
    LOG_MSG("\n");
    MY_FREE_POOL(MsgStr);
    #endif

    // Determine up-front if we will be scanning for legacy loaders
    for (i = 0; i < NUM_SCAN_OPTIONS; i++) {
        if (GlobalConfig.ScanFor[i] == 'c' || GlobalConfig.ScanFor[i] == 'C' ||
            GlobalConfig.ScanFor[i] == 'h' || GlobalConfig.ScanFor[i] == 'H' ||
            GlobalConfig.ScanFor[i] == 'b' || GlobalConfig.ScanFor[i] == 'B'
        ) {
            ScanForLegacy = TRUE;
            break;
        }
    } // for

    // If UEFI & scanning for legacy loaders & deep legacy scan, update NVRAM boot manager list
    if ((GlobalConfig.LegacyType == LEGACY_TYPE_UEFI) &&
        ScanForLegacy && GlobalConfig.DeepLegacyScan
    ) {
        BdsDeleteAllInvalidLegacyBootOptions();
        BdsAddNonExistingLegacyBootOptions();
    }

    if (GlobalConfig.HiddenTags) {
        // We temporarily modify GlobalConfig.DontScanFiles and GlobalConfig.DontScanVolumes
        // to include contents of EFI HiddenTags and HiddenLegacy variables so that we do not
        // have to re-load these UEFI variables in several functions called from this one. To
        // do this, we must be able to restore the original contents, so back them up first.
        // We do *NOT* do this with the GlobalConfig.DontScanFirmware and
        // GlobalConfig.DontScanTools variables because they are used in only one function
        // each, so it is easier to create a temporary variable for the merged contents
        // there and not modify the global variable.
        OrigDontScanFiles   = StrDuplicate (GlobalConfig.DontScanFiles);
        OrigDontScanVolumes = StrDuplicate (GlobalConfig.DontScanVolumes);

        // Add hidden tags to two GlobalConfig.DontScan* variables.
        HiddenTags = ReadHiddenTags (L"HiddenTags");
        if ((HiddenTags) && (StrLen (HiddenTags) > 0)) {
            #if REFIT_DEBUG > 0
            ALT_LOG(1, LOG_LINE_NORMAL,
                L"Merging HiddenTags into 'Dont Scan Files':- '%s'",
                HiddenTags
            );
            ALT_LOG(1, LOG_BLANK_LINE_SEP, L"X");
            #endif

            MergeStrings (&GlobalConfig.DontScanFiles, HiddenTags, L',');
        }
        MY_FREE_POOL(HiddenTags);

        HiddenLegacy = ReadHiddenTags (L"HiddenLegacy");
        if ((HiddenLegacy) && (StrLen (HiddenLegacy) > 0)) {
            #if REFIT_DEBUG > 0
            ALT_LOG(1, LOG_LINE_NORMAL,
                L"Merging HiddenLegacy into 'Dont Scan Volumes':- '%s'",
                HiddenLegacy
            );
            ALT_LOG(1, LOG_BLANK_LINE_SEP, L"X");
            #endif

            MergeStrings (&GlobalConfig.DontScanVolumes, HiddenLegacy, L',');
        }

        MY_FREE_POOL(HiddenLegacy);
    } // if GlobalConfig.HiddenTags

    // DA-TAG: Override Hidden/Disabled PreBoot Volumes if SyncAPFS is Active
    if (GlobalConfig.SyncAPFS) {
        OrigDontScanDirs = StrDuplicate (GlobalConfig.DontScanDirs);

        if (GlobalConfig.DontScanFiles) {
            while ((DontScanItem = FindCommaDelimited (GlobalConfig.DontScanFiles, i)) != NULL) {
                DeleteItem = (FoundSubStr (DontScanItem, L"Preboot:"))
                    ? TRUE
                    : (MyStriCmp (DontScanItem, L"Preboot"))
                        ? TRUE
                        : FALSE;

                if (!DeleteItem) {
                    i++;
                }
                else {
                    DeleteItemFromCsvList (DontScanItem, GlobalConfig.DontScanFiles);
                    AmendedDontScan = TRUE;
                }

                MY_FREE_POOL(DontScanItem);
            } // while
        } // if GlobalConfig

        if (GlobalConfig.DontScanDirs) {
            while ((DontScanItem = FindCommaDelimited (GlobalConfig.DontScanDirs, i)) != NULL) {
                DeleteItem = (FoundSubStr (DontScanItem, L"Preboot:"))
                    ? TRUE
                    : (MyStriCmp (DontScanItem, L"Preboot"))
                        ? TRUE : FALSE;

                if (!DeleteItem) {
                    i++;
                }
                else {
                    DeleteItemFromCsvList (DontScanItem, GlobalConfig.DontScanDirs);
                    AmendedDontScan = TRUE;
                }

                MY_FREE_POOL(DontScanItem);
            } // while
        } // if GlobalConfig

        if (GlobalConfig.DontScanVolumes) {
            while ((DontScanItem = FindCommaDelimited (GlobalConfig.DontScanVolumes, i)) != NULL) {
                DeleteItem = (FoundSubStr (DontScanItem, L"Preboot:"))
                    ? TRUE
                    : (MyStriCmp (DontScanItem, L"Preboot"))
                        ? TRUE
                        : FALSE;

                if (!DeleteItem) {
                    i++;
                }
                else {
                    DeleteItemFromCsvList (DontScanItem, GlobalConfig.DontScanVolumes);
                    AmendedDontScan = TRUE;
                }

                MY_FREE_POOL(DontScanItem);
            } // while
        } // if GlobalConfig

        #if REFIT_DEBUG > 0
        if (AmendedDontScan) {
            ALT_LOG(1, LOG_STAR_SEPARATOR,
                L"Ignored PreBoot Volumes in 'Dont Scan' List ... SyncAPFS is Active"
            );
        }
        #endif
    } // if GlobalConfig.SyncAPFS

    // Get count of options set to be scanned
    UINTN SetOptions = 0;
    for (i = 0; i < NUM_SCAN_OPTIONS; i++) {
        switch (GlobalConfig.ScanFor[i]) {
            case 'm': case 'M':
            case 'i': case 'I':
            case 'h': case 'H':
            case 'e': case 'E':
            case 'b': case 'B':
            case 'o': case 'O':
            case 'c': case 'C':
            case 'n': case 'N':
            case 'f': case 'F':
            SetOptions = SetOptions + 1;
        } // switch
    } // for

    // scan for loaders and tools, add them to the menu
    for (i = 0; i <= SetOptions; i++) {
        switch (GlobalConfig.ScanFor[i]) {
            case 'm': case 'M':
                #if REFIT_DEBUG > 0
                if (LogNewLine) LOG_MSG("\n");
                LogNewLine = TRUE;

                LOG_MSG("Scan Manual:");
                ALT_LOG(1, LOG_LINE_THIN_SEP, L"Process User Configured Stanzas");
                #endif

                ScanUserConfigured (GlobalConfig.ConfigFilename);
                break;

            case 'i': case 'I':
                #if REFIT_DEBUG > 0
                if (LogNewLine) LOG_MSG("\n");
                LogNewLine = TRUE;

                LOG_MSG("Scan Internal:");
                #endif

                ScanInternal();
                break;

            case 'h': case 'H':
                #if REFIT_DEBUG > 0
                if (LogNewLine) LOG_MSG("\n");
                LogNewLine = TRUE;

                LOG_MSG("Scan Internal (Legacy):");
                #endif

                ScanLegacyInternal();
                break;

            case 'e': case 'E':
                #if REFIT_DEBUG > 0
                if (LogNewLine) LOG_MSG("\n");
                LogNewLine = TRUE;

                LOG_MSG("Scan External:");
                #endif

                ScanExternal();
                break;

            case 'b': case 'B':
            #if REFIT_DEBUG > 0
                if (LogNewLine) LOG_MSG("\n");
                LogNewLine = TRUE;

                LOG_MSG("Scan External (Legacy):");
                #endif

                ScanLegacyExternal();
                break;

            case 'o': case 'O':
                #if REFIT_DEBUG > 0
                if (LogNewLine) LOG_MSG("\n");
                LogNewLine = TRUE;

                LOG_MSG("Scan Optical:");
                #endif

                ScanOptical();
                break;

            case 'c': case 'C':
            #if REFIT_DEBUG > 0
                if (LogNewLine) LOG_MSG("\n");
                LogNewLine = TRUE;

                LOG_MSG("Scan Optical (Legacy):");
                #endif

                ScanLegacyDisc();
                break;

            case 'n': case 'N':
                #if REFIT_DEBUG > 0
                if (LogNewLine) LOG_MSG("\n");
                LogNewLine = TRUE;

                LOG_MSG("Scan Net Boot:");
                #endif

                ScanNetboot();
                break;

            case 'f': case 'F':
                #if REFIT_DEBUG > 0
                if (LogNewLine) LOG_MSG("\n");
                LogNewLine = TRUE;

                LOG_MSG("Scan Firmware:");
                #endif

                ScanFirmwareDefined(0, NULL, NULL);
                break;
        } // switch
    } // for

    #if REFIT_DEBUG > 0
    // Reset
    LogNewLine = FALSE;
    #endif

    if (GlobalConfig.HiddenTags) {
        // Restore the backed-up GlobalConfig.DontScan* variables
        MY_FREE_POOL(GlobalConfig.DontScanFiles);
        MY_FREE_POOL(GlobalConfig.DontScanVolumes);
        GlobalConfig.DontScanFiles   = OrigDontScanFiles;
        GlobalConfig.DontScanVolumes = OrigDontScanVolumes;
    }

    if (GlobalConfig.SyncAPFS && OrigDontScanDirs && AmendedDontScan) {
        // Restore the stashed GlobalConfig.DontScanDirs variable
        MY_FREE_POOL(GlobalConfig.DontScanDirs);
        GlobalConfig.DontScanDirs = OrigDontScanDirs;
    }
    else {
        MY_FREE_POOL(OrigDontScanDirs);
    }

    if (MainMenu->EntryCount < 1) {
        #if REFIT_DEBUG > 0
        MsgStr = StrDuplicate (L"Could Not Find Boot Loaders");
        ALT_LOG(1, LOG_THREE_STAR_MID, L"%s", MsgStr);
        LOG_MSG("* WARN: %s", MsgStr);
        LOG_MSG("\n\n");
        MY_FREE_POOL(MsgStr);
        #endif
    }
    else {
        // Assign shortcut keys
        #if REFIT_DEBUG > 0
        ALT_LOG(1, LOG_BLANK_LINE_SEP, L"X");
        CHAR16 *LogSection = L"A S S I G N   S H O R T C U T   K E Y S";
        ALT_LOG(1, LOG_LINE_SEPARATOR, L"%s", LogSection);
        LOG_MSG("\n\n");
        LOG_MSG("%s", LogSection);

        MsgStr = StrDuplicate (L"Set Shortcuts:");
        ALT_LOG(1, LOG_BLANK_LINE_SEP, L"X");
        ALT_LOG(1, LOG_LINE_NORMAL, L"%s", MsgStr);
        LOG_MSG("\n");
        LOG_MSG("%s", MsgStr);
        MY_FREE_POOL(MsgStr);

        UINTN KeyNum = 0;
        #endif

        for (i = 0; i < MainMenu->EntryCount && MainMenu->Entries[i]->Row == 0; i++) {
            if (i < 9) {
                #if REFIT_DEBUG > 0
                KeyNum = i + 1;
                #endif

                ShortCutKey = (CHAR16) ('1' + i);
            }
            else if (i == 9) {
                ShortCutKey = (CHAR16) ('9' - i);

                #if REFIT_DEBUG > 0
                KeyNum = 0;
                #endif
            }
            else {
                break;
            }
            MainMenu->Entries[i]->ShortcutDigit = ShortCutKey;

            #if REFIT_DEBUG > 0
            MsgStr = PoolPrint (
                L"Set Key '%d' to Run Loader:- '%s'",
                KeyNum, MainMenu->Entries[i]->Title
            );
            ALT_LOG(1, LOG_LINE_NORMAL, L"%s", MsgStr);
            LOG_MSG("%s  - %s", OffsetNext, MsgStr);
            MY_FREE_POOL(MsgStr);
            #endif
        }  // for

        #if REFIT_DEBUG > 0
        MsgStr = PoolPrint (
            L"Assigned Shortcut Key%s to %d of %d Loader%s",
            (i == 1) ? L"" : L"s",
            i, MainMenu->EntryCount,
            (MainMenu->EntryCount == 1) ? L"" : L"s"
        );
        ALT_LOG(1, LOG_THREE_STAR_SEP, L"%s", MsgStr);
        LOG_MSG("\n\n");
        LOG_MSG("INFO: %s", MsgStr);
        LOG_MSG("\n\n");
        MY_FREE_POOL(MsgStr);
        #endif
    }

    // Wait for user acknowledgement if there were errors
    FinishTextScreen (FALSE);

    ScanningLoaders = FALSE;
} // VOID ScanForBootloaders()

// Checks to see if a specified file seems to be a valid tool.
// Returns TRUE if it passes all tests, FALSE otherwise
static
BOOLEAN IsValidTool (
    REFIT_VOLUME *BaseVolume,
    CHAR16       *PathName
) {
    UINTN    i = 0;
    CHAR16  *DontVolName = NULL, *DontPathName = NULL, *DontFileName = NULL, *DontScanThis  = NULL;
    CHAR16  *TestVolName = NULL, *TestPathName = NULL, *TestFileName = NULL, *DontScanTools = NULL;
    BOOLEAN  retval = TRUE;

    if (!FileExists (BaseVolume->RootDir, PathName)) {
        // Early return ... File does not exist
        return FALSE;
    }

    if (gHiddenTools == NULL) {
        DontScanTools = ReadHiddenTags (L"HiddenTools");
        gHiddenTools  = (DontScanTools) ? StrDuplicate (DontScanTools) : StrDuplicate (L"NotSet");

        #if REFIT_DEBUG > 0
        ALT_LOG(1, LOG_BLANK_LINE_SEP, L"X");
        #endif
    }
    else if (!MyStriCmp (gHiddenTools, L"NotSet")) {
        DontScanTools = StrDuplicate (gHiddenTools);
    }

    if (!MyStriCmp (gHiddenTools, L"NotSet")) {
        MergeStrings (&DontScanTools, GlobalConfig.DontScanTools, L',');
    }

    #if REFIT_DEBUG > 0
    ALT_LOG(1, LOG_LINE_NORMAL,
        L"Check File is Valid:- '%s'",
        PathName
    );
    #endif

    if (!FileExists (BaseVolume->RootDir, PathName) ||
        !IsValidLoader (BaseVolume->RootDir, PathName)
    ) {
        retval = FALSE;
    }
    else {
        SplitPathName (PathName, &TestVolName, &TestPathName, &TestFileName);

        while (retval && (DontScanThis = FindCommaDelimited (DontScanTools, i++))) {
            SplitPathName (DontScanThis, &DontVolName, &DontPathName, &DontFileName);

            if (MyStriCmp (TestFileName, DontFileName) &&
                ((DontPathName == NULL) || (MyStriCmp (TestPathName, DontPathName))) &&
                ((DontVolName == NULL) || (VolumeMatchesDescription (BaseVolume, DontVolName)))
            ) {
                retval = FALSE;
            }

            MY_FREE_POOL(DontVolName);
            MY_FREE_POOL(DontScanThis);
            MY_FREE_POOL(DontPathName);
            MY_FREE_POOL(DontFileName);
        } // while
    }

    MY_FREE_POOL(TestVolName);
    MY_FREE_POOL(TestPathName);
    MY_FREE_POOL(TestFileName);
    MY_FREE_POOL(DontScanTools);

    return retval;
} // BOOLEAN IsValidTool()

// Locate a single tool from the specified Locations using one of the
// specified Names and add it to the menu.
static
BOOLEAN FindTool (
    CHAR16 *Locations,
    CHAR16 *Names,
    CHAR16 *Description,
    UINTN   Icon
) {
    UINTN    j = 0;
    UINTN    k = 0;
    UINTN    VolumeIndex;
    CHAR16  *DirName   = NULL;
    CHAR16  *FileName  = NULL;
    CHAR16  *PathName  = NULL;
    BOOLEAN  FoundTool = FALSE;

    #if REFIT_DEBUG > 0
    CHAR16 *ToolStr = NULL;
    #endif

    while ((DirName = FindCommaDelimited (Locations, j++)) != NULL) {
        k = 0;
        while ((FileName = FindCommaDelimited (Names, k++)) != NULL) {
            PathName  = StrDuplicate (DirName);
            MergeStrings (&PathName, FileName, MyStriCmp (PathName, L"\\") ? 0 : L'\\');
            for (VolumeIndex = 0; VolumeIndex < VolumesCount; VolumeIndex++) {
                if (Volumes[VolumeIndex]->RootDir != NULL &&
                    IsValidTool (Volumes[VolumeIndex], PathName)
                ) {
                    #if REFIT_DEBUG > 0
                    ALT_LOG(1, LOG_LINE_NORMAL,
                        L"Adding Tag for '%s' on '%s'",
                        FileName, Volumes[VolumeIndex]->VolName
                    );
                    #endif

                    AddToolEntry (
                        Volumes[VolumeIndex],
                        PathName, Description,
                        BuiltinIcon (Icon),
                        'S', FALSE
                    );

                    #if REFIT_DEBUG > 0
                    ToolStr = PoolPrint (
                        L"Added Tool:- '%-18s    :::    %s'",
                        Description, PathName
                    );

                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);

                    if (FoundTool) {
                        LOG_MSG("%s%s", OffsetNext, Spacer);
                    }
                    LOG_MSG("%s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                    #endif

                    FoundTool = TRUE;
                } // if Volumes[VolumeIndex]->RootDir
            } // for

            MY_FREE_POOL(PathName);
            MY_FREE_POOL(FileName);
        } // while Names

        MY_FREE_POOL(DirName);
    } // while Locations

    return FoundTool;
} // VOID FindTool()

// Add the second-row tags containing built-in and external tools
VOID ScanForTools (VOID) {
    UINTN             i, j, k;
    UINTN             VolumeIndex;
    VOID             *ItemBuffer = 0;
    CHAR16           *TmpStr     = NULL;
    CHAR16           *ToolName   = NULL;
    CHAR16           *FileName   = NULL;
    CHAR16           *VolumeTag  = NULL;
    CHAR16           *RecoverVol = NULL;
    CHAR16           *MokLocations;
    CHAR16           *Description;
    UINT64            osind;
    UINT32            CsrValue;
    BOOLEAN           FoundTool;
    BOOLEAN           OtherFind;

    EFI_STATUS                     Status;
    APPLE_APFS_VOLUME_ROLE VolumeRole = 0;

    REFIT_MENU_ENTRY *MenuEntryPreCleanNvram = NULL;
    REFIT_MENU_ENTRY *MenuEntryHiddenTags    = NULL;
    REFIT_MENU_ENTRY *MenuEntryBootorder     = NULL;
    REFIT_MENU_ENTRY *MenuEntryShutdown      = NULL;
    REFIT_MENU_ENTRY *MenuEntryReset         = NULL;
    REFIT_MENU_ENTRY *MenuEntryExit          = NULL;
    REFIT_MENU_ENTRY *MenuEntryAbout         = NULL;
    REFIT_MENU_ENTRY *MenuEntryInstall       = NULL;
    REFIT_MENU_ENTRY *MenuEntryFirmware      = NULL;
    REFIT_MENU_ENTRY *MenuEntryRotateCsr     = NULL;

    #if REFIT_DEBUG > 0
    UINTN   ToolTotal  = 0;
    CHAR16 *ToolStr    = NULL;
    CHAR16 *LogSection = L"S E E K   U E F I   T O O L S";

    ALT_LOG(1, LOG_BLANK_LINE_SEP, L"X");
    ALT_LOG(1, LOG_LINE_SEPARATOR, L"%s", LogSection);
    LOG_MSG("%s", LogSection);

    if (GlobalConfig.DirectBoot) {
        LogSection = L"Skipped Loading UEFI Tools ... 'DirectBoot' is Active";
        ALT_LOG(1, LOG_LINE_NORMAL, L"%s", LogSection);
        ALT_LOG(1, LOG_BLANK_LINE_SEP, L"X");
        LOG_MSG("\n");
        LOG_MSG("%s", LogSection);
        LOG_MSG("\n\n");
    }
    else {
        LogSection = L"Get UEFI Tool Types:";
        ALT_LOG(1, LOG_BLANK_LINE_SEP, L"X");
        ALT_LOG(1, LOG_LINE_NORMAL, L"%s", LogSection);
        LOG_MSG("\n");
        LOG_MSG("%s", LogSection);
    }
    #endif

    if (GlobalConfig.DirectBoot) {
        // Early Return
        return;
    }

    MokLocations = StrDuplicate (MOK_LOCATIONS);
    if (MokLocations != NULL) {
        MergeStrings (&MokLocations, SelfDirPath, L',');
    }

    for (i = 0; i < NUM_TOOLS; i++) {
        // Reset Vars
        MY_FREE_POOL(FileName);
        MY_FREE_POOL(VolumeTag);
        MY_FREE_POOL(ToolName);

        FoundTool = FALSE;

        switch (GlobalConfig.ShowTools[i]) {
            case TAG_ABOUT:            ToolName = StrDuplicate (L"About RefindPlus");            break;
            case TAG_BOOTORDER:        ToolName = StrDuplicate (L"Manage Firmware Boot Order");  break;
            case TAG_CSR_ROTATE:       ToolName = StrDuplicate (L"Toggle CSR");                  break;
            case TAG_EXIT:             ToolName = StrDuplicate (L"Exit RefindPlus");             break;
            case TAG_FIRMWARE:         ToolName = StrDuplicate (L"Firmware Reboot");             break;
            case TAG_FWUPDATE_TOOL:    ToolName = StrDuplicate (L"Firmware Update");             break;
            case TAG_GDISK:            ToolName = StrDuplicate (L"GDisk");                       break;
            case TAG_GPTSYNC:          ToolName = StrDuplicate (L"GPT Sync");                    break;
            case TAG_HIDDEN:           ToolName = StrDuplicate (L"Hidden Tags");                 break;
            case TAG_INFO_NVRAMCLEAN:  ToolName = StrDuplicate (L"Clean Nvram");                 break;
            case TAG_INSTALL:          ToolName = StrDuplicate (L"Install RefindPlus");          break;
            case TAG_MEMTEST:          ToolName = StrDuplicate (L"Memtest");                     break;
            case TAG_MOK_TOOL:         ToolName = StrDuplicate (L"MOK Protocol");                break;
            case TAG_NETBOOT:          ToolName = StrDuplicate (L"Net Boot");                    break;
            case TAG_REBOOT:           ToolName = StrDuplicate (L"System Restart");              break;
            case TAG_RECOVERY_APPLE:   ToolName = StrDuplicate (L"Recovery (Mac)");              break;
            case TAG_RECOVERY_WINDOWS: ToolName = StrDuplicate (L"Recovery (Win)");              break;
            case TAG_SHELL:            ToolName = StrDuplicate (L"UEFI Shell");                  break;
            case TAG_SHUTDOWN:         ToolName = StrDuplicate (L"System Shutdown");             break;
            default:                                                                          continue;
        } // switch

        #if REFIT_DEBUG > 0
        ToolTotal = ToolTotal + 1;
        LOG_MSG("%s  - Type %02d ... ", OffsetNext, ToolTotal);
        #endif

        switch (GlobalConfig.ShowTools[i]) {
            case TAG_INFO_NVRAMCLEAN:
                MenuEntryPreCleanNvram = AllocateZeroPool (sizeof (REFIT_MENU_ENTRY));
                if (MenuEntryPreCleanNvram) {
                    FoundTool = TRUE;

                    MenuEntryPreCleanNvram->Title          = StrDuplicate (L"Clean NVRAM");
                    MenuEntryPreCleanNvram->Tag            = TAG_INFO_NVRAMCLEAN;
                    MenuEntryPreCleanNvram->Row            = 1;
                    MenuEntryPreCleanNvram->ShortcutDigit  = 0;
                    MenuEntryPreCleanNvram->ShortcutLetter = 0;
                    MenuEntryPreCleanNvram->Image          = BuiltinIcon (BUILTIN_ICON_TOOL_NVRAMCLEAN);

                    AddMenuEntry (MainMenu, MenuEntryPreCleanNvram);

                    #if REFIT_DEBUG > 0
                    ToolStr = PoolPrint (L"Added Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("%s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                    #endif
                }

                #if REFIT_DEBUG > 0
                if (!FoundTool) {
                    ToolStr = PoolPrint (L"Could Not Load Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("*_ WARN _*    %s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                }
                #endif

            break;
            case TAG_SHUTDOWN:
                MenuEntryShutdown = AllocateZeroPool (sizeof (REFIT_MENU_ENTRY));
                if (MenuEntryShutdown) {
                    FoundTool = TRUE;

                    MenuEntryShutdown->Title          = StrDuplicate (L"System Shutdown");
                    MenuEntryShutdown->Tag            = TAG_SHUTDOWN;
                    MenuEntryShutdown->Row            =  1;
                    MenuEntryShutdown->ShortcutDigit  =  0;
                    MenuEntryShutdown->ShortcutLetter = 'U';
                    MenuEntryShutdown->Image          = BuiltinIcon (BUILTIN_ICON_FUNC_SHUTDOWN);

                    AddMenuEntry (MainMenu, MenuEntryShutdown);

                    #if REFIT_DEBUG > 0
                    ToolStr = PoolPrint (L"Added Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("%s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                    #endif
                }

                #if REFIT_DEBUG > 0
                if (!FoundTool) {
                    ToolStr = PoolPrint (L"Could Not Load Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("*_ WARN _*    %s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                }
                #endif

            break;
            case TAG_REBOOT:
                MenuEntryReset = AllocateZeroPool (sizeof (REFIT_MENU_ENTRY));
                if (MenuEntryReset) {
                    FoundTool = TRUE;

                    MenuEntryReset->Title          = StrDuplicate (L"System Restart");
                    MenuEntryReset->Tag            = TAG_REBOOT;
                    MenuEntryReset->Row            =  1;
                    MenuEntryReset->ShortcutDigit  =  0;
                    MenuEntryReset->ShortcutLetter = 'R';
                    MenuEntryReset->Image          = BuiltinIcon (BUILTIN_ICON_FUNC_RESET);

                    AddMenuEntry (MainMenu, MenuEntryReset);

                    #if REFIT_DEBUG > 0
                    ToolStr = PoolPrint (L"Added Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("%s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                    #endif
                }

                #if REFIT_DEBUG > 0
                if (!FoundTool) {
                    ToolStr = PoolPrint (L"Could Not Load Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("*_ WARN _*    %s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                }
                #endif

            break;
            case TAG_ABOUT:
                MenuEntryAbout = AllocateZeroPool (sizeof (REFIT_MENU_ENTRY));
                if (MenuEntryAbout) {
                    FoundTool = TRUE;

                    MenuEntryAbout->Title          = StrDuplicate (L"About RefindPlus");
                    MenuEntryAbout->Tag            = TAG_ABOUT;
                    MenuEntryAbout->Row            =  1;
                    MenuEntryAbout->ShortcutDigit  =  0;
                    MenuEntryAbout->ShortcutLetter = 'A';
                    MenuEntryAbout->Image          = BuiltinIcon (BUILTIN_ICON_FUNC_ABOUT);

                    AddMenuEntry (MainMenu, MenuEntryAbout);

                    #if REFIT_DEBUG > 0
                    ToolStr = PoolPrint (L"Added Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("%s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                    #endif
                }

                #if REFIT_DEBUG > 0
                if (!FoundTool) {
                    ToolStr = PoolPrint (L"Could Not Load Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("*_ WARN _*    %s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                }
                #endif

            break;
            case TAG_EXIT:
                MenuEntryExit = AllocateZeroPool (sizeof (REFIT_MENU_ENTRY));
                if (MenuEntryExit) {
                    FoundTool = TRUE;

                    MenuEntryExit->Title          = StrDuplicate (L"Exit RefindPlus");
                    MenuEntryExit->Tag            = TAG_EXIT;
                    MenuEntryExit->Row            = 1;
                    MenuEntryExit->ShortcutDigit  = 0;
                    MenuEntryExit->ShortcutLetter = 0;
                    MenuEntryExit->Image          = BuiltinIcon (BUILTIN_ICON_FUNC_EXIT);

                    AddMenuEntry (MainMenu, MenuEntryExit);

                    #if REFIT_DEBUG > 0
                    ToolStr = PoolPrint (L"Added Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("%s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                    #endif
                }

                #if REFIT_DEBUG > 0
                if (!FoundTool) {
                    ToolStr = PoolPrint (L"Could Not Load Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("*_ WARN _*    %s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                }
                #endif

            break;
            case TAG_HIDDEN:
                if (GlobalConfig.HiddenTags) {
                    MenuEntryHiddenTags = AllocateZeroPool (sizeof (REFIT_MENU_ENTRY));
                    if (MenuEntryHiddenTags) {
                        MenuEntryHiddenTags->Title          = StrDuplicate (L"Restore Hidden Tags");
                        MenuEntryHiddenTags->Tag            = TAG_HIDDEN;
                        MenuEntryHiddenTags->Row            = 1;
                        MenuEntryHiddenTags->ShortcutDigit  = 0;
                        MenuEntryHiddenTags->ShortcutLetter = 0;
                        MenuEntryHiddenTags->Image          = BuiltinIcon (BUILTIN_ICON_FUNC_HIDDEN);

                        AddMenuEntry (MainMenu, MenuEntryHiddenTags);

                        #if REFIT_DEBUG > 0
                        ToolStr = PoolPrint (L"Added Tool:- '%s'", ToolName);
                        ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                        LOG_MSG("%s", ToolStr);
                        MY_FREE_POOL(ToolStr);
                        #endif
                    }
                    else {
                        #if REFIT_DEBUG > 0
                        if (!FoundTool) {
                            ToolStr = PoolPrint (L"Could Not Load Tool:- '%s'", ToolName);
                            ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                            LOG_MSG("*_ WARN _*    %s", ToolStr);
                            MY_FREE_POOL(ToolStr);
                        }
                        #endif
                    }
                }
                else {
                    #if REFIT_DEBUG > 0
                    ToolStr = PoolPrint (L"Did Not Enable Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG(" * NOTE *     %s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                    #endif
                }

            break;
            case TAG_FIRMWARE:
                if (EfivarGetRaw (
                    &GlobalGuid, L"OsIndicationsSupported",
                    &ItemBuffer, NULL
                ) != EFI_SUCCESS) {
                    #if REFIT_DEBUG > 0
                    ALT_LOG(1, LOG_LINE_NORMAL,
                        L"'Showtools' Includes Firmware Tool but 'OsIndicationsSupported' Variable is Missing!!"
                    );
                    #endif
                }
                else {
                    osind = *(UINT64 *) ItemBuffer;
                    if (osind & EFI_OS_INDICATIONS_BOOT_TO_FW_UI) {
                        MenuEntryFirmware = AllocateZeroPool (sizeof (REFIT_MENU_ENTRY));
                        if (MenuEntryFirmware) {
                            FoundTool = TRUE;

                            MenuEntryFirmware->Title          = StrDuplicate (L"Reboot into Firmware");
                            MenuEntryFirmware->Tag            = TAG_FIRMWARE;
                            MenuEntryFirmware->Row            = 1;
                            MenuEntryFirmware->ShortcutDigit  = 0;
                            MenuEntryFirmware->ShortcutLetter = 0;
                            MenuEntryFirmware->Image          = BuiltinIcon (BUILTIN_ICON_FUNC_FIRMWARE);

                            AddMenuEntry (MainMenu, MenuEntryFirmware);

                            #if REFIT_DEBUG > 0
                            ToolStr = PoolPrint (L"Added Tool:- '%s'", ToolName);
                            ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                            LOG_MSG("%s", ToolStr);
                            MY_FREE_POOL(ToolStr);
                            #endif
                        }
                        else {
                            #if REFIT_DEBUG > 0
                            // Fake 'FoundTool'
                            FoundTool = TRUE;

                            ToolStr = PoolPrint (L"Could Not Load Tool:- '%s'", ToolName);
                            ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                            LOG_MSG("*_ WARN _*    %s", ToolStr);
                            MY_FREE_POOL(ToolStr);
                            #endif
                        }
                    }
                    MY_FREE_POOL(ItemBuffer);
                }

                #if REFIT_DEBUG > 0
                if (!FoundTool) {
                    ToolStr = PoolPrint (L"Could Not Find Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("*_ WARN _*    %s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                }
                #endif

            break;
            case TAG_SHELL:
                j = 0;
                OtherFind = FALSE;
                while ((FileName = FindCommaDelimited (SHELL_NAMES, j++)) != NULL) {
                    if (IsValidTool (SelfVolume, FileName)) {
                        #if REFIT_DEBUG > 0
                        ALT_LOG(1, LOG_LINE_NORMAL,
                            L"Adding Shell Tag:- '%s' on '%s'",
                            FileName,
                            SelfVolume->VolName
                        );
                        #endif

                        FoundTool = TRUE;
                        AddToolEntry (
                            SelfVolume, FileName,
                            L"UEFI Shell",
                            BuiltinIcon (BUILTIN_ICON_TOOL_SHELL),
                            'S', FALSE
                        );

                        #if REFIT_DEBUG > 0
                        ToolStr = PoolPrint (L"Added Tool:- '%-18s    :::    %s'", ToolName, FileName);
                        ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                        if (OtherFind) {
                            LOG_MSG("%s%s", OffsetNext, Spacer);
                        }
                        LOG_MSG("%s", ToolStr);
                        MY_FREE_POOL(ToolStr);
                        #endif

                        OtherFind = TRUE;
                    }

                    MY_FREE_POOL(FileName);
                } // while

                if (!FoundTool) {
                    #if REFIT_DEBUG > 0
                    ToolStr = PoolPrint (L"Could Not Find Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("*_ WARN _*    %s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                    #endif
                }

                #if REFIT_DEBUG > 0
                ALT_LOG(1, LOG_LINE_NORMAL,
                    L"Scanning for Firmware Defined Shell Options"
                );
                #endif

                ScanFirmwareDefined (1, L"Shell", BuiltinIcon(BUILTIN_ICON_TOOL_SHELL));

                #if REFIT_DEBUG > 0
                ALT_LOG(1, LOG_BLANK_LINE_SEP, L"X");
                #endif

            break;
            case TAG_GPTSYNC:
                j = 0;
                while ((FileName = FindCommaDelimited (GPTSYNC_NAMES, j++)) != NULL) {
                    if (IsValidTool (SelfVolume, FileName)) {
                        #if REFIT_DEBUG > 0
                        ALT_LOG(1, LOG_LINE_NORMAL,
                            L"Adding Hybrid MBR Tag:- '%s' on '%s'",
                            FileName, SelfVolume->VolName
                        );
                        #endif

                        FoundTool = TRUE;
                        AddToolEntry (
                            SelfVolume, FileName,
                            L"Hybrid MBR tool",
                            BuiltinIcon (BUILTIN_ICON_TOOL_PART),
                            'P', FALSE
                        );

                        #if REFIT_DEBUG > 0
                        ToolStr = PoolPrint (L"Added Tool:- '%-18s    :::    %s'", ToolName, FileName);
                        ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                        if (j > 0) {
                            LOG_MSG("%s%s", OffsetNext, Spacer);
                        }
                        LOG_MSG("%s", ToolStr);
                        MY_FREE_POOL(ToolStr);
                        #endif
                    }

                    MY_FREE_POOL(FileName);
                } // while

                #if REFIT_DEBUG > 0
                if (!FoundTool) {
                    ToolStr = PoolPrint (L"Could Not Find Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("*_ WARN _*    %s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                }
                #endif

            break;
            case TAG_GDISK:
                j = 0;
                OtherFind = FALSE;
                while ((FileName = FindCommaDelimited (GDISK_NAMES, j++)) != NULL) {
                    if (IsValidTool (SelfVolume, FileName)) {
                        #if REFIT_DEBUG > 0
                        ALT_LOG(1, LOG_LINE_NORMAL,
                            L"Adding GDisk Tag:- '%s' on '%s'",
                            FileName, SelfVolume->VolName
                        );
                        #endif

                        FoundTool = TRUE;
                        AddToolEntry (
                            SelfVolume, FileName,
                            L"Partition Disks",
                            BuiltinIcon (BUILTIN_ICON_TOOL_PART),
                            'G', FALSE
                        );

                        #if REFIT_DEBUG > 0
                        ToolStr = PoolPrint (L"Added Tool:- '%-18s    :::    %s'", ToolName, FileName);
                        ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                        if (OtherFind) {
                            LOG_MSG("%s%s", OffsetNext, Spacer);
                        }
                        LOG_MSG("%s", ToolStr);
                        MY_FREE_POOL(ToolStr);
                        #endif

                        OtherFind = TRUE;
                    }

                    MY_FREE_POOL(FileName);
                } // while

                #if REFIT_DEBUG > 0
                if (!FoundTool) {
                    ToolStr = PoolPrint (L"Could Not Find Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("*_ WARN _*    %s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                }
                #endif

            break;
            case TAG_NETBOOT:
                j = 0;
                OtherFind = FALSE;
                while ((FileName = FindCommaDelimited (NETBOOT_NAMES, j++)) != NULL) {
                    if (IsValidTool (SelfVolume, FileName)) {
                        #if REFIT_DEBUG > 0
                        ALT_LOG(1, LOG_LINE_NORMAL,
                            L"Adding Netboot Tag:- '%s' on '%s'",
                            FileName, SelfVolume->VolName
                        );
                        #endif

                        FoundTool = TRUE;
                        AddToolEntry (
                            SelfVolume, FileName,
                            L"Netboot",
                            BuiltinIcon (BUILTIN_ICON_TOOL_NETBOOT),
                            'N', FALSE
                        );

                        #if REFIT_DEBUG > 0
                        ToolStr = PoolPrint (L"Added Tool:- '%-18s    :::    %s'", ToolName, FileName);
                        ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                        if (OtherFind) {
                            LOG_MSG("%s%s", OffsetNext, Spacer);
                        }
                        LOG_MSG("%s", ToolStr);
                        MY_FREE_POOL(ToolStr);
                        #endif

                        OtherFind = TRUE;
                    }

                    MY_FREE_POOL(FileName);
                } // while

                #if REFIT_DEBUG > 0
                if (!FoundTool) {
                    ToolStr = PoolPrint (L"Could Not Find Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("*_ WARN _*    %s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                }
                #endif

            break;
            case TAG_RECOVERY_APPLE:
                OtherFind = FALSE;
                for (VolumeIndex = 0; VolumeIndex < HfsRecoveryCount; VolumeIndex++) {
                    j = 0;
                    while (
                        (
                            FileName = FindCommaDelimited (GlobalConfig.MacOSRecoveryFiles, j++)
                        ) != NULL
                    ) {
                        if ((HfsRecovery[VolumeIndex]->RootDir != NULL) &&
                            (IsValidTool (HfsRecovery[VolumeIndex], FileName))
                        ) {
                            // Get a meaningful tag for the recovery tool
                            // DA-TAG: Limit to TianoCore
                            #ifdef __MAKEWITH_TIANO
                            RecoverVol = RP_GetAppleDiskLabel (HfsRecovery[VolumeIndex]);
                            #endif

                            VolumeTag = (RecoverVol)
                                ? PoolPrint (L"HFS+ Recovery : %s", RecoverVol)
                                : StrDuplicate (L"HFS+ Recovery : Recovery HD");

                            #if REFIT_DEBUG > 0
                            ALT_LOG(1, LOG_LINE_NORMAL,
                                L"Adding Mac Recovery Tag:- '%s' for '%s'",
                                FileName, VolumeTag
                            );
                            #endif

                            FoundTool = TRUE;
                            Description = PoolPrint (
                                L"%s for %s",
                                ToolName, VolumeTag
                            );
                            AddToolEntry (
                                HfsRecovery[VolumeIndex],
                                FileName, Description,
                                BuiltinIcon (BUILTIN_ICON_TOOL_APPLE_RESCUE),
                                'R', TRUE
                            );
                            MY_FREE_POOL(Description);

                            #if REFIT_DEBUG > 0
                            ToolStr = PoolPrint (
                                L"Added Tool:- '%-18s    :::    %s for %s'",
                                ToolName, FileName, VolumeTag
                            );
                            ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                            if (OtherFind) {
                                LOG_MSG("%s%s", OffsetNext, Spacer);
                            }
                            LOG_MSG("%s", ToolStr);
                            MY_FREE_POOL(ToolStr);
                            #endif

                            OtherFind = TRUE;
                        }

                        MY_FREE_POOL(FileName);
                        MY_FREE_POOL(VolumeTag);
                        MY_FREE_POOL(RecoverVol);
                    } // while
                } // for


                if (SingleAPFS) {
                    for (j = 0; j < RecoveryVolumesCount; j++) {
                        // Get a meaningful tag for the recovery tool
                        // DA-TAG: Limit to TianoCore
                        #ifdef __MAKEWITH_TIANO
                        BOOLEAN SkipSystemVolume;
                        for (k = 0; k < SystemVolumesCount; k++) {
                            SkipSystemVolume = FALSE;
                            for (VolumeIndex = 0; VolumeIndex < SkipApfsVolumesCount; VolumeIndex++) {
                                if (GuidsAreEqual (
                                    &(SkipApfsVolumes[VolumeIndex]->VolUuid),
                                    &(SystemVolumes[k]->VolUuid))
                                ) {
                                    SkipSystemVolume = TRUE;
                                    break;
                                }
                            } // for
                            if (SkipSystemVolume) {
                                continue;
                            }

                            if (
                                GuidsAreEqual (
                                    &(RecoveryVolumes[j]->PartGuid),
                                    &(SystemVolumes[k]->PartGuid)
                                )
                            ) {
                                Status = RP_GetApfsVolumeInfo (
                                    SystemVolumes[k]->DeviceHandle,
                                    NULL, NULL,
                                    &VolumeRole
                                );

                                if (!EFI_ERROR(Status)) {
                                    if (VolumeRole == APPLE_APFS_VOLUME_ROLE_SYSTEM ||
                                        VolumeRole == APPLE_APFS_VOLUME_ROLE_UNDEFINED
                                    ) {
                                        RecoverVol  = StrDuplicate (SystemVolumes[k]->VolName);
                                        TmpStr      = GuidAsString (&(SystemVolumes[k]->VolUuid));
                                        FileName    = PoolPrint (L"%s\\boot.efi", TmpStr);
                                        MY_FREE_POOL(TmpStr);

                                        break;
                                    }
                                }
                            }
                        } // for k = 0
                        #endif

                        VolumeTag = (RecoverVol)
                            ? PoolPrint (L"APFS Instance : %s", RecoverVol)
                            : StrDuplicate (L"APFS Instance : Recovery");
                        #if REFIT_DEBUG > 0
                        ALT_LOG(1, LOG_LINE_NORMAL,
                            L"Adding Mac Recovery Tag:- '%s' for '%s'",
                            FileName, VolumeTag
                        );
                        #endif

                        FoundTool = TRUE;
                        Description = PoolPrint (
                            L"%s for %s",
                            ToolName, VolumeTag
                        );
                        AddToolEntry (
                            RecoveryVolumes[j],
                            FileName, Description,
                            BuiltinIcon (BUILTIN_ICON_TOOL_APPLE_RESCUE),
                            'R', FALSE
                        );
                        MY_FREE_POOL(Description);

                        #if REFIT_DEBUG > 0
                        ToolStr = PoolPrint (
                            L"Added Tool:- '%-18s    :::    %s for %s'",
                            ToolName, FileName, VolumeTag
                        );
                        ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                        if (OtherFind) {
                            LOG_MSG("%s%s", OffsetNext, Spacer);
                        }
                        LOG_MSG("%s", ToolStr);
                        MY_FREE_POOL(ToolStr);
                        #endif

                        OtherFind = TRUE;

                        MY_FREE_POOL(FileName);
                        MY_FREE_POOL(VolumeTag);
                        MY_FREE_POOL(RecoverVol);
                    } // for k = 0
                } // if SingleAPFS

                #if REFIT_DEBUG > 0
                if (!FoundTool) {
                    BOOLEAN FlagAPFS = (!SingleAPFS && SystemVolumesCount > 0);
                    ToolStr = PoolPrint (
                        L"%s:- '%s'",
                        (FlagAPFS) ? L"Did Not Enable Tool" : L"Could Not Find Tool",
                        ToolName
                    );
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG(
                        "*_ %s _*    %s",
                        (FlagAPFS) ? L"NOTE" : L"WARN",
                        ToolStr
                    );
                    MY_FREE_POOL(ToolStr);
                }
                #endif

            break;
            case TAG_RECOVERY_WINDOWS:
                j = 0;
                OtherFind = FALSE;
                while (
                    (FileName = FindCommaDelimited (GlobalConfig.WindowsRecoveryFiles, j++)) != NULL
                ) {
                    SplitVolumeAndFilename (&FileName, &VolumeTag);
                    for (VolumeIndex = 0; VolumeIndex < VolumesCount; VolumeIndex++) {
                        if ((Volumes[VolumeIndex]->RootDir != NULL) &&
                            (MyStrStr (FileName, L"\\BOOT\\BOOT")) &&
                            (IsValidTool (Volumes[VolumeIndex], FileName)) &&
                            ((VolumeTag == NULL) || MyStriCmp (VolumeTag, Volumes[VolumeIndex]->VolName))
                        ) {
                            #if REFIT_DEBUG > 0
                            ALT_LOG(1, LOG_LINE_NORMAL,
                                L"Adding Windows Recovery Tag:- '%s' on '%s'",
                                FileName, Volumes[VolumeIndex]->VolName
                            );
                            #endif

                            FoundTool = TRUE;
                            Description = PoolPrint (
                                L"%s on %s",
                                ToolName,
                                Volumes[VolumeIndex]->VolName
                            );
                            AddToolEntry (
                                Volumes[VolumeIndex],
                                FileName,
                                Description,
                                BuiltinIcon (BUILTIN_ICON_TOOL_WINDOWS_RESCUE),
                                'R', TRUE
                            );
                            MY_FREE_POOL(Description);

                            #if REFIT_DEBUG > 0
                            ToolStr = PoolPrint (L"Added Tool:- '%-18s    :::    %s'", ToolName, FileName);
                            ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                            if (OtherFind) {
                                LOG_MSG("%s%s", OffsetNext, Spacer);
                            }
                            LOG_MSG("%s", ToolStr);
                            MY_FREE_POOL(ToolStr);
                            #endif

                            OtherFind = TRUE;
                        } // if Volumes[VolumeIndex]->RootDir
                    } // for

                    MY_FREE_POOL(FileName);
                    MY_FREE_POOL(VolumeTag);
                } // while

                #if REFIT_DEBUG > 0
                if (!FoundTool) {
                    ToolStr = PoolPrint (L"Could Not Find Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("*_ WARN _*    %s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                }
                #endif

            break;
            case TAG_MOK_TOOL:
                FoundTool = FindTool (
                    MokLocations,
                    MOK_NAMES,
                    L"MOK Utility",
                    BUILTIN_ICON_TOOL_MOK_TOOL
                );

                #if REFIT_DEBUG > 0
                if (!FoundTool) {
                    ToolStr = PoolPrint (L"Could Not Find Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("*_ WARN _*    %s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                }
                #endif

            break;
            case TAG_FWUPDATE_TOOL:
                FoundTool = FindTool (
                    MokLocations,
                    FWUPDATE_NAMES,
                    L"Firmware Update",
                    BUILTIN_ICON_TOOL_FWUPDATE
                );

                #if REFIT_DEBUG > 0
                if (!FoundTool) {
                    ToolStr = PoolPrint (L"Could Not Find Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("*_ WARN _*    %s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                }
                #endif

            break;
            case TAG_CSR_ROTATE:
                if ((GetCsrStatus (&CsrValue) == EFI_SUCCESS) && (GlobalConfig.CsrValues)) {
                    MenuEntryRotateCsr = AllocateZeroPool (sizeof (REFIT_MENU_ENTRY));
                    if (MenuEntryRotateCsr) {
                        FoundTool = TRUE;

                        MenuEntryRotateCsr->Title          = StrDuplicate (L"Toggle SIP Policy");
                        MenuEntryRotateCsr->Tag            = TAG_CSR_ROTATE;
                        MenuEntryRotateCsr->Row            = 1;
                        MenuEntryRotateCsr->ShortcutDigit  = 0;
                        MenuEntryRotateCsr->ShortcutLetter = 0;
                        MenuEntryRotateCsr->Image          = BuiltinIcon (BUILTIN_ICON_FUNC_CSR_ROTATE);

                        AddMenuEntry (MainMenu, MenuEntryRotateCsr);

                        #if REFIT_DEBUG > 0
                        ToolStr = PoolPrint (L"Added Tool:- '%s'", ToolName);
                        ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                        LOG_MSG("%s", ToolStr);
                        MY_FREE_POOL(ToolStr);
                        #endif
                    }
                }

                #if REFIT_DEBUG > 0
                if (!FoundTool) {
                    ToolStr = PoolPrint (
                        L"%s:- '%s'",
                        (!GlobalConfig.CsrValues) ? L"Did Not Enable Tool" : L"Aborted Tool Search",
                        ToolName
                    );
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG(
                        "*_ %s _*    %s",
                        (!GlobalConfig.CsrValues) ? L"NOTE" : L"WARN",
                        ToolStr
                    );
                    MY_FREE_POOL(ToolStr);
                }
                #endif

            break;
            case TAG_INSTALL:
                MenuEntryInstall = AllocateZeroPool (sizeof (REFIT_MENU_ENTRY));
                if (MenuEntryInstall) {
                    FoundTool = TRUE;

                    MenuEntryInstall->Title          = StrDuplicate (L"Install RefindPlus");
                    MenuEntryInstall->Tag            = TAG_INSTALL;
                    MenuEntryInstall->Row            = 1;
                    MenuEntryInstall->ShortcutDigit  = 0;
                    MenuEntryInstall->ShortcutLetter = 0;
                    MenuEntryInstall->Image          = BuiltinIcon (BUILTIN_ICON_FUNC_INSTALL);

                    AddMenuEntry (MainMenu, MenuEntryInstall);

                    #if REFIT_DEBUG > 0
                    ToolStr = PoolPrint (L"Added Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("%s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                    #endif
                }

                #if REFIT_DEBUG > 0
                if (!FoundTool) {
                    ToolStr = PoolPrint (L"Could Not Load Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("*_ WARN _*    %s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                }
                #endif

            break;
            case TAG_BOOTORDER:
                MenuEntryBootorder = AllocateZeroPool (sizeof (REFIT_MENU_ENTRY));
                if (MenuEntryBootorder) {
                    FoundTool = TRUE;

                    MenuEntryBootorder->Title          = StrDuplicate (L"Manage Firmware Boot Order");
                    MenuEntryBootorder->Tag            = TAG_BOOTORDER;
                    MenuEntryBootorder->Row            = 1;
                    MenuEntryBootorder->ShortcutDigit  = 0;
                    MenuEntryBootorder->ShortcutLetter = 0;
                    MenuEntryBootorder->Image          = BuiltinIcon (BUILTIN_ICON_FUNC_BOOTORDER);

                    AddMenuEntry (MainMenu, MenuEntryBootorder);

                    #if REFIT_DEBUG > 0
                    ToolStr = PoolPrint (L"Added Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("%s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                    #endif
                }

                #if REFIT_DEBUG > 0
                if (!FoundTool) {
                    ToolStr = PoolPrint (L"Could Not Load Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("*_ WARN _*    %s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                }
                #endif

            break;
            case TAG_MEMTEST:
                FoundTool = FindTool (
                    MEMTEST_LOCATIONS,
                    MEMTEST_NAMES,
                    L"Memory Test",
                    BUILTIN_ICON_TOOL_MEMTEST
                );

                #if REFIT_DEBUG > 0
                if (!FoundTool) {
                    ToolStr = PoolPrint (L"Could Not Find Tool:- '%s'", ToolName);
                    ALT_LOG(1, LOG_THREE_STAR_END, L"%s", ToolStr);
                    LOG_MSG("*_ WARN _*    %s", ToolStr);
                    MY_FREE_POOL(ToolStr);
                }
                #endif

                break;
        } // switch
    } // for

    #if REFIT_DEBUG > 0
    ToolStr = PoolPrint (L"Processed %d Tool Types", ToolTotal);
    ALT_LOG(1, LOG_THREE_STAR_SEP, L"%s", ToolStr);
    ALT_LOG(1, LOG_BLANK_LINE_SEP, L"%s", ToolStr);
    LOG_MSG("\n\n");
    LOG_MSG("INFO: %s", ToolStr);
    LOG_MSG("\n\n");
    #endif
} // VOID ScanForTools
