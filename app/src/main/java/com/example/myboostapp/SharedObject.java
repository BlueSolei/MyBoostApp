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

    private native int nativeGetValue(String name);
    private native void nativeSetValue(String name, int value);
}
