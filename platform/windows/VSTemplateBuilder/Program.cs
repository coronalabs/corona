using System;


namespace VSTemplateBuilder
{
	/// <summary>Class containing the main() method where this application starts.</summary>
	public class Program
	{
		#region Private Member Variables
		/// <summary>Namespace name extracted from the project file to be templatized.</summary>
		private static string sProjectNamespaceName;

		/// <summary>
		///  Collection of Visual Studio project file extensions this builder supports such as ".csproj" and ".vbproj".
		/// </summary>
		private static System.Collections.Specialized.StringCollection sSupportedProjectExtensions;

		#endregion


		#region Constructors
		/// <summary>Static constructor used to initialize static member variables.</summary>
		static Program()
		{
			sProjectNamespaceName = string.Empty;

			sSupportedProjectExtensions = new System.Collections.Specialized.StringCollection();
			sSupportedProjectExtensions.Add(".csproj");
			sSupportedProjectExtensions.Add(".vbproj");
		}

		#endregion


		#region Main Method
		/// <summary>The main method that gets called on application startup.</summary>
		/// <param name="args">
		///  <para>Array of command line arguments provided to this application.</para>
		///  <para>The first argument will be the path to this application's executable.</para>
		///  <para>
		///   The second argument is expected to be a path to the XML file providing this application settings
		///   to build the project template.
		///  </para>
		/// </param>
		private static void Main(string[] args)
		{
			// Do not continue if an argument was not provided.
			if ((args.Length < 1) || string.IsNullOrEmpty(args[0]))
			{
				System.Console.WriteLine("Usage: VisualStudio.ProjectTemplate.Builder <PathToXmlSettingsFile>");
				return;
			}

			// Fetch the path to the XML file which provides the settings this builder needs to create the project template.
			string settingsFilePath = args[0];
			if (System.IO.File.Exists(settingsFilePath) == false)
			{
				System.Console.WriteLine("File not found: " + settingsFilePath);
				System.Environment.ExitCode = -1;
				return;
			}

			// Set this application's current directory to where the XML file resides.
			// This allows the XML file to use relative paths to its project files.
			string settingsDirectoryPath = System.IO.Path.GetDirectoryName(settingsFilePath);
			if (string.IsNullOrEmpty(settingsDirectoryPath) == false)
			{
				System.Environment.CurrentDirectory = settingsDirectoryPath;
			}

			// Load the given XML file.
			System.Xml.XmlNode builderXmlNode = null;
			try
			{
				var xmlDocument = new System.Xml.XmlDocument();
				xmlDocument.Load(settingsFilePath);
				foreach (System.Xml.XmlNode nextXmlNode in xmlDocument.ChildNodes)
				{
					if (nextXmlNode.Name == "VSProjectTemplateBuilder")
					{
						builderXmlNode = nextXmlNode;
						break;
					}
				}
			}
			catch (Exception ex)
			{
				System.Console.WriteLine(ex.Message);
				System.Environment.ExitCode = -1;
				return;
			}
			if (builderXmlNode.Name == null)
			{
				System.Console.WriteLine("The given XML file must contain a 'VSProjectTemplateBuilder' node at its root.");
				System.Environment.ExitCode = -1;
				return;
			}

			// Fetch the "VSTemplate" node from the XML file.
			System.Xml.XmlNode vsTemplateXmlNode = null;
			foreach (System.Xml.XmlNode nextXmlNode in builderXmlNode.ChildNodes)
			{
				if (nextXmlNode.Name == "VSTemplate")
				{
					vsTemplateXmlNode = nextXmlNode;
					break;
				}
			}
			if (vsTemplateXmlNode == null)
			{
				System.Console.WriteLine(string.Format("XML file '{0}' is missing a 'VSTemplate' node.", settingsFilePath));
				System.Environment.ExitCode = -1;
				return;
			}

			// Fetch the "VSTemplate\TemplateContent\Project" node and its project file path from the XML file.
			System.Xml.XmlNode projectXmlNode = null;
			string projectFilePath = null;
			foreach (System.Xml.XmlNode outerXmlNode in vsTemplateXmlNode.ChildNodes)
			{
				if (outerXmlNode.Name == "TemplateContent")
				{
					foreach (System.Xml.XmlNode innerXmlNode in outerXmlNode.ChildNodes)
					{
						if (innerXmlNode.Name == "Project")
						{
							projectXmlNode = innerXmlNode;
							var attributeNode = innerXmlNode.Attributes.GetNamedItem("File");
							if (attributeNode != null)
							{
								projectFilePath = attributeNode.Value;
							}
							break;
						}
					}
					break;
				}
			}
			if (string.IsNullOrEmpty(projectFilePath))
			{
				string message =
						"XML file '" + settingsFilePath +
						"' does not provide a source project file path to turn into a template.";
				System.Console.WriteLine(message);
				System.Environment.ExitCode = -1;
				return;
			}
			if (System.IO.File.Exists(projectFilePath) == false)
			{
				string message =
						"Project file '" + projectFilePath + "' referenced in XML file '" +
						settingsFilePath + "' was not found.";
				System.Console.WriteLine(message);
				System.Environment.ExitCode = -1;
				return;
			}

			// Do not continue if the given Visual Studio project file is not supported by this application.
			// This application only supports C# and VB.NET project files.
			var projectFileExtensionName = System.IO.Path.GetExtension(projectFilePath);
			if (string.IsNullOrEmpty(projectFileExtensionName))
			{
				projectFileExtensionName = string.Empty;
			}
			projectFileExtensionName = projectFileExtensionName.ToLower();
			if (sSupportedProjectExtensions.Contains(projectFileExtensionName) == false)
			{
				string message =
						"Project file '" + projectFilePath + "' is not supported by this project template builder." +
						System.Environment.NewLine +
						"This builder only supports C# and VB.NET projects.";
				System.Console.WriteLine(message);
				System.Environment.ExitCode = -1;
				return;
			}

			// Extract the project file's directory path.
			var projectDirectoryPath = System.IO.Path.GetDirectoryName(projectFilePath);
			if (string.IsNullOrEmpty(projectDirectoryPath))
			{
				projectDirectoryPath = System.IO.Directory.GetCurrentDirectory();
			}

			// Fetch the root namespace name from the project file.
			// This will be used a the template name and string replace it in all project files later.
			sProjectNamespaceName = FetchProjectRootNamespaceFrom(projectFilePath);
			if (string.IsNullOrEmpty(sProjectNamespaceName))
			{
				System.Console.WriteLine("Failed to find 'RootNamespace' element in project file '" + projectFilePath + "'.");
				System.Environment.ExitCode = -1;
				return;
			}

			// Extract the output settings from the given XML file.
			OutputSettings outputSettings = null;
			foreach (System.Xml.XmlNode nextXmlNode in builderXmlNode.ChildNodes)
			{
				if (nextXmlNode.Name == "Output")
				{
					outputSettings = OutputSettings.From(nextXmlNode);
					break;
				}
			}
			if (outputSettings == null)
			{
				string message = "XML file '" + settingsFilePath + "' is missing an 'Output' node.";
				System.Console.WriteLine(message);
				System.Environment.ExitCode = -1;
				return;
			}
			if (outputSettings.IntermediateFolderPath.Length <= 0)
			{
				string message = "XML file '" + settingsFilePath + "' must provide an 'IntermediateFolderPath'.";
				System.Console.WriteLine(message);
				System.Environment.ExitCode = -1;
				return;
			}
			if (outputSettings.FolderPaths.Count <= 0)
			{
				string message = "XML file '" + settingsFilePath + "' must provide at least 1 output folder path.";
				System.Console.WriteLine(message);
				System.Environment.ExitCode = -1;
				return;
			}

			// If an output file name was not provided, then use the project file's name.
			if (outputSettings.FileName.Length <= 0)
			{
				outputSettings.FileName = System.IO.Path.GetFileNameWithoutExtension(projectFilePath) + ".zip";
			}

			// Create and clean the intermedate directory.
			outputSettings.IntermediateFolderPath =
					System.IO.Path.Combine(outputSettings.IntermediateFolderPath,
					                       System.IO.Path.GetFileNameWithoutExtension(outputSettings.FileName));
			try
			{
				if (System.IO.Directory.Exists(outputSettings.IntermediateFolderPath))
				{
					bool isRecursiveDelete = true;
					System.IO.Directory.Delete(outputSettings.IntermediateFolderPath, isRecursiveDelete);
				}
				System.IO.Directory.CreateDirectory(outputSettings.IntermediateFolderPath);
			}
			catch (Exception ex)
			{
				System.Console.WriteLine(ex.Message);
				System.Environment.ExitCode = -1;
				return;
			}

			// Traverse the XML project node, update its XML settings, and copy the project's files.
			try
			{
				foreach (System.Xml.XmlNode vsTemplateChildXmlNode in vsTemplateXmlNode.ChildNodes)
				{
					if (vsTemplateChildXmlNode.Name == "TemplateData")
					{
						foreach (System.Xml.XmlNode templateDataChildXmlNode in vsTemplateChildXmlNode.ChildNodes)
						{
							if ((templateDataChildXmlNode.Name == "Icon") ||
								(templateDataChildXmlNode.Name == "PreviewImage"))
							{
								// Fetch the image file path and verify the file actually exists.
								string sourceImageFilePath = templateDataChildXmlNode.InnerText;
								if (System.IO.File.Exists(sourceImageFilePath) == false)
								{
									System.Console.WriteLine(string.Format(
											"'{0}' file '{1}' not found.",
											templateDataChildXmlNode.Name, sourceImageFilePath));
									System.Environment.ExitCode = -1;
									return;
								}

								// Copy the file to the template's intermediate directory.
								string targetImageFileName = "__" + System.IO.Path.GetFileName(sourceImageFilePath);
								string targetImageFilePath = System.IO.Path.Combine(
										outputSettings.IntermediateFolderPath, targetImageFileName);
								System.IO.File.Copy(sourceImageFilePath, targetImageFilePath);

								// Update the XML's image file path, to be used by the final template file.
								templateDataChildXmlNode.InnerText = targetImageFileName;
							}
						}
					}
					else if (vsTemplateChildXmlNode.Name == "TemplateContent")
					{
						foreach (System.Xml.XmlNode templateContentChildXmlNode in vsTemplateChildXmlNode.ChildNodes)
						{
							if (templateContentChildXmlNode.Name == "Project")
							{
								// Replace the "Project" element's "File" attribute with a path to the template's project file.
								var attribute = templateContentChildXmlNode.Attributes.GetNamedItem("File");
								if (attribute != null)
								{
									attribute.InnerText = System.IO.Path.GetFileName(projectFilePath);
								}

								// Remove all of the "Project" node's children, if any.
								templateContentChildXmlNode.InnerXml = string.Empty;

								// Copy all of the project's relevant files to a temporary template directory.
								// This also replaces project names, namespaces, and GUIDs with template variables.
								CopyProjectDirectoryFiles(
										templateContentChildXmlNode, projectDirectoryPath,
										outputSettings.IntermediateFolderPath);
							}
						}
					}
				}
			}
			catch (Exception ex)
			{
				System.Console.WriteLine(ex.Message);
				System.Environment.ExitCode = -1;
				return;
			}

			// Create the *.vstemplate file using the updated "vsTemplateXmlNode" up above.
			try
			{
				string vsTemplateFilePath = System.IO.Path.Combine(outputSettings.IntermediateFolderPath, "MyTemplate.vstemplate");
				var vsTemplateXmlDocument = new System.Xml.XmlDocument();
				vsTemplateXmlDocument.AppendChild(vsTemplateXmlDocument.ImportNode(vsTemplateXmlNode, true));
				vsTemplateXmlDocument.Save(vsTemplateFilePath);
			}
			catch (Exception ex)
			{
				System.Console.WriteLine("Failed to create *.vstemplate file. Reason:");
				System.Console.WriteLine(ex.Message);
				System.Environment.ExitCode = -1;
				return;
			}

			// Zip up the project template folder.
			string intermediateZipFilePath = System.IO.Path.Combine(
					System.IO.Path.GetDirectoryName(outputSettings.IntermediateFolderPath), outputSettings.FileName);
			try
			{
				if (System.IO.File.Exists(intermediateZipFilePath))
				{
					System.IO.File.Delete(intermediateZipFilePath);
				}
				bool isZippingBaseDirectory = false;
				System.IO.Compression.ZipFile.CreateFromDirectory(
						outputSettings.IntermediateFolderPath, intermediateZipFilePath,
						System.IO.Compression.CompressionLevel.NoCompression, isZippingBaseDirectory);
			}
			catch (Exception ex)
			{
				System.Console.WriteLine(
						"Failed to create '" + intermediateZipFilePath + "' project template zip file. Reason:");
				System.Console.WriteLine(ex.Message);
				System.Environment.ExitCode = -1;
				return;
			}

			// Copy the zipped up project template to the given destination(s).
			try
			{
				foreach (string directoryPath in outputSettings.FolderPaths)
				{
					if (System.IO.Directory.Exists(directoryPath) == false)
					{
						System.IO.Directory.CreateDirectory(directoryPath);
					}
					string outputFilePath = System.IO.Path.Combine(directoryPath, outputSettings.FileName);
					System.IO.File.Copy(intermediateZipFilePath, outputFilePath, true);
				}
			}
			catch (Exception ex)
			{
				System.Console.WriteLine(ex.Message);
				System.Environment.ExitCode = -1;
				return;
			}
		}

