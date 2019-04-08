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

        String name = getString(R.string.shared_object_name);
        payload = new SharedObject(name);
        payload.setValue(0);

        new Thread(new Runnable() {
            @Override
            public void run() {
                for(int i = 0; i < 1000; ++i)
                {
                    try
                    {
                        Thread.sleep(1000);
                        payload.setValue(i);
                        Log.d(TAG, String.format("MyService waked up. no. %d, set value to %s", i, i));
                    }
                    catch (Exception e)
                    {
                        Log.d(TAG, "MyService payload Sleep() failed.");
                    }
                }
            }
        }).start();

        return START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}
