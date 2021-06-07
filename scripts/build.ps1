<#
setup vc vars
#>
cmd.exe /c "call `"C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat`" && set > %temp%\vcvars.txt"

Get-Content "$env:temp\vcvars.txt" | Foreach-Object {
  if ($_ -match "^(.*?)=(.*)$") {
    Set-Content "env:\$($matches[1])" $matches[2]
  }
}
$env:CL = '/MP'

qmake -config release -r eNoseAnnotator.pro
nmake

cd "app/release/"
Get-ChildItem
Rename-Item -Path "app.exe" -NewName "eNoseAnnotator.exe"
