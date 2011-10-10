; example1.nsi
;
; This script is perhaps one of the simplest NSIs you can make. All of the
; optional settings are left to their default settings. The installer simply 
; prompts the user asking them where to install, and drops a copy of example1.nsi
; there. 

!include MUI2.nsh


;--------------------------------

; The name of the installer
Name "StarBackup 0.2"

; The file to write
OutFile "StarBackup-0.2.exe"

; The default installation directory
InstallDir $PROGRAMFILES\StarBackup

SetCompressor /SOLID lzma

; Request application privileges for Windows Vista
RequestExecutionLevel admin


;--------------------------------

; Pages

;Page directory

;Page instfiles


!define MUI_COMPONENTSPAGE_NODESC

!define MUI_FINISHPAGE_RUN "$INSTDIR\StarBackup.exe"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY  
!insertmacro MUI_PAGE_COMPONENTS  
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES



; First is default
;LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"
;LoadLanguageFile "${NSISDIR}\Contrib\Language files\French.nlf"
;LoadLanguageFile "${NSISDIR}\Contrib\Language files\Dutch.nlf"
;LoadLanguageFile "${NSISDIR}\Contrib\Language files\German.nlf"
;LoadLanguageFile "${NSISDIR}\Contrib\Language files\Korean.nlf"
;LoadLanguageFile "${NSISDIR}\Contrib\Language files\Russian.nlf"
;LoadLanguageFile "${NSISDIR}\Contrib\Language files\Spanish.nlf"
;LoadLanguageFile "${NSISDIR}\Contrib\Language files\Swedish.nlf"
;LoadLanguageFile "${NSISDIR}\Contrib\Language files\TradChinese.nlf"
;LoadLanguageFile "${NSISDIR}\Contrib\Language files\SimpChinese.nlf"
;LoadLanguageFile "${NSISDIR}\Contrib\Language files\Slovak.nlf"

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English" ;first language is the default language
  !insertmacro MUI_LANGUAGE "French"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Spanish"
  !insertmacro MUI_LANGUAGE "SpanishInternational"
  !insertmacro MUI_LANGUAGE "SimpChinese"
  !insertmacro MUI_LANGUAGE "TradChinese"
  !insertmacro MUI_LANGUAGE "Japanese"
  !insertmacro MUI_LANGUAGE "Korean"
  !insertmacro MUI_LANGUAGE "Italian"
  !insertmacro MUI_LANGUAGE "Dutch"
  !insertmacro MUI_LANGUAGE "Danish"
  !insertmacro MUI_LANGUAGE "Swedish"
  !insertmacro MUI_LANGUAGE "Norwegian"
  !insertmacro MUI_LANGUAGE "NorwegianNynorsk"
  !insertmacro MUI_LANGUAGE "Finnish"
  !insertmacro MUI_LANGUAGE "Greek"
  !insertmacro MUI_LANGUAGE "Russian"
  !insertmacro MUI_LANGUAGE "Portuguese"
  !insertmacro MUI_LANGUAGE "PortugueseBR"
  !insertmacro MUI_LANGUAGE "Polish"
  !insertmacro MUI_LANGUAGE "Ukrainian"
  !insertmacro MUI_LANGUAGE "Czech"
  !insertmacro MUI_LANGUAGE "Slovak"
  !insertmacro MUI_LANGUAGE "Croatian"
  !insertmacro MUI_LANGUAGE "Bulgarian"
  !insertmacro MUI_LANGUAGE "Hungarian"
  !insertmacro MUI_LANGUAGE "Thai"
  !insertmacro MUI_LANGUAGE "Romanian"
  !insertmacro MUI_LANGUAGE "Latvian"
  !insertmacro MUI_LANGUAGE "Macedonian"
  !insertmacro MUI_LANGUAGE "Estonian"
  !insertmacro MUI_LANGUAGE "Turkish"
  !insertmacro MUI_LANGUAGE "Lithuanian"
  !insertmacro MUI_LANGUAGE "Slovenian"
  !insertmacro MUI_LANGUAGE "Serbian"
  !insertmacro MUI_LANGUAGE "SerbianLatin"
  !insertmacro MUI_LANGUAGE "Arabic"
  !insertmacro MUI_LANGUAGE "Farsi"
  !insertmacro MUI_LANGUAGE "Hebrew"
  !insertmacro MUI_LANGUAGE "Indonesian"
  !insertmacro MUI_LANGUAGE "Mongolian"
  !insertmacro MUI_LANGUAGE "Luxembourgish"
  !insertmacro MUI_LANGUAGE "Albanian"
  !insertmacro MUI_LANGUAGE "Breton"
  !insertmacro MUI_LANGUAGE "Belarusian"
  !insertmacro MUI_LANGUAGE "Icelandic"
  !insertmacro MUI_LANGUAGE "Malay"
  !insertmacro MUI_LANGUAGE "Bosnian"
  !insertmacro MUI_LANGUAGE "Kurdish"
  !insertmacro MUI_LANGUAGE "Irish"
  !insertmacro MUI_LANGUAGE "Uzbek"
  !insertmacro MUI_LANGUAGE "Galician"
  !insertmacro MUI_LANGUAGE "Afrikaans"
  !insertmacro MUI_LANGUAGE "Catalan"
  !insertmacro MUI_LANGUAGE "Esperanto"

;--------------------------------

; The stuff to install
Section "-StarBackup" !Required

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File dlls\QtGui4.dll
  File dlls\QtCore4.dll
  File dlls\QtXml4.dll
  File dlls\mingwm10.dll
  File dlls\libgcc_s_dw2-1.dll
  File ..\StarBackup-build-desktop\release\StarBackup.exe
  File StarBackup.ico
  File starbackup_*qm
  WriteUninstaller "uninstall.exe"

  CreateDirectory "$SMPROGRAMS\StarBackup"
  CreateShortCut "$SMPROGRAMS\StarBackup\StarBackup.lnk" "$INSTDIR\StarBackup.exe" "" "$INSTDIR\StarBackup.ico" 0 SW_SHOWNORMAL
  CreateShortCut "$SMPROGRAMS\StarBackup\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  
SectionEnd ; end the section

Section "Create Desktop ShortCut"
  CreateShortCut "$DESKTOP\StarBackup.lnk" "$INSTDIR\StarBackup.exe" "" "$INSTDIR\StarBackup.ico" 0 SW_SHOWNORMAL
SectionEnd ; end the section


Section "Launch StarBackup at Windows Startup"
  CreateShortCut "$SMSTARTUP\StarBackup.lnk" "$INSTDIR\StarBackup.exe" "-hide" "$INSTDIR\StarBackup.ico" 0 SW_SHOWMINIMIZED
SectionEnd ; end the section


;--------------------------------

; Uninstaller

Section "Uninstall"

  ; Remove files and uninstaller
  Delete $INSTDIR\*.*

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\StarBackup\*.*"
  Delete "$DESKTOP\StarBackup.lnk"

  ; Remove directories used
  RMDir "$SMPROGRAMS\StarBackup"
  RMDir "$INSTDIR"

SectionEnd

