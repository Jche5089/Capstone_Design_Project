package com.example.arduino_application;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;

import com.naver.maps.map.MapFragment;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    public void sensor_map(View view){
        Toast.makeText(this, "지도관련 화면 전환 테스트 메시지", Toast.LENGTH_SHORT).show();
        Intent intent = new Intent(getApplicationContext(), gps_activity.class);
        startActivity(intent);
    }

    public void set_bt(View view){
        Toast.makeText(this, "센서 모니터 화면 전환 테스트 메시지", Toast.LENGTH_SHORT).show();
        Intent intent = new Intent(getApplicationContext(),
                bt_setting_activity.class);
        startActivity(intent);
    }

}