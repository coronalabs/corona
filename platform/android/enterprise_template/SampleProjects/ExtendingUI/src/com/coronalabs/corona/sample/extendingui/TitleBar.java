package com.coronalabs.corona.sample.extendingui;


/** Displays a title bar to the user. */
public class TitleBar extends android.widget.RelativeLayout {
	/**
	 * Creates a new title bar view.
	 * @param context The context this view will be associated with. Typically the activity context. Cannot be null.
	 */
	public TitleBar(android.content.Context context) {
		super(context);
		
		// Set the layout for this title bar to fill the width of its parent view.
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
		
		// Set up a text view that will display title text centered within the title bar.
		android.widget.TextView titleText = new android.widget.TextView(getContext());
		titleText.setPadding(8, 8, 8, 8);
		titleText.setGravity(android.view.Gravity.CENTER);
		titleText.setText("Corona Sample Application");
		titleText.setTypeface(android.graphics.Typeface.DEFAULT_BOLD);
		titleText.setTextColor(android.graphics.Color.rgb(255, 165, 0));
		titleText.setBackgroundColor(android.graphics.Color.TRANSPARENT);
		layoutParams = new android.widget.RelativeLayout.LayoutParams(
								android.widget.RelativeLayout.LayoutParams.WRAP_CONTENT,
								android.widget.RelativeLayout.LayoutParams.WRAP_CONTENT);
		layoutParams.addRule(android.widget.RelativeLayout.CENTER_IN_PARENT);
		addView(titleText, layoutParams);
		
		// Set up an "About" button right justified on the title bar.
		// This will display application information to the user via an alert dialog.
		android.widget.ImageButton aboutButton = new android.widget.ImageButton(getContext());
		aboutButton.setBackgroundColor(android.graphics.Color.TRANSPARENT);
		aboutButton.setImageResource(R.drawable.ic_dialog_info);
		aboutButton.setOnClickListener(new android.view.View.OnClickListener() {
			@Override
			public void onClick(android.view.View view) {
				android.app.AlertDialog.Builder builder;
				builder = new android.app.AlertDialog.Builder(view.getContext());
				builder.setIcon(R.drawable.ic_dialog_info);
				builder.setTitle("About");
				builder.setMessage("This sample application demonstrates how to add your own " +
				                   "custom UI on top of the main Corona view.");
				builder.show();
			}
		});
		layoutParams = new android.widget.RelativeLayout.LayoutParams(
								android.widget.RelativeLayout.LayoutParams.WRAP_CONTENT,
								android.widget.RelativeLayout.LayoutParams.WRAP_CONTENT);
		layoutParams.addRule(android.widget.RelativeLayout.ALIGN_PARENT_RIGHT);
		addView(aboutButton, layoutParams);
	}
}
