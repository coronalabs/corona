using System;


namespace VsixSigner
{
	public class Program
	{
		public static void Main(string[] args)
		{
			// Output information on how to use the command line tool if not given enough arguments.
			if (args.Length < 3)
			{
				System.Console.WriteLine("Visual Studio Extension Signer");
				System.Console.WriteLine("This tool is used to digitally sign a vsix file with a pfx certificate.");
				System.Console.WriteLine();
				System.Console.WriteLine("Usage:  VsixSigner <PfxFilePath> <PfxPassword> <VsixFilePath>");
				System.Console.WriteLine("  PfxFilePath     Path to the PFX certificate file to sign with.");
				System.Console.WriteLine("  PfxPassword     The password assigned to the PFX file.");
				System.Console.WriteLine("  VsixFilePath    Path to the VSIX file to digitally sign.");
				return;
			}

			// Fetch and validate the PFX file path.
			string pfxFilePath = args[0];
			if (string.IsNullOrEmpty(pfxFilePath))
			{
				System.Console.WriteLine("You must provide a valid path to a PFX file.");
				System.Environment.ExitCode = -1;
				return;
			}
			if (System.IO.File.Exists(pfxFilePath) == false)
			{
				System.Console.WriteLine("PFX file not found: " + pfxFilePath);
				System.Environment.ExitCode = -1;
				return;
			}

			// Fetch the PFX password.
			string pfxPassword = args[1];
			if (pfxPassword == null)
			{
				pfxPassword = string.Empty;
			}

			// Fetch and validate the path to the VSIX file.
			string vsixFilePath = args[2];
			if (string.IsNullOrEmpty(vsixFilePath))
			{
				System.Console.WriteLine("You must provide a valid path to the VSIX file to digitally sign.");
				System.Environment.ExitCode = -1;
				return;
			}
			if (System.IO.File.Exists(vsixFilePath) == false)
			{
				System.Console.WriteLine("VSIX file not found: " + pfxFilePath);
				System.Environment.ExitCode = -1;
				return;
			}

			// Digitally sign the VSIX file's contents.
			bool wasSigned = false;
			System.IO.Packaging.Package vsixPackage = null;
			try
			{
				// Set up the signature manager.
				vsixPackage = System.IO.Packaging.Package.Open(vsixFilePath, System.IO.FileMode.Open);
				var signatureManager = new System.IO.Packaging.PackageDigitalSignatureManager(vsixPackage);
				signatureManager.CertificateOption = System.IO.Packaging.CertificateEmbeddingOption.InSignaturePart;

				// Create a collection of paths to all of VSIX file's internal files to be signed.
				var vsixPartPaths = new System.Collections.Generic.List<System.Uri>();
				foreach (var packagePart in vsixPackage.GetParts())
				{
					vsixPartPaths.Add(packagePart.Uri);
				}
				vsixPartPaths.Add(
						System.IO.Packaging.PackUriHelper.GetRelationshipPartUri(signatureManager.SignatureOrigin));
				vsixPartPaths.Add(signatureManager.SignatureOrigin);
				vsixPartPaths.Add(
						System.IO.Packaging.PackUriHelper.GetRelationshipPartUri(new Uri("/", UriKind.RelativeOrAbsolute)));

				// Create digital signatures for all of the VSIX's internal files/parts.
				var certificate = new System.Security.Cryptography.X509Certificates.X509Certificate2(pfxFilePath, pfxPassword);
				signatureManager.Sign(vsixPartPaths, certificate);

				// Verify that the VSIX file was correctly signed.
				if (signatureManager.IsSigned &&
				    (signatureManager.VerifySignatures(true) == System.IO.Packaging.VerifyResult.Success))
				{
					wasSigned = true;
				}
			}
			catch (Exception ex)
			{
				System.Console.WriteLine(ex.Message);
				System.Environment.ExitCode = -1;
				return;
			}
			finally
			{
				if (vsixPackage != null)
				{
					try { vsixPackage.Close(); }
					catch (Exception) { }
				}
			}
			
			// If the digital signatures applied to the VSIX are invalid, then notify the user.
			if (wasSigned == false)
			{
				System.Console.WriteLine("The digital signatures applied to the VSIX file are invalid.");
				System.Environment.ExitCode = -1;
				return;
			}

			// Signing was successful.
			System.Console.WriteLine("Successfully signed the VSIX file.");
		}
	}
}
