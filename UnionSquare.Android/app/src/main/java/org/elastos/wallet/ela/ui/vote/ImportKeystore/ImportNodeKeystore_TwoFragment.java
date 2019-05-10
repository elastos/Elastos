package org.elastos.wallet.ela.ui.vote.ImportKeystore;


import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;

import butterknife.BindView;

/**
 * 节点 NodeKeystore 第二步骤
 */
public class ImportNodeKeystore_TwoFragment extends BaseFragment {


    @BindView(R.id.iv_title_left)
    ImageView ivTitleLeft;
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.tv_name)
    TextView tvName;



    @Override
    protected int getLayoutId() {
        return R.layout.fragment_node_keystore_two;
    }


    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.export_node_keystore));
        ivTitleLeft.setOnClickListener(v -> _mActivity.onBackPressed());
    }


}


