# Register C++ Native F3DThumbnailProvider COM component
# Run as Administrator

$newClsid = "{C7A3F5D2-8E4B-4F6A-9D1E-3B5C7F8A2E6D}"
$oldClsid = "{E8F4A2C1-7B3D-4E8F-9A6B-5C2D1E3F4A5B}"
$dllPath = "C:\Users\G11\AppData\Local\F3DThumbnailProvider\F3DThumbnailProviderNative.dll"
$shellExGuid = "{E357FCCD-A995-4576-B01F-234630154E96}"

Write-Output "=== Registering Native C++ Thumbnail Provider ==="
Write-Output "CLSID: $newClsid"
Write-Output "DLL: $dllPath"

# 1. Remove old .NET CLSID registration from HKLM
Remove-Item -Path "HKLM:\SOFTWARE\Classes\CLSID\$oldClsid" -Recurse -Force -ErrorAction SilentlyContinue
Write-Output "Removed old .NET CLSID from HKLM"

# 2. Remove old .NET CLSID registration from HKCU
Remove-Item -Path "HKCU:\SOFTWARE\Classes\CLSID\$oldClsid" -Recurse -Force -ErrorAction SilentlyContinue
Write-Output "Removed old .NET CLSID from HKCU"

# 3. Register new C++ CLSID in HKLM
$hklmBase = "HKLM:\SOFTWARE\Classes\CLSID\$newClsid"
New-Item -Path $hklmBase -Force | Out-Null
Set-ItemProperty -Path $hklmBase -Name "(default)" -Value "F3DThumbnailProvider" -Type String
Set-ItemProperty -Path $hklmBase -Name "DisableProcessIsolation" -Value 1 -Type DWord

# Key difference: InprocServer32 points directly to our DLL (not mscoree.dll!)
New-Item -Path "$hklmBase\InprocServer32" -Force | Out-Null
Set-ItemProperty -Path "$hklmBase\InprocServer32" -Name "(default)" -Value $dllPath -Type String
Set-ItemProperty -Path "$hklmBase\InprocServer32" -Name "ThreadingModel" -Value "Both" -Type String

New-Item -Path "$hklmBase\ProgId" -Force | Out-Null
Set-ItemProperty -Path "$hklmBase\ProgId" -Name "(default)" -Value "F3DThumbnailProvider" -Type String
Write-Output "Registered new CLSID in HKLM"

# 4. Also register in HKCU
$hkcuBase = "HKCU:\SOFTWARE\Classes\CLSID\$newClsid"
New-Item -Path $hkcuBase -Force | Out-Null
Set-ItemProperty -Path $hkcuBase -Name "(default)" -Value "F3DThumbnailProvider" -Type String
Set-ItemProperty -Path $hkcuBase -Name "DisableProcessIsolation" -Value 1 -Type DWord

New-Item -Path "$hkcuBase\InprocServer32" -Force | Out-Null
Set-ItemProperty -Path "$hkcuBase\InprocServer32" -Name "(default)" -Value $dllPath -Type String
Set-ItemProperty -Path "$hkcuBase\InprocServer32" -Name "ThreadingModel" -Value "Both" -Type String
Write-Output "Registered new CLSID in HKCU"

# 5. Update ShellEx entries to point to new CLSID
# .f3d extension
New-Item -Path "HKLM:\SOFTWARE\Classes\.f3d\ShellEx\$shellExGuid" -Force | Out-Null
Set-ItemProperty -Path "HKLM:\SOFTWARE\Classes\.f3d\ShellEx\$shellExGuid" -Name "(default)" -Value $newClsid -Type String

New-Item -Path "HKCU:\SOFTWARE\Classes\.f3d\ShellEx\$shellExGuid" -Force | Out-Null
Set-ItemProperty -Path "HKCU:\SOFTWARE\Classes\.f3d\ShellEx\$shellExGuid" -Name "(default)" -Value $newClsid -Type String

# Fusion360File.f3d ProgID
New-Item -Path "HKLM:\SOFTWARE\Classes\Fusion360File.f3d\ShellEx\$shellExGuid" -Force | Out-Null
Set-ItemProperty -Path "HKLM:\SOFTWARE\Classes\Fusion360File.f3d\ShellEx\$shellExGuid" -Name "(default)" -Value $newClsid -Type String

New-Item -Path "HKCU:\SOFTWARE\Classes\Fusion360File.f3d\ShellEx\$shellExGuid" -Force | Out-Null
Set-ItemProperty -Path "HKCU:\SOFTWARE\Classes\Fusion360File.f3d\ShellEx\$shellExGuid" -Name "(default)" -Value $newClsid -Type String
Write-Output "Updated ShellEx entries"

# 6. Clear thumbnail cache
$thumbCacheDir = "$env:LOCALAPPDATA\Microsoft\Windows\Explorer"
Get-ChildItem $thumbCacheDir -Filter "thumbcache_*.db" -ErrorAction SilentlyContinue | Remove-Item -Force -ErrorAction SilentlyContinue
Get-ChildItem $thumbCacheDir -Filter "iconcache_*.db" -ErrorAction SilentlyContinue | Remove-Item -Force -ErrorAction SilentlyContinue
Write-Output "Thumbnail cache cleared"

# 7. Restart Explorer
Stop-Process -Name explorer -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 2
Start-Process explorer.exe
Write-Output "Explorer restarted"

# 8. Verify registration
Write-Output "`n=== Verification ==="
$verifyHklm = Get-ItemProperty "HKLM:\SOFTWARE\Classes\CLSID\$newClsid\InprocServer32" -ErrorAction SilentlyContinue
if ($verifyHklm) {
    Write-Output "HKLM InprocServer32: $($verifyHklm.'(default)')"
    Write-Output "HKLM ThreadingModel: $($verifyHklm.ThreadingModel)"
} else {
    Write-Output "ERROR: HKLM registration failed!"
}

$verifyShellEx = Get-ItemProperty "HKLM:\SOFTWARE\Classes\.f3d\ShellEx\$shellExGuid" -ErrorAction SilentlyContinue
if ($verifyShellEx) {
    Write-Output "ShellEx .f3d: $($verifyShellEx.'(default)')"
}

Write-Output "`n=== Registration Complete ==="
