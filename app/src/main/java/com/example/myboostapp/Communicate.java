package com.example.myboostapp;

public class Communicate {

    public String ps()
    {
        return nativePS();
    }

    private native String nativePS();
}
