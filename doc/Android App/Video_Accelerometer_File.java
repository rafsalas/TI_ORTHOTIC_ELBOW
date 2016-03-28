package com.n8k9.imu_video.inertial_video;

import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.CamcorderProfile;
import android.media.MediaRecorder;
import android.os.Bundle;
import android.os.Handler;
import android.os.Vibrator;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.util.Arrays;
import java.util.Timer;
import java.util.TimerTask;


public class Video_Accelerometer_File extends Activity implements SensorEventListener {
    // CAMERA Variables
    private Camera mCamera;
    private CameraPreview mPreview;
    private MediaRecorder mediaRecorder;
    private Button capture, switchCamera;
    private Context myContext;
    private LinearLayout cameraPreview;
    private boolean cameraFront = false;

    // ACCELEROMETER Variables
    private float lastX, lastY, lastZ;
    private SensorManager sensorManager;
    private Sensor accelerometer;
    private Sensor gyroscope;
    private float deltaX = 0;
    private float deltaY = 0;
    private float deltaZ = 0;
    private float vibrateThreshold = 0;
    private TextView currentT;
    private TextView currentX, currentY, currentZ;
    private TextView currentGyroX, currentGyroY, currentGyroZ;

    // VIBRATOR
    public Vibrator v;
    boolean cont_vibrate=true;
    private Button upButton, downButton;
    private TextView vibrateCount;
    int vibrate_count=1;
    long pulse_duration=500; // Milliseconds


    // TIME VARIABLES
    Timer timer;
    TimerTask timerTask;

    final Handler handler = new Handler();
    /////////////////////////////

    // SENSOR VARIABLES
    // Angular Speeds from Gyroscope
    private float[] gyro = new float[3];

    // Rotation Matrix from Gyroscope Data
    private float[] gyroMatrix = new float[9];

    // Orientation Angles from Gyroscope Matrix
    private float[] gyroOrientation = new float[3];

    // Magnetic Field Vector
    private float[] magnet = new float[3];

    // Accelerometer Vector
    private float[] accel = new float[3];

    // Orientation Angles from Accelerometer and Magnet
    private float[] accMagOrientation = new float[3];

    // Final Orientation Angles from Sensor Fusion
    private float[] fusedOrientation = new float[3];

    // Accelerometer and Magnetometer Based Rotation Matrix
    private float[] rotationMatrix = new float[9];


    // FILE VARIABLES
    EditText editTextFileName;
    Button saveButton;
    private int data_i = 0;
    private int data_i_max = 10000;
    private String [] data_x = new String[data_i_max];
    private String [] data_y = new String[data_i_max];
    private String [] data_z = new String[data_i_max];
    private String [] data_gyro_x = new String[data_i_max];
    private String [] data_gyro_y = new String[data_i_max];
    private String [] data_gyro_z = new String[data_i_max];
    private String [] data_magnet_x = new String[data_i_max];
    private String [] data_magnet_y = new String[data_i_max];
    private String [] data_magnet_z = new String[data_i_max];


    private String [] data_t = new String[data_i_max];

