using System;
using System.Diagnostics;

class F3DLauncher
{
    static int Main(string[] args)
    {
        if (args.Length == 0)
        {
            Console.Error.WriteLine("Usage: F3DLauncher.exe <file_path>");
            return 1;
        }

        string filePath = args[0];
        string vbsPath = @"d:\fusion360\open_f3d.vbs";

        try
        {
            ProcessStartInfo psi = new ProcessStartInfo();
            psi.FileName = "wscript.exe";
            psi.Arguments = "\"" + vbsPath + "\" \"" + filePath + "\"";
            psi.UseShellExecute = false;
            psi.CreateNoWindow = true;
            Process.Start(psi);
            return 0;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine("Error: " + ex.Message);
            return 2;
        }
    }
}
