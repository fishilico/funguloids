; "Those Funny Funguloids!" install script
;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;General

  ;Name and file
  !define APPNAME "Those Funny Funguloids! v1.01"
  Name "${APPNAME}"
  OutFile "funguloids-win32-1.01.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\Those Funny Funguloids"

  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Those Funny Funguloids" ""


  ;Pack the exe header with UPX
  !packhdr exeheader.dat "upx --best exeheader.dat"

;--------------------------------
;Interface Configuration

  !define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\box-install.ico"
  !define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\box-uninstall.ico"
  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "installer\installer_kuva.bmp"
  !define MUI_WELCOMEFINISHPAGE_BITMAP "installer\installer_wizard.bmp"
  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "installer\license.txt"
  !insertmacro MUI_PAGE_DIRECTORY

  Var STARTMENU_FOLDER
  !define MUI_STARTMENUPAGE_DEFAULTFOLDER "Those Funny Funguloids"
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU"
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Those Funny Funguloids"
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  !insertmacro MUI_PAGE_STARTMENU "Application" $STARTMENU_FOLDER

  !define MUI_FINISHPAGE_NOAUTOCLOSE
  !insertmacro MUI_PAGE_INSTFILES

  !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\readme.html"
  !define MUI_FINISHPAGE_LINK "Visit the 'Those Funny Funguloids!' website"
  !define MUI_FINISHPAGE_LINK_LOCATION "http://funguloids.sourceforge.net"
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "Main data" SecMain

  SetOutPath "$INSTDIR"

  File Funguloids.exe

  File cg.dll
  File DevIL.dll
  File fmodex.dll
  File ILU.dll
  File ILUT.dll
  File msvc*.dll
  File OgreMain.dll
  File OgrePlatform.dll
  File zlib1.dll

  File gamesettings.cfg
  File plugins.cfg
  File resources.cfg

  File readme.html
  File *.mpk

  SetOutPath "$INSTDIR\plugins"
  File plugins\*.dll

  SetOutPath "$INSTDIR\music"
  File music\*.lua

  SetOutPath "$INSTDIR\docs"
  File docs\*.*


  ;Create shortcuts
  SetOutPath "$INSTDIR"
  !insertmacro MUI_STARTMENU_WRITE_BEGIN "Application"
  CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
  CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Those Funny Funguloids!.lnk" "$INSTDIR\Funguloids.exe"
  CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Readme.lnk" "$INSTDIR\readme.html"
  CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  CreateShortCut "$DESKTOP\Those Funny Funguloids!.lnk" "$INSTDIR\Funguloids.exe"
  !insertmacro MUI_STARTMENU_WRITE_END

  ;Store installation folder
  WriteRegStr HKCU "Software\Those Funny Funguloids" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Those Funny Funguloids" "DisplayName" "${APPNAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Those Funny Funguloids" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Those Funny Funguloids" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Those Funny Funguloids" "NoRepair" 1


SectionEnd

;--------------------------------
;Uninstaller Section

Var start_folder


Section "Uninstall"

  Delete "$INSTDIR\*.exe"
  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\*.mpk"
  Delete "$INSTDIR\*.html"
  Delete "$INSTDIR\*.lst"
  Delete "$INSTDIR\*.cfg"
  Delete "$INSTDIR\*.log"
  Delete "$INSTDIR\docs\*.*"
  RMDir "$INSTDIR\docs"
  Delete "$INSTDIR\plugins\*.*"
  RMDir "$INSTDIR\plugins"
  Delete "$INSTDIR\music\playlist.lua"
  RMDir "$INSTDIR\music"
  Delete "$INSTDIR\media\*.*"
  RMDir "$INSTDIR\media"

  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"

  !insertmacro MUI_STARTMENU_GETFOLDER "Application" $start_folder
  Delete "$SMPROGRAMS\$start_folder\*.lnk"
  Delete "$DESKTOP\Those Funny Funguloids!.lnk"
  RMDir "$SMPROGRAMS\$start_folder"

  DeleteRegKey HKCU "Software\Those Funny Funguloids"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Those Funny Funguloids"

SectionEnd
