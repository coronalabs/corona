using System;


namespace CoronaLabs.Corona.WinRT.Interop
{
	/// <summary>Stores a weak reference to an object and only provides read-only access to it.</summary>
	/// <remarks>
	///  If you intend to store a weak reference to a .NET object on the C++/CX side of the application,
	///  then it must be wrapped by the weak reference class. The reason is because C++/CX Platform::WeakReference
	///  objects will throw an exception if you attempt to give it a .NET object that does not implement a C++/CX interface.
	/// </remarks>
	public class DotNetReadOnlyWeakReference : IReadOnlyWeakReference
	{
		#region Private Member Variables
		/// <summary>Stores a weak reference to the given object.</summary>
		private WeakReference fWeakReference;

		#endregion


		#region Constructors
		/// <summary>Creates an empty weak reference object that always returns null.</summary>
		public DotNetReadOnlyWeakReference() : this(null)
		{
		}

		/// <summary>Creates a read-only weak reference to the given object.</summary>
		/// <param name="referent">
		///  <para>The object to store a weak reference to.</para>
		///  <para>Can be null, in which case the new weak reference object will always return null.</para>
		/// </param>
		public DotNetReadOnlyWeakReference(object referent)
		{
			fWeakReference = new WeakReference(referent);
		}

		#endregion


		#region Public Methods/Properties
		/// <summary>Fetches the stored reference and returns it as of type "object", if still available.</summary>
		/// <returns>
		///  <para>Returns the stored reference as of type "object".</para>
		///  <para>Returns null if the weak referenced object has been garbage collected.</para>
		/// </returns>
		public object GetAsObject()
		{
			return fWeakReference.Target;
		}

		#endregion
	}
}
