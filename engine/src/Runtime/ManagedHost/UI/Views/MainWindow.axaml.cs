using System;
using Avalonia.Controls;
using Avalonia.Interactivity;

namespace PrismManagedHost.UI.Views;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
    }

    private void Button_OnClick(object sender, RoutedEventArgs e)
    {
        Console.WriteLine("Click!");
    }
}
