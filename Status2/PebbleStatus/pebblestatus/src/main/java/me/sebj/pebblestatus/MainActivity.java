package me.sebj.pebblestatus;

import android.app.Activity;
import android.app.Fragment;
import android.bluetooth.BluetoothAdapter;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.os.BatteryManager;
import android.os.Bundle;
import android.preference.PreferenceFragment;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.getpebble.android.kit.Constants;
import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.PebbleKit.PebbleDataReceiver;
import com.getpebble.android.kit.util.PebbleDictionary;

import java.lang.reflect.Method;
import java.util.UUID;


public class MainActivity extends Activity {
    private static final UUID PEBBLE_APP_UUID = UUID.fromString("193f3741-c59a-4ea6-9688-d5e8f0e2c3cb");

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        if (savedInstanceState == null) {
            //startService(new Intent(this, PebbleStatusService.class));

            getFragmentManager().beginTransaction().add(R.id.container, new MainFragment()).commit();
        }
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.open_app:
                //Open Pebble app
                try {
                    PebbleKit.startAppOnPebble(getApplicationContext(), PEBBLE_APP_UUID);
                } catch (Exception e) {
                    //
                }

                break;

            case R.id.action_settings:
                //Show settings
                Intent i = new Intent(this, PrefsActivity.class);
                startActivity(i);

