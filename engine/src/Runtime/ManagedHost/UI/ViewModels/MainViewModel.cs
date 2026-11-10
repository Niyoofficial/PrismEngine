using CommunityToolkit.Mvvm.ComponentModel;
using PrismManagedHost.UI.ViewModels;

namespace PrismManagedHost.UI.ViewModels;

public partial class MainViewModel : ViewModelBase
{
    [ObservableProperty]
    public partial string Greeting { get; set; } = "Welcome to Avalonia!";
}