    //Timer
    private long data_period = 5;// 5 Milliseconds, 200 Hz
    private int max_time = 300; // Terminate Camera Data Storage (Seconds)
    private int cutoff_time = 60; // Terminate Sensor Data Storage (Seconds)
    private int max_size = 50; // Megabytes
    private double g = 9.8; // Gravitational Constant (m/s^2)
    private long start_time = System.currentTimeMillis();
    private long time_1=System.currentTimeMillis();


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.video_accelerometer_file);

        //Camera
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        myContext = this;
        initialize();

        // Initialize Storage
        for(int k=0;k<data_i_max;k++) {
            // TIME
            data_t[k] = "0.0";

            // ACCELERATION
            data_x[k] = "0.0";
            data_y[k] = "0.0";
            data_z[k] = "0.0";
            // ORIENTATION
            data_gyro_x[k] = "0.0";
            data_gyro_y[k] = "0.0";
            data_gyro_z[k] = "0.0";

            // Magnetometer
            data_magnet_x[k] = "0.0";
            data_magnet_y[k] = "0.0";
            data_magnet_z[k] = "0.0";
        }



        // Initialize Matrix Variables
        gyroOrientation[0] = 0.0f;
        gyroOrientation[1] = 0.0f;
        gyroOrientation[2] = 0.0f;

        // Initialize Gyroscope Matrix with Identity Matrix
        gyroMatrix[0] = 1.0f; gyroMatrix[1] = 0.0f; gyroMatrix[2] = 0.0f;
        gyroMatrix[3] = 0.0f; gyroMatrix[4] = 1.0f; gyroMatrix[5] = 0.0f;
        gyroMatrix[6] = 0.0f; gyroMatrix[7] = 0.0f; gyroMatrix[8] = 1.0f;

        // Initialize Sensor Data Display
        initializeViews();

        // Initialize Sensor Interrupts
        sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        initializeListeners();

        /*
        //Accelerometer
        if (sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER) != null) {
            // success! we have an accelerometer

            accelerometer = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
            sensorManager.registerListener(this, accelerometer, SensorManager.SENSOR_DELAY_NORMAL);
            vibrateThreshold = accelerometer.getMaximumRange() / 2;
        } else {
            // fai! we dont have an accelerometer!
        }

        //Gyroscope
        if (sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE) != null) {
            // success! we have a gyroscope

            gyroscope = sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
            sensorManager.registerListener(this, gyroscope, SensorManager.SENSOR_DELAY_NORMAL);
        } else {
            // fai! we dont have a gyroscope!
        }
        */


        //initialize vibration
        v = (Vibrator) this.getSystemService(Context.VIBRATOR_SERVICE);

        //File
        editTextFileName=(EditText)findViewById(R.id.editTextFileNameXML);

    }


    private int findFrontFacingCamera() {
        int cameraId = -1;
        // Search for the front facing camera
        int numberOfCameras = Camera.getNumberOfCameras();
        for (int i = 0; i < numberOfCameras; i++) {
            CameraInfo info = new CameraInfo();
            Camera.getCameraInfo(i, info);
            if (info.facing == CameraInfo.CAMERA_FACING_FRONT) {
                cameraId = i;
                cameraFront = true;
                break;
            }
        }
        return cameraId;
    }

    private int findBackFacingCamera() {
        int cameraId = -1;
        // Search for the back facing camera
        // get the number of cameras
        int numberOfCameras = Camera.getNumberOfCameras();
        // for every camera check
        for (int i = 0; i < numberOfCameras; i++) {
            CameraInfo info = new CameraInfo();
            Camera.getCameraInfo(i, info);
            if (info.facing == CameraInfo.CAMERA_FACING_BACK) {
                cameraId = i;
                cameraFront = false;
                break;
            }
        }
        return cameraId;
    }

    public void onResume() {
        super.onResume();

        //Accelerometer
        //sensorManager.registerListener(this, accelerometer, SensorManager.SENSOR_DELAY_NORMAL);
        ///////////////

        //Camera
        if (!hasCamera(myContext)) {
            Toast toast = Toast.makeText(myContext, "Sorry, your phone does not have a camera!", Toast.LENGTH_LONG);
            toast.show();
            finish();
        }
        if (mCamera == null) {
            // if the front facing camera does not exist
            if (findFrontFacingCamera() < 0) {
                Toast.makeText(this, "No front facing camera found.", Toast.LENGTH_LONG).show();
                switchCamera.setVisibility(View.GONE);
            }
            mCamera = Camera.open(findBackFacingCamera());
            mPreview.refreshCamera(mCamera);
        }
        ////////
    }


    public void startTimer() {
        timer = new Timer();

        initializeTimerTask();

        timer.schedule(timerTask, 0, data_period);
    }

    public void stoptimertask(View v) {
        if (timer != null) {
            timer.cancel();
            timer = null;
        }
    }

    public void initializeTimerTask() {

        timerTask = new TimerTask() {
            public void run() {

                //use a handler to run a toast that shows the current timestamp
                handler.post(new Runnable() {
                    public void run() {
                        /*
                        //get the current timeStamp
                        Calendar calendar = Calendar.getInstance();
                        SimpleDateFormat simpleDateFormat = new SimpleDateFormat("ss a");
                        final String strDate = simpleDateFormat.format(calendar.getTime());

                        //show the toast
                        int duration = Toast.LENGTH_SHORT;
                        Toast toast = Toast.makeText(getApplicationContext(), strDate, duration);
                        toast.show();
                        */

                        // FILE STORAGE
                        //Data Rate Condition
                        if (recording) {
                            time_1 = System.currentTimeMillis();

                            if (data_i < data_i_max && (double)((time_1 - start_time) / 1000)<cutoff_time) {
                                // TIME

                                data_t[data_i] = Double.toString(((double)(System.currentTimeMillis() - start_time) / 1000));

                                // ACCELERATION
                                data_x[data_i] = Double.toString((double)(accel[0]/g));
                                data_y[data_i] = Double.toString((double)(accel[1]/g));
                                data_z[data_i] = Double.toString((double)(accel[2]/g));
                                // ORIENTATION
                                data_gyro_x[data_i] = Double.toString((double)gyro[0]);
                                data_gyro_y[data_i] = Double.toString((double)gyro[1]);
                                data_gyro_z[data_i] = Double.toString((double)gyro[2]);

                                // Magnetometer
                                data_magnet_x[data_i] = Double.toString((double)magnet[0]);
                                data_magnet_y[data_i] = Double.toString((double)magnet[1]);
                                data_magnet_z[data_i] = Double.toString((double)magnet[2]);


                            }
                            // ELSE Do Nothing

                            data_i = data_i + 1;

                        }



                    }
                });
            }
        };
    }

    public void initialize() {
        cameraPreview = (LinearLayout) findViewById(R.id.camera_preview);

        mPreview = new CameraPreview(myContext, mCamera);
        cameraPreview.addView(mPreview);

        capture = (Button) findViewById(R.id.button_capture);
        capture.setOnClickListener(captrureListener);

        switchCamera = (Button) findViewById(R.id.button_ChangeCamera);
        switchCamera.setOnClickListener(switchCameraListener);

        upButton = (Button) findViewById(R.id.button_up);
        upButton.setOnClickListener(upButtonListener);

        downButton = (Button) findViewById(R.id.button_down);
        downButton.setOnClickListener(downButtonListener);



    }

    OnClickListener upButtonListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            if(vibrate_count<10)
            {
                vibrate_count=vibrate_count+1;
                vibrateCount.setText(Float.toString(vibrate_count));
            }
        }
    };

    OnClickListener downButtonListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            if(vibrate_count>1)
            {
                vibrate_count = vibrate_count - 1;
                vibrateCount.setText(Float.toString(vibrate_count));
            }
        }
    };


    OnClickListener switchCameraListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            // get the number of cameras
            if (!recording) {
                int camerasNumber = Camera.getNumberOfCameras();
                if (camerasNumber > 1) {
                    // release the old camera instance
                    // switch camera, from the front and the back and vice versa

                    releaseCamera();
                    chooseCamera();
                } else {
                    Toast toast = Toast.makeText(myContext, "Sorry, your phone has only one camera!", Toast.LENGTH_LONG);
                    toast.show();
                }
            }
        }
    };

    public void chooseCamera() {
        // if the camera preview is the front
        if (cameraFront) {
            int cameraId = findBackFacingCamera();
            if (cameraId >= 0) {
                // open the backFacingCamera
                // set a picture callback
                // refresh the preview

                mCamera = Camera.open(cameraId);
                // mPicture = getPictureCallback();
                mPreview.refreshCamera(mCamera);
            }
        } else {
            int cameraId = findFrontFacingCamera();
            if (cameraId >= 0) {
                // open the backFacingCamera
                // set a picture callback
                // refresh the preview

                mCamera = Camera.open(cameraId);
                // mPicture = getPictureCallback();
                mPreview.refreshCamera(mCamera);
            }
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        // when on Pause, release camera in order to be used from other
        // applications
        releaseCamera();

        //Accelerometer
        sensorManager.unregisterListener(this);

    }

    private boolean hasCamera(Context context) {
        // check if the device has camera
        if (context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_CAMERA)) {
            return true;
        } else {
            return false;
        }
    }

    boolean recording = false;
    OnClickListener captrureListener = new OnClickListener() {
        @Override
        public void onClick(View v) {

            if (recording) {
                // stop recording and release camera
                mediaRecorder.stop(); // stop the recording
                releaseMediaRecorder(); // release the MediaRecorder object
                recording = false;

                // VIBRATE
                //cont_vibrate=true;

                //File
                String filename=editTextFileName.getText().toString();
                editTextFileName.setText("");
                FileOutputStream fos;
                try {
                    File myFile = new File("/sdcard/"+filename+".txt");
                    myFile.createNewFile();
                    FileOutputStream fOut = new

                            FileOutputStream(myFile);
                    OutputStreamWriter myOutWriter = new

                            OutputStreamWriter(fOut);
                    for(int i=1;i<data_i_max;i++)
                    {
                        myOutWriter.append(
                                // Accelerometer
                                data_x[i]+" "+data_y[i]+" "+data_z[i]+
                                // Gyroscope
                                " "+data_gyro_x[i]+" "+data_gyro_y[i]+" "+data_gyro_z[i]+
                                // Magnetometer
                                " "+data_magnet_x[i]+" "+data_magnet_y[i]+" "+data_magnet_z[i]+
                                // Blank Time Fields
                                " "+"0.0"+" "+"0.0"+" "+"0.0"+" "+"0.0"+" "+"0.0"+" "+"0.0"+" "+"0.0"+
                                // Seconds Count
                                " "+data_t[i]+" ");
                    }
                    myOutWriter.close();
                    fOut.close();

                    Toast.makeText(getApplicationContext(),"Saving: " + filename,Toast.LENGTH_LONG).show();


                } catch (FileNotFoundException e) {e.printStackTrace();}
                catch (IOException e) {e.printStackTrace();}



            } else {
                if (!prepareMediaRecorder()) {
                    Toast.makeText(Video_Accelerometer_File.this, "Fail in prepareMediaRecorder()!\n - Ended -", Toast.LENGTH_LONG).show();
                    finish();
                }

                //Timer
                start_time=System.currentTimeMillis();

                Toast.makeText(getApplicationContext(),"Recording: " + editTextFileName.getText().toString(),Toast.LENGTH_LONG).show();


                // work on UiThread for better performance
                runOnUiThread(new Runnable() {
                    public void run() {
                        // If there are stories, add them to the table

                        try {
                            // CAMERA
                            mediaRecorder.start();

                            // VIBRATE
                            cont_vibrate=true;


                            // TIMER
////////////////////////////////////////////
                            //startTimer();
////////////////////////////////////////////

                        } catch (final Exception ex) {
                            // Log.i("---","Exception in thread");
                        }
                    }
                });

                recording = true;
            }
        }
    };

    private void releaseMediaRecorder() {
        if (mediaRecorder != null) {
            mediaRecorder.reset(); // clear recorder configuration
            mediaRecorder.release(); // release the recorder object
            mediaRecorder = null;
            mCamera.lock(); // lock camera for later use
        }
    }

    private boolean prepareMediaRecorder() {

        mediaRecorder = new MediaRecorder();

        mCamera.unlock();
        mediaRecorder.setCamera(mCamera);

        mediaRecorder.setAudioSource(MediaRecorder.AudioSource.CAMCORDER);
        mediaRecorder.setVideoSource(MediaRecorder.VideoSource.CAMERA);

        mediaRecorder.setProfile(CamcorderProfile.get(CamcorderProfile.QUALITY_480P)); // Initially QUALITY_720P));

        mediaRecorder.setOutputFile("/sdcard/"+editTextFileName.getText().toString()+".mp4");
        mediaRecorder.setMaxDuration(max_time*10000); // Set max duration 60 sec.
        mediaRecorder.setMaxFileSize(max_size*1000000); // Set max file size 50M

        try {
            mediaRecorder.prepare();
        } catch (IllegalStateException e) {
            releaseMediaRecorder();
            return false;
        } catch (IOException e) {
            releaseMediaRecorder();
            return false;
        }
        return true;

    }

    private void releaseCamera() {
        // stop and release camera
        if (mCamera != null) {
            mCamera.release();
            mCamera = null;
        }
    }

    public void initializeViews() {
        currentX = (TextView) findViewById(R.id.currentX);
        currentY = (TextView) findViewById(R.id.currentY);
        currentZ = (TextView) findViewById(R.id.currentZ);
        //currentGyroX = (TextView) findViewById(R.id.currentGyroX);
        //currentGyroY = (TextView) findViewById(R.id.currentGyroY);
        //currentGyroZ = (TextView) findViewById(R.id.currentGyroZ);
        currentT = (TextView) findViewById(R.id.currentT);
        vibrateCount = (TextView) findViewById(R.id.vibrateCount);

    }

    public void initializeListeners() {
        sensorManager.registerListener(this,
                sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
                SensorManager.SENSOR_DELAY_NORMAL);

        sensorManager.registerListener(this,
                sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE),
                SensorManager.SENSOR_DELAY_NORMAL);

        sensorManager.registerListener(this,
                sensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD),
                SensorManager.SENSOR_DELAY_NORMAL);
    }



