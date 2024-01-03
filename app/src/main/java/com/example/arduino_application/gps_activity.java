package com.example.arduino_application;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.content.pm.PackageManager;
import android.graphics.PointF;
import android.os.Bundle;
import android.widget.Toast;

import com.example.arduino_application.databinding.ActivityGpsBinding;
import com.naver.maps.geometry.LatLng;
import com.naver.maps.map.LocationTrackingMode;
import com.naver.maps.map.MapFragment;
import com.naver.maps.map.NaverMap;
import com.naver.maps.map.OnMapReadyCallback;
import com.naver.maps.map.util.FusedLocationSource;
import androidx.databinding.DataBindingUtil;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;

import java.util.ArrayList;
import java.util.List;

public class gps_activity extends AppCompatActivity implements OnMapReadyCallback {

    private static final int LOCATION_PERMISSION_REQUEST_CODE = 1000;
    private static final String[] PERMISSIONS = {
            android.Manifest.permission.ACCESS_FINE_LOCATION,
            android.Manifest.permission.ACCESS_COARSE_LOCATION
    };
    List<LatLng> lstLatLng = new ArrayList<>();
    private ActivityGpsBinding binding;
    private NaverMap naverMap;
    private FusedLocationSource locationSource;

    private MapFragment mapFragment;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_gps);
        //binding = DataBindingUtil.setContentView(this, R.layout.activity_gps);
            //Toast.makeText(this, "권한부여됨", Toast.LENGTH_SHORT).show();
            FragmentManager fm = getSupportFragmentManager();
            mapFragment = (MapFragment) fm.findFragmentById(R.id.map);
            if(mapFragment == null){
                fm.beginTransaction().add(R.id.map, mapFragment).commit();
            }
            mapFragment.getMapAsync(this);
            locationSource = new FusedLocationSource(this, LOCATION_PERMISSION_REQUEST_CODE);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                          @NonNull int[] granResults){
        if(locationSource.onRequestPermissionsResult(requestCode, permissions, granResults)){
            if (!locationSource.isActivated()) {
                naverMap.setLocationTrackingMode(LocationTrackingMode.None);
                return;
            }else{
                naverMap.setLocationTrackingMode(LocationTrackingMode.Follow);
            }
        }
        super.onRequestPermissionsResult(requestCode, permissions, granResults);
    }

    @Override
    public void onMapReady(@NonNull NaverMap naverMap) {
        this.naverMap = naverMap;
        // 현재 위치
        naverMap.setLocationSource(locationSource);
        // 현재 위치 버튼 기능
        naverMap.getUiSettings().setLocationButtonEnabled(true);
        // 위치를 추적하면서 카메라도 따라 움직인다.
        naverMap.setLocationTrackingMode(LocationTrackingMode.Follow);
        //Toast.makeText(getApplicationContext(), "onMapReady", Toast.LENGTH_SHORT).show();
    }
}
