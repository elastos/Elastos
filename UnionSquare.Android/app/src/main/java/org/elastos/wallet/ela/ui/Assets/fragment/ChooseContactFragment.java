package org.elastos.wallet.ela.ui.Assets.fragment;

import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Contact;
import org.elastos.wallet.ela.ui.Assets.adapter.ChooseContactRecAdapter;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.utils.RxEnum;

import java.util.List;

import butterknife.BindView;
import butterknife.Unbinder;

public class ChooseContactFragment extends BaseFragment implements CommonRvListener {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.rv)
    RecyclerView rv;
    Unbinder unbinder;
    private List<Contact> list;
    private ChooseContactRecAdapter adapter;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_contact_choose;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.choosecontact);
        list = new RealmUtil().queryAllContact();
        setRecycleView();
    }

    private void setRecycleView() {
        if (adapter == null) {
            adapter = new ChooseContactRecAdapter(getContext(), list);
            rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rv.setAdapter(adapter);
            adapter.setCommonRvListener(this);

        } else {
            adapter.notifyDataSetChanged();
        }
    }

    @Override
    public void onRvItemClick(int position, Object o) {
        post(RxEnum.CHOOSECONTACT.ordinal(), "选择联系人", list.get(position));
        popBackFragment();
    }
}