                break;
        }
        return true;
    }

    @Override
    protected void onPause() {
        super.onPause();

        Log.i(getClass().toString(), "Main activity paused");
    }

    @Override
    protected void onResume() {
        Log.i(getClass().toString(), "Main activity resumed");

        super.onResume();
    }

    @Override
    protected void onDestroy() {
        Log.i(getClass().toString(), "Main activity destroyed");
    }

    public static class MainFragment extends Fragment {

        private static final UUID PEBBLE_APP_UUID = UUID.fromString("193f3741-c59a-4ea6-9688-d5e8f0e2c3cb");

        private BroadcastReceiver pConnectReceiver;
        private BroadcastReceiver pDisconnectReceiver;
        private PebbleDataReceiver pData;

        static SharedPreferences sharedPrefs;

        static TextView connectionStatus;
        static TextView batteryStatus;

        public MainFragment() {
        }

        public void updateConnectionStatus() {
            BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

            boolean bluetooth = (mBluetoothAdapter != null && mBluetoothAdapter.isEnabled());

            if (!bluetooth) {
                //Bluetooth off
                connectionStatus.setText("Bluetooth off");
            } else {
                //Bluetooth on
                //Check if Pebble connected, report accordingly
                connectionStatus.setText(PebbleKit.isWatchConnected(getActivity().getApplicationContext())? "Pebble connected" : "Pebble disconnected");
            }
        }

        public int getBatteryLevel() {
            Intent batteryIntent = getActivity().registerReceiver(null, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));

            int level = batteryIntent.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
            int scale = batteryIntent.getIntExtra(BatteryManager.EXTRA_SCALE, -1);

            if (level == -1 || scale == -1) {
                return 50;
            }

            return (int)(((float)level / (float)scale) * 100.0f);
        }

        public void sendPhoneBattery() {
            PebbleDictionary data = new PebbleDictionary();
            int battery = getBatteryLevel();
            data.addInt8(2, (byte)battery);

            PebbleKit.sendDataToPebble(getActivity().getApplicationContext(), PEBBLE_APP_UUID, data);
        }

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            sharedPrefs = PreferenceManager.getDefaultSharedPreferences(getActivity());

            pConnectReceiver = new BroadcastReceiver() {
              @Override
              public void onReceive(Context context, Intent intent) {
                updateConnectionStatus();
              }
            };

            pDisconnectReceiver = new BroadcastReceiver() {
              @Override
              public void onReceive(Context context, Intent intent) {
                updateConnectionStatus();
              }
            };

            PebbleKit.registerPebbleConnectedReceiver(getActivity(), pConnectReceiver);
            PebbleKit.registerPebbleDisconnectedReceiver(getActivity(), pDisconnectReceiver);

            pData = new PebbleDataReceiver(PEBBLE_APP_UUID) {
                @Override
                public void receiveData(Context context, int transactionId, PebbleDictionary data) {
                    PebbleKit.sendAckToPebble(context, transactionId);

                    if (data.contains(1)) {
                        //Pebble battery
                        int pebbleBattery = data.getInteger(1).intValue();
                        batteryStatus.setText("Pebble battery: " + pebbleBattery + "%");

                        sendPhoneBattery();

                    } else if (data.contains(3)) {
                        //Action command
                        Log.i(this.getClass().toString(), "Received action command");

                        String action = sharedPrefs.getString("pref_action", "action_wifi");

                        if (action.equals("action_data")) {
                            //Mobile Data
                            toggleMobileData(getActivity());
                            Toast.makeText(getActivity(), "Toggling Mobile Data", Toast.LENGTH_SHORT).show();

                        } else if (action.equals("action_wifi")) {
                            //Wi-Fi
                            final WifiManager wifi = (WifiManager)getActivity().getSystemService(Context.WIFI_SERVICE);
                            wifi.setWifiEnabled(!wifi.isWifiEnabled());

                            Toast.makeText(getActivity(), "Toggling Wi-Fi", Toast.LENGTH_SHORT).show();

                        } else if (action.equals("action_google_voice_search")) {
                            //Google Voice Search
                            Intent intent = new Intent(Intent.ACTION_MAIN);
                            intent.setClassName("com.google.android.googlequicksearchbox", "com.google.android.googlequicksearchbox.VoiceSearchActivity");
                            try {
                                startActivity(intent);
                            } catch (ActivityNotFoundException e) {
                                Toast.makeText(getActivity(), "Unable to launch Google Voice Search not found", Toast.LENGTH_SHORT).show();
                            }

                        }

                    }
                }
            };

            IntentFilter filter = new IntentFilter();
            filter.addAction(Constants.INTENT_APP_RECEIVE);
            getActivity().registerReceiver(pData, filter);
        }

        //Mobile data controls
        private static void toggleMobileData(Context paramContext) {
            try {
                ConnectivityManager connectivityManager = (ConnectivityManager) paramContext.getSystemService(Context.CONNECTIVITY_SERVICE);
                Method setMobileDataEnabledMethod = ConnectivityManager.class.getDeclaredMethod("setMobileDataEnabled", Boolean.TYPE);
                setMobileDataEnabledMethod.setAccessible(true);
                setMobileDataEnabledMethod.invoke(connectivityManager, !isAPNEnabled(paramContext));
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        private static boolean isAPNEnabled(Context paramContext) {
            try {
                NetworkInfo networkInfo = ((ConnectivityManager) paramContext.getSystemService(Context.CONNECTIVITY_SERVICE)).getActiveNetworkInfo();
                return networkInfo.isConnected();
            } catch (Exception e) {
                return false;
            }
        }


        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
            View mView = inflater.inflate(R.layout.fragment_main, container, false);

            //Text views
            connectionStatus = (TextView)mView.findViewById(R.id.connectionStatus);
            batteryStatus = (TextView)mView.findViewById(R.id.batteryStatus);

            //Update UI
            updateConnectionStatus();
            sendPhoneBattery();

            return mView;
        }

        @Override
        public void onDestroy() {
            super.onDestroy();
            getActivity().unregisterReceiver(pConnectReceiver);
            getActivity().unregisterReceiver(pDisconnectReceiver);
            getActivity().unregisterReceiver(pData);
            Log.i(getClass().toString(),"Fragment destroyed");
        }
    }

    public static class PrefsActivity extends Activity {
        @Override
        protected void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // Display the fragment as the main content.
            getFragmentManager().beginTransaction().replace(android.R.id.content,new PrefsFragment()).commit();
        }

        public static class PrefsFragment extends PreferenceFragment {
            @Override
            public void onCreate(Bundle savedInstanceState) {
                super.onCreate(savedInstanceState);

                addPreferencesFromResource(R.xml.preferences);
            }
        }
    }
}
