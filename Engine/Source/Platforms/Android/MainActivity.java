package com.example.myapp;

import android.app.Activity;
import android.widget.TextView;
import android.os.Bundle;

public class MainActivity extends Activity {

    // Used to load the "native-lib" library on application startup.
	static {
		System.loadLibrary("native-lib");
	}

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Example of a call to a native method.
        TextView  tv = new TextView(this);
        tv.setText(stringFromJNI());

        setContentView(tv);
    }

	public native String stringFromJNI();
}
