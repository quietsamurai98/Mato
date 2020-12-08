$A = Start-Process -FilePath .\compile.bat -Wait -passthru -NoNewWindow;$a.ExitCode
& "..\..\dragonruby.exe" | Out-Default
