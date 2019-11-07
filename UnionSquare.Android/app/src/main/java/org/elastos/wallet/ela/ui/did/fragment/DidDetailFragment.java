package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.did.entity.DIDListEntity;
import org.elastos.wallet.ela.ui.did.presenter.DIDListPresenter;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.Log;

import butterknife.BindView;
import butterknife.ButterKnife;
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
    Unbinder unbinder1;
    private DIDListEntity.DIDBean didInfo;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_did_detail;
    }

    @Override
    protected void setExtraData(Bundle data) {
        didInfo = data.getParcelable("didInfo");
        Log.i("???", didInfo.toString());
        if ("Pending".equals(didInfo.getStatus())) {
            tvEdit.setVisibility(View.GONE);
        }
        putData();

    }

    private void putData() {
        tvDidname.setText(didInfo.getDidName());
        tvDid.setText(didInfo.getId());
        // endDate = didInfo.getExpires();
        // tvDate.setText(DateUtil.timeNYR(endDate, getContext()));

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.adddid));
        new DIDListPresenter().getResolveDIDInfo(didInfo.getWalletId(), 0, 1, didInfo.getId(), this);
    }

    @OnClick({R.id.tv_edit, R.id.tv_credentialinfo, R.id.tv_did, R.id.tv_didpk})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_did:
                ClipboardUtil.copyClipboar(getBaseActivity(), tvDid.getText().toString());
                break;
            case R.id.tv_didpk:
                ClipboardUtil.copyClipboar(getBaseActivity(), tvDidpk.getText().toString());
                break;
            case R.id.tv_edit:
                break;
            case R.id.tv_credentialinfo:
                //凭证信息
                start(CredentialFragment.class, getArguments());
                break;
        }
    }


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getResolveDIDInfo":

                DIDListEntity didListEntity = JSON.parseObject(((CommmonStringEntity) baseEntity).getData(), DIDListEntity.class);
                if (didListEntity != null && didListEntity.getDID() != null && didListEntity.getDID().size() > 0) {
                    for (DIDListEntity.DIDBean didBean : didListEntity.getDID()) {
                        didBean.setWalletId((String) o);
                    }
                    DIDListEntity.DIDBean didBean = didListEntity.getDID().get(0);
                    tvDidpk.setText(didBean.getPublicKey().get(0).getPublicKey());
                    tvValiddate.setText(DateUtil.timeNYR(didBean.getIssuanceDate(), getContext())
                            + getString(R.string.to) + DateUtil.timeNYR(didBean.getExpires(), getContext()));
                }

                break;
        }

    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        // TODO: inflate a fragment view
        View rootView = super.onCreateView(inflater, container, savedInstanceState);
        unbinder1 = ButterKnife.bind(this, rootView);
        return rootView;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder1.unbind();
    }
}
