package org.elastos.wallet.ela.utils;

import com.blankj.utilcode.util.CacheDiskUtils;

import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.ui.did.entity.DIDInfoEntity;
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
        CacheDiskUtils.getInstance(file).put("area", (Serializable) list, CacheDiskUtils.DAY * 360);
    }

    public static ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean> getProducerList() {
        Wallet wallet = new RealmUtil().queryDefauleWallet();
        return (ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean>) CacheDiskUtils.getInstance(file)
                .getSerializable("list" + wallet.getWalletId());
    }

    public static void setProducerList(List<VoteListBean.DataBean.ResultBean.ProducersBean> list) {
        Wallet wallet = new RealmUtil().queryDefauleWallet();
        CacheDiskUtils.getInstance(file).put("list" + wallet.getWalletId(), (Serializable) list, CacheDiskUtils.DAY * 360);
    }

    public static ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> getCRProducerList() {
        Wallet wallet = new RealmUtil().queryDefauleWallet();
        ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> list = (ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean>) CacheDiskUtils.getInstance(file)
                .getSerializable("CRlist1" + wallet.getWalletId());
        return list == null ? new ArrayList<>() : list;
    }

    public static void setCRProducerList(List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> list) {
        Wallet wallet = new RealmUtil().queryDefauleWallet();
        CacheDiskUtils.getInstance(file).put("CRlist1" + wallet.getWalletId(), (Serializable) list, CacheDiskUtils.DAY * 360);
    }


    public static ArrayList<DIDInfoEntity> getDIDInfoList() {
        ArrayList<DIDInfoEntity> list = (ArrayList<DIDInfoEntity>) CacheDiskUtils.getInstance(file)
                .getSerializable("DIDInfoList");
        return list == null ? new ArrayList<>() : list;
    }

    //Set<String> serverList = new HashSet<>();
    public static void setDIDInfoList(List<DIDInfoEntity> list) {
        CacheDiskUtils.getInstance(file).put("DIDInfoList", (Serializable) list, CacheDiskUtils.DAY * 360);
    }

    public static CredentialSubjectBean getCredentialSubjectBean(String id) {
        CredentialSubjectBean bean = CacheDiskUtils.getInstance(file)
                .getParcelable(id, CredentialSubjectBean.CREATOR);
        if (bean == null) {
            bean = new CredentialSubjectBean(id);
        } else if (bean.getDid() == null) {
            bean.setDid(id);
        }

        return bean;

    }

    public static void setCredentialSubjectBean(CredentialSubjectBean credentialSubjectBean) {
        if (credentialSubjectBean == null) {
            return;
        }
        if (credentialSubjectBean.getSocial() != null || credentialSubjectBean.getInfo() != null || credentialSubjectBean.getIntro() != null) {
            CacheDiskUtils.getInstance(file)
                    .put(credentialSubjectBean.getDid(), credentialSubjectBean, CacheDiskUtils.DAY * 360);
        } else {
            CacheDiskUtils.getInstance(file).remove(credentialSubjectBean.getDid());
        }
    }

    public static void remove(String key) {

        CacheDiskUtils.getInstance(file).remove(key);

    }

    public static ArrayList<String> getIps() {
        ArrayList<String> list = (ArrayList<String>) CacheDiskUtils.getInstance(file)
                .getSerializable("ips");
        return list == null ? new ArrayList<>() : list;
    }

    //Set<String> serverList = new HashSet<>();
    public static void setIps(List<String> list) {
        CacheDiskUtils.getInstance(file).put("ips", (Serializable) list, CacheDiskUtils.DAY * 360);
    }
}
