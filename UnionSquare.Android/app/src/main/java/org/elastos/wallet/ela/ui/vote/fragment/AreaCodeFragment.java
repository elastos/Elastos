package org.elastos.wallet.ela.ui.vote.fragment;

import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.vote.adapter.AreaAdapter;
import org.elastos.wallet.ela.ui.vote.bean.Area;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.view.SideBar;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.List;

import butterknife.BindView;


public class AreaCodeFragment extends BaseFragment implements CommonRvListener {


    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.rv)
    RecyclerView rv;

    @BindView(R.id.sb)
    SideBar sb;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_areacode;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        registReceiver();
        tvTitle.setText(getString(R.string.pleasechoosearea));
        AppUtlis.getArea(getContext(), new AppUtlis.OnGetAreaListner() {
            @Override
            public void onGetArea(List<Area> listArea) {

                post(10086, null, listArea);
            }
        });

    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == 10086) {
            setRecyclerView((List<Area>) result.getObj());

        }
    }

    private void setRecyclerView(List<Area> list) {
        if (list == null || list.size() == 0) {
            return;
        }

        LinearLayoutManager linearLayoutManager = new LinearLayoutManager(this.getContext());
        rv.setLayoutManager(linearLayoutManager);
        AreaAdapter adapter = new AreaAdapter(this.getContext(), list);
        rv.setAdapter(adapter);
        adapter.setCommonRvListener(this);
        sb.setOnSelectLetterListner(new SideBar.OnSelectLetterListner() {
            @Override
            public void onSelectLetter(String letter) {
                for (int i = 0; i < list.size(); i++) {
                    String l = list.get(i).getEn().trim().charAt(0) + "";
                    if (TextUtils.equals(letter, l)) {
                        linearLayoutManager.scrollToPositionWithOffset(i, 0);
                        linearLayoutManager.setStackFromEnd(true);
                        break;
                    }
                }
            }
        });
    }

    @Override
    public void onRvItemClick(int position, Object o) {
        post(RxEnum.AREA.ordinal(), null, o);
        popBackFragment();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
   /*   if (readThread != null && readThread.isAlive()) {
            readThread.stop();
            list.clear();
        }*/

    }
}
