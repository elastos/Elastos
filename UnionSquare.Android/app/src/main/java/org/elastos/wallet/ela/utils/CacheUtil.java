package org.elastos.wallet.ela.utils;

import com.blankj.utilcode.util.CacheDoubleUtils;

import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
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
    public static ArrayList<CRListBean.DataBean.ResultBean.ProducersBean> getCRProducerList() {
        Wallet wallet = new RealmUtil().queryDefauleWallet();
        return (ArrayList<CRListBean.DataBean.ResultBean.ProducersBean>) CacheDoubleUtils.getInstance()
                .getSerializable("CRlist" + wallet.getWalletId());
    }

    public static void setCRProducerList(List<CRListBean.DataBean.ResultBean.ProducersBean> list) {
        Wallet wallet = new RealmUtil().queryDefauleWallet();
        CacheDoubleUtils.getInstance().put("CRlist" + wallet.getWalletId(), (Serializable) list, CacheDoubleUtils.DAY * 360);
    }
}