		#endregion


		#region Private Methods
		private static void CopyProjectDirectoryFiles(
			System.Xml.XmlNode projectXmlNode, string sourceDirectoryPath, string targetDirectoryPath)
		{
			// If given an empty path, then assume it references the current directory.
			if (string.IsNullOrEmpty(sourceDirectoryPath))
			{
				sourceDirectoryPath = System.IO.Directory.GetCurrentDirectory();
			}
			if (string.IsNullOrEmpty(targetDirectoryPath))
			{
				targetDirectoryPath = System.IO.Directory.GetCurrentDirectory();
			}

			// Do not continue if the source and target reference the same directory.
			if (sourceDirectoryPath == targetDirectoryPath)
			{
				throw new InvalidOperationException(
						"Arguments 'sourceDirectoryPath' and 'targetDirectoryPath' cannot reference the same directory.");
			}

			// Do not continue if the source directory does not exist.
			if (System.IO.Directory.Exists(sourceDirectoryPath) == false)
			{
				return;
			}

			// Create the target directory if it does not exist.
			// Note: Allow an exception to be thrown here if creation failed. We want it to bubble up to the caller.
			if (System.IO.Directory.Exists(targetDirectoryPath) == false)
			{
				System.IO.Directory.CreateDirectory(targetDirectoryPath);
			}

			// Copy all of the relevant source files.
			foreach (string nextFilePath in System.IO.Directory.EnumerateFiles(sourceDirectoryPath))
			{
				// Skip hidden files.
				if ((System.IO.File.GetAttributes(nextFilePath) & System.IO.FileAttributes.Hidden) != 0)
				{
					continue;
				}

				// Skip files starting with a period. (ie: Unix style hidden files.)
				string fileName = System.IO.Path.GetFileName(nextFilePath);
				if (fileName.StartsWith("."))
				{
					continue;
				}

				// Skip files having the following extensions.
				string extensionName = System.IO.Path.GetExtension(nextFilePath);
				if (string.IsNullOrEmpty(extensionName) == false)
				{
					extensionName = extensionName.ToLower();
					if ((extensionName == ".user") ||
						(extensionName == ".sln") ||
						(extensionName == ".sdf") ||
						(extensionName == ".suo"))
					{
						continue;
					}
				}

				// Copy the source file and string replace its project/namespace names with templated variables.
				string targetFilePath = System.IO.Path.Combine(targetDirectoryPath, System.IO.Path.GetFileName(fileName));
				CopyProjectFile(projectXmlNode, nextFilePath, targetFilePath);
			}

			// Traverse the source directory's subdirectories and copy there files too.
			foreach (string sourceSubdirectoryPath in System.IO.Directory.EnumerateDirectories(sourceDirectoryPath))
			{
				// Extract the directory name.
				string directoryName = System.IO.Path.GetFileName(sourceSubdirectoryPath);
				string lowerCaseDirectoryName = directoryName.ToLower();

				// Skip the following directory names. They should not be part of a project template.
				if ((lowerCaseDirectoryName == "bin") ||
					(lowerCaseDirectoryName == "debug") ||
					(lowerCaseDirectoryName == "release") ||
					(lowerCaseDirectoryName == "obj"))
				{
					continue;
				}

				// Create an XML element for this directory and add it under the given project XML node.
				var folderXmlNode = projectXmlNode.OwnerDocument.CreateElement("Folder", projectXmlNode.NamespaceURI);
				var xmlAttribute = projectXmlNode.OwnerDocument.CreateAttribute("Name");
				xmlAttribute.Value = directoryName;
				folderXmlNode.Attributes.Append(xmlAttribute);
				xmlAttribute = projectXmlNode.OwnerDocument.CreateAttribute("TargetFolderName");
				xmlAttribute.Value = directoryName;
				folderXmlNode.Attributes.Append(xmlAttribute);
				projectXmlNode.AppendChild(folderXmlNode);

				// Copy the subdirectory's files.
				string targetSubdirectoryPath = System.IO.Path.Combine(targetDirectoryPath, directoryName);
				CopyProjectDirectoryFiles(folderXmlNode, sourceSubdirectoryPath, targetSubdirectoryPath);
			}
		}

