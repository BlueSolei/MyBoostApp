package com.example.myboostapp;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

public class MyService extends Service {

    private static final String TAG =  MyService.class.getSimpleName();

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    SharedObject payload;
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "MyService start...");

        new Thread(new Runnable() {
            @Override
            public void run() {
                String name = getString(R.string.shared_object_name);
                payload = new SharedObject(name);

                //payload.setValue(0);

                for(int i = 0; i < 10; ++i)
                {
                    try
                    {
                        Thread.sleep(2000);
                        int answer = payload.getValue();
                        Log.d(TAG, String.format("MyService waked up. no. %d, get value to %s", i, answer));
                    }
                    catch (Exception e)
                    {
                        Log.d(TAG, "MyService payload Sleep() failed.");
                    }
                }
                payload.stop();
            }
        }).start();

        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}
