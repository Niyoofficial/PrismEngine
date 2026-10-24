using CommunityToolkit.Mvvm.ComponentModel;
using PrismSharp.UI.ViewModels;

namespace PrismSharp.UI.ViewModels;

public partial class MainViewModel : ViewModelBase
{
    [ObservableProperty]
    public partial string Greeting { get; set; } = "Welcome to Avalonia!";
}
