//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "TextEditorSupport.h"

NSString* getDirNameWithMainLua(NSString* file_name)
{
	NSString *ret = file_name;
	NSFileManager *fm = [NSFileManager defaultManager];
	BOOL isDir;
	while( (ret = [ret stringByDeletingLastPathComponent]) ) {
		if([ret isEqualToString:@"/"] || [ret isEqualToString:@""] )
			return nil;
		if( [fm fileExistsAtPath:[ret stringByAppendingPathComponent:@"main.lua"] isDirectory:&isDir] ) {
			if(!isDir)
				return ret;
		}
	}
	return nil;
}

void TextEditorSupport_LaunchTextEditorWithFile(NSString* file_name, NSInteger line_number)
{
	NSString* app_path = nil;
	NSString* file_extension = nil;
	NSString* launchTextEditorWithFile = [[NSUserDefaults standardUserDefaults] stringForKey:@"launchTextEditorWithFile"];

    // NSLog(@"TextEditorSupport_LaunchTextEditorWithFile: '%@', %ld", file_name, line_number);

	if (launchTextEditorWithFile != nil && [launchTextEditorWithFile length] > 0)
	{
		// We have a user preference which is a template for launching a text editor
		NSString* line_number_string = [NSString stringWithFormat:@"%ld", line_number];

        if(line_number == 0)
        {
            line_number = 1;
        }
        
		launchTextEditorWithFile = [launchTextEditorWithFile stringByReplacingOccurrencesOfString:@"{{FILENAME}}" withString:file_name];
		launchTextEditorWithFile = [launchTextEditorWithFile stringByReplacingOccurrencesOfString:@"{{LINENUMBER}}" withString:line_number_string];
		
		NSMutableArray *cmdArgs = [[launchTextEditorWithFile componentsSeparatedByString:@" "] mutableCopy];
		NSString *cmd = [cmdArgs objectAtIndex:0];
		[cmdArgs removeObjectAtIndex:0];
		
		@try
		{
			// The try/catch should protect us from garbage in the user default
			[NSTask launchedTaskWithLaunchPath:cmd arguments:cmdArgs];
		}
		@catch(NSException* the_exception)
		{
			[[NSWorkspace sharedWorkspace] openFile:file_name];
		}
		
		return;
	}

	// Get the path of the application configured to open files of this type
	BOOL ret_flag = [[NSWorkspace sharedWorkspace] getInfoForFile:file_name application:&app_path type:&file_extension];
	
	if(YES == ret_flag)
	{
		//		NSLog(@"file_name=%@, ext=%@", app_path, file_extension);
		// Technical Q&A QA1544 http://developer.apple.com/library/mac/#qa/qa2007/qa1544.html
		// I'm not completely sure what getInfoForFile returns so this might need to be retested if we localize the Corona Simulator name.
		NSString* this_app_name = [[NSFileManager defaultManager] displayNameAtPath:[[NSBundle mainBundle] bundlePath]];
		
        // NSLog(@"TextEditorSupport_LaunchTextEditorWithFile: this_app_name=%@", this_app_name);

		NSString* base_app_name = [app_path lastPathComponent];
		if([base_app_name isEqualToString:@"Xcode.app"])
		{
			// FIXME: Should try to dynamically find correct path for xed
			// FIXME: As of 2013-02 the line number arg doesn't actually work on xed

            if(line_number == 0)
            {
                line_number = 1;
            }
            
			NSString* line_number_string_arg = [NSString stringWithFormat:@"%ld", line_number];
			@try
			{
				[NSTask launchedTaskWithLaunchPath:@"/usr/bin/xed" arguments:[NSArray arrayWithObjects:@"-l", line_number_string_arg, file_name, nil]];
			}
			@catch(NSException* the_exception)
			{
				[[NSWorkspace sharedWorkspace] openFile:file_name];
			}
		}
		else if([base_app_name isEqualToString:@"TextMate.app"])
		{
			// TextMate's URL scheme found here:
			// http://blog.macromates.com/2007/the-textmate-url-scheme/
			
            if(line_number == 0)
            {
                line_number = 1;
            }
            
			@try
			{
				NSString* url_string = [NSString stringWithFormat:@"txmt://open?url=file://%@&line=%ld", file_name, line_number];
				url_string = [url_string stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
				NSURL* textmate_url = [NSURL URLWithString:url_string];
				[[NSWorkspace sharedWorkspace] openURL:textmate_url];
			}
			@catch(NSException* the_exception)
			{
				[[NSWorkspace sharedWorkspace] openFile:file_name];
			}
		}
		else if([base_app_name isEqualToString:@"Sublime Text 2.app"] || [base_app_name isEqualToString:@"Sublime Text.app"] || [base_app_name isEqualToString:@"Sublime Text 3.app"] )
		{
			// From http://www.sublimetext.com/docs/2/osx_command_line.html
            
            // It turns out that the Sublime Text "subl" command line processor is horribly unreliable unless the
            // "--wait" option is used.  It doesn't affect our use of the tool so we add it just to make opening
            // files work well (though we also have to terminate the task ourselves as it seems each subl hangs
            // around forever otherwise).  We also use the "--add" option to put the file's siblings in Sublime
            // Text's sidebar (ST3 works better than ST2 doing this).
			
			NSString* filename_with_line_number_arg = nil;
            NSString* dirname = getDirNameWithMainLua(file_name);
            NSString* addarg = @"--add";
			NSString* sublime_path = [NSString stringWithFormat:@"%@/Contents/SharedSupport/bin/subl", app_path];

            // If we get 0 for the line number, don't try to set it at all which has the effect of having Sublime Text
            // leave the cursor on whichever line it's currently on (or put it on line 1 if the file isn't known)
            if (line_number == 0)
            {
                filename_with_line_number_arg = [NSString stringWithFormat:@"%@", file_name];
            }
            else
            {
                filename_with_line_number_arg = [NSString stringWithFormat:@"%@:%ld", file_name, line_number];
            }
            
			@try
			{
                NSMutableArray* args = [NSMutableArray arrayWithObject:@"--wait"];
                if ([dirname length] > 0)
                {
                    [args addObject:addarg];
                    [args addObject:dirname];
                }
                [args addObject:filename_with_line_number_arg];
                
                // NSLog(@"sublime_path '%@', args %@", sublime_path, args);
                
                NSTask *sublTask = [[[NSTask alloc] init] autorelease];
                
                [sublTask setLaunchPath:sublime_path];
                [sublTask setArguments:args];
                [sublTask launch];
                // The subl subprocess hangs around unless we terminate it so schedule that
                [sublTask performSelector:@selector(terminate) withObject:nil afterDelay:3.0];
			}
			@catch(NSException* the_exception)
			{
				[[NSWorkspace sharedWorkspace] openFile:file_name];
			}
		}
		else if([base_app_name isEqualToString:@"Visual Studio Code.app"])
		{
			// code --help
			@try
			{
				NSString* dirname = getDirNameWithMainLua(file_name);
				NSString* execPath = [NSString stringWithFormat:@"%@/Contents/Resources/app/bin/code", app_path];

				NSMutableArray* args = [[[NSMutableArray alloc] init] autorelease];
				
				[args addObject:@"-r"];
				
				if ([dirname length] > 0)
				{
					[args addObject:@"--add"];
					[args addObject:dirname];
				}

				if(line_number>0)
				{
					[args addObject:@"--goto"];
					[args addObject:[NSString stringWithFormat:@"%@:%ld", file_name, line_number]];
				}

				[args addObject:file_name];

				NSTask *task = [[[NSTask alloc] init] autorelease];

				[task setLaunchPath:execPath];
				[task setArguments:args];
				[task launch];
			}
			@catch(NSException* the_exception)
			{
				[[NSWorkspace sharedWorkspace] openFile:file_name];
			}
		}
		else if([base_app_name isEqualToString:@"Atom.app"])
		{
			// atom --help
			@try
			{
				NSString* dirname = getDirNameWithMainLua(file_name);
				NSString* execPath = [NSString stringWithFormat:@"%@/Contents/Resources/app/atom.sh", app_path];

				NSMutableArray* args = [NSMutableArray arrayWithCapacity:5];

				if ([dirname length] > 0)
				{
					[args addObject:dirname];
				}

				NSString *filename_with_line_number_arg = file_name;
				if (line_number)
				{
					filename_with_line_number_arg = [NSString stringWithFormat:@"%@:%ld", file_name, line_number];
				}
				[args addObject:filename_with_line_number_arg];

				NSTask *task = [[[NSTask alloc] init] autorelease];

				[task setLaunchPath:execPath];
				[task setArguments:args];
				[task launch];
				[task performSelector:@selector(terminate) withObject:nil afterDelay:3.0];
			}
			@catch(NSException* the_exception)
			{
				[[NSWorkspace sharedWorkspace] openFile:file_name];
			}
		}
		else if([base_app_name isEqualToString:@"TextWrangler.app"])
		{
			// FIXME: Should try to dynamically find correct path for TextEdit's command line tool
            if(line_number == 0)
            {
                line_number = 1;
            }
            
			NSString* line_number_string_arg = [NSString stringWithFormat:@"+%ld", line_number];
			
			@try
			{
				[NSTask launchedTaskWithLaunchPath:@"/usr/local/bin/edit" arguments:[NSArray arrayWithObjects:line_number_string_arg, file_name, nil]];
			}
			@catch(NSException* the_exception)
			{
				[[NSWorkspace sharedWorkspace] openFile:file_name];
			}
		}
		else if([base_app_name isEqualToString:@"BBEdit.app"])
		{
			// FIXME: Should try to dynamically find correct path for xed
            if(line_number == 0)
            {
                line_number = 1;
            }
            
			NSString* line_number_string_arg = [NSString stringWithFormat:@"+%ld", line_number];

			@try
			{
				[NSTask launchedTaskWithLaunchPath:@"/usr/local/bin/bbedit" arguments:[NSArray arrayWithObjects:line_number_string_arg, file_name, nil]];
			}
			@catch(NSException* the_exception)
			{
				[[NSWorkspace sharedWorkspace] openFile:file_name];
			}
		}
		else if([base_app_name isEqualToString:@"MacVim.app"])
		{
			// Found better way asking on the MacVim mailing list.
			// This solves the already open problem.
			//	mvim://open?url=file:///Users/ewing/TEMP/fee.lua&line=3
			// see ":h mvim://"
			// This is based on TextMate's URL scheme found here:
			// http://blog.macromates.com/2007/the-textmate-url-scheme/
			// Bug: 3426. If there is a space in the path, the path needs to be escaped with %20.
			// Unfortunately, even after I do this, MacVim seems to have a bug that doesn't handle this case properly.
			// I will file a bug with them.
            if(line_number == 0)
            {
                line_number = 1;
            }
            
			NSString* url_string = [NSString stringWithFormat:@"mvim://open?url=file://%@&line=%ld", file_name, line_number];
			url_string = [url_string stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
			NSURL* mvim_url = [NSURL URLWithString:url_string];
			[[NSWorkspace sharedWorkspace] openURL:mvim_url];	
		}
		// If our app is bound to Lua files as the default app, we want to redirect to something else since we currently don't have a built-in editor.
		// Note: this_app_name is "Corona Simulator" while base_app_name is "Corona Simulator.app", so we use hasPrefix for simplicity.
		else if([base_app_name hasPrefix:this_app_name])
		{
			[[NSWorkspace sharedWorkspace] openFile:file_name withApplication:@"TextEdit.app"];
		}
		else
		{
			[[NSWorkspace sharedWorkspace] openFile:file_name];
		}
	}
	else
	{
		[[NSWorkspace sharedWorkspace] openFile:file_name withApplication:@"TextEdit.app"];	
	}
}
