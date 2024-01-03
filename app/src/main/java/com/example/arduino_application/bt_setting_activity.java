package com.example.arduino_application;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.le.ScanResult;
import android.content.Intent;
import android.os.Bundle;
import android.widget.TextView;
import android.widget.Toast;

import com.gun0912.tedpermission.PermissionListener;
import com.gun0912.tedpermission.normal.TedPermission;

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

public class bt_setting_activity extends AppCompatActivity implements BluetoothGuide.OnCheckModelListener, BluetoothGuide.OnNotifyValueListener<sensor> {
    TextView temp_View = null;
    TextView hum_View = null;
    TextView ox_View = null;
    TextView pm_View = null;
    private final BluetoothGuide bluetoothGuide = new BluetoothGuide();
    private final PermissionListener permissionListener = new PermissionListener() {
        @Override
        public void onPermissionGranted() {
            // Start Scan Device
            bluetoothGuide.scanDevices();
        }

        @Override
        public void onPermissionDenied(List<String> deniedPermissions) {

        }
    };

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if( bluetoothGuide.onActivityResult(requestCode, resultCode))
        {
            TedPermission.create()
                    .setPermissionListener(permissionListener)
                    .setDeniedMessage("Denied Permission.")
                    .setPermissions(Manifest.permission.ACCESS_FINE_LOCATION, Manifest.permission.BLUETOOTH_SCAN)
                    .check();
        }
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_bt_setting);

        temp_View = findViewById(R.id.temp_value);
        hum_View = findViewById(R.id.hum_value);
        ox_View = findViewById(R.id.ox_value);
        pm_View = findViewById(R.id.pm_value);

        //add listeners
        bluetoothGuide
                .setOnCheckModelListener(this)
                .setOnNotifyValueListener(this);

        //Bluetooth System On with permission
        if(bluetoothGuide.isOn()){
            TedPermission.create()
                    .setPermissionListener(permissionListener)
                    .setDeniedMessage("Denied Permission")
                    .setPermissions(Manifest.permission.ACCESS_FINE_LOCATION)
                    .check();
        }else{
            bluetoothGuide.on(this);
        }
    }

    @Override
    protected void onDestroy(){
        super.onDestroy();
        bluetoothGuide.disconnectGATTAll();
        bluetoothGuide.onComplete();
    }

    @Override
    public boolean isChecked(byte[] bytes) {
        return sensor.isType(bytes);
    }

    @Override
public void scannedDevice(ScanResult result) {
        // start connecting device
        bluetoothGuide.connGATT(getApplicationContext(),result.getDevice());
    }

    @Override
    public void onValue(BluetoothDevice deivce, sensor value) {
        // show the data that is notified value
        //textView.setText(String.valueOf(value.temperature));
        //textView1.setText(String.valueOf(value.humidity));
    }

    @Override
    public sensor formatter(BluetoothGattCharacteristic characteristic) {
        //Format the data
        byte[] a = characteristic.getValue();
        //long c = byteArrayToLong(a);
        //int c = ByteBuffer.wrap(a).getInt();
        //String c = new String()
        if (a != null && a.length > 0) {
            String value = new String(a, StandardCharsets.UTF_8);
            String[] values = value.split(",");
            if (values.length >= 2) {
                String value1 = values[0];
                String value2 = values[1];

                // UI 업데이트
                //temp_View.setText(value1);
                //hum_View.setText(value2);
                ox_View.setText(value1);
                pm_View.setText(value2);
            }
        }
        // 디버깅용
        //Toast.makeText(getApplicationContext(), "formatter method success!", Toast.LENGTH_SHORT).show();
        Integer value = characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_UINT16, 0);
        Integer value2 = characteristic.getIntValue(BluetoothGattCharacteristic.FORMAT_UINT16, 0);
        int temperature = value;
        int humidity = value2;
        return new sensor(
                System.currentTimeMillis(),
                temperature,
                humidity
        );
    }
}