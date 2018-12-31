package com.coronalabs.corona.sample.extendingui;


/** Displays a toolbar providing color buttons for changing the app's background color. */
public class ToolBar extends android.widget.RelativeLayout {
	/** Button for changing the Corona background color to black. */
	private android.widget.ToggleButton fBlackButton;
	
	/** Button for changing the Corona background color to white. */
	private android.widget.ToggleButton fWhiteButton;
	
	/** Button for changing the Corona background color to red. */
	private android.widget.ToggleButton fRedButton;
	
	/** Button for changing the Corona background color to green. */
	private android.widget.ToggleButton fGreenButton;
	
	/** Button for changing the Corona background color to blue. */
	private android.widget.ToggleButton fBlueButton;
	
	
	/**
	 * Creates a new toolbar view.
	 * @param context The context this view will be associated with. Typically the activity context. Cannot be null.
	 */
	public ToolBar(android.content.Context context) {
		super(context);
		
		// Set the layout for this toolbar to fill the width of its parent view.
		android.widget.RelativeLayout.LayoutParams layoutParams;
		layoutParams = new android.widget.RelativeLayout.LayoutParams(
								android.widget.RelativeLayout.LayoutParams.MATCH_PARENT,
								android.widget.RelativeLayout.LayoutParams.WRAP_CONTENT);
		setLayoutParams(layoutParams);
		
		// Set up the background to display a dark gradient background color.
		android.graphics.drawable.GradientDrawable backgroundDrawable;
		backgroundDrawable = new android.graphics.drawable.GradientDrawable(
					android.graphics.drawable.GradientDrawable.Orientation.TOP_BOTTOM,
					new int[] { android.graphics.Color.DKGRAY ,android.graphics.Color.BLACK });
		setBackgroundDrawable(backgroundDrawable);
		
		// Set up a linear layout to contain the toolbar's buttons.
		// This linear layout will be centered within this toolbar.
		android.widget.LinearLayout linearLayout = new android.widget.LinearLayout(getContext());
		layoutParams = new android.widget.RelativeLayout.LayoutParams(
								android.widget.RelativeLayout.LayoutParams.WRAP_CONTENT,
								android.widget.RelativeLayout.LayoutParams.WRAP_CONTENT);
		layoutParams.addRule(android.widget.RelativeLayout.CENTER_IN_PARENT);
		linearLayout.setLayoutParams(layoutParams);
		linearLayout.setPadding(0, 8, 0, 8);
		linearLayout.setGravity(android.view.Gravity.CENTER);
		addView(linearLayout);
		
		// Create a button click listener to be shared by all of the below color buttons.
		ColorButtonEventHandler buttonListener = new ColorButtonEventHandler();
		
		// Set up the "Black" button.
		fBlackButton = new android.widget.ToggleButton(getContext());
		fBlackButton.setTag(Integer.valueOf(android.graphics.Color.BLACK));
		fBlackButton.setText("Black");
		fBlackButton.setTextOn(fBlackButton.getText());
		fBlackButton.setTextOff(fBlackButton.getText());
		fBlackButton.setOnClickListener(buttonListener);
		linearLayout.addView(fBlackButton);
		
		// Set up the "White" button. Select it by default.
		fWhiteButton = new android.widget.ToggleButton(getContext());
		fWhiteButton.setTag(Integer.valueOf(android.graphics.Color.WHITE));
		fWhiteButton.setText("White");
		fWhiteButton.setTextOn(fWhiteButton.getText());
		fWhiteButton.setTextOff(fWhiteButton.getText());
		fWhiteButton.setOnClickListener(buttonListener);
		fWhiteButton.setChecked(true);
		linearLayout.addView(fWhiteButton);
		
		// Set up the "Red" button.
		fRedButton = new android.widget.ToggleButton(getContext());
		fRedButton.setTag(Integer.valueOf(android.graphics.Color.RED));
		fRedButton.setText("Red");
		fRedButton.setTextOn(fRedButton.getText());
		fRedButton.setTextOff(fRedButton.getText());
		fRedButton.setOnClickListener(buttonListener);
		linearLayout.addView(fRedButton);
		
		// Set up the "Green" button.
		fGreenButton = new android.widget.ToggleButton(getContext());
		fGreenButton.setTag(Integer.valueOf(android.graphics.Color.GREEN));
		fGreenButton.setText("Green");
		fGreenButton.setTextOn(fGreenButton.getText());
		fGreenButton.setTextOff(fGreenButton.getText());
		fGreenButton.setOnClickListener(buttonListener);
		linearLayout.addView(fGreenButton);
		
		// Set up the "Blue" button.
		fBlueButton = new android.widget.ToggleButton(getContext());
		fBlueButton.setTag(Integer.valueOf(android.graphics.Color.BLUE));
		fBlueButton.setText("Blue");
		fBlueButton.setTextOn(fBlueButton.getText());
		fBlueButton.setTextOff(fBlueButton.getText());
		fBlueButton.setOnClickListener(buttonListener);
		linearLayout.addView(fBlueButton);
	}
	
	
	/** Handles click/tap events from a color button. */
	private class ColorButtonEventHandler implements android.view.View.OnClickListener {
		@Override
		public void onClick(android.view.View view) {
			// Check the toggle button that matches the given color and uncheck the other toggle.
			// Don't allow a toggle button to be unchecked if it was tapped twice.
			fBlackButton.setChecked(fBlackButton == view);
			fWhiteButton.setChecked(fWhiteButton == view);
			fRedButton.setChecked(fRedButton == view);
			fGreenButton.setChecked(fGreenButton == view);
			fBlueButton.setChecked(fBlueButton == view);
			
			// Fetch the color from the button's tag.
			final int backgroundColor = ((Integer)view.getTag()).intValue();
			
			// Update Corona's background color.
			// This is done by sending a task to be executed by the Corona runtime thread.
			com.ansca.corona.CoronaActivity activity = com.ansca.corona.CoronaEnvironment.getCoronaActivity();
			if (activity != null) {
				activity.getRuntimeTaskDispatcher().send(new com.ansca.corona.CoronaRuntimeTask() {
					@Override
					public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
						// *** We are now running on the Corona runtime thread. ***
						
						// Fetch the Corona runtime's Lua state.
						com.naef.jnlua.LuaState luaState = runtime.getLuaState();
						
						// Call Lua function:  display.setDefault("background", red, green, blue)
						// We'll do this by fetching the Lua "display" module and then fetch that module's setDefault() function.
						// Arguments are passed to the function via the Lua state's push() methods.
						luaState.getGlobal("display");
						luaState.getField(-1, "setDefault");
						luaState.pushString("background");
						luaState.pushInteger(android.graphics.Color.red(backgroundColor));
						luaState.pushInteger(android.graphics.Color.green(backgroundColor));
						luaState.pushInteger(android.graphics.Color.blue(backgroundColor));
						luaState.call(4, 0);
						luaState.pop(1);
					}
				});
			}
		}
	}
}
