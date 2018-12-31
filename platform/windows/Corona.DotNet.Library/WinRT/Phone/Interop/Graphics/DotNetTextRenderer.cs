using System;


namespace CoronaLabs.Corona.WinRT.Phone.Interop.Graphics
{
	/// <summary>Renders text to a grayscaled bitmap using .NET classes.</summary>
	public class DotNetTextRenderer : WinRT.Interop.Graphics.ITextRenderer
	{
		#region Private Member Variables
		/// <summary>Provides font settings, wrap width, clipping width/height, text to be renderered, etc.</summary>
		private WinRT.Interop.Graphics.TextRendererSettings fSettings;

		#endregion


		#region Events
		/// <summary>
		///  <para>Raised when this renderer is about to render text to the referenced bitmap.</para>
		///  <para>
		///   The handler of this event can customize text rendering by drawing to the bitmap before this renderer does.
		///  </para>
		/// </summary>
		public event EventHandler<DotNetWriteableBitmapEventArgs> Rendering;

		#endregion


		#region Constructors/Destructors
		/// <summary>Creates a new renderer used to draw text to a bitmap.</summary>
		public DotNetTextRenderer()
		{
			fSettings = new WinRT.Interop.Graphics.TextRendererSettings();
		}

		#endregion


		#region Public Methods/Properties
		/// <summary>Gets the configuration this renderer will use to render text to a bitmap.</summary>
		/// <value>The settings this renderer will use to render text to a bitmap.</value>
		public WinRT.Interop.Graphics.TextRendererSettings Settings
		{
			get { return fSettings; }
		}

		/// <summary>
		///  Determines the size of the bitmap this text renderer will produce when calling the CreateBitmap() method.
		/// </summary>
		/// <returns>
		///  <para>
		///   Returns a result object providing the bitmap information calculated by this text renderer if flagged successful.
		///  </para>
		///  <para>
		///   Returns a failure result object if there was an error, in which case the the result's Message property
		///   would provide details as to what went wrong.
		///  </para>
		/// </returns>
		public WinRT.Interop.Graphics.BitmapInfoResult MeasureBitmap()
		{
			bool isRequestingBitmapInfoOnly = true;
			var result = AcquireBitmapData(isRequestingBitmapInfoOnly);
			if (result.HasFailed)
			{
				return WinRT.Interop.Graphics.BitmapInfoResult.FailedWith(result.Message);
			}
			return WinRT.Interop.Graphics.BitmapInfoResult.SucceededWith(result.BitmapInfo);
		}

		/// <summary>Creates a new bitmap with text drawn to it according to the settings applied to this renderer.</summary>
		/// <returns>
		///  <para>Returns a result object which will provide a new grayscaled bitmap if flagged as successful.</para>
		///  <para>
		///   Returns a failure result object if there was an error, in which case the the result's Message property
		///   would provide details as to what went wrong.
		///  </para>
		/// </returns>
		public WinRT.Interop.Graphics.BitmapResult CreateBitmap()
		{
			bool isRequestingBitmapInfoOnly = false;
			var result = AcquireBitmapData(isRequestingBitmapInfoOnly);
			if (result.HasFailed)
			{
				return WinRT.Interop.Graphics.BitmapResult.FailedWith(result.Message);
			}
			return WinRT.Interop.Graphics.BitmapResult.SucceededWith(result.Bitmap);
		}

		#endregion


