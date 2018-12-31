using System;


namespace CoronaLabs.Corona.WinRT.Phone.Interop.Graphics
{
	/// <summary>Stores an uncompressed bitmap's pixels and information such as width, height, orientation, etc.</summary>
	public class DotNetBitmap : WinRT.Interop.Graphics.IBitmap
	{
		#region Member Variables
		/// <summary>Provides the bitmap's pixel width, height, format, orientation, etc.</summary>
		private WinRT.Interop.Graphics.BitmapInfo fBitmapInfo;

		/// <summary>Stores the memory address to a bitmap's "pinned" pixel array.</summary>
		private System.Runtime.InteropServices.GCHandle fByteBufferHandle;

		#endregion


		#region Constructors/Destructors
		/// <summary>Creates a new empty bitmap whose width and height are set to zero.</summary>
		public DotNetBitmap()
		{
			fBitmapInfo = WinRT.Interop.Graphics.BitmapInfo.Empty;
		}

		/// <summary>Creates a new bitmap with the given information.</summary>
		/// <param name="info">
		///  <para>The bitmap's pixel width, height, format, orientation, etc.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		/// <param name="byteBufferHandle">Handle storing the memory address to a bitmap's "pinned" pixel array.</param>
		private DotNetBitmap(WinRT.Interop.Graphics.BitmapInfo info, System.Runtime.InteropServices.GCHandle byteBufferHandle)
		{
			// Validate.
			if (info == null)
			{
				throw new NullReferenceException("info");
			}

			// Store the given settings.
			fBitmapInfo = info;
			fByteBufferHandle = byteBufferHandle;
		}

		/// <summary>Finalizer/Destructor used to clean up this bitmap's unmanaged resources.</summary>
		/// <remarks>Warning: This method is called on the garbage collector's thread.</remarks>
		~DotNetBitmap()
		{
			ReleaseByteBuffer();
		}

		#endregion


		#region Public Methods/Properties
		/// <summary>Gets information about this bitmap such as its width, height, pixel format, etc.</summary>
		/// <value>
		///  <para>Read-only information about this bitmap.</para>
		///  <para>If the width and height is zero, then this bitmap object has no bytes.</para>
		/// </value>
		public WinRT.Interop.Graphics.BitmapInfo Info
		{
			get { return fBitmapInfo; }
		}

		/// <summary>Gets the memory address of the first byte of the bitmap's byte buffer.</summary>
		/// <remarks>
		///  This property is intended to be read by native C/C++ code to access the bitmap's pixels as a byte array.
		/// </remarks>
		/// <returns>
		///  <para>
		///   The memory address of the first byte of the bitmap's contiguous byte buffer,
		///   which can be converted to a C/C++ pointer to easily access its bytes.
		///  </para>
		///  <para>Returns zero if the bitmap's byte buffer has been released, which is the equivalent to a null pointer.</para>
		/// </returns>
		public long ByteBufferMemoryAddress
		{
			get
			{
				long memoryAddress = 0;
				try
				{
					if (fByteBufferHandle.IsAllocated)
					{
						memoryAddress = fByteBufferHandle.AddrOfPinnedObject().ToInt64();
					}
				}
				catch (Exception) { }

				return memoryAddress;
			}
		}

		/// <summary>Deletes this bitmap's byte buffer and resets the pixel width and height to zero.</summary>
		/// <remarks>It is okay to call this method if the bitmap has already been released.</remarks>
		public void ReleaseByteBuffer()
		{
			try
			{
				if (fByteBufferHandle.IsAllocated)
				{
					fByteBufferHandle.Free();
				}
			}
			catch (Exception) { }
		}

		#endregion


		#region Public Static Methods
		/// <summary>
		///  Creates a DotNetBitmap which wraps the given WriteableBitmap, making its pixels available to C++/CX.
		/// </summary>
		/// <param name="bitmap">The WriteableBitmap object to be wrapped.</param>
		/// <returns>
		///  <para>Returns a new DotNetBitmap wrapping the given bitmap object.</para>
		///  <para>Returns null if given a null argument.</para>
		/// </returns>
		public static DotNetBitmap From(System.Windows.Media.Imaging.WriteableBitmap bitmap)
		{
			// Validate.
			if (bitmap == null)
			{
				return null;
			}

			// Return an empty bitmap if the given writeable bitmap is empty.
			if ((bitmap.PixelWidth <= 0) || (bitmap.PixelHeight <= 0))
			{
				return new DotNetBitmap();
			}

			var handle = System.Runtime.InteropServices.GCHandle.Alloc(
								bitmap.Pixels, System.Runtime.InteropServices.GCHandleType.Pinned);
			var settings = new WinRT.Interop.Graphics.BitmapSettings();
			settings.PixelWidth = bitmap.PixelWidth;
			settings.PixelHeight = bitmap.PixelHeight;
			settings.PixelFormat = WinRT.Interop.Graphics.PixelFormat.BGRA;
			settings.Orientation = WinRT.Interop.RelativeOrientation2D.Upright;
			return new DotNetBitmap(new WinRT.Interop.Graphics.BitmapInfo(settings), handle);
		}

		#endregion
	}
}
