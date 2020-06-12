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

package org.elastos.wallet.ela.ui.Assets.fragment;

import android.app.Dialog;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;

import org.elastos.did.DID;
import org.elastos.did.DIDDocument;
import org.elastos.did.DIDStore;
import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.DIDStoreException;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.SubWallet;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.fragment.mulsignwallet.ShowMulWallletPublicKeyFragment;
import org.elastos.wallet.ela.ui.Assets.presenter.WalletManagePresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.mulwallet.CreatMulWalletPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.WalletManageViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonBooleanEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonObjEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.bean.ISubWalletListEntity;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.ui.did.fragment.AddDIDFragment;
import org.elastos.wallet.ela.ui.did.fragment.DidDetailFragment;
import org.elastos.wallet.ela.ui.did.presenter.AddDIDPresenter;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.FileUtile;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener2;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

public class WalletManageFragment extends BaseFragment implements WarmPromptListener, WalletManageViewData, CommmonStringWithMethNameViewData, NewBaseViewData {

    private static final String DELETE = "delete";

    private static final String OUTPORTMN = "outportmm";
    private static final String DIDINIT = "didinit";
    private static final String OUTPORTMUPK = "outportmupk";
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_updatename)
    TextView tvUpdatename;
    @BindView(R.id.ll_updatename)
    LinearLayout llUpdatename;
    @BindView(R.id.ll_updatepwd)
    LinearLayout llUpdatepwd;
    @BindView(R.id.ll_exportkeystore)
    LinearLayout llExportkeystore;
    @BindView(R.id.ll_exportmnemonic)
    LinearLayout llExportmnemonic;
    @BindView(R.id.ll_sign)
    LinearLayout llSign;
    @BindView(R.id.ll_did)
    LinearLayout llDid;
    @BindView(R.id.ll_exportreadonly)
    LinearLayout llExportreadonly;
    @BindView(R.id.ll_showmulpublickey)
    LinearLayout llShowmulpublickey;
    @BindView(R.id.ll_showwalletpublickey)
    LinearLayout llShowwalletpublickey;
    private DialogUtil dialogUtil;
    String dialogAction = null;
    private Dialog dialog;
    private Wallet wallet;
    private WalletManagePresenter presenter;
    private String payPasswd;
    private DIDStore store;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_wallet_manage;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        wallet = data.getParcelable("wallet");
        if (wallet != null)
            tvUpdatename.setText(wallet.getWalletName());


    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.walletmanage);
        dialogUtil = new DialogUtil();
        presenter = new WalletManagePresenter();
        registReceiver();
        switch (wallet.getType()) {
            //0 普通单签 1单签只读 2普通多签 3多签只读
            case 0:
                llShowwalletpublickey.setVisibility(View.GONE);
                break;
            case 1:
                llDid.setVisibility(View.GONE);
                llUpdatepwd.setVisibility(View.GONE);
                llExportmnemonic.setVisibility(View.GONE);
                llShowwalletpublickey.setVisibility(View.GONE);
                break;
            case 2:
                llDid.setVisibility(View.GONE);
                llExportmnemonic.setVisibility(View.GONE);
                llExportreadonly.setVisibility(View.GONE);
                llShowmulpublickey.setVisibility(View.GONE);
                break;
            case 3:
                llDid.setVisibility(View.GONE);
                llUpdatepwd.setVisibility(View.GONE);
                llExportmnemonic.setVisibility(View.GONE);
                llExportreadonly.setVisibility(View.GONE);
                llShowmulpublickey.setVisibility(View.GONE);
                break;
        }
    }

    @OnClick({R.id.tv_delete, R.id.ll_updatename, R.id.ll_updatepwd, R.id.ll_exportkeystore, R.id.ll_exportmnemonic,
            R.id.ll_sign, R.id.ll_exportreadonly, R.id.ll_showmulpublickey, R.id.ll_showwalletpublickey,
            R.id.ll_nodeconect, R.id.ll_did, R.id.ll_sync})
    public void onViewClicked(View view) {
        Bundle bundle = null;
        dialogAction = null;
        switch (view.getId()) {
            case R.id.ll_sync:
                //重置同步数据
                start(SyncResetFragment.class, getArguments());
                break;
            case R.id.tv_delete:
                //删除钱包弹框
                dialogUtil.showWarmPrompt1(getBaseActivity(), getString(R.string.deletewallletornot), new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        dialog = dialogUtil.showWarmPromptInput(getBaseActivity(), null, null, WalletManageFragment.this);
                        dialogAction = DELETE;
                    }
                });
                break;
            case R.id.ll_updatename:
                //修改钱包名称
                bundle = new Bundle();
                bundle.putString("walletId", wallet.getWalletId());
                start(WalletUpdateName.class, bundle);
                break;
            case R.id.ll_updatepwd:
                //修改密码
                bundle = new Bundle();
                bundle.putString("walletId", wallet.getWalletId());
                start(WalletUpdataPwdFragment.class, bundle);
                break;
            case R.id.ll_exportkeystore:
                //导出keystore
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                start(OutportKeystoreFragment.class, bundle);
                break;
            case R.id.ll_exportmnemonic:
                //导出助记词弹框
                dialog = dialogUtil.showWarmPromptInput(getBaseActivity(), null, null, this);
                dialogAction = OUTPORTMN;
                break;
            case R.id.ll_sign:
                //签名
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                start(SignTransactionFragment.class, bundle);
                break;
            case R.id.ll_exportreadonly:
                //导出只读
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                start(ExportReadOnlyFragment.class, bundle);
                break;
            case R.id.ll_showmulpublickey:
                //查看多签公钥
                presenter.getPubKeyInfo(wallet.getWalletId(), this);
                dialogAction = OUTPORTMUPK;
                break;
            case R.id.ll_showwalletpublickey:
                //查看多签钱包公钥
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                start(ShowMulWallletPublicKeyFragment.class, bundle);
                break;
            case R.id.ll_nodeconect:
                //节点连接设置
                start(NodeConnectSetFragment.class, getArguments());
                break;
            case R.id.ll_did:
                //did
                new AddDIDPresenter().getAllSubWallets(wallet.getWalletId(), this);

                break;

        }
    }


    @Override
    public void affireBtnClick(View view) {
        //这里只见他showWarmPromptInput的确认
        if (DELETE.equals(dialogAction) && (wallet.getType() == 1 || wallet.getType() == 3)) {
            //0 普通单签 1单签只读 2普通多签 3多签只读
            //不需要密码
            presenter.destroyWallet(wallet.getWalletId(), this);
            return;
        }
        //下面都需要密码
        payPasswd = ((EditText) view).getText().toString().trim();
        if (TextUtils.isEmpty(payPasswd)) {
            showToastMessage(getString(R.string.pwdnoempty));
            return;
        }
        switch (dialogAction) {
            case DELETE:
            case OUTPORTMUPK:
            case DIDINIT:
                //删除钱包  验证密码
                //查看多签公钥的兼容  验证密码
                //初始化did
                presenter.verifyPayPassword(wallet.getWalletId(), payPasswd, this);
                break;


            case OUTPORTMN:
                //导出助记词
                presenter.exportWalletWithMnemonic(wallet.getWalletId(), payPasswd, this);
                break;
        }

    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.UPDATA_WALLET_NAME.ordinal()) {
            wallet.setWalletName(result.getName());
            tvUpdatename.setText(result.getName());

        }
        if (integer == RxEnum.ADDPROPERTY.ordinal()) {
            //增加子钱包后
            initDid();
        }
    }

    private void initDid() {

        try {
            store = getMyDID().getDidStore();
            if (store.containsPrivateIdentity()) {//什么时候必须containsPrivateIdentity3
                initDIDAndresolve();
            } else {
                //获得私钥用于初始化did
                //did初始化获得密码
                dialog = dialogUtil.showWarmPromptInput(getBaseActivity(), null, null, this);
                dialogAction = DIDINIT;
            }
        } catch (DIDException e) {
            e.printStackTrace();
        }

    }

    private void initDIDAndresolve() {
        DID did = getMyDID().initDID(payPasswd);
        if (TextUtils.isEmpty(wallet.getDid()))
            new RealmUtil().upDataWalletDid(wallet.getWalletId(), getMyDID().getDidString());
        presenter.forceDIDResolve(did.toString(), this, null);

    }

    @Override
    public void onDestoryWallet(String data) {
        dialog.dismiss();
        RealmUtil realmUtil = new RealmUtil();
        realmUtil.deleteWallet(wallet.getWalletId());//删除钱包和其子钱包
        // realmUtil.deleteSubWallet(wallet.getWalletId());
        File file = new File(MyApplication.getRoutDir() + File.separator + wallet.getWalletId() + File.separator + "store");
        FileUtile.delFile(file);
        List<Wallet> wallets = realmUtil.queryUserAllWallet();
        if (wallets == null || wallets.size() == 0) {
            //没有其他钱包了
            toHomeWalletFragment();
            return;
        }
        post(RxEnum.DELETE.ordinal(), null, wallet.getWalletId());
        showToastMessage(getString(R.string.deletewalletsucess));
        popBackFragment();
    }

    @Override
    public void onGetCommonData(String methodname, String data) {
        //exportWalletWithMnemonic
        switch (methodname) {

            case "exportWalletWithMnemonic":
                dialog.dismiss();
                Bundle bundle = new Bundle();
                bundle.putString("mnemonic", data);
                bundle.putParcelable("wallet", wallet);
                start(OutportMnemonicFragment.class, bundle);
                break;

        }

    }


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "exportxPrivateKey":
                String privateKey = ((CommmonStringEntity) baseEntity).getData();
                dialog.dismiss();
                try {
                    store.initPrivateIdentity(privateKey, payPasswd);
                    initDIDAndresolve();
                } catch (DIDException e) {
                    e.printStackTrace();
                    showToast(getString(R.string.didinitfaile));
                }
                break;
            case "forceDIDResolve":
                Bundle bundle1 = new Bundle();
                bundle1.putParcelable("wallet", wallet);
                DIDDocument doc = (DIDDocument) ((CommmonObjEntity) baseEntity).getData();
                if (doc == null) {
                    //doc 为空  创建did
                    start(AddDIDFragment.class, bundle1);
                } else {
                    //展示did详情
                    try {
                        store.storeDid(doc);//存储本地
                        start(DidDetailFragment.class, bundle1);
                    } catch (DIDStoreException e) {
                        e.printStackTrace();
                    }

                }
                break;
            case "getAllSubWallets":
                ISubWalletListEntity subWalletListEntity = (ISubWalletListEntity) baseEntity;
                for (SubWallet subWallet : subWalletListEntity.getData()) {
                    if (subWallet.getChainId().equals(MyWallet.IDChain)) {
                        initDid();
                        return;
                    }
                }
                //没有对应的子钱包 需要打开idchain
                showOpenDIDWarm(subWalletListEntity);

                break;
            case "getPubKeyInfo":
                String pubKeyInfo = ((CommmonStringEntity) baseEntity).getData();
                JSONObject pubKeyInfoJsonData = JSON.parseObject(pubKeyInfo);
                String derivationStrategy = pubKeyInfoJsonData.getString("derivationStrategy");
                int n = pubKeyInfoJsonData.getIntValue("n");
                String requestPubKey;
                if ("BIP44".equals(derivationStrategy) && n > 1) {
                    requestPubKey = pubKeyInfoJsonData.getString("xPubKey");

                } else {
                    requestPubKey = pubKeyInfoJsonData.getString("xPubKeyHDPM");
                }
                if (!TextUtils.isEmpty(requestPubKey)) {
                    Bundle bundle = new Bundle();
                    bundle.putParcelable("wallet", wallet);
                    bundle.putString("requestPubKey", requestPubKey);
                    start(ShowMulsignPublicKeyFragment.class, bundle);
                } else {
                    dialog = dialogUtil.showWarmPromptInput(getBaseActivity(), null, null, this);
                }
                break;
            case "verifyPassPhrase":
                boolean result1 = ((CommmonBooleanEntity) baseEntity).getData();
                if (result1) {
                    //助记词密码正确
                    presenter.destroyWallet(wallet.getWalletId(), this);
                } else {
                    showToastMessage(getString(R.string.error_20003));
                }
                break;
            case "verifyPayPassword":

                boolean result = ((CommmonBooleanEntity) baseEntity).getData();
                if (result) {
                    dialog.dismiss();
                    if (OUTPORTMUPK.equals(dialogAction)) {
                        //查看多签公钥
                        presenter.getPubKeyInfo(wallet.getWalletId(), this);
                    } else if (DELETE.equals(dialogAction)) {
                        //目前只在删除一种情况调用
                        presenter.getMasterWalletBasicInfo(wallet.getWalletId(), this);
                    } else if (DIDINIT.equals(dialogAction)) {
                        new CreatMulWalletPresenter().exportxPrivateKey(wallet.getWalletId(), payPasswd, this);
                    }
                } else {
                    showToastMessage(getString(R.string.error_20003));
                }

                break;
            case "getMasterWalletBasicInfo":
                //目前只在删除一种情况顺序调用
                /*if (!DELETE.equals(dialogAction)) {
                    return;
                }*/
                String data = ((CommmonStringEntity) baseEntity).getData();
                JSONObject jsonData = JSON.parseObject(data);
                boolean hasPassPhrase = jsonData.getBoolean("HasPassPhrase");
                if (hasPassPhrase) {
                    dialog = dialogUtil.showWarmPromptInput3(getBaseActivity(), null, null, new WarmPromptListener2() {
                        @Override
                        public void affireBtnClick(View view) {
                            //先验证助记词密码
                            String passphrase = ((EditText) view).getText().toString().trim();
                            if (TextUtils.isEmpty(passphrase)) {
                                showToastMessage(getString(R.string.pwdnoempty));
                                return;
                            }
                            presenter.verifyPassPhrase(wallet.getWalletId(), passphrase, payPasswd, WalletManageFragment.this);
                        }

                        @Override
                        public void noAffireBtnClick(View view) {
                            //直接删除
                            presenter.destroyWallet(wallet.getWalletId(), WalletManageFragment.this);
                        }
                    });

                } else {

                    if (DELETE.equals(dialogAction)) {
                        //删除的回调
                        presenter.destroyWallet(wallet.getWalletId(), this);
                    }

                }
                break;
        }

    }

    private void showOpenDIDWarm(ISubWalletListEntity subWalletListEntity) {
        new DialogUtil().showCommonWarmPrompt(getBaseActivity(), getString(R.string.noidchainopenornot),
                getString(R.string.toopen), getString(R.string.cancel), false, new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        Bundle bundle = new Bundle();

                        ArrayList<String> chainIds = new ArrayList<>();
                        for (SubWallet iSubWallet : subWalletListEntity.getData()) {
                            chainIds.add(iSubWallet.getChainId());
                            bundle.putString("walletId", iSubWallet.getBelongId());
                        }
                        bundle.putStringArrayList("chainIds", chainIds);
                        start(AddAssetFragment.class, bundle);
                    }
                });
    }
}
