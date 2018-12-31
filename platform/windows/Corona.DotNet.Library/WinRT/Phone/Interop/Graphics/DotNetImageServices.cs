using System;


namespace CoronaLabs.Corona.WinRT.Phone.Interop.Graphics
{
	/// <summary>Provides image decoders, encoders, and text renderers that are implemented in .NET.</summary>
	/// <remarks>An instance of this class is expected to be given to a CoronaRuntimeSettings object on startup.</remarks>
	public class DotNetImageServices : WinRT.Interop.Graphics.IImageServices
	{
		#region Events
		/// <summary>
		///  <para>Raised when the CreateTextRenderer() method has been called.</para>
		///  <para>This event is intended to allow the owner to monitor the created text renderers.</para>
		/// </summary>
		public event EventHandler<DotNetTextRendererEventArgs> CreatedTextRenderer;

		#endregion


		#region Constructors
		/// <summary>
		///  Creates a new image service provider used to create .NET implemented image decoders, encoders, etc.
		/// </summary>
		public DotNetImageServices()
		{
		}

		#endregion


		#region Public Methods
		/// <summary>Creates a new decoder used to decode images such as PNGs and JPEGs to uncompressed bitmaps.</summary>
		/// <returns>Returns a new image decoder object.</returns>
		public WinRT.Interop.Graphics.IImageDecoder CreateImageDecoder()
		{
			return new DotNetImageDecoder();
		}

		/// <summary>Creates a new renderer used to draw text to an uncompressed bitmap.</summary>
		/// <returns>Returns a new text rendering object.</returns>
		public WinRT.Interop.Graphics.ITextRenderer CreateTextRenderer()
		{
			// Create the text renderer.
			var textRenderer = new DotNetTextRenderer();

			// Notify the system that a new text renderer was created.
			// Note: The CoronaPanel listens for this event so that it can add handlers to the text renderer's events.
			if (this.CreatedTextRenderer != null)
			{
				this.CreatedTextRenderer.Invoke(this, new DotNetTextRendererEventArgs(textRenderer));
			}

			// Return the newly created text renderer.
			return textRenderer;
		}

		#endregion
	}
}
