package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.SubWallet;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.fragment.AddAssetFragment;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.bean.ISubWalletListEntity;
import org.elastos.wallet.ela.ui.did.entity.AllPkEntity;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.ui.did.entity.DIDInfoEntity;
import org.elastos.wallet.ela.ui.did.entity.DIDListEntity;
import org.elastos.wallet.ela.ui.did.presenter.AddDIDPresenter;
import org.elastos.wallet.ela.ui.did.presenter.DIDListPresenter;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.elastos.wallet.ela.utils.widget.TextConfigDataPicker;
import org.elastos.wallet.ela.utils.widget.TextConfigNumberPicker;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.Iterator;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

public class AddDIDFragment extends BaseFragment implements NewBaseViewData {

    @BindView(R.id.tv_title)
    TextView tvTitle;

    AddDIDPresenter presenter;
    @BindView(R.id.et_didname)
    EditText etDidname;
    @BindView(R.id.tv_walletname)
    TextView tvWalletname;
    @BindView(R.id.rl_selectwallet)
    RelativeLayout rlSelectwallet;
    @BindView(R.id.tv_didpk)
    TextView tvDidpk;
    @BindView(R.id.tv_did)
    TextView tvDid;
    @BindView(R.id.tv_date)
    TextView tvDate;
    @BindView(R.id.rl_outdate)
    RelativeLayout rlOutdate;
    String[] walletNames;
    List<Wallet> wallets;
    Wallet tempWallet;
    private DIDInfoEntity didInfo;
    private long endDate;
    private boolean useDraft;
    private DIDListPresenter didListPresenter;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_add_did;
    }

    @Override
    protected void setExtraData(Bundle data) {

        useDraft = data.getBoolean("useDraft");
        if (useDraft) {
            didInfo = data.getParcelable("didInfo");
            tempWallet = new RealmUtil().queryUserWallet(didInfo.getWalletId());
            putData();
        } else {
            didInfo = new DIDInfoEntity();
            didInfo.setOperation("create");
            List<DIDInfoEntity.PublicKeyBean> list = new ArrayList<>();
            DIDInfoEntity.PublicKeyBean publicKeyBean = new DIDInfoEntity.PublicKeyBean();
            publicKeyBean.setId("#primary");
            list.add(publicKeyBean);
            didInfo.setPublicKey(list);

            wallets = new RealmUtil().queryTypeUserAllWallet(0);
            presenter = new AddDIDPresenter();
            didListPresenter = new DIDListPresenter();
            for (int i = 0; i < wallets.size(); i++) {
                presenter.getAllSubWallets1(wallets.get(i).getWalletId(), this);
            }

        }
    }

    private void putData() {
        etDidname.setEnabled(false);
        rlSelectwallet.setVisibility(View.GONE);
        etDidname.setText(didInfo.getDidName());
        tvWalletname.setText(tempWallet.getWalletName());
        tvDid.setText(didInfo.getId());
        tvDidpk.setText(didInfo.getPublicKey().get(0).getPublicKey());
        endDate = didInfo.getExpires();
        tvDate.setText(DateUtil.timeNYR(endDate, getContext()));

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.adddid));
        registReceiver();

    }

    private void setData() {
        didInfo.setId(getText(tvDid));
        didInfo.setWalletId(tempWallet.getWalletId());
        didInfo.getPublicKey().get(0).setPublicKey(getText(tvDidpk));
        didInfo.setExpires(endDate);
        didInfo.setDidName(getText(etDidname));
    }

    @OnClick({R.id.rl_selectwallet, R.id.rl_outdate, R.id.tv_next})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_next:
                String didName = etDidname.getText().toString().trim();
                if (TextUtils.isEmpty(didName)) {
                    showToast(getString(R.string.plziputdidname));
                    break;
                }
                String did = tvDid.getText().toString().trim();
                if (TextUtils.isEmpty(did)) {
                    showToast(getString(R.string.plzselectwallet));
                    break;
                }

                if (endDate == 0) {
                    showToast(getString(R.string.plzselctoutdate));
                    break;
                }
                setData();

                Bundle bundle = new Bundle();
                bundle.putParcelable("didInfo", didInfo);
                bundle.putBoolean("useDraft", useDraft);
                start(PersonalInfoFragment.class, bundle);
                break;
            case R.id.rl_selectwallet:
                if (wallets == null || wallets.size() == 0) {
                    showToast(getString(R.string.noavailablewalllet));
                }
                walletNames = new String[wallets.size()];
                for (int i = 0; i < wallets.size(); i++) {
                    walletNames[i] = wallets.get(i).getWalletName();
                }
                new DialogUtil().showCommonSelect(getBaseActivity(), walletNames, new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {

                        tvWalletname.setText("");
                        tvDid.setText("");
                        tvDidpk.setText("");
                        presenter.getAllSubWallets(wallets.get(((TextConfigNumberPicker) view).getValue()).getWalletId(), AddDIDFragment.this);
                    }
                });
                break;
            case R.id.rl_outdate:
                Calendar calendar = Calendar.getInstance();
                long minData = calendar.getTimeInMillis();
                int year = calendar.get(Calendar.YEAR);
                calendar.set(Calendar.YEAR, year + 5);
                new DialogUtil().showTime(getBaseActivity(), minData, calendar.getTimeInMillis(), new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        String date = ((TextConfigDataPicker) view).getYear() + "-" + (((TextConfigDataPicker) view).getMonth() + 1)
                                + "-" + ((TextConfigDataPicker) view).getDayOfMonth();
                        endDate = DateUtil.parseToLong(date) / 1000;

                        tvDate.setText(getString(R.string.validtime) + DateUtil.timeNYR(endDate, getContext()));
                    }
                });
                break;


        }
    }

    @Override
    public boolean onBackPressedSupport() {
        String didName = etDidname.getText().toString().trim();
        String walletName = tvWalletname.getText().toString().trim();
        if (!TextUtils.isEmpty(didName) && !TextUtils.isEmpty(walletName)) {
            new DialogUtil().showCommonWarmPrompt(getBaseActivity(), getString(R.string.keepeditornot),
                    getString(R.string.keep), getString(R.string.nokeep), true, new WarmPromptListener() {
                        @Override
                        public void affireBtnClick(View view) {
                            setData();
                            //保存草稿
                            CacheUtil.setCredentialSubjectBean( credentialSubjectBean);
                            keep();
                            getBaseActivity().pop();
                        }
                    });
            return true;
        }
        return super.onBackPressedSupport();


    }

    private void keep() {
        ArrayList<DIDInfoEntity> infoEntities = CacheUtil.getDIDInfoList();
        if (infoEntities.contains(didInfo)) {
            infoEntities.remove(didInfo);
        }
        didInfo.setIssuanceDate(new Date().getTime() / 1000);
        didInfo.setStatus("Unpublished");
        infoEntities.add(didInfo);
        CacheUtil.setDIDInfoList(infoEntities);
        post(RxEnum.KEEPDRAFT.ordinal(), null, infoEntities);
        showToast(getString(R.string.keepsucess));
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getAllSubWallets":
                ISubWalletListEntity subWalletListEntity1 = (ISubWalletListEntity) baseEntity;
                for (SubWallet subWallet : subWalletListEntity1.getData()) {
                    if (subWallet.getChainId().equals(MyWallet.IDChain)) {
                        didListPresenter.getResolveDIDInfo((String) o, 0, 1, "", this);
                        return;
                    }
                }
                //没有对应的子钱包 需要打开idchain
                showOpenDIDWarm(subWalletListEntity1);
                break;
            case "getAllSubWallets1":
                ISubWalletListEntity subWalletListEntity = (ISubWalletListEntity) baseEntity;
                for (SubWallet subWallet : subWalletListEntity.getData()) {
                    //先判断有无子
                    if (subWallet.getChainId().equals(MyWallet.IDChain)) {
                        didListPresenter.getResolveDIDInfo1((String) o, 0, 1, "", this);
                        return;
                    }
                }


                break;
            case "getAllPublicKeys":
                //{"MaxCount":110,"PublicKeys":["03a3a59e3bc3ddd9a048119d5cee1e8266484e1c1b7c5d529f5d7681066066495f"]}
                AllPkEntity allPkEntity = JSON.parseObject(((CommmonStringEntity) baseEntity).getData(), AllPkEntity.class);

                if (allPkEntity.getPublicKeys() == null || allPkEntity.getPublicKeys().size() == 0) {
                    return;
                }
                for (Wallet wallet : wallets) {
                    if (wallet.getWalletId().equals(o)) {
                        tempWallet = wallet;
                        tvWalletname.setText(tempWallet.getWalletName());
                        tvDidpk.setText(allPkEntity.getPublicKeys().get(0));
                        presenter.getDIDByPublicKey(tempWallet.getWalletId(), allPkEntity.getPublicKeys().get(0), this);
                        return;
                    }
                }
                break;
            case "getDIDByPublicKey":
                String did = ((CommmonStringEntity) baseEntity).getData();
                tvDid.setText(did);
                ArrayList<DIDInfoEntity> infoEntities = CacheUtil.getDIDInfoList();
                for (DIDInfoEntity didInfoEntity : infoEntities) {
                    if (didInfoEntity.getId().equals(did)) {
                        //草稿里有此did
                        didInfo = didInfoEntity;
                        showOpenDraftWarm();
                        break;
                    }
                }
                break;
            case "getResolveDIDInfo1":
                DIDListEntity didListEntity1 = JSON.parseObject(((CommmonStringEntity) baseEntity).getData(), DIDListEntity.class);
                if (didListEntity1 != null && didListEntity1.getDID() != null && didListEntity1.getDID().size() > 0) {
                    Iterator<Wallet> iterator = wallets.iterator();
                    while (iterator.hasNext()) {
                        Wallet wallet = iterator.next();
                        if (wallet.getWalletId().equals(o))
                            iterator.remove();
                    }
                }
                break;
            case "getResolveDIDInfo":
                DIDListEntity didListEntity = JSON.parseObject(((CommmonStringEntity) baseEntity).getData(), DIDListEntity.class);
                if (didListEntity != null && didListEntity.getDID() != null && didListEntity.getDID().size() > 0) {
                    showToast(getString(R.string.didregisted));
                } else {
                    presenter.getAllPublicKeys((String) o, MyWallet.IDChain, 0, 1, AddDIDFragment.this);
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

    private void showOpenDraftWarm() {
        new DialogUtil().showCommonWarmPrompt(getBaseActivity(), getString(R.string.usedraftornot),
                getString(R.string.sure), getString(R.string.cancel), false, new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        //充填数据
                        useDraft = true;
                        etDidname.setText(didInfo.getDidName());
                        endDate = didInfo.getExpires();
                        if (endDate != 0) {
                            tvDate.setText(getString(R.string.validtime) + DateUtil.timeNYR(endDate, getContext()));
                        } else {
                            tvDate.setText(null);
                        }
                    }
                });
    }

    private CredentialSubjectBean credentialSubjectBean;

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();

        if (integer == RxEnum.UPDATAPROPERTY.ordinal()) {
            presenter.getAllSubWallets((String) result.getObj(), this);
        }
        if (integer == RxEnum.RETURCER.ordinal()) {
            credentialSubjectBean = (CredentialSubjectBean) result.getObj();
        }


    }

}
