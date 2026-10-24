using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
using Avalonia.Rendering.Composition;
using SkiaSharp;
using System;
using System.Numerics;
using Avalonia.Media.Immutable;
using Avalonia.Platform;

namespace PrismSharp.UI.Controls;

public class SceneViewControl : Control
{
    public SceneViewControl()
    {
    }

    protected override void OnAttachedToVisualTree(VisualTreeAttachmentEventArgs e)
    {
        base.OnAttachedToVisualTree(e);

        LayoutUpdated += OnLayoutUpdated;

        /*var visual = ElementComposition.GetElementVisual(this);
        if (visual == null) return;

        var compositor = visual.Compositor;
        var interop = compositor.TryGetCompositionGpuInterop().GetAwaiter().GetResult();
        KnownPlatformGraphicsExternalSemaphoreHandleTypes
        //KnownPlatformGraphicsExternalImageHandleTypes
        interop.ImportImage()

        handler = new SceneViewVisualHandler();

        var customVisual = compositor.CreateCustomVisual(handler);
        customVisual.Size = new Vector2(600, 400);
        customVisual.Offset = new(0, 0, 0);

        ElementComposition.SetElementChildVisual(this, customVisual);
        customVisual.SendHandlerMessage(SceneViewVisualHandler.StartMessage);*/
    }

    public override void Render(DrawingContext context)
    {
        var pen = new Pen(Brushes.DarkOrange, 3);
        context.DrawRectangle(null, pen, new Rect(0, 0, 500, 500));
    }

    private void OnLayoutUpdated(object? sender, EventArgs e)
    {
        Console.WriteLine($"Bounds: {Bounds}, Width: {Bounds.Width}, Height: {Bounds.Height}, IsVisible: {IsVisible}");
    }

    private CompositionCustomVisualHandler handler;
}

internal class SceneViewVisualHandler : CompositionCustomVisualHandler
{
    public static readonly object StopMessage = new(), StartMessage = new();

    public override void OnRender(ImmediateDrawingContext drawingContext)
    {
        var pen = new ImmutablePen(Brushes.BlueViolet, 3);
        drawingContext.DrawRectangle(null, pen, new Rect(100, 100, 200, 200));
    }

    public override void OnMessage(object message)
    {
        if (message == StartMessage)
        {
            RegisterForNextAnimationFrameUpdate();
        }
    }
}
