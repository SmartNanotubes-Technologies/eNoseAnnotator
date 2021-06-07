if (-not (Test-Path -LiteralPath C:/Qwt-6.1.5)) 
{
    $client = new-object System.Net.WebClient
    $client.DownloadFile('https://sourceforge.net/projects/qwt/files/qwt/6.1.5/qwt-6.1.5.zip',"qwt-6.1.5.zip")
    
    7z t qwt-6.1.5.zip
    7z x qwt-6.1.5.zip -oapp/lib/
    cd "app/lib/qwt-6.1.5/"
    
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
    
    <# 
    build app
    #>
    qmake qwt.pro
    nmake
    nmake install
    cd "../../.."
}

