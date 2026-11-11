; QuickDesk POS — установщик для кассовых станций.
; Отличие от обычного клиента: рядом с exe кладётся pos.ini, из-за чего клиент
; стартует в режиме «только входящие» и берёт постоянный пароль с сервера.

#define MyAppName "QuickDesk POS"
#define MyAppPublisher "QuickCoder"
#define MyAppExeName "QuickDesk.exe"

#ifndef MyAppVersion
  #define MyAppVersion "1.0.0"
#endif
#ifndef MyPublishDir
  #define MyPublishDir "..\..\publish\Release"
#endif
#ifndef MyOutputDir
  #define MyOutputDir "..\..\publish"
#endif
#ifndef MyIconPath
  #define MyIconPath "..\..\QuickDesk\res\QuickDesk.ico"
#endif

[Setup]
AppId={{C4A1E7B2-5D3F-4A88-9E21-7B6C0D5A9F31}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputDir={#MyOutputDir}
OutputBaseFilename=QuickDesk-POS-setup
SetupIconFile={#MyIconPath}
UninstallDisplayIcon={app}\{#MyAppExeName}
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
VersionInfoVersion={#MyAppVersion}
CloseApplications=force
CloseApplicationsFilter=*.exe

[Languages]
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "autostart"; Description: "Запускать кассу при входе в Windows"; GroupDescription: "Автозапуск:"

[Files]
Source: "{#MyPublishDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[INI]
; Признак кассы: файл рядом с исполняемым файлом включает режим только входящих.
Filename: "{app}\pos.ini"; Section: "main"; Key: "server"; String: "wss://qd.lyashenko.pw"
Filename: "{app}\pos.ini"; Section: "main"; Key: "policy_url"; String: "https://qd.lyashenko.pw/v1/pos/policy"
Filename: "{app}\pos.ini"; Section: "main"; Key: "enroll_url"; String: "https://qd.lyashenko.pw/v1/pos/enroll"

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\Удалить {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"

[Registry]
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; \
  ValueName: "QuickDeskPOS"; ValueData: """{app}\{#MyAppExeName}"""; Flags: uninsdeletevalue; Tasks: autostart

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Запустить кассу сейчас"; Flags: nowait postinstall skipifsilent
