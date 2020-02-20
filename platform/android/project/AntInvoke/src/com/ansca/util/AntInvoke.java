//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.util;

import java.io.File;
import java.util.Map;
import java.lang.System;

import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.DefaultLogger;
import org.apache.tools.ant.Project;
import org.apache.tools.ant.ProjectHelper;

/*
 * javap -classpath AntInvoke.jar -protected com.ansca.util.AntInvoke -s
 * 

Compiled from "AntInvoke.java"
public class com.ansca.util.AntInvoke extends java.lang.Object{
public com.ansca.util.AntInvoke();
  Signature: ()V
protected static java.lang.String runAntTask(java.lang.String, java.lang.String, java.util.Map);
  Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/util/Map;)Ljava/lang/String;
}

 */

public class AntInvoke {
	/**
	 * Run an ant task.
	 * 
	 * @param buildFilePath			Absolute path to build.xml
	 * @param targetName			Target to run within the build.xml
	 * @param properties			Properties to set
	 * @return						Result code. "" is success, otherwise an error message.
	 */
	protected static String runAntTask( String buildFilePath, String targetName, int logLevel, Map<String,String> properties ) {
		String errorMessage = null;
		
		try {
			File buildFile = new File( buildFilePath );
			ProjectHelper projectHelper = ProjectHelper.getProjectHelper();
			Project project = new Project();
			DefaultLogger listener = new DefaultLogger();

			// Translate our log levels to those understood by ant
			if (logLevel == 1) {
				listener.setMessageOutputLevel(Project.MSG_VERBOSE);
			}
			else if (logLevel > 1) {
				listener.setMessageOutputLevel(Project.MSG_DEBUG);
			}

			// Execute the targeted ant task.
			listener.setOutputPrintStream( System.out );
			listener.setErrorPrintStream( System.out );
			project.addBuildListener( listener );
			project.setUserProperty("ant.file", buildFile.getAbsolutePath());
			if (properties != null) {
				for (String key : properties.keySet()) {
					project.setProperty(key, properties.get(key));
				}
			}
			project.init();
			project.addReference("ant.projectHelper", projectHelper);
			projectHelper.parse(project, buildFile);
			project.executeTarget(targetName);
		}
		catch (BuildException ex) {
			// Fetch the inner exception's message, which contains the root cause of the build error.
			// This also removes the ant "build.xml" stack trace from the message.
			Throwable rootException = ex;
			while (rootException.getCause() != null) {
				rootException = rootException.getCause();
			}
			errorMessage = rootException.getMessage();

			// If the root exception does not have a message, then use the top level exception's message.
			if ((errorMessage == null) || (errorMessage.length() <= 0)) {
				errorMessage = ex.getMessage();
				if ((errorMessage == null) || (errorMessage.length() <= 0)) {
					errorMessage = ex.getClass().getName();
				}
			}
		}
		catch (Exception ex) {
			// An unknown exception occurred.
			errorMessage = ex.getMessage();
			if ((errorMessage == null) || (errorMessage.length() <= 0)) {
				errorMessage = ex.getClass().getName();
			}
		}

		// Return an error message if the ant task failed.
		// Return an empty string if succeeded.
		return (errorMessage != null) ? errorMessage : "";
	}
}
