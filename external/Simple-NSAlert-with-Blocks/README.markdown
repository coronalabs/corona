Super simple changes to the OmniGroups NSAlert extensions.

It always amazes me how much work it is to put up a alert before you do something 'dangerous' 
For god sakes, you just want to give them a chance to cancel!

This is a super simple modification to the OmniGroup's NSAlert extensions.  With this you get
a single c function that launches a alert -- You pass it a block to decide what to do.

In my case I usually only check for the 'OK' button being pressed and then proceed with the task 
but obviously you can do whatever you like.

Example:

    OABeginAlertSheet(@"Are you Sure?"
      		      	, @"ok", @"cancel", nil, [sender window], ^(NSAlert*aleret,NSInteger code){
    		    			  NSLog(@"got return code %d",code);
    				  }, @"Please Confirm you really want to delete");


This makes runing NSAlerts dead easy --