package com.n8k9.ti_elbow.ti_elbow;

import android.app.Activity;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.Vibrator;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.util.Arrays;
import java.util.UUID;

public class CalibrationActivity extends Activity {

    TextView textView;

    RangeSeekBar<Integer> rangeSeekBar;// = new RangeSeekBar<Integer>(this);

    Button btnUp_LowerLimit, btnDown_LowerLimit;
    Button btnUp_UpperLimit, btnDown_UpperLimit;
    Button btn_Transmit, btn_Receive, btn_Disconnect;

    TextView txt_Timer;

    String Line_1;
    String Line_2;
    String Line_3;
    String Line_4;
    String Line_5;

    private int LowerLimit;
    private int UpperLimit;

    public Vibrator v;

    String address = null;
    private ProgressDialog progress;
    BluetoothAdapter myBluetooth = null;
    BluetoothSocket btSocket = null;
    private boolean isBtConnected = false;

    //SPP UUID. Look for it
    static final UUID myUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        Intent newint = getIntent();
        address = newint.getStringExtra(BluetoothActivity.EXTRA_ADDRESS); //receive the address of the bluetooth device

        // Setup Layout View
        setContentView(R.layout.calibration_activity);



        // Setup RangeSeekBar
        //RangeSeekBar<Integer> rangeSeekBar = new RangeSeekBar<Integer>(this);
        rangeSeekBar = new RangeSeekBar<Integer>(this);
        // Set the range
        rangeSeekBar.setRangeValues(0, 180);
        rangeSeekBar.setSelectedMinValue(60);
        rangeSeekBar.setSelectedMaxValue(120);

        // Add to Layout
        LinearLayout layout = (LinearLayout) findViewById(R.id.seekbar2);
        layout.addView(rangeSeekBar);
        //layout.setBackgroundColor(Color.BLACK);




        // Call Widgets
        btnUp_LowerLimit = (Button)findViewById(R.id.button_up_LowerLimit);
        btnDown_LowerLimit = (Button)findViewById(R.id.button_down_LowerLimit);

        btnUp_UpperLimit = (Button)findViewById(R.id.button_up_UpperLimit);
        btnDown_UpperLimit = (Button)findViewById(R.id.button_down_UpperLimit);

        btn_Transmit = (Button)findViewById(R.id.buttonTransmit);
        btn_Receive = (Button)findViewById(R.id.buttonReceive);
        btn_Disconnect = (Button)findViewById(R.id.buttonDisconnect);

        txt_Timer = (TextView)findViewById(R.id.textViewTimer);

        Line_1="Enter Angle Limits\n";
        Line_2="Between 0* and 180*\n";
        Line_3="\n";
        Line_4="Press Transmit\n";
        Line_5="to Start Calibration\n";

        txt_Timer.setText(Line_1+Line_2+Line_3+Line_4+Line_5);

        // Initialize Vibration
        v = (Vibrator) this.getSystemService(Context.VIBRATOR_SERVICE);

        new ConnectBT().execute(); //Call the class to connect