/////////////////
// SENSOR UPDATES
/////////////////

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {

    }

    @Override
    public void onSensorChanged(SensorEvent event) {

        if(recording && cont_vibrate) {
            long buzz_pattern[] = new long[2*vibrate_count];
            Arrays.fill(buzz_pattern,pulse_duration);
            v.vibrate(buzz_pattern,-1);
            cont_vibrate=false;
        }


        switch(event.sensor.getType()) {
            case Sensor.TYPE_ACCELEROMETER:
                System.arraycopy(event.values, 0, accel, 0, 3);

                // USE TO CALCULATE ORIENTATION
                //calculateAccMagOrientation();

                displayCleanTime();
                displayCurrentTime((System.currentTimeMillis()-start_time)/1000);

                displayCleanValuesAccelerometer();
                displayCurrentValuesAccelerometer(accel[0],accel[1],accel[2]);




                break;

            case Sensor.TYPE_GYROSCOPE:
                System.arraycopy(event.values, 0, gyro, 0, 3);

                // USE TO CALCULATE ORIENTATION
                //gyroFunction(event);


                displayCleanTime();
                displayCurrentTime((System.currentTimeMillis()-start_time)/1000);

                //displayCleanValuesGyroscope();
                //displayCurrentValuesGyroscope(gyro[0], gyro[1], gyro[2]);

                break;

            case Sensor.TYPE_MAGNETIC_FIELD:
                System.arraycopy(event.values, 0, magnet, 0, 3);

                break;



        }

        if (recording) {
            time_1 = System.currentTimeMillis();

            if (data_i < data_i_max && (double)((time_1 - start_time) / 1000)<cutoff_time) {
                // TIME
                data_t[data_i] = Double.toString((double)((System.currentTimeMillis() - start_time) / 1000.));

                // ACCELERATION
                data_x[data_i] = Double.toString((double)(accel[0]/g));
                data_y[data_i] = Double.toString((double)(accel[1]/g));
                data_z[data_i] = Double.toString((double)(accel[2]/g));
                // ORIENTATION
                data_gyro_x[data_i] = Double.toString((double)gyro[0]);
                data_gyro_y[data_i] = Double.toString((double)gyro[1]);
                data_gyro_z[data_i] = Double.toString((double)gyro[2]);

                // Magnetometer
                data_magnet_x[data_i] = Double.toString((double)magnet[0]);
                data_magnet_y[data_i] = Double.toString((double)magnet[1]);
                data_magnet_z[data_i] = Double.toString((double)magnet[2]);


            }
            // ELSE Do Nothing

            data_i = data_i + 1;

        }


    }

