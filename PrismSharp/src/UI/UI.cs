using System.Threading;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Logging;

namespace PrismSharp.UI;

public static class UIManager
{
	public static void Initialize()
    {
		var thread = new Thread(() =>
		{
            var lifetime = new ClassicDesktopStyleApplicationLifetime
            {
                ShutdownMode = ShutdownMode.OnLastWindowClose,
            };

            AppBuilder.Configure<App>()
                .UsePlatformDetect()
                .WithInterFont()
                .LogToTrace()
                .SetupWithLifetime(lifetime);

            lifetime.Start();
        })
		{
			IsBackground = true
		};
		thread.SetApartmentState(ApartmentState.STA);
		thread.Start();
	}
}
