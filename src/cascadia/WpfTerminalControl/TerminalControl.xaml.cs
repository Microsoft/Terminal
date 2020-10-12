﻿// <copyright file="TerminalControl.xaml.cs" company="Microsoft Corporation">
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
// </copyright>

namespace Microsoft.Terminal.Wpf
{
    using System;
    using System.Threading;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Input;
    using System.Windows.Media;
    using Microsoft.VisualStudio.Shell;
    using Task = System.Threading.Tasks.Task;

    /// <summary>
    /// A basic terminal control. This control can receive and render standard VT100 sequences.
    /// </summary>
    public partial class TerminalControl : UserControl
    {
        private int accumulatedDelta = 0;

        /// <summary>
        /// Gets size of the terminal renderer.
        /// </summary>
        private Size TerminalRendererSize
        {
            get => this.termContainer.TerminalRendererSize;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="TerminalControl"/> class.
        /// </summary>
        public TerminalControl()
        {
            this.InitializeComponent();

            this.termContainer.TerminalScrolled += this.TermControl_TerminalScrolled;
            this.termContainer.UserScrolled += this.TermControl_UserScrolled;
            this.scrollbar.MouseWheel += this.Scrollbar_MouseWheel;

            this.GotFocus += this.TerminalControl_GotFocus;
        }

        /// <summary>
        /// Gets the current character rows available to the terminal.
        /// </summary>
        public int Rows => this.termContainer.Rows;

        /// <summary>
        /// Gets the current character columns available to the terminal.
        /// </summary>
        public int Columns => this.termContainer.Columns;

        /// <summary>
        /// Gets or sets a value indicating whether if the renderer should automatically resize to fill the control
        /// on user action.
        /// </summary>
        public bool AutoFill
        {
            get => this.termContainer.AutoFill;
            set => this.termContainer.AutoFill = value;
        }

        /// <summary>
        /// Sets the connection to a terminal backend.
        /// </summary>
        public ITerminalConnection Connection
        {
            set => this.termContainer.Connection = value;
        }

        /// <summary>
        /// Sets the theme for the terminal. This includes font family, size, color, as well as background and foreground colors.
        /// </summary>
        /// <param name="theme">The color theme to use in the terminal.</param>
        /// <param name="fontFamily">The font family to use in the terminal.</param>
        /// <param name="fontSize">The font size to use in the terminal.</param>
        public void SetTheme(TerminalTheme theme, string fontFamily, short fontSize)
        {
            PresentationSource source = PresentationSource.FromVisual(this);

            if (source == null)
            {
                return;
            }

            this.termContainer.SetTheme(theme, fontFamily, fontSize);

            // DefaultBackground uses Win32 COLORREF syntax which is BGR instead of RGB.
            byte b = Convert.ToByte((theme.DefaultBackground >> 16) & 0xff);
            byte g = Convert.ToByte((theme.DefaultBackground >> 8) & 0xff);
            byte r = Convert.ToByte(theme.DefaultBackground & 0xff);

            this.terminalGrid.Background = new SolidColorBrush(Color.FromRgb(r, g, b));
        }

        /// <summary>
        /// Gets the selected text in the terminal, clearing the selection. Otherwise returns an empty string.
        /// </summary>
        /// <returns>Selected text, empty string if no content is selected.</returns>
        public string GetSelectedText()
        {
            return this.termContainer.GetSelectedText();
        }

        /// <summary>
        /// Resizes the terminal to the specified rows and columns.
        /// </summary>
        /// <param name="rows">Number of rows to display.</param>
        /// <param name="columns">Number of columns to display.</param>
        /// <param name="cancellationToken">Cancellation token for this task.</param>
        /// <returns>A <see cref="Task"/> representing the asynchronous operation.</returns>
        public async Task ResizeAsync(uint rows, uint columns, CancellationToken cancellationToken)
        {
            this.termContainer.Resize(rows, columns);

            await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync(cancellationToken);
            this.terminalGrid.Margin = this.CalculateMargins();
        }

        /// <summary>
        /// Resizes the terminal to the specified dimensions.
        /// </summary>
        /// <param name="rendersize">Rendering size for the terminal in device independent units.</param>
        /// <returns>A tuple of (int, int) representing the number of rows and columns in the terminal.</returns>
        public (int rows, int columns) TriggerResize(Size rendersize)
        {
            var dpiScale = VisualTreeHelper.GetDpi(this);
            rendersize.Width *= dpiScale.DpiScaleX;
            rendersize.Height *= dpiScale.DpiScaleY;

            this.termContainer.Resize(rendersize);

            return (this.Rows, this.Columns);
        }

        /// <inheritdoc/>
        protected override void OnRenderSizeChanged(SizeChangedInfo sizeInfo)
        {
            var dpiScale = VisualTreeHelper.GetDpi(this);

            // termContainer requires scaled sizes.
            this.termContainer.TerminalControlSize = new Size()
            {
                Width = (sizeInfo.NewSize.Width - this.scrollbar.ActualWidth) * dpiScale.DpiScaleX,
                Height = (sizeInfo.NewSize.Height - this.scrollbar.ActualWidth) * dpiScale.DpiScaleY,
            };

            if (this.AutoFill == false)
            {
                // Renderer will not resize on control resize. We have to manually calculate the margin to fill in the space.
                this.terminalGrid.Margin = this.CalculateMargins(sizeInfo.NewSize);

                // Margins stop resize events, therefore we have to manually check if more space is available and raise
                //  a resize event if needed.
                this.termContainer.RaiseResizedIfDrawSpaceIncreased();
            }

            base.OnRenderSizeChanged(sizeInfo);
        }

        /// <summary>
        /// Calculates the margins that should surround the terminal renderer, if any.
        /// </summary>
        /// <param name="controlSize">New size of the control. Uses the control's current size if not provided.</param>
        /// <returns>The new terminal control margin thickness in device independent units.</returns>
        private Thickness CalculateMargins(Size controlSize = default)
        {
            var dpiScale = VisualTreeHelper.GetDpi(this);
            double width = 0, height = 0;

            if (controlSize == default)
            {
                controlSize = new Size()
                {
                    Width = this.terminalUserControl.ActualWidth,
                    Height = this.terminalUserControl.ActualHeight,
                };
            }

            // During initialization, the terminal renderer size will be 0 and the terminal renderer
            // draws on all available space. Therefore no margins are needed until resized.
            if (this.TerminalRendererSize.Width != 0)
            {
                width = controlSize.Width - (this.TerminalRendererSize.Width / dpiScale.DpiScaleX);
            }

            if (this.TerminalRendererSize.Height != 0)
            {
                height = controlSize.Height - (this.TerminalRendererSize.Height / dpiScale.DpiScaleX);
            }

            width -= this.scrollbar.ActualWidth;

            // Prevent negative margin size.
            width = width < 0 ? 0 : width;
            height = height < 0 ? 0 : height;

            return new Thickness(0, 0, width, height);
        }

        private void TerminalControl_GotFocus(object sender, RoutedEventArgs e)
        {
            e.Handled = true;
            this.termContainer.Focus();
        }

        private void Scrollbar_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            this.TermControl_UserScrolled(sender, e.Delta);
        }

        private void TermControl_UserScrolled(object sender, int delta)
        {
            var lineDelta = 120 / SystemParameters.WheelScrollLines;
            this.accumulatedDelta += delta;

            if (this.accumulatedDelta < lineDelta && this.accumulatedDelta > -lineDelta)
            {
                return;
            }

            this.Dispatcher.InvokeAsync(() =>
            {
                var lines = -this.accumulatedDelta / lineDelta;
                this.scrollbar.Value += lines;
                this.accumulatedDelta = 0;

                this.termContainer.UserScroll((int)this.scrollbar.Value);
            });
        }

        private void TermControl_TerminalScrolled(object sender, (int viewTop, int viewHeight, int bufferSize) e)
        {
            this.Dispatcher.InvokeAsync(() =>
            {
                this.scrollbar.Minimum = 0;
                this.scrollbar.Maximum = e.bufferSize - e.viewHeight;
                this.scrollbar.Value = e.viewTop;
                this.scrollbar.ViewportSize = e.viewHeight;
            });
        }

        private void Scrollbar_Scroll(object sender, System.Windows.Controls.Primitives.ScrollEventArgs e)
        {
            var viewTop = (int)e.NewValue;
            this.termContainer.UserScroll(viewTop);
        }
    }
}
