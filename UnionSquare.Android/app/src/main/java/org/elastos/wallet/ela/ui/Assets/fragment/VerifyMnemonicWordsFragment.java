package org.elastos.wallet.ela.ui.Assets.fragment;


import android.os.Bundle;
import android.support.v7.widget.DefaultItemAnimator;
import android.support.v7.widget.GridLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.Assets.adapter.CommonTextViewAdapter;
import org.elastos.wallet.ela.ui.Assets.adapter.CommonTextViewTwoAdapter;
import org.elastos.wallet.ela.ui.Assets.bean.Word;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 验证助记词
 */
public class VerifyMnemonicWordsFragment extends BaseFragment {


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

    private String openType;

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
      //  bundle.putString("type", "manager");
        openType = data.getString("openType","");
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.verify_mnemonic_words));
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
                if (putList.size()>=readList.size()){
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


    public static VerifyMnemonicWordsFragment newInstance() {
        Bundle args = new Bundle();
        VerifyMnemonicWordsFragment fragment = new VerifyMnemonicWordsFragment();
        fragment.setArguments(args);
        return fragment;
    }


    @OnClick(R.id.sb_create_wallet)
    public void onViewClicked() {
        if (putList.toString().equals(readList.toString())) {
            if ("manager".equals(openType)){
                //钱包管理的导出助记词
                popTo(WallletManageFragment.class,false);
            }else {
                //成功跳转首页
                toMainFragment();
            }

        } else {
            showToast(getString(R.string.wrongMnemonic));

        }

    }


}
