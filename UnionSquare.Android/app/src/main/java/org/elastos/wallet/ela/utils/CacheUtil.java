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

import com.blankj.utilcode.util.CacheDiskUtils;

import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.bean.IPEntity;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.did.entity.DIDInfoEntity;
import org.elastos.wallet.ela.ui.mine.bean.MessageEntity;
import org.elastos.wallet.ela.ui.vote.bean.Area;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;

import java.io.File;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

public class CacheUtil {
    private static File file = new File(MyApplication.getAppContext().getFilesDir().getParent() + "/keeps");

    public static ArrayList<Area> getArea() {
        return (ArrayList<Area>) CacheDiskUtils.getInstance(file)
                .getSerializable("area");
    }

    public static void setArea(List<Area> list) {
        if (list == null) {
            return;
        }
        if (list.size() == 0) {
            CacheDiskUtils.getInstance(file).remove("area");
            return;
        }
        CacheDiskUtils.getInstance(file).put("area", (Serializable) list, CacheDiskUtils.DAY * 360);
    }

    public static void removeArea() {
        CacheDiskUtils.getInstance(file).remove("area");
    }

    /*
     */

    /**
     * 慎用
     *//*

    public static void clear() {
        CacheDiskUtils.getInstance(file).clear();
    }
*/
    public static void converDepositBean2String() {
        Wallet wallet = new RealmUtil().queryDefauleWallet();
        ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean> list = (ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean>) CacheDiskUtils.getInstance(file)
                .getSerializable("list" + wallet.getWalletId());
        if (list != null && list.size() > 0) {
            setProducerList(list);
            CacheDiskUtils.getInstance(file).remove("list" + wallet.getWalletId());
        }
    }

    public static void setProducerList(List<VoteListBean.DataBean.ResultBean.ProducersBean> list) {
        if (list == null) {
            return;
        }
        Wallet wallet = new RealmUtil().queryDefauleWallet();
        if (list.size() == 0) {
            CacheDiskUtils.getInstance(file).remove("depositlistlist" + wallet.getWalletId());
            return;
        }

        ArrayList<String> listString = new ArrayList<>();
        for (VoteListBean.DataBean.ResultBean.ProducersBean bean : list) {
            listString.add(bean.getOwnerpublickey());
        }
        CacheDiskUtils.getInstance(file).put("depositlistlist" + wallet.getWalletId(), listString, CacheDiskUtils.DAY * 360);
    }

    public static ArrayList<String> getProducerListString() {
        Wallet wallet = new RealmUtil().queryDefauleWallet();
        ArrayList<String> list = (ArrayList<String>) CacheDiskUtils.getInstance(file)
                .getSerializable("depositlistlist" + wallet.getWalletId());
        return list == null ? new ArrayList<>() : list;
    }

    public static void setProducerListString(List<String> list) {
        if (list == null) {
            return;
        }
        Wallet wallet = new RealmUtil().queryDefauleWallet();
        if (list.size() == 0) {
            CacheDiskUtils.getInstance(file).remove("depositlistlist" + wallet.getWalletId());
            return;
        }

        CacheDiskUtils.getInstance(file).put("depositlistlist" + wallet.getWalletId(), (Serializable) list, CacheDiskUtils.DAY * 360);
    }

    public static void converCrBean2String() {
        Wallet wallet = new RealmUtil().queryDefauleWallet();
        ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> list = (ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean>) CacheDiskUtils.getInstance(file)
                .getSerializable("CRlist1" + wallet.getWalletId());
        if (list != null && list.size() > 0) {
            setCRProducerList(list);
            CacheDiskUtils.getInstance(file).remove("CRlist1" + wallet.getWalletId());
        }
    }


    public static void setCRProducerList(List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> list) {
        if (list == null) {
            return;
        }
        Wallet wallet = new RealmUtil().queryDefauleWallet();
        if (list.size() == 0) {
            CacheDiskUtils.getInstance(file).remove("CRlistString" + wallet.getWalletId());
            return;
        }
        ArrayList<String> listString = new ArrayList<>();
        for (CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean : list) {
            listString.add(bean.getDid());
        }

        CacheDiskUtils.getInstance(file).put("CRlistString" + wallet.getWalletId(), listString, CacheDiskUtils.DAY * 360);
    }

    public static ArrayList<String> getCRProducerListString() {
        Wallet wallet = new RealmUtil().queryDefauleWallet();
        ArrayList<String> list = (ArrayList<String>) CacheDiskUtils.getInstance(file)
                .getSerializable("CRlistString" + wallet.getWalletId());
        return list == null ? new ArrayList<>() : list;
    }

    public static void setCRProducerListString(List<String> list) {
        if (list == null) {
            return;
        }
        Wallet wallet = new RealmUtil().queryDefauleWallet();
        if (list.size() == 0) {
            CacheDiskUtils.getInstance(file).remove("CRlistString" + wallet.getWalletId());
            return;
        }

        CacheDiskUtils.getInstance(file).put("CRlistString" + wallet.getWalletId(), (Serializable) list, CacheDiskUtils.DAY * 360);
    }

    public static ArrayList<DIDInfoEntity> getDIDInfoList() {
        ArrayList<DIDInfoEntity> list = (ArrayList<DIDInfoEntity>) CacheDiskUtils.getInstance(file)
                .getSerializable("DIDInfoList");
        return list == null ? new ArrayList<>() : list;
    }

    //Set<String> serverList = new HashSet<>();
    public static void setDIDInfoList(List<DIDInfoEntity> list) {
        if (list == null) {
            return;
        }
        if (list.size() == 0) {
            CacheDiskUtils.getInstance(file).remove("DIDInfoList");
            return;
        }
        CacheDiskUtils.getInstance(file).put("DIDInfoList", (Serializable) list, CacheDiskUtils.DAY * 360);
    }

    public static void remove(String key) {

        CacheDiskUtils.getInstance(file).remove(key);

    }

    public static ArrayList<IPEntity> getIps() {
        ArrayList<IPEntity> list = (ArrayList<IPEntity>) CacheDiskUtils.getInstance(file)
                .getSerializable("ips");
        return list == null ? new ArrayList<>() : list;
    }

    //Set<String> serverList = new HashSet<>();
    public static void setIps(List<IPEntity> list) {
        if (list == null) {
            return;
        }
        if (list.size() == 0) {
            CacheDiskUtils.getInstance(file).remove("ips");
            return;
        }
        CacheDiskUtils.getInstance(file).put("ips", (Serializable) list, CacheDiskUtils.DAY * 360);
    }


    private static void setMessage(List<MessageEntity> list, String key) {
        if (list == null) {
            return;
        }
        if (list.size() == 0) {
            CacheDiskUtils.getInstance(file).remove(key);
            return;
        }
        CacheDiskUtils.getInstance(file).put(key, (Serializable) list, CacheDiskUtils.DAY * 360);
    }

    private static ArrayList<MessageEntity> getMessage(String key) {
        ArrayList<MessageEntity> list = (ArrayList<MessageEntity>) CacheDiskUtils.getInstance(file)
                .getSerializable(key);
        return list == null ? new ArrayList<>() : list;
    }

    public static void setUnReadMessage(List<MessageEntity> list) {
        setMessage(list, "unReadMessage");
    }

    public static ArrayList<MessageEntity> getUnReadMessage() {

        return getMessage("unReadMessage");
    }

    public static void setReadMessage(List<MessageEntity> list) {
        setMessage(list, "readMessage");
    }

    public static ArrayList<MessageEntity> getReadMessage() {

        return getMessage("readMessage");
    }


}