		private static void CopyProjectFile(
			System.Xml.XmlNode projectXmlNode, string sourceFilePath, string targetFilePath)
		{
			// Copy the given source file to the template directory.
			bool requiresStringSubstitution = FileRequiresStringSubstition(sourceFilePath);
			if (requiresStringSubstitution)
			{
				// This is a text file that requires string substitutions. Do it line by line below.
				System.IO.StreamReader reader = null;
				System.IO.StreamWriter writer = null;
				try
				{
					reader = new System.IO.StreamReader(sourceFilePath);
					writer = new System.IO.StreamWriter(targetFilePath, false);
					while (reader.EndOfStream == false)
					{
						string nextLine = reader.ReadLine();
						if (nextLine != null)
						{
							// Replace project/namespace name.
							nextLine = nextLine.Replace(sProjectNamespaceName, "$safeprojectname$");

							// Replace GUID in project file.
							if (nextLine.Contains("<ProjectGuid>") && nextLine.Contains("</ProjectGuid>"))
							{
								nextLine = "    <ProjectGuid>$guid1$</ProjectGuid>";
							}

							// Replace GUID in "AssemblyInfo.*" files.
							// Note: This is not a full proof means of handling it, but will work for our own projects.
							if (nextLine.Contains("[assembly:") &&
							    (nextLine.Contains("Guid(") || nextLine.Contains("GuidAttribute(")))
							{
								nextLine = "[assembly: System.Runtime.InteropServices.Guid(\"$guid2$\")]";
							}
							else if (nextLine.Contains("<Assembly: ") &&
									 (nextLine.Contains("Guid(") || nextLine.Contains("GuidAttribute(")))
							{
								nextLine = "<Assembly: System.Runtime.InteropServices.Guid(\"$guid2$\")>";
							}

							writer.WriteLine(nextLine);
						}
					}
				}
				finally
				{
					if (reader != null)
					{
						try { reader.Dispose(); }
						catch (Exception) { }
					}
					if (writer != null)
					{
						try { writer.Dispose(); }
						catch (Exception) { }
					}
				}

				// If this is the "WMAppManifest.xml" file, then replace its publisher and project GUIDs.
				if (System.IO.Path.GetFileName(targetFilePath).ToLower() == "wmappmanifest.xml")
				{
					try
					{
						bool wasXmlModified = false;
						var xmlDocument = new System.Xml.XmlDocument();
						xmlDocument.Load(targetFilePath);
						foreach (System.Xml.XmlNode rootXmlChildNode in xmlDocument.ChildNodes)
						{
							if (rootXmlChildNode.Name == "Deployment")
							{
								foreach (System.Xml.XmlNode deploymentXmlChildNode in rootXmlChildNode.ChildNodes)
								{
									if (deploymentXmlChildNode.Name == "App")
									{
										System.Xml.XmlNode attribute;
										attribute = deploymentXmlChildNode.Attributes.GetNamedItem("ProductID");
										if (attribute != null)
										{
											attribute.InnerText = "{$guid3$}";
											wasXmlModified = true;
										}
										attribute = deploymentXmlChildNode.Attributes.GetNamedItem("PublisherID");
										if (attribute != null)
										{
											attribute.InnerText = "{$guid4$}";
											wasXmlModified = true;
										}
										break;
									}
								}
								break;
							}
						}
						if (wasXmlModified)
						{
							xmlDocument.Save(targetFilePath);
						}
					}
					catch (Exception) { }
				}
			}
			else
			{
				// The given file does not require string substitutions. Just copy the file as-is.
				System.IO.File.Copy(sourceFilePath, targetFilePath);
			}

			// Add the copied file to the
			// Create an XML element for the copied file and add it under the given project XML node.
			string targetFileName = System.IO.Path.GetFileName(targetFilePath);
			var itemXmlNode = projectXmlNode.OwnerDocument.CreateElement("ProjectItem", projectXmlNode.NamespaceURI);
			var xmlAttribute = projectXmlNode.OwnerDocument.CreateAttribute("ReplaceParameters");
			xmlAttribute.Value = requiresStringSubstitution ? "true" : "false";
			itemXmlNode.Attributes.Append(xmlAttribute);
			xmlAttribute = projectXmlNode.OwnerDocument.CreateAttribute("TargetFileName");
			xmlAttribute.Value = targetFileName;
			itemXmlNode.Attributes.Append(xmlAttribute);
			itemXmlNode.InnerText = targetFileName;
			projectXmlNode.AppendChild(itemXmlNode);
		}

