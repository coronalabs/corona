using System;


namespace CoronaLabs.Corona.WinRT.Phone.Interop.Graphics
{
	/// <summary>Provides a text renderer object that is involved with the event.</summary>
	public class DotNetTextRendererEventArgs : EventArgs
	{
		/// <summary>Reference to the text renderer that is involved with the event.</summary>
		private DotNetTextRenderer fTextRenderer;


		/// <summary>Creates a new event arguments object with the given text renderer reference.</summary>
		/// <param name="textRenderer">
		///  <para>Reference to the text renderer that is involved with the event.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		public DotNetTextRendererEventArgs(DotNetTextRenderer textRenderer)
		{
			fTextRenderer = textRenderer;
		}

		/// <summary>Gets a reference to the text renderer that is involved with the event.</summary>
		/// <value>Reference to the text renderer involved with the event.</value>
		public DotNetTextRenderer TextRenderer
		{
			get { return fTextRenderer; }
		}
	}
}
