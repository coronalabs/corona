using System;


namespace CoronaLabs.Corona.WinRT.Phone.Interop.UI
{
	/// <summary>Provides access to the .NET CoronaPanel control's features and events to C++/CX.</summary>
	public class DotNetCoronaControlAdapter : DotNetControlAdapter, Corona.WinRT.Interop.UI.ICoronaControlAdapter
	{
		#region Constants
		/// <summary>
		///  Fully qualified class type name of the CoronaPanel control in the Corona.Controls.DotNet.dll library.
		/// </summary>
		private const String CoronaPanelFullTypeName = "CoronaLabs.Corona.WinRT.Phone.CoronaPanel";

		#endregion


		#region Member Variables
		/// <summary>Provides access to the CoronaPanel.RenderSurface property via reflection.</summary>
		private static System.Reflection.PropertyInfo sRenderSurfaceProperty;

		/// <summary>Provides access to the CoronaPanel.Runtime property via reflection.</summary>
		private static System.Reflection.PropertyInfo sRuntimeProperty;

		#endregion


		#region Constructors
		/// <summary>Creates a new adapter which wraps the given CoronaPanel control.</summary>
		/// <param name="coronaPanel">
		///  <para>Reference to the CoronaPanel control that this adapter will wrap.</para>
		///  <para>Cannot be null or else an exception will be thrown.</para>
		/// </param>
		public DotNetCoronaControlAdapter(System.Windows.FrameworkElement coronaPanel) : base(coronaPanel)
		{
			// Verify that the argument is of type "CoronaPanel", which resides in the "Corona.Controls.DotNet.dll" library.
			// Note: This must be done via reflection because the CoronaPanel control cannot be in this library.
			//       The reason is because the Visual Studio UI designer will fail to load/display a control from a
			//       library that implements native C++/CX interface like this library does.
			if (DotNetCoronaControlAdapter.CanWrap(coronaPanel) == false)
			{
				throw new ArgumentException("Argument \"coronaPanel\" must be of type \"" + CoronaPanelFullTypeName + "\".");
			}

			// Load the control's properties via reflection, if not already done.
			if (sRenderSurfaceProperty == null)
			{
				sRenderSurfaceProperty = coronaPanel.GetType().GetProperty("RenderSurface");
			}
			if (sRuntimeProperty == null)
			{
				sRuntimeProperty = coronaPanel.GetType().GetProperty("Runtime");
			}
		}

		#endregion


		#region Public Methods/Properties
		/// <summary>Gets the 3D rendering surface that the Corona runtime renders to.</summary>
		/// <value>Reference to the control's 3D rendering surface.</value>
		public Corona.WinRT.Interop.Graphics.IRenderSurface RenderSurface
		{
			get
			{
				var coronaPanel = this.ReferencedControl;
				if (coronaPanel == null)
				{
					return null;
				}
				return sRenderSurfaceProperty.GetValue(coronaPanel) as Corona.WinRT.Interop.Graphics.IRenderSurface;
			}
		}

		/// <summary>
		///  Gets a reference to the Corona runtime which executes the Corona Lua scripts and renders to the
		///  control's 3D rendering surface.
		/// </summary>
		/// <value>Reference to the Corona runtime used to render to this control.</value>
		public CoronaRuntime Runtime
		{
			get
			{
				var coronaPanel = this.ReferencedControl;
				if (coronaPanel == null)
				{
					return null;
				}
				return sRenderSurfaceProperty.GetValue(coronaPanel) as Corona.WinRT.CoronaRuntime;
			}
		}

		#endregion


		#region Static Methods
		/// <summary>Determines if the given object can be wrapped by this adapter class.</summary>
		/// <param name="control">Reference to an object to test. Can be null.</param>
		/// <returns>
		///  <para>Returns true if the given object is of type "CoronaPanel" and can be wrapped by this adapter class.</para>
		///  <para>Returns false if given null or an object type other than "CoronaPanel".</para>
		/// </returns>
		public static bool CanWrap(object control)
		{
			if (control == null)
			{
				return false;
			}
			return (control.GetType().FullName == CoronaPanelFullTypeName);
		}

		#endregion
	}
}
