package org.elastos.wallet.ela.ui.did.fragment;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonLongEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;
import org.elastos.wallet.ela.ui.did.entity.DIDInfoEntity;
import org.elastos.wallet.ela.ui.did.entity.DIDListEntity;
import org.elastos.wallet.ela.ui.did.presenter.DIDListPresenter;
import org.elastos.wallet.ela.ui.vote.activity.VoteTransferActivity;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class DidDetailFragment extends BaseFragment implements NewBaseViewData {


    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.tv_didname)
    TextView tvDidname;
    @BindView(R.id.tv_didpk)
    TextView tvDidpk;
    @BindView(R.id.tv_did)
    TextView tvDid;
    @BindView(R.id.tv_validdate)
    TextView tvValiddate;
    Unbinder unbinder;
    @BindView(R.id.tv_edit)
    TextView tvEdit;
    private DIDInfoEntity didInfo;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_did_detail;
    }

    @Override
    protected void setExtraData(Bundle data) {
        didInfo = data.getParcelable("didInfo");
        if ("Pending".equals(didInfo.getStatus())) {
            tvEdit.setVisibility(View.GONE);
            ivTitleRight.setVisibility(View.GONE);
        }
        putData();

    }

    private void putData() {
        tvDidname.setText(didInfo.getDidName());
        tvDid.setText("did:ela:" + didInfo.getId());

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(didInfo.getDidName());
        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleRight.setImageResource(R.mipmap.del_icon);
        new DIDListPresenter().getResolveDIDInfo(didInfo.getWalletId(), 0, 1, didInfo.getId(), this);
        registReceiver();
    }

    @OnClick({R.id.tv_edit, R.id.tv_credentialinfo, R.id.tv_did, R.id.tv_didpk, R.id.iv_title_right})
    public void onViewClicked(View view) {
        Bundle bundle;
        switch (view.getId()) {
            case R.id.tv_did:
                ClipboardUtil.copyClipboar(getBaseActivity(), tvDid.getText().toString());
                break;
            case R.id.tv_didpk:
                ClipboardUtil.copyClipboar(getBaseActivity(), tvDidpk.getText().toString());
                break;
            case R.id.tv_edit:
                bundle = new Bundle();
                bundle.putParcelable("didInfo", didInfo);
                bundle.putString("type", Constant.EDITDID);
                start(AddDIDFragment.class, bundle);
                break;
            case R.id.tv_credentialinfo:
                //凭证信息
                bundle = new Bundle();
                bundle.putString("did", didInfo.getId());
                start(CredentialFragment.class, bundle);
                break;
            case R.id.iv_title_right:
                new CRSignUpPresenter().getFee(didInfo.getWalletId(), MyWallet.IDChain, "", "8USqenwzA5bSAvj1mG4SGTABykE9n5RzJQ", "0", DidDetailFragment.this);

                break;
        }
    }


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getResolveDIDInfo":

                DIDListEntity didListEntity = JSON.parseObject(((CommmonStringEntity) baseEntity).getData(), DIDListEntity.class);
                if (didListEntity != null && didListEntity.getDID() != null && didListEntity.getDID().size() > 0) {
                    for (DIDInfoEntity didBean : didListEntity.getDID()) {
                        didBean.setWalletId((String) o);
                    }
                    didInfo = didListEntity.getDID().get(0);
                    tvDidpk.setText(didInfo.getPublicKey().get(0).getPublicKey());
                    tvValiddate.setText(DateUtil.timeNYR(didInfo.getIssuanceDate(), getContext())
                            + getString(R.string.to) + DateUtil.timeNYR(didInfo.getExpires(), getContext()));
                }

                break;
            case "getFee":
                didInfo.setOperation("deactivate");
                JSONObject json = (JSONObject) JSON.toJSON(didInfo);
                Intent intent = new Intent(getActivity(), VoteTransferActivity.class);
                intent.putExtra("wallet", new RealmUtil().queryUserWallet(didInfo.getWalletId()));
                intent.putExtra("chainId", MyWallet.IDChain);
                intent.putExtra("inputJson", JSON.toJSONString(json));
                intent.putExtra("fee", ((CommmonLongEntity) baseEntity).getData());
                intent.putExtra("type", Constant.DIDSIGNUP);
                startActivity(intent);
                break;
        }

    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();


        if (integer == RxEnum.TRANSFERSUCESS.ordinal()) {
            new DialogUtil().showTransferSucess(getBaseActivity(), new WarmPromptListener() {
                @Override
                public void affireBtnClick(View view) {
                    //删除这个草稿?//todo
                    toDIDListFragment();
                }
            });
        }
    }

}
