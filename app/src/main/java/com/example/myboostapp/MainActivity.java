package com.example.myboostapp;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private SharedObject payload;
    private TextView tv;
    private int counter = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        tv = findViewById(R.id.sample_text);

        String payloadName = getString(R.string.shared_object_name);
        payload = new SharedObject(payloadName);
        payload.getValue();

        //startService(new Intent(this, MyService.class));
        try
        {
            Thread.sleep(5000);
        }
        catch (Exception e)
        {

        }

        Button button = findViewById(R.id.read_from_server);
        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // Example of a call to a native method
                tv.setText(String.format("%d: Payload value is %s", counter++, payload.getValue()));
            }
        });
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}
