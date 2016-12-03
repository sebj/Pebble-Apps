package me.sebj.pebblestorage;

import android.app.Activity;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.util.PebbleDictionary;

import java.util.UUID;

public class MainActivity extends Activity {
    private static final String TAG = "me.sebj.pebblestorage";
    private static final UUID PEBBLE_APP_UUID = UUID.fromString("d1a51f41-4046-410a-9929-5e5508cb00a8");
    private static final int charLimit = 112;


    private PebbleKit.PebbleAckReceiver ackReceiver;
    private PebbleKit.PebbleNackReceiver nackReceiver;

    static TextView textView;
    static EditText textInput;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        textInput = (EditText)findViewById(R.id.textInput);
        textView = (TextView)findViewById(R.id.textView);

        Button openAppButton = (Button)findViewById(R.id.openAppButton);
        openAppButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //Open app on Pebble
                try {
                    PebbleKit.startAppOnPebble(getApplicationContext(), PEBBLE_APP_UUID);
                } catch (Exception e) {
                    //
                }

                //Hide keyboard
                InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(textInput.getWindowToken(), 0);
            }
        });

        Button sendButton = (Button)findViewById(R.id.sendButton);
        sendButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                textView.setText(textInput.getText());

                sendText(textView.getText().toString());

                textInput.setText("");

                //Hide keyboard
                InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(textInput.getWindowToken(), 0);
            }
        });


        setupAckNackReceivers();


        Intent intent = getIntent();
        String action = intent.getAction();
        String type = intent.getType();

        if (Intent.ACTION_SEND.equals(action) && type != null) {
            if ("text/plain".equals(type)) {
                // Handle text being sent
                handleShareText(intent);
            }
        }
    }

    public void setupAckNackReceivers() {
        /*if (ackReceiver == null) {
            Log.i(getLocalClassName(), "Setup ack receiver");
            ackReceiver = new PebbleKit.PebbleAckReceiver(PEBBLE_APP_UUID) {
                @Override
                public void receiveAck(Context context, int transactionId) {
                    Log.i(getLocalClassName(), "Received ack for transaction " + transactionId);
                }
            };
            PebbleKit.registerReceivedAckHandler(getApplicationContext(), ackReceiver);
        }

        if (nackReceiver == null) {
            Log.i(getLocalClassName(), "Setup nack receiver");
            nackReceiver = new PebbleKit.PebbleNackReceiver(PEBBLE_APP_UUID) {
                @Override
                public void receiveNack(Context context, int transactionId) {
                    Log.i(getLocalClassName(), "Received nack for transaction " + transactionId);
                }
            };
            PebbleKit.registerReceivedNackHandler(getApplicationContext(), nackReceiver);
        }*/
    }

    public void removeAckNackReceivers() {
        /*if (ackReceiver != null) {
            Log.i(getLocalClassName(), "Removing ack receiver");
            try {
                unregisterReceiver(ackReceiver);
            } catch (Exception e) {
                //Meh
            }

            ackReceiver = null;
        }

        if (nackReceiver != null) {
            Log.i(getLocalClassName(), "Removing nack receiver");
            try {
                unregisterReceiver(nackReceiver);
            } catch (Exception e) {
                //Meh
            }

            nackReceiver = null;
        }*/
    }

    @Override
    protected void onPause() {
        super.onPause();

        removeAckNackReceivers();
    }

    @Override
    protected void onResume() {
        super.onResume();

        setupAckNackReceivers();
    }

    @Override
    protected void onStop() {
        super.onStop();

        removeAckNackReceivers();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        removeAckNackReceivers();
    }

    public void handleShareText(Intent intent) {
        String sharedText = intent.getStringExtra(Intent.EXTRA_TEXT);
        if (sharedText != null) {

            textView.setText(sharedText);

            setupAckNackReceivers();

            sendText(sharedText);
        }
    }

    public void sendText(String text) {
        if (text != null && text.length() > 0) {
            String textToSend = text;

            if (text.length() > charLimit) {
                textToSend = text.substring(0, charLimit);

                Toast.makeText(getApplicationContext(), "112 character limit - text shortened", Toast.LENGTH_LONG).show();
            }

            sendDataToWatch(textToSend);
        }
    }

    public void sendDataToWatch(String theString) {
        boolean connected = PebbleKit.isWatchConnected(getApplicationContext());
        if (connected) {
            if (PebbleKit.areAppMessagesSupported(getApplicationContext())) {

                //Send data
                PebbleDictionary data = new PebbleDictionary();
                data.addString(0, theString);

                try {
                    PebbleKit.sendDataToPebble(getApplicationContext(), PEBBLE_APP_UUID, data);
                } catch (Exception e) {
                    //
                }

                Toast.makeText(getApplicationContext(), "Sent text to Pebble", Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(getApplicationContext(), "AppMessages not supported on Pebble", Toast.LENGTH_LONG).show();
            }
        } else {
            Toast.makeText(getApplicationContext(), "Pebble not connected", Toast.LENGTH_LONG).show();
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
        int id = item.getItemId();
        if (id == R.id.send_clipboard) {
            ClipboardManager clipboard = (ClipboardManager)getSystemService(CLIPBOARD_SERVICE);
            
            if (clipboard != null) {
                sendText(clipboard.getText().toString());
            }
        }

        return super.onOptionsItemSelected(item);
    }

}
