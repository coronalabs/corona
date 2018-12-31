using System;


namespace CoronaLabs.Corona.WinRT.Phone.Interop.Graphics
{
	/// <summary>Provides a WriteableBitmap object that is involved with the event.</summary>
	public class DotNetWriteableBitmapEventArgs : EventArgs
	{
		/// <summary>Reference to the writeable bitmap involved with the event.</summary>
		private System.Windows.Media.Imaging.WriteableBitmap fWriteableBitmap;


		/// <summary>Creates a new event arguments object with the given WriteableBitmap reference.</summary>
		/// <param name="bitmap">
		///  <para>Reference to the bitmap involved with the event.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		public DotNetWriteableBitmapEventArgs(System.Windows.Media.Imaging.WriteableBitmap bitmap)
		{
			// Validate argument.
			if (bitmap == null)
			{
				throw new NullReferenceException();
			}

			// Store the given bitmap reference.
			fWriteableBitmap = bitmap;
		}

		/// <summary>Gets a reference to the bitmap involved with the event.</summary>
		/// <value>Reference to the bitmap involved with the event.</value>
		public System.Windows.Media.Imaging.WriteableBitmap WriteableBitmap
		{
			get { return fWriteableBitmap; }
		}
	}
}