		#region Private Methods
		/// <summary>
		///  Calculates the bitmap dimensions and creates a new bitmap (if requested) with text drawn to
		///  according to the settings applied to this text renderer.
		/// </summary>
		/// <param name="isRequestingBitmapInfoOnly">
		///  <para>Set to true to not produce a bitmap and measure what the bitmap size will be instead.</para>
		///  <para>Set to false to create a bitmap with text rendered to it.</para>
		/// </param>
		/// <returns>
		///  <para>
		///   Returns a result object with the requested bitmap data if flagged successful.
		///   If the "isRequestingBitmapInfoOnly" argument was set true, then the result object will only provide
		///   BitmapInfo and its Bitmap property will be null.
		///  </para>
		///  <para>
		///   Returns a failure result object if there was an error, in which case the the result's Message property
		///   would provide details as to what went wrong.
		///  </para>
		/// </returns>
		private TextBitmapDataResult AcquireBitmapData(bool isRequestingBitmapInfoOnly)
		{
			// Do not continue if there is no text to render.
			if (fSettings.Text.Length <= 0)
			{
				return new TextBitmapDataResult("No text to render.");
			}

			// Do not continue if not called on the main UI thread.
			// Note: Microsoft's WriteableBitmap.render() method used below can only be called on the main UI thread.
			if (System.Windows.Deployment.Current.Dispatcher.CheckAccess() == false)
			{
				return new TextBitmapDataResult("Text can only be rendered on the main UI thread.");
			}

			// Do not continue if Corona is currently synchronized with the rendering thread.
			// Note: Calling the WriteableBitmap.render() while the rendering thread is blocked will cause deadlock.
			if (Direct3DSurfaceAdapter.IsSynchronizedWithRenderingThread)
			{
				return new TextBitmapDataResult("Cannot render text while Corona is synchronized with the rendering thread.");
			}

			// Create a Xaml text control to be used to render text to a bitmap.
			var textBlock = new System.Windows.Controls.TextBlock();
			textBlock.Foreground = new System.Windows.Media.SolidColorBrush(System.Windows.Media.Colors.White);
			textBlock.VerticalAlignment = System.Windows.VerticalAlignment.Top;
			textBlock.Text = fSettings.Text;

			// Set up the font.
			if (string.IsNullOrEmpty(fSettings.FontSettings.FamilyName) == false)
			{
				if (string.IsNullOrEmpty(fSettings.FontSettings.FilePath) == false)
				{
					System.IO.FileStream stream = null;
					try
					{
						stream = System.IO.File.OpenRead(fSettings.FontSettings.FilePath);
						textBlock.FontSource = new System.Windows.Documents.FontSource(stream);
					}
					catch (Exception) { }
					finally
					{
						if (stream != null)
						{
							try
							{
								stream.Close();
								stream.Dispose();
							}
							catch (Exception) { }
						}
					}
				}
				textBlock.FontFamily = new System.Windows.Media.FontFamily(fSettings.FontSettings.FamilyName);
			}
			textBlock.FontSize = fSettings.FontSettings.PointSize;
			textBlock.FontWeight =
					fSettings.FontSettings.IsBold ? System.Windows.FontWeights.Bold : System.Windows.FontWeights.Normal;
			textBlock.FontStyle =
					fSettings.FontSettings.IsItalic ? System.Windows.FontStyles.Italic : System.Windows.FontStyles.Normal;

			// Set up the horizontal alignment of the text.
			if (fSettings.HorizontalAlignment == WinRT.Interop.Graphics.HorizontalAlignment.Center)
			{
				textBlock.TextAlignment = System.Windows.TextAlignment.Center;
			}
			else if (fSettings.HorizontalAlignment == WinRT.Interop.Graphics.HorizontalAlignment.Right)
			{
				textBlock.TextAlignment = System.Windows.TextAlignment.Right;
			}
			else
			{
				textBlock.TextAlignment = System.Windows.TextAlignment.Left;
			}

			// Set up multiline text wrapping if enabled.
			if (fSettings.BlockWidth > 0)
			{
				// Enable text wrapping at the given pixel width.
				textBlock.TextWrapping = System.Windows.TextWrapping.Wrap;
				textBlock.MaxWidth = (double)fSettings.BlockWidth;
				textBlock.Width = (double)fSettings.BlockWidth;
			}
			else
			{
				// Disable text wrapping.
				textBlock.TextWrapping = System.Windows.TextWrapping.NoWrap;
			}
			if (fSettings.BlockHeight > 0)
			{
				textBlock.MaxHeight = (double)fSettings.BlockHeight;
				textBlock.Height = (double)fSettings.BlockHeight;
			}

			// Calculate a pixel width and height for the bitmap to render text to.
			int bitmapWidth = fSettings.BlockWidth;
			if (bitmapWidth <= 0)
			{
				bitmapWidth = (int)(Math.Ceiling(textBlock.ActualWidth) + 0.1);
			}
			if ((fSettings.ClipWidth > 0) && (bitmapWidth > fSettings.ClipWidth))
			{
				bitmapWidth = fSettings.ClipWidth;
			}
			int bitmapHeight = fSettings.BlockHeight;
			if (bitmapHeight <= 0)
			{
				bitmapHeight = (int)(Math.Ceiling(textBlock.ActualHeight) + 0.1);
			}
			if ((fSettings.ClipHeight > 0) && (bitmapHeight > fSettings.ClipHeight))
			{
				bitmapHeight = fSettings.ClipHeight;
			}

			// Stop here if the caller is only requesting the resulting bitmap's measurements.
			if (isRequestingBitmapInfoOnly)
			{
				var bitmapSettings = new WinRT.Interop.Graphics.BitmapSettings();
				bitmapSettings.PixelFormat = WinRT.Interop.Graphics.PixelFormat.Grayscale;
				bitmapSettings.PremultipliedAlphaApplied = true;
				bitmapSettings.PixelWidth = bitmapWidth;
				bitmapSettings.PixelHeight = bitmapHeight;
				return new TextBitmapDataResult(new WinRT.Interop.Graphics.BitmapInfo(bitmapSettings));
			}

			// Determine if the text contains any visible/printable characters.
			bool hasPrintableCharacters = false;
			foreach (char nextCharacter in textBlock.Text.ToCharArray())
			{
				if ((System.Char.IsWhiteSpace(nextCharacter) == false) && (System.Char.IsControl(nextCharacter) == false))
				{
					hasPrintableCharacters = true;
					break;
				}
			}

			// If there is no text to render, then stop here and return an empty bitmap.
			// Note: This is a huge optimization. No point in rendering text below if you can't see the characters.
			if (hasPrintableCharacters == false)
			{
				var bitmapSettings = new WinRT.Interop.Graphics.BitmapSettings();
				bitmapSettings.PixelFormat = WinRT.Interop.Graphics.PixelFormat.Grayscale;
				bitmapSettings.PremultipliedAlphaApplied = true;
				bitmapSettings.PixelWidth = bitmapWidth;
				bitmapSettings.PixelHeight = bitmapHeight;
				var emptyGrayscaleBitmap = new WinRT.Interop.Graphics.Bitmap();
				emptyGrayscaleBitmap.FormatUsing(new WinRT.Interop.Graphics.BitmapInfo(bitmapSettings));
				return new TextBitmapDataResult(emptyGrayscaleBitmap);
			}

			// If the text has been clipped, then shift the text within the bounds of the bitmap.
			System.Windows.Media.TranslateTransform transform = null;
			if (textBlock.ActualWidth > (double)bitmapWidth)
			{
				transform = new System.Windows.Media.TranslateTransform();
				if (textBlock.TextAlignment == System.Windows.TextAlignment.Right)
				{
					transform.X = bitmapWidth - textBlock.ActualWidth;
				}
				else if (textBlock.TextAlignment == System.Windows.TextAlignment.Center)
				{
					transform.X = (bitmapWidth - textBlock.ActualWidth) / 2.0;
				}
			}

			// Render the text to a 32-bit color bitmap.
			System.Windows.Media.Imaging.WriteableBitmap writeableBitmap = null;
			try
			{
				// Create the bitmap.
				writeableBitmap = new System.Windows.Media.Imaging.WriteableBitmap(bitmapWidth, bitmapHeight);

				// Create a rectangle used to render a black background.
				var backgroundColor = System.Windows.Media.Colors.Black;
				var backgroundRectangle = new System.Windows.Shapes.Rectangle();
				backgroundRectangle.Width = bitmapWidth;
				backgroundRectangle.Height = bitmapHeight;
				backgroundRectangle.Fill = new System.Windows.Media.SolidColorBrush(backgroundColor);

				// Convert the background color object to a 32-bit integer.
				// To be compared with the bitmap's integer pixel array below.
				int backgroundColorAsInt32 =
						((int)backgroundColor.B) |
						((int)backgroundColor.G << 8) |
						((int)backgroundColor.R << 16) |
						((int)backgroundColor.A << 24);

				// Attempt to render text to the bitmap.
				// Note: There is a bug on Windows Phone where WriteableBitmap sometimes fails to render/inavlidate
				//       content if a Xaml DrawingSurfaceBackgroundGrid is being used by the application.
				//       If this happens, then we must attempt to render again. There is no other known work-around.
				bool wasDrawn = false;
				for (int renderAttempt = 1; renderAttempt <= 3; renderAttempt++)
				{
					// Notify the owner(s) that we're about to render to the bitmap.
					if (this.Rendering != null)
					{
						this.Rendering.Invoke(this, new DotNetWriteableBitmapEventArgs(writeableBitmap));
					}

					// Render the text and its black background to the bitmap.
					writeableBitmap.Render(backgroundRectangle, null);
					writeableBitmap.Render(textBlock, transform);
					writeableBitmap.Invalidate();

					// --- Verify that the above text was successfully drawn to the bitmap. ---

					// First, check that the black rectangle was drawn to the bitmap.
					// This is a fast check because we only need to read one pixel in the bitmap's top-left corner.
					if (writeableBitmap.Pixels[0] != backgroundColorAsInt32)
					{
						continue;
					}

					// Next, check that text was drawn to the bitmap.
					if (hasPrintableCharacters)
					{
						// Traverse all pixels in the bitmap until we find 1 pixel that does not match the background color.
						for (int pixelIndex = writeableBitmap.Pixels.Length - 1; pixelIndex >= 0; pixelIndex--)
						{
							if (writeableBitmap.Pixels[pixelIndex] != backgroundColorAsInt32)
							{
								wasDrawn = true;
								break;
							}
						}
					}
					else
					{
						// The given text does not contain any visible characters. So, we're done.
						wasDrawn = true;
					}

					// Stop now if we've successfully drawn to the bitmap.
					if (wasDrawn)
					{
						break;
					}
				}

				// Log a failure if we were unable to render text.
				// Note: This still returns a bitmap. Should we?
				if (wasDrawn == false)
				{
					String message = "Failed to create a bitmap for text: \"" + textBlock.Text + "\"\r\n";
					Corona.WinRT.Interop.Logging.LoggingServices.Log(message);
				}
			}
			catch (Exception ex)
			{
				return new TextBitmapDataResult(ex.Message);
			}

			// Convert the 32-bit color bitmap to 8-bit grayscale.
			var rgbaBitmap = DotNetBitmap.From(writeableBitmap);
			var bitmapConveter = new WinRT.Interop.Graphics.BitmapConverter();
			bitmapConveter.PixelFormat = WinRT.Interop.Graphics.PixelFormat.Grayscale;
			var grayscaleBitmap = bitmapConveter.CreateBitmapFrom(rgbaBitmap);
			rgbaBitmap.ReleaseByteBuffer();
			if (grayscaleBitmap == null)
			{
				return new TextBitmapDataResult("Failed to convert the 32-bit color text to a grayscale bitmap.");
			}

			// Return the text as a grayscaled bitmap.
			return new TextBitmapDataResult(grayscaleBitmap);
		}

