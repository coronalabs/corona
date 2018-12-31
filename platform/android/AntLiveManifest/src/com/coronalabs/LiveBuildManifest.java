//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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
