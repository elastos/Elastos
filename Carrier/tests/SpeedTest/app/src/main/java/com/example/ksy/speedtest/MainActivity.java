package com.example.ksy.speedtest;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.SwitchCompat;
import android.util.Log;
import android.view.DragEvent;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.PopupMenu;
import android.widget.TextView;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

import com.google.zxing.integration.android.IntentIntegrator;
import com.google.zxing.integration.android.IntentResult;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.List;

public class MainActivity extends AppCompatActivity {
    private static final String TAG = "MainActivity";
    private static final int MSG_SHOW_PROGRESS_DIALOG = 1000;
    private static final int MSG_UPDATE_PROGRESS_DIALOG = 1001;
    private static final int MSG_SHOW_TEST_RESULT = 1002;
    private static final int MSG_CONNECT_CHANGED = 2003;
    private static final int MSG_FRIEND_REQUEST = 2004;
    private static final int MSG_FRIEND_CHANGED = 2005;
    private static final int MSG_FRIEND_MESSAGE = 2006;

    private int percentFinished = 0;
    private LinearLayout mConnecting;
    private LinearLayout mConnected;
    private LinearLayout mFriend;
    private String mSelectedFriend;

    private SwitchCompat mSwitch1;
    private SwitchCompat mSwitch2;

    private RecyclerView mFriendList;

    private FriendAdapter mAdapter;