		#endregion


		#region Private TextBitmapDataResult Class
		/// <summary>
		///  Private class providing the results generated by the DotnetTextRenderer.CreateBitmapData() method.
		/// </summary>
		private class TextBitmapDataResult
		{
			#region Private Member Variables
			/// <summary>Set true if the operation was a success. Set false if it failed.</summary>
			private bool fHasSucceeded;

			/// <summary>Provides details about the result of the operation. Typically used for error messages.</summary>
			private String fMessage;

			/// <summary>Provides information about the bitmap. Expected to be null if the operation failed.</summary>
			private WinRT.Interop.Graphics.BitmapInfo fBitmapInfo;

			/// <summary>The bitmap that the operation produced. Expected to be null if the operation failed.</summary>
			private WinRT.Interop.Graphics.IBitmap fBitmap;

			#endregion


			#region Constructors
			/// <summary>Creates a failure result object with the given message.</summary>
			/// <param name="message">The failure message. Can be null or empty.</param>
			public TextBitmapDataResult(String message)
			{
				fHasSucceeded = false;
				fMessage = (message != null) ? message : String.Empty;
			}

			/// <summary>Creates a result object with the given bitmap information.</summary>
			/// <param name="info">
			///  The bitmap information to be provided. Setting this to null will create a failure result object.
			/// </param>
			public TextBitmapDataResult(WinRT.Interop.Graphics.BitmapInfo info)
			{
				fHasSucceeded = (info != null);
				fMessage = String.Empty;
				fBitmapInfo = info;
				fBitmap = null;
			}

