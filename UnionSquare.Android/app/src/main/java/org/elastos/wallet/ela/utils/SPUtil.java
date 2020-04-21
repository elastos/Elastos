/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package org.elastos.wallet.ela.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.support.annotation.NonNull;

import java.util.Iterator;
import java.util.Map;
import java.util.Set;


public class SPUtil {

    private static final String detrust_fileName = "ela_sp";
    private static final String LANGUAGE = "language";
    private static final String SERVER = "server";
    private static final String SERVERLIST = "serverList";
    private Context context;


    public SPUtil(Context context) {
        this.context = context;
    }

    /**
     * SharedPreferences存储简单的配置文件
     */
    private SharedPreferences getSharedPreferences(@NonNull String fileName) {
        return context.getSharedPreferences(fileName, Context.MODE_PRIVATE);
    }

    /**
     * 存储多个数据
     *
     * @param map
     */
    public void setSharedPreferencesKeyVale(@NonNull String fileName, @NonNull Map<String, String> map) {
        SharedPreferences.Editor edit = getSharedPreferences(fileName).edit();
        Set<String> keySet = map.keySet();
        Iterator<String> iterator = keySet.iterator();
        while (iterator.hasNext()) {
            String key = iterator.next();
            edit.putString(key, map.get(key));
        }
        edit.commit();
    }

    /**
     * 存储单个String数据
     */
    public void setSharedPreferencesKeyVale(String key, String value) {
        SharedPreferences.Editor edit = getSharedPreferences(detrust_fileName).edit();
        edit.putString(key, value);
        edit.commit();
    }

    /**
     * 获取单个String数据
     */
    public String getSharedPreferencesKeyVale(String key, String defValue) {
        return getSharedPreferences(detrust_fileName).getString(key, defValue);
    }

    /**
     * 存储单个set数据
     */
    public void setSharedPreferencesKeyVale(String key, Set<String> value) {
        SharedPreferences.Editor edit = getSharedPreferences(detrust_fileName).edit();
        edit.putStringSet(key, value);
        edit.commit();
    }

    /**
     * 获取单个set数据
     */
    public Set<String> getSharedPreferencesKeyVale(String key, Set<String> defValue) {
        return getSharedPreferences(detrust_fileName).getStringSet(key, defValue);
    }

    /**
     * 存储单个int数据
     */
    public void setSharedPreferencesKeyVale(String key, int value) {
        SharedPreferences.Editor edit = getSharedPreferences(detrust_fileName).edit();
        edit.putInt(key, value);
        edit.commit();
    }

    /**
     * 获取单个int数据
     */
    public int getSharedPreferencesKeyVale(String key, int defValue) {
        return getSharedPreferences(detrust_fileName).getInt(key, defValue);
    }

    //1english  0chinese
    public void setLanguage(int value) {
        setSharedPreferencesKeyVale(LANGUAGE, value);
    }


    public int getLanguage() {
        return getSharedPreferencesKeyVale(LANGUAGE, -1);
    }

    public void setDefaultServer(String value) {
        setSharedPreferencesKeyVale(SERVER, value);
    }


    public String getDefaultServer(String defaultServer) {
        return getSharedPreferencesKeyVale(SERVER, defaultServer);
    }

    public void setDefaultServerList(Set<String> value) {
        setSharedPreferencesKeyVale(SERVERLIST, value);
    }


    public Set<String> getDefaultServerList(Set<String> defaultSet) {
        return getSharedPreferencesKeyVale(SERVERLIST, defaultSet);
    }


    /**
     * 获取所有的存储数据
     *
     * @return
     */
    public Map<String, ?> getSharedPreferencesVales(@NonNull String fileName) {
        return getSharedPreferences(fileName).getAll();
    }

    /**
     * 获取单个数据
     */
    public String getSharedPreferenceVales(@NonNull String fileName, String key) {
        return getSharedPreferences(fileName).getString(key, "");
    }

    /**
     * 获取单个数据
     */
    public String getSharedPreferenceVales(String key) {
        return getSharedPreferences(detrust_fileName).getString(key, "");
    }


    //是不是第一次登陆
    public boolean getFristLogin() {
        return getSharedPreferences(detrust_fileName).getBoolean("isfrist1", true);
    }

    //设置不是第一次登陆
    public void setFristLogin() {
        SharedPreferences.Editor edit = getSharedPreferences(detrust_fileName).edit();
        edit.remove("isfrist");
        edit.putBoolean("isfrist1", false);
        edit.commit();
    }//是不是第一次登陆
    public boolean isRefreshCache() {
        return getSharedPreferences(detrust_fileName).getBoolean("refreshCache", true);
    }

    //设置不是第一次登陆
    public void setRefreshCache() {
        SharedPreferences.Editor edit = getSharedPreferences(detrust_fileName).edit();
        edit.putBoolean("refreshCache", false);
        edit.commit();
    }

    //是不是第一次登陆
    public String getDeviceId() {
        return getSharedPreferences(detrust_fileName).getString("deviceId", "");
    }

    //设置不是第一次登陆
    public void setDeviceId(String id) {
        SharedPreferences.Editor edit = getSharedPreferences(detrust_fileName).edit();
        edit.putString("deviceId", id);
        edit.commit();
    }

    //是不是打开消息通知红点
    public boolean isOpenRedPoint() {
        return getSharedPreferences(detrust_fileName).getBoolean("openReadPoint", true);
    }

    //设置是不是打开消息通知红点
    public void setOpenRedPoint(boolean tag) {
        SharedPreferences.Editor edit = getSharedPreferences(detrust_fileName).edit();
        edit.putBoolean("openReadPoint", tag);
        edit.commit();
    } //是不是打开消息通知
    public boolean isOpenSendMsg() {
        return getSharedPreferences(detrust_fileName).getBoolean("openSendMsg", true);
    }

    //设置是不是打开消息通知
    public void setOpenSendMsg(boolean tag) {
        SharedPreferences.Editor edit = getSharedPreferences(detrust_fileName).edit();
        edit.putBoolean("openSendMsg", tag);
        edit.commit();
    }
}
