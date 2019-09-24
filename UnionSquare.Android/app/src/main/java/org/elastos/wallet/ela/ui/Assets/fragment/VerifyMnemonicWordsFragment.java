package org.elastos.wallet.ela.ui.Assets.fragment;


import android.os.Bundle;
import android.support.v7.widget.DefaultItemAnimator;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.CreateWalletBean;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.listener.RealmTransactionAbs;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.adapter.CommonTextViewAdapter;
import org.elastos.wallet.ela.ui.Assets.adapter.CommonTextViewTwoAdapter;
import org.elastos.wallet.ela.ui.Assets.bean.Word;
import org.elastos.wallet.ela.ui.Assets.fragment.mulsignwallet.CreateMulWalletFragment;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonCreateSubWalletPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.CreateMasterWalletPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonCreateSubWalletViewData;
import org.elastos.wallet.ela.ui.Assets.viewdata.CreaterWalletViewData;
import org.elastos.wallet.ela.utils.RxEnum;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;
import io.github.xudaojie.qrcodelib.BuildConfig;

/**
 * 验证助记词
 */
public class VerifyMnemonicWordsFragment extends BaseFragment implements CreaterWalletViewData, CommonCreateSubWalletViewData {


    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    private String mnemonic = "";
    private ArrayList<Word> putList;
    private List<Word> readList;
    @BindView(R.id.rv_mnemonic_put)
    RecyclerView rvMnemonicPut;
    @BindView(R.id.rv_mnemonic_read)
    RecyclerView rvMnemonicRead;

    private int openType;
    private CreateWalletBean createWalletBean;
    private RealmUtil realmUtil;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_verify_mnemonic_words;
    }


    public List<Word> sort(List<Word> temp) {
        ArrayList<Word> newList = new ArrayList<Word>(temp);
        Collections.shuffle(newList);
        return newList;
    }

    @Override
    protected void setExtraData(Bundle data) {
        mnemonic = data.getString("mnemonic");
        openType = data.getInt("openType", RxEnum.CREATEDEFAULT.ordinal());
        createWalletBean = data.getParcelable("createWalletBean");
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.verify_mnemonic_words));
        realmUtil = new RealmUtil();
        String[] temp = mnemonic.split(" ");
        readList = new ArrayList<>();
        for (int i = 0; i < temp.length; i++) {
            readList.add(new Word(temp[i], true));
        }
        putList = new ArrayList<>();
        rvMnemonicPut.setLayoutManager(new GridLayoutManager(this.getContext(), 4));
        rvMnemonicPut.setItemAnimator(new DefaultItemAnimator());
        rvMnemonicRead.setLayoutManager(new GridLayoutManager(this.getContext(), 4));
        rvMnemonicRead.setItemAnimator(new DefaultItemAnimator());
        CommonTextViewAdapter putAdapter = new CommonTextViewAdapter(putList, this.getContext());
        CommonTextViewTwoAdapter readAdapter = new CommonTextViewTwoAdapter(sort(readList), this.getContext());
        putAdapter.setOnItemOnclickListner(new CommonTextViewAdapter.OnItemClickListner() {
            @Override
            public void onItemClick(View v, int position) {
                putAdapter.removeData(position);
            }
        });
        rvMnemonicPut.setAdapter(putAdapter);


        readAdapter.setOnItemOnclickListner(new CommonTextViewTwoAdapter.OnItemClickListner() {
            @Override
            public void onItemClick(View v, int position) {
                if (putList.size() >= readList.size()) {
                    return;
                }
                ((TextView) v).setTextColor(getResources().getColor(R.color.qmui_config_color_50_white));
                readAdapter.moveRandomData();
                putList.add(new Word(((TextView) v).getText().toString(), false));
                putAdapter.notifyItemInserted(putList.size() - 1);

            }
        });
        rvMnemonicRead.setAdapter(readAdapter);
    }


    @OnClick(R.id.sb_create_wallet)
    public void onViewClicked() {
        if (BuildConfig.DEBUG || putList.toString().equals(readList.toString())) {
            if (openType == RxEnum.MANAGER.ordinal()) {
                //钱包管理的导出助记词
                popTo(WallletManageFragment.class, false);
            } else if (openType == RxEnum.PRIVATEKEY.ordinal()) {
                popTo(CreateMulWalletFragment.class, false);
                post(RxEnum.CREATEPRIVATEKEY.ordinal(), null, createWalletBean);
            } else {
                new CreateMasterWalletPresenter().createMasterWallet(createWalletBean.getMasterWalletID(), createWalletBean.getMnemonic(), createWalletBean.getPhrasePassword(),
                        createWalletBean.getPayPassword(), createWalletBean.getSingleAddress(), this);
            }

        } else {
            showToast(getString(R.string.wrongMnemonic));

        }

    }


    @Override
    public void onCreateMasterWallet(String baseInfo) {
        if (baseInfo != null) {
            new CommonCreateSubWalletPresenter().createSubWallet(createWalletBean.getMasterWalletID(), MyWallet.ELA, this);

        }
    }

    @Override
    public void onCreateSubWallet(String data) {
        if (data != null) {
            //创建Mainchain子钱包
            Wallet masterWallet = realmUtil.updateWalletDetial(createWalletBean.getMasterWalletName(), createWalletBean.getMasterWalletID(), data);
            realmUtil.updateSubWalletDetial(createWalletBean.getMasterWalletID(), data, new RealmTransactionAbs() {
                @Override
                public void onSuccess() {
                    realmUtil.updateWalletDefault(createWalletBean.getMasterWalletID(), new RealmTransactionAbs() {
                        @Override
                        public void onSuccess() {
                            post(RxEnum.ONE.ordinal(), null, masterWallet);
                            //成功跳转首页
                            toMainFragment();
                        }
                    });
                }
            });


        }
    }
}