		private static bool FileRequiresStringSubstition(string filePath)
		{
			using (var reader = new System.IO.StreamReader(filePath))
			{
				while (reader.EndOfStream == false)
				{
					string nextLine = reader.ReadLine();
					if (string.IsNullOrEmpty(nextLine) == false)
					{
						if (nextLine.Contains(sProjectNamespaceName) ||
							nextLine.Contains("[assembly: ") ||
							nextLine.Contains("<Assembly: ") ||
							nextLine.Contains("<ProjectGuid>"))
						{
							return true;
						}
					}
				}
			}
			return false;
		}

		private static string FetchProjectRootNamespaceFrom(string projectFilePath)
		{
			// Validate argument.
			if (string.IsNullOrEmpty(projectFilePath))
			{
				throw new ArgumentNullException();
			}

			// Load the project's XML and extract its namespace.
			try
			{
				var xmlDocument = new System.Xml.XmlDocument();
				xmlDocument.Load(projectFilePath);
				foreach (System.Xml.XmlNode projectXmlChildNode in xmlDocument.DocumentElement.ChildNodes)
				{
					if (projectXmlChildNode.Name != "PropertyGroup")
					{
						continue;
					}
					foreach (System.Xml.XmlNode propertyGroupXmlChildNode in projectXmlChildNode.ChildNodes)
					{
						if (propertyGroupXmlChildNode.Name == "RootNamespace")
						{
							return propertyGroupXmlChildNode.InnerText;
						}
					}
				}

			}
			catch (Exception) { }

			// Failed to find the project's namespace.
			return null;
		}

		#endregion
	}
}
