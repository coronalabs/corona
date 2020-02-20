//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.coronalabs;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.util.Vector;

import org.apache.tools.ant.Task;
import org.apache.tools.ant.types.Path;


public class LiveBuildManifest extends Task {

	private String output, root;
	private Vector<Path> paths = new Vector<Path>(); 
	
	public void setRoot(String root) {
		this.root = root;
	}
	public void setOutput(String output) {
		this.output = output;
	}

	public void addPath(Path path) {
		paths.add(path);
	}


	public void execute() {
		BufferedWriter out = null;
		try {
			out = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(output), "UTF-8"));
			for (Path pathSet : paths) {
				for (String path: pathSet.list()) {
					File f = new File(path);
					String relPath = path;
					if(path.startsWith(root)) {
						relPath = path.substring(root.length());
					}
					if(File.separatorChar == '\\') {
						relPath = relPath.replace('\\', '/');
					}
					String line = "0 / "+ Long.toString(f.lastModified()/1000L) + " / " + relPath + " //\n";
					out.write(line);
				}
			}
		} catch (Exception ignored) {
		} finally {
			try {
				out.close();
			} catch (Exception ignored) {
			}
		}
	}
}
