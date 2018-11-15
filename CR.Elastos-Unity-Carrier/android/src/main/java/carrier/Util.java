package carrier;

import com.elastos_rn_framework.MainApplication;
import android.content.Context;
import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.InputStream;
import java.lang.reflect.Method;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;

import com.facebook.react.bridge.WritableNativeMap;
import com.facebook.react.bridge.WritableArray;

public class Util {
    private Context mContext;
    private static String TAG = "[ CarrierPlugin ]";

    public static Util _util;
    public static Util singleton() {
        if (_util == null) {
            _util = new Util();
        }
        return _util;
    }

    public Util(){
        mContext = MainApplication.getContext();
    }

    public String getCarrierFilePath(String name){
        try{
            return mContext.getFilesDir().getAbsolutePath()+"/elastos_carrier/"+name;

        }catch(Exception e){
            error("getStoragePaths() failed" + e);
        }
        return "";
    }

    public void log(String log){
        Log.d(TAG, log);
    }
    public void error(String log){
        Log.e(TAG, log);
    }

    public String hash256(String string) {
        MessageDigest md = null;
        String result = null;
        byte[] bt = string.getBytes();
        try {
            md = MessageDigest.getInstance("SHA-256");
            md.update(bt);
            result = bytes2Hex(md.digest()); // to HexString
        } catch (NoSuchAlgorithmException e) {
            return null;
        }
        return result;
    }

    public String bytes2Hex(byte[] bts) {
        String des = "";
        String tmp = null;
        for (int i = 0; i < bts.length; i++) {
            tmp = (Integer.toHexString(bts[i] & 0xFF));
            if (tmp.length() == 1) {
                des += "0";
            }
            des += tmp;
        }
        return des;
    }

    public JSONObject parseOneParam(String key, Object value) throws JSONException {
        JSONObject jsonObject = new JSONObject();
        jsonObject.put(key, value);
        return jsonObject;
    }

    public WritableNativeMap toJsObject(HashMap<String, String> map){
        WritableNativeMap rs = new WritableNativeMap();
        for (String key : map.keySet()) {
            String val = map.get(key);
            rs.putString(key, val);
        }
        return rs;
    }
}
