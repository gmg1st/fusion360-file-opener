# Fix F3DThumbnailProvider COM registration
# Run as Administrator

$clsid = "{E8F4A2C1-7B3D-4E8F-9A6B-5C2D1E3F4A5B}"
$dllPath = "C:\Users\G11\AppData\Local\F3DThumbnailProvider\F3DThumbnailProvider.dll"

# 1. Fix HKCU registration - ensure full path to mscoree.dll
$hkcuBase = "HKCU:\SOFTWARE\Classes\CLSID\$clsid"
Set-ItemProperty -Path "$hkcuBase\InprocServer32" -Name "(default)" -Value "C:\Windows\System32\mscoree.dll" -Type String
Set-ItemProperty -Path "$hkcuBase\InprocServer32" -Name "ThreadingModel" -Value "Both" -Type String
Set-ItemProperty -Path "$hkcuBase\InprocServer32" -Name "Class" -Value "F3DThumbnailProvider.F3DThumbnailProviderClass" -Type String
Set-ItemProperty -Path "$hkcuBase\InprocServer32" -Name "Assembly" -Value "F3DThumbnailProvider, Version=0.0.0.0, Culture=neutral, PublicKeyToken=null" -Type String
Set-ItemProperty -Path "$hkcuBase\InprocServer32" -Name "RuntimeVersion" -Value "v4.0.30319" -Type String
Set-ItemProperty -Path "$hkcuBase\InprocServer32" -Name "CodeBase" -Value "file:///C:/Users/G11/AppData/Local/F3DThumbnailProvider/F3DThumbnailProvider.dll" -Type String
# Add DisableProcessIsolation to force in-proc loading
Set-ItemProperty -Path $hkcuBase -Name "DisableProcessIsolation" -Value 1 -Type DWord -ErrorAction SilentlyContinue

# 2. Register in HKLM (requires admin)
$hklmBase = "HKLM:\SOFTWARE\Classes\CLSID\$clsid"
New-Item -Path $hklmBase -Force -ErrorAction SilentlyContinue | Out-Null
Set-ItemProperty -Path $hklmBase -Name "(default)" -Value "F3DThumbnailProvider" -Type String
Set-ItemProperty -Path $hklmBase -Name "DisableProcessIsolation" -Value 1 -Type DWord

New-Item -Path "$hklmBase\InprocServer32" -Force -ErrorAction SilentlyContinue | Out-Null
Set-ItemProperty -Path "$hklmBase\InprocServer32" -Name "(default)" -Value "C:\Windows\System32\mscoree.dll" -Type String
Set-ItemProperty -Path "$hklmBase\InprocServer32" -Name "ThreadingModel" -Value "Both" -Type String
Set-ItemProperty -Path "$hklmBase\InprocServer32" -Name "Class" -Value "F3DThumbnailProvider.F3DThumbnailProviderClass" -Type String
Set-ItemProperty -Path "$hklmBase\InprocServer32" -Name "Assembly" -Value "F3DThumbnailProvider, Version=0.0.0.0, Culture=neutral, PublicKeyToken=null" -Type String
Set-ItemProperty -Path "$hklmBase\InprocServer32" -Name "RuntimeVersion" -Value "v4.0.30319" -Type String
Set-ItemProperty -Path "$hklmBase\InprocServer32" -Name "CodeBase" -Value "file:///C:/Users/G11/AppData/Local/F3DThumbnailProvider/F3DThumbnailProvider.dll" -Type String

New-Item -Path "$hklmBase\ProgId" -Force -ErrorAction SilentlyContinue | Out-Null
Set-ItemProperty -Path "$hklmBase\ProgId" -Name "(default)" -Value "F3DThumbnailProvider.F3DThumbnailProviderClass" -Type String

# 3. Ensure ShellEx entries point to our CLSID
$shellExGuid = "{E357FCCD-A995-4576-B01F-234630154E96}"

# .f3d
New-Item -Path "HKCU:\SOFTWARE\Classes\.f3d\ShellEx\$shellExGuid" -Force -ErrorAction SilentlyContinue | Out-Null
Set-ItemProperty -Path "HKCU:\SOFTWARE\Classes\.f3d\ShellEx\$shellExGuid" -Name "(default)" -Value $clsid -Type String

# Fusion360File.f3d ProgID
New-Item -Path "HKCU:\SOFTWARE\Classes\Fusion360File.f3d\ShellEx\$shellExGuid" -Force -ErrorAction SilentlyContinue | Out-Null
Set-ItemProperty -Path "HKCU:\SOFTWARE\Classes\Fusion360File.f3d\ShellEx\$shellExGuid" -Name "(default)" -Value $clsid -Type String

# Also in HKLM
New-Item -Path "HKLM:\SOFTWARE\Classes\.f3d\ShellEx\$shellExGuid" -Force -ErrorAction SilentlyContinue | Out-Null
Set-ItemProperty -Path "HKLM:\SOFTWARE\Classes\.f3d\ShellEx\$shellExGuid" -Name "(default)" -Value $clsid -Type String

Write-Output "Registration complete!"
Write-Output "HKLM CLSID registered: $(Test-Path $hklmBase)"
Write-Output "HKCU CLSID registered: $(Test-Path $hkcuBase)"

# 4. Clear thumbnail cache
$thumbCacheDir = "$env:LOCALAPPDATA\Microsoft\Windows\Explorer"
Get-ChildItem $thumbCacheDir -Filter "thumbcache_*.db" -ErrorAction SilentlyContinue | Remove-Item -Force -ErrorAction SilentlyContinue
Write-Output "Thumbnail cache cleared"

# 5. Restart Explorer
Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 2
Start-Process explorer.exe
Write-Output "Explorer restarted"
