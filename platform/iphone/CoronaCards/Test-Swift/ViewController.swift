//
//  ViewController.swift
//  CoronaCardsTest-Swift
//
//  Created by Walter Luh on 12/7/14.
//
//

import UIKit

class ViewController : UIViewController
{
	var coronaController : CoronaViewController!

	override func viewDidLoad()
	{
		super.viewDidLoad()
		// Do any additional setup after loading the view, typically from a nib.

		self.coronaController = CoronaViewController()
		self.addChildViewController(self.coronaController)

		var parent : UIView = self.view
		var coronaView = self.coronaController.view as CoronaView
		
		// Resize and center CoronaView
		var rect = parent.frame
		rect.size.width *= 0.5
		rect.size.height *= 0.5
		coronaView.frame = rect // Half the size of the parent
		coronaView.center = parent.center // Position at center

		parent.addSubview(coronaView)

		coronaView.run()


//let event: [NSObject: AnyObject] = ["hi": 3, "name": "myEvent", ]
//var result: AnyObject? = coronaView.sendEvent( event );

	}

	override func didReceiveMemoryWarning()
	{
		super.didReceiveMemoryWarning()
		// Dispose of any resources that can be recreated.
	}


}

