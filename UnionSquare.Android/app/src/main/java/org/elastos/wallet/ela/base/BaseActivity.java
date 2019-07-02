package org.elastos.wallet.ela.base;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Toast;

import com.scwang.smartrefresh.layout.SmartRefreshLayout;
import com.trello.rxlifecycle2.LifecycleTransformer;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.SupportActivity;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.di.component.ActivityComponent;
import org.elastos.wallet.ela.di.component.DaggerActivityComponent;
import org.elastos.wallet.ela.di.moudule.ActivityModule;
import org.elastos.wallet.ela.utils.StatusBarUtil;
import org.greenrobot.eventbus.EventBus;

import javax.inject.Inject;

import butterknife.ButterKnife;
import butterknife.Unbinder;

/**
 * author: fangxiaogang
 * date: 2018/9/1
 */

public abstract class BaseActivity<T extends BaseContract.Basepresenter> extends SupportActivity implements BaseContract.Baseview {

    @Nullable
    @Inject
    protected T mPresenter;

    protected ActivityComponent mActivityComponent;

    public SmartRefreshLayout getRefreshLayout() {
        return refreshLayout;
    }

    public void setRefreshLayout(SmartRefreshLayout refreshLayout) {
        this.refreshLayout = refreshLayout;
    }

    private SmartRefreshLayout refreshLayout;
    private Unbinder unbinder;

    protected abstract int getLayoutId();

    protected abstract void initView();


    protected void initInjector() {
    }


    protected void setExtraData(Intent data) {
    }


    private void getExtra() {
        Intent intent = getIntent();
        if (intent != null) {
            setExtraData(intent);
        }
    }

    private InputMethodManager inputmanger;

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        if (inputmanger == null) {
            inputmanger = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        }
        if (inputmanger.isActive()) {
            View view = getWindow().peekDecorView();
            inputmanger.hideSoftInputFromWindow(view.getWindowToken(), 0);

        }

        return super.dispatchTouchEvent(ev);
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        initActivityComponent();
        int layoutId = getLayoutId();
        setContentView(layoutId);
        StatusBarUtil.setColor(this, getResources().getColor(R.color.white));
        initInjector();
        attachView();
        unbinder = ButterKnife.bind(this);
        getExtra();
        initView();
    }

    public void popBackFragment(View view) {
        onBackPressed();
    }

    /**
     * 初始化ActivityComponent
     */
    private void initActivityComponent() {
        mActivityComponent = DaggerActivityComponent.builder()
                .applicationComponent(((MyApplication) getApplication()).getApplicationComponent())
                .activityModule(new ActivityModule(this))
                .build();
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
        unbinder.unbind();
        destroyView();
    }


    /**
     * 贴上view
     */
    private void attachView() {
        if (mPresenter != null) {
            mPresenter.attachview(this);
        }
    }

    /**
     * 分离view
     */
    private void destroyView() {
        if (mPresenter != null) {
            mPresenter.destroyview();
        }
    }

    @Override
    public void showLoading() {

    }

    @Override
    public void hideLoading() {

    }

    @Override
    public void showSuccess(String message) {

    }

    @Override
    public void showFaild(String message) {

    }


    @Override
    public <T> LifecycleTransformer<T> bindToLife() {
        return this.bindToLifecycle();
    }

    /**
     * Toast提示
     **/
    public void showToastMessage(@NonNull String message) {
        if (message.length() > 15) {
            Toast.makeText(this, message, Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(this, message, Toast.LENGTH_LONG).show();
        }
    }

    protected void post(int code, String name, Object obj) {
        EventBus.getDefault().post(new BusEvent(code, name, obj));
    }


    public synchronized MyWallet getWallet() {

        if (MyApplication.getMyWallet() == null) {
            MyApplication.setMyWallet(new MyWallet());
        }
        return MyApplication.getMyWallet();
    }


    public void registReceiver() {
        if (!EventBus.getDefault().isRegistered(this))
            EventBus.getDefault().register(this);
    }

    protected void onErrorRefreshLayout(SmartRefreshLayout refreshLayout) {
        this.refreshLayout = refreshLayout;
        // onError();
    }

    public void onError() {
        if (refreshLayout != null)
            finishLoadRefresh(refreshLayout);
    }

    protected void finishLoadRefresh(SmartRefreshLayout refreshLayout) {
        if (refreshLayout.isRefreshing()) {
            refreshLayout.finishRefresh();
        }
        if (refreshLayout.isLoading()) {
            refreshLayout.finishLoadMore();
        }
    }

    protected void onDestroyRefreshLayout() {
        if (refreshLayout != null) {
            refreshLayout = null;
        }
    }
}
