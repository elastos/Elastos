package org.elastos.wallet.ela;

import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;

import org.elastos.wallet.R;
import org.elastos.wallet.core.MasterWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.bean.SubWalletBasicInfo;
import org.elastos.wallet.ela.ui.Assets.fragment.HomeWalletFragment;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonObjectWithMethNameViewData;
import org.elastos.wallet.ela.ui.main.MainFragment;

import java.util.List;

import butterknife.BindView;


public class FirstFragment extends BaseFragment implements CommmonObjectWithMethNameViewData {

    private String[] data;
    @BindView(R.id.tv_word)
    TextView tvWord;
    FirstPresenter firstPresenter;
    private RealmUtil realmUtil;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_first;
    }

    @Override
    protected void initView(View view) {
        firstPresenter = new FirstPresenter();
        firstPresenter.getAllMasterWalletIds(this);
        realmUtil = new RealmUtil();

    }

    private void onFirst(String[] data) {
        if (data.length > 0) {
            // Bundle bundle = new Bundle();
            // bundle.putStringArrayList("ids", (ArrayList<String>) data);
            //对比本地数据库并同步
            sync(data);
            startWithPop(MainFragment.class, getArguments());
        } else {
            startWithPop(HomeWalletFragment.class, null);
        }
    }


    public static FirstFragment newInstance() {
        Bundle args = new Bundle();
        FirstFragment fragment = new FirstFragment();
        fragment.setArguments(args);
        return fragment;
    }



  /*  @Override
    protected void requstPermissionOk() {
        //申请权限成功
        super.requstPermissionOk();
        deayedSkipActivity();
    }*/

    private void deayedSkipActivity() {
        new Handler().postDelayed(new Runnable() {
            @Override
            public void run() {
                onFirst(data);
            }
        }, 0);
    }

    @Override
    public void onGetCommonData(String methodname, Object data) {
        switch (methodname) {
            case "getAllMasterWalletIds":
                this.data = (String[]) data;
                deayedSkipActivity();
                break;
            case "getMasterWalletBaseEntity":
                MasterWallet masterWallet = (MasterWallet) data;
                SubWalletBasicInfo.InfoBean.AccountBean account = JSON.parseObject(masterWallet.GetBasicInfo(), SubWalletBasicInfo.InfoBean.AccountBean.class);
                boolean singleAddress = account.isSingleAddress();
                int type = getType(account);
                Wallet newWallet = new Wallet();
                newWallet.setWalletId(masterWallet.GetID());
                newWallet.setType(type);
                newWallet.setWalletName("Unknown");
                newWallet.setSingleAddress(singleAddress);
                realmUtil.updateWalletDetial(newWallet);

                break;
        }

    }

    public void sync(String[] masterWalletIds) {
        List<Wallet> list = realmUtil.queryUserAllWallet();
        for (String id : masterWalletIds) {
            boolean flag = false;
            for (Wallet wallet : list) {
                String walletId = wallet.getWalletId();
                if (id.equals(walletId)) {
                    flag = true;
                    break;
                }

            }
            if (!flag) {
                firstPresenter.getMasterWalletBaseEntity(id, this);

            }

        }
        for (Wallet wallet : list) {
            boolean flag = false;
            String walletId = wallet.getWalletId();
            for (String id : masterWalletIds) {
                if (id.equals(walletId)) {
                    flag = true;
                    break;
                }
            }
            if (!flag) {
                realmUtil.deleteWallet(wallet.getWalletId());//删除钱包和其子钱包
            }
        }

    }

    private int getType(SubWalletBasicInfo.InfoBean.AccountBean account) {
        boolean Readonly = account.isReadonly();
        int N = account.getN();
        int M = account.getM();
        //0 普通单签 1单签只读 2普通多签 3多签只读
        if (N > 1) {
            //多签
            if (Readonly) {
                return 3;
            } else {
                return 2;
            }

        } else {
            //单签
            if (Readonly) {
                return 1;
            } else {
                return 0;
            }

        }

    }
}
