package com.example.myboostapp;

public class SharedObject {

    private String name;
    private int fd = -1;

    public SharedObject(String name)
    {
        this.name = name;
        fd = getSharedObjectFD(name);
    }

    public SharedObject(int fd)
    {
        this.name = String.format("<fd:%d>", fd);
        this.fd = fd;
        createSharedObject(fd);
    }

    public int getValue()
    {
        return nativeGetValue(name);
    }

    public void setValue(int value)
    {
        nativeSetValue(name, value);
    }

    public int getFD()
    {
        return fd;
    }

    private native void createSharedObject(int fd);
    private native int getSharedObjectFD(String name);
    private native int nativeGetValue(String name);
    private native void nativeSetValue(String name, int value);
}
