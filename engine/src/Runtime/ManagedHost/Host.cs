using PrismManagedHost.UI;
using System;
using System.Runtime.InteropServices;

namespace PrismManagedHost;

[StructLayout(LayoutKind.Sequential)]
public unsafe struct ExportsTable
{
}

public static class Host
{
    private static ExportsTable s_exports;

    [UnmanagedCallersOnly]
    public static unsafe int Bootstrap(ExportsTable* exports)
    {
        s_exports = *exports;

        UIManager.Initialize();

        return 0;
    }
}
