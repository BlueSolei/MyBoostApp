package com.example.myboostapp;

public class SharedObject {

    private String name;

    public SharedObject(String name)
    {
        this.name = name;
    }

    public int getValue()
    {
        return nativeGetValue(name);
    }

    public void setValue(int value)
    {
        nativeSetValue(name, value);
    }

    public void stop(){
        nativeStopServer(name);
    }

    private native int nativeStopServer(String name);
    private native int nativeGetValue(String name);
    private native void nativeSetValue(String name, int value);
}
