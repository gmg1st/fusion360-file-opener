' F3D File Opener - VBScript Launcher
' Acts as the F3D file association handler.
' Checks if Fusion 360 is running (via ready file), starts it if needed,
' then writes the file path to a request file that the F3DFileOpener Add-In monitors.

Option Explicit

Dim fso, WshShell, WshProcessEnv
Dim filePath, tempDir, readyFile, requestFile, logFile
Dim maxWait, waited, launcherPath, productionDir, latestDir, subDir

Set fso = CreateObject("Scripting.FileSystemObject")
Set WshShell = CreateObject("WScript.Shell")
Set WshProcessEnv = WshShell.Environment("Process")

' Get the F3D file path from command line arguments
If WScript.Arguments.Count = 0 Then
    WScript.Quit 1
End If
filePath = WScript.Arguments(0)

' Set up file paths for IPC
tempDir = WshProcessEnv("TEMP")
If tempDir = "" Then tempDir = WshProcessEnv("TMP")
If tempDir = "" Then tempDir = "C:\Temp"

readyFile = tempDir & "\fusion360_f3d_opener_ready.txt"
requestFile = tempDir & "\fusion360_f3d_opener_request.txt"
logFile = tempDir & "\fusion360_f3d_opener.log"

' Write a log entry
Sub WriteLog(msg)
    On Error Resume Next
    Dim f
    Set f = fso.OpenTextFile(logFile, 8, True) ' 8 = ForAppending
    f.WriteLine "[" & Now & "] [VBS] " & msg
    f.Close
    On Error GoTo 0
End Sub

WriteLog "=== F3D opener launched ==="
WriteLog "File path: " & filePath

' Check if Fusion 360 process is actually running (not just stale ready file)
Function IsFusionRunning()
    On Error Resume Next
    Dim wmiSvc, procQuery, procList
    Set wmiSvc = GetObject("winmgmts:\\.\root\cimv2")
    procQuery = "SELECT * FROM Win32_Process WHERE Name='Fusion360.exe'"
    Set procList = wmiSvc.ExecQuery(procQuery)
    If Err.Number = 0 Then
        IsFusionRunning = (procList.Count > 0)
    Else
        ' WMI failed, fall back to ready file check with freshness (30 seconds)
        If fso.FileExists(readyFile) Then
            Dim readyAge
            readyAge = DateDiff("s", fso.GetFile(readyFile).DateLastModified, Now)
            IsFusionRunning = (readyAge <= 30)
        Else
            IsFusionRunning = False
        End If
    End If
    On Error GoTo 0
End Function

' Check if Fusion 360 is already running
maxWait = 60  ' Maximum wait time in seconds
waited = 0

Dim fusionRunning
fusionRunning = IsFusionRunning()

If Not fusionRunning Then
    ' Delete stale ready file if it exists (Fusion crashed previously without cleanup)
    If fso.FileExists(readyFile) Then
        fso.DeleteFile readyFile
        WriteLog "Deleted stale ready file (Fusion process not running but ready file existed)"
    End If

    WriteLog "Fusion 360 not running, starting it..."

    ' Find the latest FusionLauncher.exe
    ' Try D drive (mklink real path) first
    productionDir = "D:\Fusion360\Installation\Local\Autodesk\webdeploy\production"
    If Not fso.FolderExists(productionDir) Then
        ' Fall back to C drive (symlink path)
        productionDir = "C:\Users\G11\AppData\Local\Autodesk\webdeploy\production"
    End If

    WriteLog "Looking for FusionLauncher.exe in: " & productionDir

    launcherPath = ""
    If fso.FolderExists(productionDir) Then
        Set latestDir = fso.GetFolder(productionDir)
        ' Find the subdirectory containing BOTH FusionLauncher.exe AND FusionLauncher.exe.ini
        ' The .ini file is required for the launcher to work correctly.
        Dim bestDate, currentDate
        bestDate = #1/1/2000#
        For Each subDir In latestDir.SubFolders
            If fso.FileExists(subDir.Path & "\FusionLauncher.exe") And fso.FileExists(subDir.Path & "\FusionLauncher.exe.ini") Then
                currentDate = subDir.DateLastModified
                If currentDate > bestDate Then
                    bestDate = currentDate
                    launcherPath = subDir.Path & "\FusionLauncher.exe"
                End If
            End If
        Next

        ' Fallback: if no directory has both files, try reading the desktop shortcut
        If launcherPath = "" Then
            Dim shellObj, sc, shortcutPath
            Set shellObj = CreateObject("WScript.Shell")
            shortcutPath = "C:\Users\G11\Desktop\Autodesk Fusion.lnk"
            If fso.FileExists(shortcutPath) Then
                Set sc = shellObj.CreateShortcut(shortcutPath)
                If fso.FileExists(sc.TargetPath) Then
                    launcherPath = sc.TargetPath
                    WriteLog "Using desktop shortcut target: " & launcherPath
                End If
            End If
        End If
    End If

    If launcherPath = "" Then
        WriteLog "ERROR: Cannot find FusionLauncher.exe"
        WScript.Quit 1
    End If

    WriteLog "Found launcher: " & launcherPath

    ' Start Fusion 360
    WshShell.Run Chr(34) & launcherPath & Chr(34), 1, False
    WriteLog "Fusion 360 started, waiting for add-in to be ready..."

    ' Wait for the ready file to appear
    Do While Not fso.FileExists(readyFile) And waited < maxWait
        WScript.Sleep 1000
        waited = waited + 1
    Loop

    If Not fso.FileExists(readyFile) Then
        WriteLog "WARNING: Timeout waiting for Fusion 360 to be ready (" & maxWait & "s)"
        WriteLog "Writing request file anyway - add-in will pick it up when ready"
    Else
        WriteLog "Fusion 360 is ready after " & waited & " seconds"
    End If
Else
    ' Fusion process is running - verify Add-In is alive via heartbeat (ready file freshness)
    If fso.FileExists(readyFile) Then
        Dim hbAge
        hbAge = DateDiff("s", fso.GetFile(readyFile).DateLastModified, Now)
        If hbAge > 30 Then
            WriteLog "WARNING: Fusion running but Add-In heartbeat is stale (" & hbAge & "s old), Add-In may have crashed"
        Else
            WriteLog "Fusion 360 already running (heartbeat: " & hbAge & "s ago)"
        End If
    Else
        WriteLog "Fusion 360 running but no ready file - writing request anyway"
    End If
End If

' Write the file path to the request file
' Use UTF-8 encoding via ADODB.Stream to handle Chinese characters
Dim stream
Set stream = CreateObject("ADODB.Stream")
stream.Type = 2 ' adTypeText
stream.Charset = "utf-8"
stream.Open
stream.WriteText filePath
stream.SaveToFile requestFile, 2 ' 2 = adSaveCreateOverWrite
stream.Close

WriteLog "Request file written: " & requestFile
WriteLog "=== Done ==="

WScript.Quit 0
