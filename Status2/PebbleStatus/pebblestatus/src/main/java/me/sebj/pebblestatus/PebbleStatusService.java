package me.sebj.pebblestatus;

import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.widget.Toast;

public class PebbleStatusService extends Service {
    public static Handler mMyServiceHandler = null;
    public static Boolean mIsServiceRunning = false;

    public PebbleStatusService() {
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        Toast.makeText(this, "Congrats! MyService Created", Toast.LENGTH_LONG).show();
        Log.d(getClass().toString(), "onCreate");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Toast.makeText(this, "My Service Started", Toast.LENGTH_LONG).show();
        Log.d(getClass().toString(), "onStartCommand");
        return Service.START_NOT_STICKY;
    }
    @Override
    public void onDestroy() {
        Toast.makeText(this, "MyService Stopped", Toast.LENGTH_LONG).show();
        Log.d(getClass().toString(), "onDestroy");

        mIsServiceRunning = false;
    }
}