			/// <summary>Creates a result object providing the given bitmap.</summary>
			/// <param name="bitmap">
			///  The bitmap to be provided. Setting this to null will create a failure result object.
			/// </param>
			public TextBitmapDataResult(WinRT.Interop.Graphics.IBitmap bitmap)
			{
				fHasSucceeded = (bitmap != null);
				fMessage = String.Empty;
				fBitmapInfo = (bitmap != null) ? bitmap.Info : null;
				fBitmap = bitmap;
			}

			#endregion


			#region Public Properties
			/// <summary>Determines if the operation has succeeded.</summary>
			/// <value>
			///  <para>
			///   Set to true if the operation was a success. This means that BitmapInfo and/or Bitmap properties will
			///   provide the results of the operation.
			///  </para>
			///  <para>
			///   Set to false if the operation failed. The Message property will typically provide details why the
			///   failure occurred.
			///  </para>
			/// </value>
			public bool HasSucceeded
			{
				get { return fHasSucceeded; }
			}

			/// <summary>Determines if the operation failed.</summary>
			/// <value>
			///  <para>
			///   Set to true if the operation failed. The Message property will typically provide details why the
			///   failure occurred.
			///  </para>
			///  <para>
			///   Set to false if the operation was a success. This means that BitmapInfo and/or Bitmap properties will
			///   provide the results of the operation.
			///  </para>
			/// </value>
			public bool HasFailed
			{
				get { return !fHasSucceeded; }
			}

			/// <summary>Gets a message providing details about the result of the operation.</summary>
			/// <value>
			///  Message providing details about the result of the operation.
			///  Typically provides an error message if the operation failed.
			/// </value>
			public string Message
			{
				get { return fMessage; }
			}

			/// <summary>Gets information about the bitmap that was provided by the operation.</summary>
			/// <value>
			///  <para>Information about the bitmap if the operation succeeded.</para>
			///  <para>Set to null if the operation failed.</para>
			/// </value>
			public WinRT.Interop.Graphics.BitmapInfo BitmapInfo
			{
				get { return fBitmapInfo; }
			}

			/// <summary>Gets the bitmap that was produced by the operation.</summary>
			/// <value>
			///  <para>The bitmap that was produced by the operation.</para>
			///  <para>Set to null if the operation failed.</para>
			/// </value>
			public WinRT.Interop.Graphics.IBitmap Bitmap
			{
				get { return fBitmap; }
			}

			#endregion
		}

		#endregion
	}
}