    private ProgressDialog progress;
    private boolean finishedTranferring = false;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                customScan();
            }
        });

        mConnecting = findViewById(R.id.layout_connecting);
        mConnecting.setVisibility(View.VISIBLE);

        mConnected = findViewById(R.id.layout_connected);
        mConnected.setVisibility(View.GONE);

        mFriend = findViewById(R.id.linear_friend);

        initList();

        Thread startThread = new Thread(new StartThread("SpeedTest.deb", "/sdcard/SpeedTest.deb"));
        startThread.start();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        stopCarrier();
    }

    private void initList() {
        mFriendList = findViewById(R.id.list_friends);
        mFriendList.setLayoutManager(new LinearLayoutManager(
                this, LinearLayoutManager.VERTICAL, false));

        ArrayList<Friend> list = new ArrayList<>();
        mAdapter = new FriendAdapter(list);
        mAdapter.setOnClickListener(new FriendAdapter.OnItemClickListener() {
            @Override
            public void onItemClick(View view, final Friend friend) {
                Log.d("Friend", "click friend:" + friend.mUid);
                //if (!friend.mConnected) return;

                if (friend.mSessionConnected) {
                    if (mSelectedFriend != null && mSelectedFriend.equals(friend.mUid)) return;
                    mSelectedFriend = friend.mUid;
                    showFriend();
                } else {
                    showMenu(mFriendList, friend);
                }
            }
        });

        mFriendList.setAdapter(mAdapter);
        mFriendList.addItemDecoration(new ItemDecoration(this, LinearLayoutManager.VERTICAL));
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            String userId = getUserId();
            String address = getAddress();
            if (userId == null || userId.isEmpty() || address == null || address.isEmpty()) {
                Log.d("MainActivity", "carrier is not ready");
                return true;
            }

            Intent intent = new Intent(MainActivity.this, QRCodeActivity.class);
            intent.putExtra("myUserId", userId);
            intent.putExtra("myAddress", address);
            startActivity(intent);
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        IntentResult intentResult = IntentIntegrator.parseActivityResult(requestCode, resultCode, data);
        if(intentResult != null) {
            if(intentResult.getContents() == null) {
                Toast.makeText(this,"Nothing was found",Toast.LENGTH_LONG).show();
            } else {
                Toast.makeText(this,"Scanned successfully",Toast.LENGTH_LONG).show();
                String address = intentResult.getContents();
                Log.d("MainActivity", "scan address: " + address);
                alertAddFriend(address);
            }
        } else {
            super.onActivityResult(requestCode,resultCode,data);
        }
    }

    private void alertAddFriend(final String address) {
        AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this)
                .setTitle("Add Friend")
                .setMessage(address)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        dialogInterface.dismiss();
                        addFriend(address);
                    }
                })
                .setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i) {
                        dialogInterface.dismiss();
                    }
                });
        builder.create().show();
    }

    public void customScan() {
        new IntentIntegrator(this)
                .setOrientationLocked(false)
                .setCaptureActivity(ScanActivity.class)
                .initiateScan();
    }

    public void OnConnectionChanged(boolean connected)
    {
        Message msg = new Message();
        msg.what = MSG_CONNECT_CHANGED;
        Bundle bundle = new Bundle();
        bundle.putBoolean("connection", connected);
        msg.setData(bundle);
        mHandler.sendMessage(msg);
    }

    public void OnFriendMessage(String uid, String message)
    {
        Message msg = new Message();
        msg.what = MSG_FRIEND_MESSAGE;
        Bundle bundle = new Bundle();
        bundle.putString("friendUid", uid);
        bundle.putString("friendMsg", message);
        msg.setData(bundle);
        mHandler.sendMessage(msg);
    }

    public void OnFriendRequest(String uid, String hello)
    {
        Message msg = new Message();
        msg.what = MSG_FRIEND_REQUEST;
        Bundle bundle = new Bundle();
        bundle.putString("friendUid", uid);
        bundle.putString("friendHello", hello);
        msg.setData(bundle);
        mHandler.sendMessage(msg);
    }

    public void OnFriendConnectionChanged(String uid, boolean online)
    {
        Message msg = new Message();
        msg.what = MSG_FRIEND_CHANGED;
        Bundle bundle = new Bundle();
        bundle.putString("friendUid", uid);
        bundle.putBoolean("friendOnline", online);
        msg.setData(bundle);
        mHandler.sendMessage(msg);
    }

    Handler mHandler = new Handler(new Handler.Callback() {
        @Override
        public boolean handleMessage(Message message) {
            switch (message.what) {
                case MSG_SHOW_PROGRESS_DIALOG:
                    handleShowProgressDialog(message);
                    break;
                case MSG_UPDATE_PROGRESS_DIALOG:
                    handleUpdateProgressDialog(message);
                    break;
                case MSG_SHOW_TEST_RESULT:
                    handleShowTestResult(message);
                    break;
                case MSG_CONNECT_CHANGED:
                    handleConnectionChanged(message);
                    break;
                case MSG_FRIEND_REQUEST:
                    handleFriendRequest(message);
                    break;
                case MSG_FRIEND_CHANGED:
                    handleFriendConnectionChanged(message);
                    break;
                case MSG_FRIEND_MESSAGE:
                    handleFriendMessage(message);
                    break;
                default:
                    break;
            }
            return false;
        }
    });

    private void handleConnectionChanged(Message msg) {
        Bundle bundle = msg.getData();
        Boolean connected = bundle.getBoolean("connection");
        if (connected) {
            mConnecting.setVisibility(View.GONE);
            mConnected.setVisibility(View.VISIBLE);
            ArrayList<Friend> list = getFriendList();
            mAdapter.clearFriends();
            mAdapter.addFriendList(list);
        } else {
            mConnecting.setVisibility(View.VISIBLE);
            mConnected.setVisibility(View.GONE);
            mAdapter.clearFriends();
        }
    }

    private void handleFriendRequest(Message msg) {
        Bundle bundle = msg.getData();
        final String uid = bundle.getString("friendUid");
        String hello = bundle.getString("friendHello");

        AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this)
                .setTitle("Friend Request")
                .setMessage(uid + " say: " + hello)
                .setPositiveButton("Accept", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        Toast.makeText(MainActivity.this, "You accepted!", Toast.LENGTH_LONG).show();
                        if (acceptFriend(uid) == 0) {
                            mAdapter.addFriend(new Friend(uid, false));
                        }
                    }
                })
                .setNegativeButton("Ignore", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        Toast.makeText(MainActivity.this, "You ignored!", Toast.LENGTH_LONG).show();
                    }
                });
        builder.create().show();
    }

    private void handleFriendConnectionChanged(Message msg) {
        Bundle bundle = msg.getData();
        String uid = bundle.getString("friendUid");
        Boolean online = bundle.getBoolean("friendOnline");
        mAdapter.friendStatusChanged(uid, online, false);
    }

    private void handleFriendMessage(Message msg) {
        Bundle bundle = msg.getData();
        final String uid = bundle.getString("friendUid");
        String hello = bundle.getString("friendMsg");

        AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this)
                .setTitle("Speedtest Request")
                .setMessage(uid + " say: " + hello)
                .setPositiveButton("Accept", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        Toast.makeText(MainActivity.this, "You accepted!", Toast.LENGTH_LONG).show();
                        acceptTestSpeed(uid);
                    }
                })
                .setNegativeButton("Ignore", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        Toast.makeText(MainActivity.this, "You ignored!", Toast.LENGTH_LONG).show();
                        //refuseTestSpeed(uid);
                    }
                });
        builder.create().show();
    }

    private void handleShowProgressDialog(Message msg) {
        Bundle bundle = msg.getData();
        int max = bundle.getInt("max");
        showProgressDialog(max);
    }

    private void handleUpdateProgressDialog(Message msg) {
        Bundle bundle = msg.getData();
        int increment = bundle.getInt("increment");
        int prog = progress.getProgress();
        prog += increment;
        progress.setProgress(prog);
        String tip = String.format("%.2f", prog * 1.0 / 1024);
        tip += "KB";
        progress.setMessage(tip);

        if (prog == progress.getMax()) {
            finishedTranferring = true;
        }
    }

    private void handleShowTestResult(Message msg) {
        Bundle bundle = msg.getData();
        String result = bundle.getString("result");
        final AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this)
                .setTitle("Test Result")
                .setMessage(result)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        // todo: dismiss
                    }
                });
        builder.create().show();
    }

    private void showFriend() {
        mFriend.setVisibility(View.VISIBLE);
    }

    public void showMenu(View v, final Friend friend) {
        PopupMenu popup = new PopupMenu(this, v);

        popup.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
            public boolean onMenuItemClick(MenuItem item) {
                String action = String.valueOf(item.getTitle());

                if (action.equals("delete")) {
                    removeFriend(friend.mUid);
                    //mAdapter.deleteFriend(friend);
                    return true;
                } else if (action.equals("test speed")) {
                    requestTestSpeed(friend.mUid);
                    return true;
                } else {
                    return false;
                }
            }
        });

        popup.inflate(R.menu.menu_popup);

        if (!friend.mConnected) {
            popup.getMenu().removeItem(R.id.menu_testspeed);
        }

        popup.show();
    }

    public void showProgressDialog(int max){
        progress=new ProgressDialog(this);
        progress.setTitle("ProgressDialog");
        progress.setMessage("Transferring file...");
        progress.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
        progress.setMax(max);
        progress.show();
    }

    public void showTestResult(String result) {
        while (!finishedTranferring) {
            try {
                Thread.sleep(10);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        Message msg = new Message();
        msg.what = MSG_SHOW_TEST_RESULT;
        Bundle bundle = new Bundle();
        bundle.putString("result", result);
        msg.setData(bundle);
        mHandler.sendMessage(msg);
    }

    private static Boolean firstUpdate = true;
    public void updateProgress(int max, int increment) {
        if (firstUpdate) {
            firstUpdate = false;
            Message msg = new Message();
            msg.what = MSG_SHOW_PROGRESS_DIALOG;
            Bundle bundle = new Bundle();
            bundle.putInt("max", max);
            msg.setData(bundle);
            mHandler.sendMessage(msg);
        }

        Message msg = new Message();
        msg.what = MSG_UPDATE_PROGRESS_DIALOG;
        Bundle bundle = new Bundle();
        bundle.putInt("increment", increment);
        msg.setData(bundle);
        mHandler.sendMessage(msg);
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native void startCarrier(AssetManager assetManager, String fileTransferred, String fileReceived);
    public native void stopCarrier();
    public native int addFriend(String uid);
    public native ArrayList<Friend> getFriendList();
    public native int acceptFriend(String uid);
    public native int removeFriend(String uid);
    public native static String getUserId();
    public native static String getAddress();
    public native void requestTestSpeed(String uid);
    public native void acceptTestSpeed(String uid);
    public native void refuseTestSpeed(String uid);

    class StartThread implements Runnable {
        private String fileTransferred;
        private String fileReceived;

        StartThread(String fileTransferred, String fileReceived) {
            this.fileTransferred = fileTransferred;
            this.fileReceived = fileReceived;
        }

        @Override
        public void run() {
            startCarrier(getAssets(), fileTransferred, fileReceived);
        }
    }
}