/////////////////////
// END SENSOR UPDATES
/////////////////////


//////////////////
// TEXTBOX UPDATES
//////////////////

    public void displayCleanTime() {
        currentT.setText("0.0");
    }

    public void displayCleanValuesAccelerometer() {
        currentX.setText("0.0");
        currentY.setText("0.0");
        currentZ.setText("0.0");
    }

    public void displayCleanValuesGyroscope() {

        currentGyroX.setText("0.0");
        currentGyroY.setText("0.0");
        currentGyroZ.setText("0.0");
    }

    public void displayCurrentTime(Long t) {
        currentT.setText(Long.toString(t));
    }

    // display the current x,y,z accelerometer values
    public void displayCurrentValuesAccelerometer(Float x, Float y, Float z) {
        currentX.setText(Float.toString(x));
        currentY.setText(Float.toString(y));
        currentZ.setText(Float.toString(z));
    }

    public void displayCurrentValuesGyroscope(Float x, Float y, Float z) {
        currentGyroX.setText(Float.toString(x));
        currentGyroY.setText(Float.toString(y));
        currentGyroZ.setText(Float.toString(z));
    }
//////////////////////
// END TEXTBOX UPDATES
//////////////////////

///////////////////////////
// ORIENTATION CALCULATIONS
///////////////////////////

    public void calculateAccMagOrientation() {
        if(SensorManager.getRotationMatrix(rotationMatrix, null, accel, magnet)) {
            SensorManager.getOrientation(rotationMatrix, accMagOrientation);
        }
    }

    public static final float EPSILON = 0.000000001f;

    private void getRotationVectorFromGyro(float[] gyroValues,
                                           float[] deltaRotationVector,
                                           float timeFactor)
    {
        float[] normValues = new float[3];

        // Calculate the angular speed of the sample
        float omegaMagnitude =
                (float)Math.sqrt(gyroValues[0] * gyroValues[0] +
                        gyroValues[1] * gyroValues[1] +
                        gyroValues[2] * gyroValues[2]);

        // Normalize the rotation vector if it's big enough to get the axis
        if(omegaMagnitude > EPSILON) {
            normValues[0] = gyroValues[0] / omegaMagnitude;
            normValues[1] = gyroValues[1] / omegaMagnitude;
            normValues[2] = gyroValues[2] / omegaMagnitude;
        }

        // Integrate around this axis with the angular speed by the timestep
        // in order to get a delta rotation from this sample over the timestep
        // We will convert this axis-angle representation of the delta rotation
        // into a quaternion before turning it into the rotation matrix.
        float thetaOverTwo = omegaMagnitude * timeFactor;
        float sinThetaOverTwo = (float)Math.sin(thetaOverTwo);
        float cosThetaOverTwo = (float)Math.cos(thetaOverTwo);
        deltaRotationVector[0] = sinThetaOverTwo * normValues[0];
        deltaRotationVector[1] = sinThetaOverTwo * normValues[1];
        deltaRotationVector[2] = sinThetaOverTwo * normValues[2];
        deltaRotationVector[3] = cosThetaOverTwo;
    }

    private static final float NS2S = 1.0f / 1000000000.0f;
    private float timestamp;
    private boolean initState = true;

    public void gyroFunction(SensorEvent event) {


        // don't start until first accelerometer/magnetometer orientation has been acquired
        if (accMagOrientation == null)
            return;

        // initialisation of the gyroscope based rotation matrix
        if(initState) {
            float[] initMatrix = new float[9];
            initMatrix = getRotationMatrixFromOrientation(accMagOrientation);
            float[] test = new float[3];
            SensorManager.getOrientation(initMatrix, test);
            gyroMatrix = matrixMultiplication(gyroMatrix, initMatrix);
            initState = false;
        }

        // copy the new gyro values into the gyro array
        // convert the raw gyro data into a rotation vector
        float[] deltaVector = new float[4];
        if(timestamp != 0) {
            final float dT = (event.timestamp - timestamp) * NS2S;
            System.arraycopy(event.values, 0, gyro, 0, 3);
            getRotationVectorFromGyro(gyro, deltaVector, dT / 2.0f);
        }

        // measurement done, save current time for next interval
        timestamp = event.timestamp;

        // convert rotation vector into rotation matrix
        float[] deltaMatrix = new float[9];
        SensorManager.getRotationMatrixFromVector(deltaMatrix, deltaVector);

        // apply the new rotation interval on the gyroscope based rotation matrix
        gyroMatrix = matrixMultiplication(gyroMatrix, deltaMatrix);

        // get the gyroscope based orientation from the rotation matrix
        SensorManager.getOrientation(gyroMatrix, gyroOrientation);
    }


    private float[] getRotationMatrixFromOrientation(float[] o) {
        float[] xM = new float[9];
        float[] yM = new float[9];
        float[] zM = new float[9];

        float sinX = (float)Math.sin(o[1]);
        float cosX = (float)Math.cos(o[1]);
        float sinY = (float)Math.sin(o[2]);
        float cosY = (float)Math.cos(o[2]);
        float sinZ = (float)Math.sin(o[0]);
        float cosZ = (float)Math.cos(o[0]);

        // rotation about x-axis (pitch)
        xM[0] = 1.0f; xM[1] = 0.0f; xM[2] = 0.0f;
        xM[3] = 0.0f; xM[4] = cosX; xM[5] = sinX;
        xM[6] = 0.0f; xM[7] = -sinX; xM[8] = cosX;

        // rotation about y-axis (roll)
        yM[0] = cosY; yM[1] = 0.0f; yM[2] = sinY;
        yM[3] = 0.0f; yM[4] = 1.0f; yM[5] = 0.0f;
        yM[6] = -sinY; yM[7] = 0.0f; yM[8] = cosY;

        // rotation about z-axis (azimuth)
        zM[0] = cosZ; zM[1] = sinZ; zM[2] = 0.0f;
        zM[3] = -sinZ; zM[4] = cosZ; zM[5] = 0.0f;
        zM[6] = 0.0f; zM[7] = 0.0f; zM[8] = 1.0f;

        // rotation order is y, x, z (roll, pitch, azimuth)
        float[] resultMatrix = matrixMultiplication(xM, yM);
        resultMatrix = matrixMultiplication(zM, resultMatrix);
        return resultMatrix;
    }


    private float[] matrixMultiplication(float[] A, float[] B) {
        float[] result = new float[9];

        result[0] = A[0] * B[0] + A[1] * B[3] + A[2] * B[6];
        result[1] = A[0] * B[1] + A[1] * B[4] + A[2] * B[7];
        result[2] = A[0] * B[2] + A[1] * B[5] + A[2] * B[8];

        result[3] = A[3] * B[0] + A[4] * B[3] + A[5] * B[6];
        result[4] = A[3] * B[1] + A[4] * B[4] + A[5] * B[7];
        result[5] = A[3] * B[2] + A[4] * B[5] + A[5] * B[8];

        result[6] = A[6] * B[0] + A[7] * B[3] + A[8] * B[6];
        result[7] = A[6] * B[1] + A[7] * B[4] + A[8] * B[7];
        result[8] = A[6] * B[2] + A[7] * B[5] + A[8] * B[8];

        return result;
    }

//////////////////////////////
// END ORIENTATION CALCULATION
//////////////////////////////

}

