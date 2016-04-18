package com.n8k9.ti_elbow.ti_elbow;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.LinearLayout;


public class MainActivity extends Activity {

    Button button1; // Bluetooth Activity

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        addListenerOnButton();


//        // Setup the new range seek bar
//        RangeSeekBar<Integer> rangeSeekBar = new RangeSeekBar<Integer>(this);
//        // Set the range
//        rangeSeekBar.setRangeValues(0, 180);
//        rangeSeekBar.setSelectedMinValue(60);
//        rangeSeekBar.setSelectedMaxValue(120);
//
//        // Add to layout
//        LinearLayout layout = (LinearLayout) findViewById(R.id.seekbar_placeholder);
//        layout.addView(rangeSeekBar);
//        //layout.setBackgroundColor(Color.BLACK);

    }

    public void addListenerOnButton() {

        final Context context = this;

        button1 = (Button) findViewById(R.id.button1);


        button1.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                Intent intent = new Intent(context, BluetoothActivity.class);
                //Intent intent = new Intent(context, CalibrationActivity.class);
                startActivity(intent);
            }

        });

    }

}