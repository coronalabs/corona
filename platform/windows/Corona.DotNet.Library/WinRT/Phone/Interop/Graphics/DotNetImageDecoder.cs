using System;


namespace CoronaLabs.Corona.WinRT.Phone.Interop.Graphics
{
	/// <summary>Decodes an image source such as a PNG or JPEG file to an uncompressed bitmap using the .NET framework.</summary>
	public class DotNetImageDecoder : WinRT.Interop.Graphics.IImageDecoder
	{
		#region Member Variables
		/// <summary>Configuration which tell this decoder what format the final format of the bitmap should be.</summary>
		private WinRT.Interop.Graphics.ImageDecoderSettings fSettings;

		#endregion


		#region Constructors
		/// <summary>Creates a new image decoder.</summary>
		public DotNetImageDecoder()
		{
			fSettings = new WinRT.Interop.Graphics.ImageDecoderSettings();
		}

		#endregion


		#region Public Methods/Properties
		/// <summary>Gets the configuration this image decoder will use when decoding an image to a bitmap.</summary>
		/// <value>
		///  Modifiable settings that the caller should set up before decoding an image. This tells the decoder
		///  what pixel formats the caller supports, maximum pixel width/height the resulting bitmap should be
		///  constrained to, whether or not premultiplied alpha should be applied, etc.
		/// </value>
		public WinRT.Interop.Graphics.ImageDecoderSettings Settings
		{
			get { return fSettings; }
		}

		/// <summary>Not currently supported on Windows Phone. Will always return an error result.</summary>
		/// <param name="filePath">The name and path of the image file to be decoded.</param>
		/// <returns>Always returns an error result since this method is not currently supported on Windows Phone.</returns>
		public WinRT.Interop.Graphics.BitmapInfoResult DecodeInfoFromFile(string filePath)
		{
			return WinRT.Interop.Graphics.BitmapInfoResult.FailedWith("Not implemented.");
		}

		/// <summary>Decodes the given image file and returns an uncompressed bitmap.</summary>
		/// <param name="filePath">The name and path of the image file to be decoded.</param>
		/// <returns>
		///  <para>Returns the result of the image decoding operation.</para>
		///  <para>
		///   If the result object's HasSucceeded property is set true, then this decoder succeeded in decoding
		///   the given image file and the result object's Bitmap property would provide the bitmap that was produced.
		///  </para>
		///  <para>
		///   If the result object's HasSucceeded property is false, then this decoder failed to decode the
		///   given image file. The reason why would then be provided by the result object's Message property.
		///  </para>
		/// </returns>
		public WinRT.Interop.Graphics.BitmapResult DecodeFromFile(string filePath)
		{
			// Validate the given image file path.
			if (String.IsNullOrEmpty(filePath))
			{
				return WinRT.Interop.Graphics.BitmapResult.FailedWith("Image decoder given a null or empty string.");
			}

			// Check that the given file exists and if its an external or embedded resource file.
			bool isExternalFile = true;
			var resourceServices = Corona.WinRT.Interop.Storage.DotNetResourceServices.Instance;
			if (System.IO.File.Exists(filePath) == false)
			{
				if (resourceServices.ContainsFile(filePath) == false)
				{
					String message = String.Format("Image file '{0}' not found.", filePath);
					return WinRT.Interop.Graphics.BitmapResult.FailedWith(message);
				}
				isExternalFile = false;
			}

			// Ensure that the preferred pixel format is in the supported pixel format set.
			fSettings.SupportedPixelFormats.Add(fSettings.PreferredPixelFormat);

			Corona.WinRT.Interop.Graphics.IBitmap bitmap = null;
			System.IO.Stream stream = null;
			try
			{
				if (isExternalFile)
				{
					stream = System.IO.File.OpenRead(filePath);
				}
				else
				{
					stream = resourceServices.OpenFile(filePath);
				}
				var writeableBitmap = new System.Windows.Media.Imaging.WriteableBitmap(0, 0);
				writeableBitmap.SetSource(stream);
				bitmap = DotNetBitmap.From(writeableBitmap);
				writeableBitmap = null;

				// Convert the loaded bitmap if it does not meet the decoder's requirements.
				if ((fSettings.SupportedPixelFormats.Contains(bitmap.Info.PixelFormat) == false) ||
				    (bitmap.Info.PixelWidth > fSettings.MaxPixelWidth) ||
				    (bitmap.Info.PixelHeight > fSettings.MaxPixelHeight))
				{
					var converter = new WinRT.Interop.Graphics.BitmapConverter();
					converter.CopySettingsFrom(this);
//					if (converter.CanConvert(bitmap))
//					{
//						converter.Convert(bitmap);
//					}
//					else
//					{
						var convertedBitmap = converter.CreateBitmapFrom(bitmap);
						bitmap.ReleaseByteBuffer();
						bitmap = convertedBitmap;
//					}
				}
			}
			catch (Exception ex)
			{
				if (bitmap != null)
				{
					try { bitmap.ReleaseByteBuffer(); }
					catch (Exception) { }
				}
				return WinRT.Interop.Graphics.BitmapResult.FailedWith(ex.Message);
			}
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
			if (bitmap == null)
			{
				return WinRT.Interop.Graphics.BitmapResult.FailedWith("Failed to load bitmap");
			}

			return WinRT.Interop.Graphics.BitmapResult.SucceededWith(bitmap);
		}

		#endregion
	}
}
