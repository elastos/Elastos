package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.ChooseContactFragment;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener1;
import org.elastos.wallet.ela.ui.did.adapter.PersonalShowRecAdapetr;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.ui.did.entity.PersonalInfoItemEntity;
import org.elastos.wallet.ela.ui.did.presenter.DIDUIPresenter;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.DateUtil;

import java.util.ArrayList;
import java.util.Date;

import butterknife.BindView;
import butterknife.OnClick;

public class DIDCardDetailFragment extends BaseFragment {

    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_left)
    ImageView ivTitleLeft;
    @BindView(R.id.rv_show)
    RecyclerView rvShow;
    @BindView(R.id.tv_address)
    TextView tvAddress;
    @BindView(R.id.rl_address)
    RelativeLayout rlAddress;
    @BindView(R.id.ll_add)
    LinearLayout llAdd;
    @BindView(R.id.tv_didname)
    TextView tvDidname;
    @BindView(R.id.tv_didpk)
    TextView tvDidpk;
    @BindView(R.id.tv_did)
    TextView tvDid;
    @BindView(R.id.tv_validdate)
    TextView tvValiddate;
    private DIDUIPresenter diduiPresenter;

    private CredentialSubjectBean credentialSubjectBean;

    private ArrayList<PersonalInfoItemEntity> listShow;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_didcarddetail;
    }

    @Override
    protected void setExtraData(Bundle data) {

        String address = data.getString("address");

        if (address == null) {
            rlAddress.setVisibility(View.GONE);
        } else {
            tvAddress.setText(address);
            llAdd.setVisibility(View.VISIBLE);
        }
        Date expries = (Date) data.getSerializable("expires");
        tvDidname.setText(data.getString("name", ""));
        tvDid.setText(data.getString("didString"));
        tvValiddate.setText(getString(R.string.to) + DateUtil.timeNYR(expries, getContext()));
        String pro = data.getString("pro");
        credentialSubjectBean = JSON.parseObject(pro, CredentialSubjectBean.class);
    }

    @Override
    protected void initView(View view) {
        ivTitleLeft.setImageResource(R.mipmap.window_750_close);
        tvTitle.setText(R.string.cardinfo);
        diduiPresenter = new DIDUIPresenter();
        if (!credentialSubjectBean.whetherEmpty()) {
            initItemDate();
        }
    }

    private void initItemDate() {
        String showData[] = getResources().getStringArray(R.array.personalinfo_chose);
        //  String choseData[] = getResources().getStringArray(R.array.personalinfo_chose);
        /*  Map<Integer, String>*/
        listShow = new ArrayList<>();

        for (int i = 0; i < showData.length; i++) {
            PersonalInfoItemEntity personalInfoItemEntity = new PersonalInfoItemEntity();
            personalInfoItemEntity.setIndex(i);
            personalInfoItemEntity.setHintChose(showData[i]);
            personalInfoItemEntity.setHintShow1(showData[i]);
            if (i == 5) {
                personalInfoItemEntity.setHintShow1(getString(R.string.phonecode));
                personalInfoItemEntity.setHintShow2(getString(R.string.phonenumber));
            }
            listShow.add(personalInfoItemEntity);
        }
        diduiPresenter.convertCredential2List(this, listShow, credentialSubjectBean);
        setRecycleViewShow();

    }

    private void setRecycleViewShow() {
        PersonalShowRecAdapetr adapterShow = new PersonalShowRecAdapetr(getContext(), listShow);
        rvShow.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
        adapterShow.setCommonRvListener(new CommonRvListener1() {
            @Override
            public void onRvItemClick(View view, int position, Object o) {
                PersonalInfoItemEntity personalInfoItemEntity = (PersonalInfoItemEntity) o;
                if (personalInfoItemEntity.getIndex() == 7) {
                    //去个人简介详情
                    Bundle bundle = new Bundle();
                    bundle.putString("content", personalInfoItemEntity.getText1());
                    start(ShowPersonalIntroFragemnt.class, bundle);
                }
                if (personalInfoItemEntity.getIndex() > 13) {
                    //去个人简介详情
                    Bundle bundle = new Bundle();
                    bundle.putString("title", personalInfoItemEntity.getText1() == null ? "" : personalInfoItemEntity.getText1());
                    bundle.putString("content", personalInfoItemEntity.getText2());
                    start(ShowPersonalIntroFragemnt.class, bundle);
                }
            }
        });
        rvShow.setAdapter(adapterShow);
    }


    @OnClick({R.id.tv_add, R.id.tv_did})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_did:
                ClipboardUtil.copyClipboar(getBaseActivity(), tvDid.getText().toString());
                break;
            case R.id.tv_add:
                //添加联系人
                Bundle bundle = getArguments();
                bundle.putString("type", "update");
                start(ChooseContactFragment.class, bundle);
                break;
        }
    }


}