        // Angle Limit Commands
        btnUp_LowerLimit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                LowerLimit = rangeSeekBar.getSelectedMinValue();
                UpperLimit = rangeSeekBar.getSelectedMaxValue();
                if (LowerLimit < 180 && LowerLimit < UpperLimit)
                    rangeSeekBar.setSelectedMinValue(LowerLimit + 1);
            }
        });

        btnDown_LowerLimit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                LowerLimit=rangeSeekBar.getSelectedMinValue();
                UpperLimit=rangeSeekBar.getSelectedMaxValue();
                if(LowerLimit>0 && LowerLimit<UpperLimit) rangeSeekBar.setSelectedMinValue(LowerLimit-1);
            }
        });

        btnUp_UpperLimit.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                LowerLimit=rangeSeekBar.getSelectedMinValue();
                UpperLimit=rangeSeekBar.getSelectedMaxValue();
                if(UpperLimit<180 && UpperLimit>LowerLimit) rangeSeekBar.setSelectedMaxValue(UpperLimit + 1);
            }
        });

        btnDown_UpperLimit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                LowerLimit=rangeSeekBar.getSelectedMinValue();
                UpperLimit=rangeSeekBar.getSelectedMaxValue();
                if(UpperLimit>0 && UpperLimit>LowerLimit) rangeSeekBar.setSelectedMaxValue(UpperLimit - 1);
            }
        });

        btn_Transmit.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                Transmit(); //close connection
            }
        });

        btn_Receive.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                Receive(); //close connection
            }
        });

        btn_Disconnect.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                Disconnect(); //close connection
            }
        });


    }

    private void Receive()
    {
        if (btSocket!=null)
        {
            try
            {
                // For Reading
                byte[] buffer = new byte[1024];
                int bytes;

                bytes = btSocket.getInputStream().read(buffer);

                String value = new String(buffer, "UTF-8");

                msg("Read: "+value);


            }
            catch (IOException e)
            {
                msg("Error");
            }
        }
    }


    private void Transmit()
    {
//        timer_sequence();

        if (btSocket!=null)
        {
            try
            {
                // For Writing
                LowerLimit=rangeSeekBar.getSelectedMinValue();
                UpperLimit=rangeSeekBar.getSelectedMaxValue();

                String string_LowerLimit = String.format("%03d",LowerLimit);
                String string_UpperLimit = String.format("%03d",UpperLimit);
                String string_Transmit = "a" + string_LowerLimit + string_UpperLimit + "q";//string_LowerLimit + string_UpperLimit;
                btSocket.getOutputStream().write(string_Transmit.toString().getBytes());
                msg("Sent: "+string_LowerLimit+"  "+string_UpperLimit);

               timer_sequence();
            }
            catch (IOException e)
            {
                msg("Error");
            }
        }

    }


    private void timer_sequence()
    {

        v.vibrate(500);
        // First Timer
        new CountDownTimer(5000, 200){
            public void onTick(long millisUntilFinished){

//                long buzz_pattern[] = new long[2];
//                Arrays.fill(buzz_pattern, 100);
//                v.vibrate(buzz_pattern,-1);

                Line_1="Prepare to Flex\n";
                Line_2="Forearm at 90* in\n";
                Line_3=Long.toString(millisUntilFinished/1000)+"   Seconds\n";
                Line_4="\n";
                Line_5="";

                txt_Timer.setText(Line_1+Line_2+Line_3+Line_4+Line_5);
            }

            public void onFinish() {


                v.vibrate(500);
                // Second Timer
                new CountDownTimer(10000, 500){
                    public void onTick(long millisUntilFinished){
//                        long buzz_pattern[] = new long[2];
//                        Arrays.fill(buzz_pattern, 250);
//                        v.vibrate(buzz_pattern,-1);

                        Line_1="Flex Bicep for\n";
                        Line_2=Long.toString(millisUntilFinished/1000)+"   Seconds\n";
                        Line_3="\n";
                        Line_4="\n";
                        Line_5="";

                        txt_Timer.setText(Line_1+Line_2+Line_3+Line_4+Line_5);
                    }

                    public void onFinish() {

//                        long buzz_pattern[] = new long[4];
//                        Arrays.fill(buzz_pattern, 1000);
//                        v.vibrate(buzz_pattern,-1);

                        txt_Timer.setText("Calibration Complete");
                    }

                    //txt_Timer.setText("done!");

                }.start();

                //txt_Timer.setText("done!");
            }
        }.start();
    }

    private void Disconnect()
    {
        if (btSocket!=null) //If the btSocket is busy
        {
            try
            {
                btSocket.close(); //close connection
            }
            catch (IOException e)
            { msg("Error");}
        }
        finish(); //return to the first layout
    }

    // fast way to call Toast
    private void msg(String s)
    {
        Toast.makeText(getApplicationContext(), s, Toast.LENGTH_LONG).show();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_calibration, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    private class ConnectBT extends AsyncTask<Void, Void, Void>  // UI thread
    {
        private boolean ConnectSuccess = true; //if it's here, it's almost connected

        @Override
        protected void onPreExecute()
        {
            progress = ProgressDialog.show(CalibrationActivity.this, "Connecting...", "Please wait!!!");  //show a progress dialog
        }

        @Override
        protected Void doInBackground(Void... devices) //while the progress dialog is shown, the connection is done in background
        {
            try
            {
                if (btSocket == null || !isBtConnected)
                {
                    myBluetooth = BluetoothAdapter.getDefaultAdapter();//get the mobile bluetooth device
                    BluetoothDevice dispositivo = myBluetooth.getRemoteDevice(address);//connects to the device's address and checks if it's available
                    btSocket = dispositivo.createInsecureRfcommSocketToServiceRecord(myUUID);//create a RFCOMM (SPP) connection
                    BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
                    btSocket.connect();//start connection
                }
            }
            catch (IOException e)
            {
                ConnectSuccess = false;//if the try failed, you can check the exception here
            }
            return null;
        }
        @Override
        protected void onPostExecute(Void result) //after the doInBackground, it checks if everything went fine
        {
            super.onPostExecute(result);

            if (!ConnectSuccess)
            {
                msg("Connection Failed. Is it a SPP Bluetooth? Try again.");
                finish();
            }
            else
            {
                msg("Connected.");
                isBtConnected = true;
            }
            progress.dismiss();
        }
    }

}
