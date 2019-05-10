package org.elastos.wallet.ela.ui.vote.ImportKeystore;


import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 节点 NodeKeystore
 */
public class ImportNodeKeystoreFragment extends BaseFragment {


    @BindView(R.id.iv_title_left)
    ImageView ivTitleLeft;
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.tv_name)
    TextView tvName;
    @BindView(R.id.ll_1)
    LinearLayout ll1;
    @BindView(R.id.et_pwd)
    EditText etPwd;
    @BindView(R.id.et_pwd_agin)
    EditText etPwdAgin;
    @BindView(R.id.tv_outport)
    TextView tvOutport;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_node_keystore;
    }


    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.export_node_keystore));

    }


    @OnClick({R.id.iv_title_left, R.id.tv_outport})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.iv_title_left:
                _mActivity.onBackPressed();
                break;
            case R.id.tv_outport:
                start(ImportNodeKeystore_TwoFragment.class);
                break;
        }
    }
}


