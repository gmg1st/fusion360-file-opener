# Register C++ Native Thumbnail Provider in HKLM
# MUST run as Administrator

$newClsid = "{C7A3F5D2-8E4B-4F6A-9D1E-3B5C7F8A2E6D}"
$oldClsid = "{E8F4A2C1-7B3D-4E8F-9A6B-5C2D1E3F4A5B}"
$dllPath = "C:\Users\G11\AppData\Local\F3DThumbnailProvider\F3DThumbnailProviderNative.dll"
$shellExGuid = "{E357FCCD-A995-4576-B01F-234630154E96}"

# 1. Remove old .NET CLSID from HKLM
Remove-Item -Path "HKLM:\SOFTWARE\Classes\CLSID\$oldClsid" -Recurse -Force -ErrorAction SilentlyContinue
Write-Output "Removed old .NET CLSID from HKLM"

# 2. Register new C++ CLSID in HKLM
$hklmBase = "HKLM:\SOFTWARE\Classes\CLSID\$newClsid"
New-Item -Path $hklmBase -Force | Out-Null
Set-ItemProperty -Path $hklmBase -Name "(default)" -Value "F3DThumbnailProvider"
Set-ItemProperty -Path $hklmBase -Name "DisableProcessIsolation" -Value 1 -Type DWord

New-Item -Path "$hklmBase\InprocServer32" -Force | Out-Null
Set-ItemProperty -Path "$hklmBase\InprocServer32" -Name "(default)" -Value $dllPath
Set-ItemProperty -Path "$hklmBase\InprocServer32" -Name "ThreadingModel" -Value "Both"
Write-Output "Registered new CLSID in HKLM"

# 3. Update HKLM ShellEx to point to new CLSID
Set-ItemProperty -Path "HKLM:\SOFTWARE\Classes\.f3d\ShellEx\$shellExGuid" -Name "(default)" -Value $newClsid -Force
Set-ItemProperty -Path "HKLM:\SOFTWARE\Classes\Fusion360File.f3d\ShellEx\$shellExGuid" -Name "(default)" -Value $newClsid -Force -ErrorAction SilentlyContinue
Write-Output "Updated HKLM ShellEx"

# 4. Clear thumbnail cache
$thumbCacheDir = "$env:LOCALAPPDATA\Microsoft\Windows\Explorer"
Get-ChildItem $thumbCacheDir -Filter "thumbcache_*.db" -ErrorAction SilentlyContinue | Remove-Item -Force -ErrorAction SilentlyContinue
Write-Output "Cache cleared"

# 5. Restart Explorer
Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 2
Start-Process explorer.exe
Write-Output "Explorer restarted"

# Verify
Write-Output "`n=== Verification ==="
$v = Get-ItemProperty "HKLM:\SOFTWARE\Classes\CLSID\$newClsid\InprocServer32" -ErrorAction SilentlyContinue
Write-Output "HKLM InprocServer32: $($v.'(default)')"
$s = Get-ItemProperty "HKLM:\SOFTWARE\Classes\.f3d\ShellEx\$shellExGuid" -ErrorAction SilentlyContinue
Write-Output "HKLM ShellEx: $($s.'(default)')"
Write-Output "Done!"
