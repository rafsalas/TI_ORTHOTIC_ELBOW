package com.n8k9.imu_video.inertial_video;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.widget.Button;
import android.view.View;
import android.view.View.OnClickListener;

public class MainActivity extends Activity {

    Button button1; //Bluetooth
    Button button2; //Accelerometer
    Button button3; //Sensor List
    Button button4; //Storage
    Button button5; //Bluetooth P2P
    Button button6; //Video
    Button button7; //Video and Accelerometer
    Button button8; //Video, Accelerometer, File Saving



    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        addListenerOnButton();
    }

    public void addListenerOnButton() {

        final Context context = this;

        button1 = (Button) findViewById(R.id.button1);
        button2 = (Button) findViewById(R.id.button2);
        button3 = (Button) findViewById(R.id.button3);
        button4 = (Button) findViewById(R.id.button4);
        button5 = (Button) findViewById(R.id.button5);
        button6 = (Button) findViewById(R.id.button6);
        button7 = (Button) findViewById(R.id.button7);
        button8 = (Button) findViewById(R.id.button8);



        button1.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                Intent intent = new Intent(context, BluetoothActivity.class);
                startActivity(intent);
            }

        });

        button2.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                Intent intent = new Intent(context, AccelerometerActivity.class);
                startActivity(intent);
            }

        });

        button3.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                Intent intent = new Intent(context, SensorListActivity.class);
                startActivity(intent);
            }

        });

        button4.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                Intent intent = new Intent(context, StorageActivity.class);
                startActivity(intent);
            }

        });

        button5.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                Intent intent = new Intent(context, BluetoothP2PActivity.class);
                startActivity(intent);
            }

        });

        button6.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                Intent intent = new Intent(context, VideoActivity.class);
                startActivity(intent);
            }

        });

        button7.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                Intent intent = new Intent(context, Video_Accelerometer.class);
                startActivity(intent);
            }

        });

        button8.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                Intent intent = new Intent(context, Video_Accelerometer_File.class);
                startActivity(intent);
            }

        });

    }

}