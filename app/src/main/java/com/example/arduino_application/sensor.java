package com.example.arduino_application;

public class sensor {
    public static final String DEVICE_TYPE = "0";       // LYWSD03MMC "5b 05"

    public long updateTime = 0;
    public int temperature = 0;
    public int humidity = 0;

    public sensor() {
    }

    public sensor(long updateTime, int temperature, int humidity) {
        this.updateTime = updateTime;
        this.temperature = temperature;
        this.humidity = humidity;
    }

    public static boolean isType(byte[] serviceData)
    {
        return byteArrayToHex(serviceData).toLowerCase().contains(DEVICE_TYPE);
    }

    public static String byteArrayToHex(byte[] a) {
        StringBuilder sb = new StringBuilder();
        for(final byte b: a)
            sb.append(String.format("%02x ", b&0xff));
        return sb.toString();
    }
}
