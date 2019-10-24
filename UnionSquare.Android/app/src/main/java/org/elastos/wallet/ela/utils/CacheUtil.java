package org.elastos.wallet.ela.utils;

import com.blankj.utilcode.util.CacheDoubleUtils;

import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.did.entity.DIDInfoEntity;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

public class CacheUtil {
    public static ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean> getProducerList() {
        Wallet wallet = new RealmUtil().queryDefauleWallet();
        return (ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean>) CacheDoubleUtils.getInstance()
                .getSerializable("list" + wallet.getWalletId());
    }

    public static void setProducerList(List<VoteListBean.DataBean.ResultBean.ProducersBean> list) {
        Wallet wallet = new RealmUtil().queryDefauleWallet();
        CacheDoubleUtils.getInstance().put("list" + wallet.getWalletId(), (Serializable) list, CacheDoubleUtils.DAY * 360);
    }

    public static ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> getCRProducerList() {
        Wallet wallet = new RealmUtil().queryDefauleWallet();
        ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> list = (ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean>) CacheDoubleUtils.getInstance()
                .getSerializable("CRlist1" + wallet.getWalletId());
        return list == null ? new ArrayList<>() : list;
    }

    public static void setCRProducerList(List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> list) {
        Wallet wallet = new RealmUtil().queryDefauleWallet();
        CacheDoubleUtils.getInstance().put("CRlist1" + wallet.getWalletId(), (Serializable) list, CacheDoubleUtils.DAY * 360);
    }


    public static ArrayList<DIDInfoEntity> getDIDInfoList() {
        ArrayList<DIDInfoEntity> list = (ArrayList<DIDInfoEntity>) CacheDoubleUtils.getInstance()
                .getSerializable("DIDInfoList");
        return list == null ? new ArrayList<>() : list;
    }

    //Set<String> serverList = new HashSet<>();
    public static void setDIDInfoList(List<DIDInfoEntity> list) {
        CacheDoubleUtils.getInstance().put("DIDInfoList", (Serializable) list, CacheDoubleUtils.DAY * 360);
    }

}
