using System;


namespace VSTemplateBuilder
{
	public class OutputSettings
	{
		#region Private Member Variables
		private string fFileName;

		private string fIntermediateFolderPath;

		private System.Collections.Specialized.StringCollection fFolderPaths;

		#endregion


		#region Constructors
		public OutputSettings()
		{
			fFileName = string.Empty;
			fIntermediateFolderPath = string.Empty;
			fFolderPaths = new System.Collections.Specialized.StringCollection();
		}

		#endregion


		#region Public Methods/Properties
		public string FileName
		{
			get { return fFileName; }
			set
			{
				if (value == null)
				{
					value = string.Empty;
				}
				fFileName = value;
			}
		}

		public string IntermediateFolderPath
		{
			get { return fIntermediateFolderPath; }
			set
			{
				if (value == null)
				{
					value = string.Empty;
				}
				fIntermediateFolderPath = value;
			}
		}

		public System.Collections.Specialized.StringCollection FolderPaths
		{
			get { return fFolderPaths; }
		}

		#endregion


		#region Public Static Methods
		public static OutputSettings From(System.Xml.XmlNode xmlNode)
		{
			// Validate.
			if (xmlNode == null)
			{
				return null;
			}

			// Extract the output settings from the given XML node.
			OutputSettings settings = null;
			try
			{
				settings = new OutputSettings();
				foreach (System.Xml.XmlAttribute attribute in xmlNode.Attributes)
				{
					if (attribute.Name == "FileName")
					{
						settings.FileName = attribute.Value;
					}
					else if (attribute.Name == "IntermediateFolderPath")
					{
						settings.IntermediateFolderPath = attribute.Value;
					}
				}
				foreach (System.Xml.XmlNode nextXmlNode in xmlNode.ChildNodes)
				{
					if (nextXmlNode.Name == "Folder")
					{
						var attribute = nextXmlNode.Attributes.GetNamedItem("Path");
						if ((attribute != null) && (attribute.Value != null))
						{
							settings.FolderPaths.Add(attribute.Value);
						}
					}
				}
			}
			catch (Exception) { }

			// Return the settings extracted from the given XML node.
			return settings;
		}

		#endregion
	}
}
