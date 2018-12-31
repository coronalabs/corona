using System;


namespace CoronaLabs.Corona.WinRT.Interop.Storage
{
	/// <summary>Provides access to embedded resources, such as Corona's widget library images.</summary>
	public class DotNetResourceServices : IResourceServices
	{
		#region Private Member Variables
		/// <summary>Prefix used by all resource names embedded in this library.</summary>
		private string fResourceNamePrefix;

		/// <summary>Stores a collection of all resource names embedded in this library for fast existence checks.</summary>
		private System.Collections.Generic.HashSet<string> fResourceNameSet;

		#endregion


		#region Pre-allocated Instance
		/// <summary>Provides a pre-allocated instance of this class.</summary>
		public static readonly DotNetResourceServices Instance = new DotNetResourceServices();

		#endregion


		#region Constructors
		/// <summary>Creates a new .NET implemented IResourceServices object.</summary>
		/// <remarks>Constructor made private to prevent instances from being made.</remarks>
		private DotNetResourceServices()
		{
			// Build the resource name prefixe that all embedded resources Corona uses.
			fResourceNamePrefix = "CoronaLabs.Corona.EmbeddedResources.";

			// Fetch the names of all embedded resources belonging to this Corona library for fast look-ups.
			fResourceNameSet = new System.Collections.Generic.HashSet<string>();
			var resourceNameArray = this.GetType().Assembly.GetManifestResourceNames();
			foreach (var nextName in resourceNameArray)
			{
				if ((string.IsNullOrEmpty(nextName) == false) && nextName.StartsWith(fResourceNamePrefix))
				{
					fResourceNameSet.Add(nextName);
				}
			}
		}

		#endregion


		#region Public Methods/Properties
		/// <summary>Determines if the given resource name exists as an embedded file within the application's library.</summary>
		/// <param name="resourceName">Unique name of the resource.</param>
		/// <returns>Returns true if the resource name was found and is an embedded file. Returns false if not found.</returns>
		public bool ContainsFile(string resourceName)
		{
			// Validate the given name.
			if (string.IsNullOrEmpty(resourceName))
			{
				return false;
			}

			// Add the resource name prefix if not found.
			if (resourceName.StartsWith(fResourceNamePrefix) == false)
			{
				resourceName = fResourceNamePrefix + resourceName;
			}
			
			// Determine if the given resource name exists within this library.
			return fResourceNameSet.Contains(resourceName);
		}

		/// <summary>Opens a stream to the given embedded resource file.</summary>
		/// <param name="resourceName">Unique name of the resource.</param>
		/// <returns>
		///  <para>Returns a read-only stream to the embedded resource's bytes.</para>
		///  <para>Returns null if the resource name was not found.</para>
		/// </returns>
		public System.IO.Stream OpenFile(string resourceName)
		{
			// Validate the given name.
			if (string.IsNullOrEmpty(resourceName))
			{
				return null;
			}

			// Add the resource name prefix if not found.
			if (resourceName.StartsWith(fResourceNamePrefix) == false)
			{
				resourceName = fResourceNamePrefix + resourceName;
			}

			// Attempt to open a stream to the given resource.
			return this.GetType().Assembly.GetManifestResourceStream(resourceName);
		}

		#endregion
	}
}
