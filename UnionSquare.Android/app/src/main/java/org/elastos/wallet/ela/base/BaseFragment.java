/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package org.elastos.wallet.ela.base;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.blankj.utilcode.util.ToastUtils;
import com.scwang.smartrefresh.layout.SmartRefreshLayout;
import com.scwang.smartrefresh.layout.footer.ClassicsFooter;
import com.scwang.smartrefresh.layout.header.ClassicsHeader;
import com.trello.rxlifecycle2.LifecycleTransformer;
import com.trello.rxlifecycle2.android.FragmentEvent;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.DID.MyDID;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.ElaWallet.WalletNet;
import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.SupportFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.ui.Assets.fragment.HomeWalletFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.WalletManageFragment;
import org.elastos.wallet.ela.ui.did.fragment.DIDListFragment;
import org.elastos.wallet.ela.ui.did.fragment.DidDetailFragment;
import org.elastos.wallet.ela.ui.main.MainFragment;
import org.elastos.wallet.ela.ui.proposal.ProposalFragment;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.SPUtil;
import org.greenrobot.eventbus.EventBus;

import butterknife.ButterKnife;
import butterknife.Unbinder;
import permissions.dispatcher.NeedsPermission;
import permissions.dispatcher.OnNeverAskAgain;
import permissions.dispatcher.OnPermissionDenied;
import permissions.dispatcher.OnShowRationale;
import permissions.dispatcher.PermissionRequest;
import permissions.dispatcher.RuntimePermissions;

/**
 * date: 2018/9/13
 */
@RuntimePermissions
public abstract class BaseFragment<T extends BaseContract.Basepresenter> extends SupportFragment implements BaseContract.Baseview {

    private String requstStr = "";
    private Unbinder unbinder;
    public Context mContext;


    protected abstract int getLayoutId();

    protected void initInjector() {
    }

    protected abstract void initView(View view);

    protected View mRootView;


    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        initInjector();
        attachView();
        mContext = getContext();

    }

    public MyWallet getMyWallet() {
        return getBaseActivity().getWallet();
    }

    public MyDID getMyDID() {
        return getBaseActivity().getMyDID();
    }

    protected String getText(TextView et) {
        String text = et.getText().toString().trim();
        if (!TextUtils.isEmpty(text)) {
            return text;
        }
        return null;
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        inflaterView(inflater, container);
        int Language = new SPUtil(getContext()).getLanguage();
        int bg = R.mipmap.bg_test;
        if (Language != 0) {
            bg = R.mipmap.bg_e_test;
        }
        int id = (MyApplication.currentWalletNet != WalletNet.MAINNET
                && MyApplication.currentWalletNet != WalletNet.ALPHAMAINNET) ? bg : R.drawable.commonbg;

        mRootView.setBackgroundResource(id);//为每个页面设置默认背景
        unbinder = ButterKnife.bind(this, mRootView);
        showLoading();
        Bundle bundle = getArguments();
        if (bundle != null) {
            setExtraData(bundle);
        }
        initView(mRootView);
       /* int num = getActivity().getSupportFragmentManager().getBackStackEntryCount();
        String numString = "++++++++++++++++++++++++++++++++++Fragment回退栈数量：" + num;
        Log.d("Fragment", numString);
        for (int i = 0; i < num; i++) {
            FragmentManager.BackStackEntry backstatck = getActivity().getSupportFragmentManager().getBackStackEntryAt(i);
            Log.d("++++++++++++++++++++++++++++++", backstatck.getName());
        }*/
        return mRootView;
    }


    /*取出activity传递的数据@param data*/
    protected void setExtraData(Bundle data) {
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unbinder.unbind();
        detachView();
        if (EventBus.getDefault().isRegistered(this)) {
            EventBus.getDefault().unregister(this);
        }
        onDestroyRefreshLayout();
        if (smartRefreshLayout != null) {
            smartRefreshLayout = null;
        }
    }


    private void inflaterView(LayoutInflater inflater, @Nullable ViewGroup container) {
        if (mRootView == null) {
            //Log.i(getClass().getSimpleName(),"444");
            mRootView = inflater.inflate(getLayoutId(), container, false);
        } else {
            // Log.i(getClass().getSimpleName(),"333");
            container.removeView(mRootView);
            mRootView = inflater.inflate(getLayoutId(), container, false);
        }
    }


    /**
     * 贴上view
     */
    private void attachView() {
      /*  if (mPresenter != null) {
            mPresenter.attachview(this);
        }*/
    }

    /**
     * 分离view
     */
    private void detachView() {
     /*   if (mPresenter != null) {
            mPresenter.destroyview();
        }*/
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
        ToastUtils.showShort(message);
    }

    @Override
    public <V> LifecycleTransformer<V> bindToLife() {
       // return this.bindToLifecycle();
        return bindUntilEvent(FragmentEvent.DESTROY);
    }


    public void setToobar(Toolbar toolbar, TextView toolbar_title, String... text) {
        toolbar.setNavigationIcon(R.mipmap.top_nav_back);
        toolbar.setNavigationOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                _mActivity.onBackPressed();
            }
        });

        toolbar_title.setText(text[0]);

        if (text.length > 1) {
            TextView tvRight = toolbar.findViewById(R.id.tv_title_right);
            tvRight.setVisibility(View.VISIBLE);
            tvRight.setText(text[1]);
        }

    }

    public void showToast(String message) {

        ToastUtils.showShort(message);
    }

    public void showToastMessage(String message) {
        getBaseActivity().showToastMessage(message);
    }

    public BaseActivity getBaseActivity() {
        return (BaseActivity) getActivity();
    }

    /**
     * 基本的额存储，读取，使用照相机权限
     */

    /*
     * 下面是请求权限的申请
     * */
    protected void requstPermissionOk() {
        //請求成功
    }

    protected void requstPermissionDefeated() {
        //授權失敗
    }

    /**
     * 1申请授权
     */
    public void requstManifestPermission(String requstStr) {
        this.requstStr = requstStr;
        BaseFragmentPermissionsDispatcher.setPermissionOkWithCheck(this);
    }

    //2成功的毀掉
    @NeedsPermission({Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.CAMERA})
    public void setPermissionOk() {
        requstPermissionOk();
    }

    /**
     * 3用户拒绝后回调3
     **/
    @OnPermissionDenied({Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.CAMERA})
    public void showPermissionDenied() {
        showToastMessage(getString(R.string.refusepermission));
        requstPermissionDefeated();
    }

    /**
     * 4用户拒绝后再次申请权限会回调5
     **/
    @OnShowRationale({Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.CAMERA})
    public void showRationaleForPermission(final PermissionRequest request) {
        new AlertDialog.Builder(getBaseActivity(), AlertDialog.THEME_HOLO_LIGHT)
                .setPositiveButton(R.string.allow, (dialog, which) -> request.proceed())
                .setNegativeButton(getString(R.string.refuse), (dialog, which) -> request.cancel())
                .setCancelable(false)
                .setMessage(requstStr)
                .show();
    }

    /**
     * 5如果在系统申请弹窗中勾选了不在提示并且拒绝的回调4
     **/
    @OnNeverAskAgain({Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.CAMERA})
    public void onPermissionNeverAskAgain() {
        new AlertDialog.Builder(getBaseActivity(), AlertDialog.THEME_HOLO_LIGHT)
                .setPositiveButton(getString(R.string.sure), (dialogInterface, i) -> {
                    dialogInterface.dismiss();
                    getAppDetailSettingIntent();
                })
                .setNegativeButton(getString(R.string.refuse), (dialogInterface, i) -> {
                    dialogInterface.dismiss();
                })
                .setCancelable(false)
                .setMessage(R.string.noaskpermission)
                .show();
    }

    /**
     * 6重写此方法6
     **/
    @SuppressLint("NeedOnRequestPermissionsResult")
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        BaseFragmentPermissionsDispatcher.onRequestPermissionsResult(this, requestCode, grantResults);
    }

    private void getAppDetailSettingIntent() {
        Intent localIntent = new Intent();
        localIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        if (Build.VERSION.SDK_INT >= 9) {
            localIntent.setAction("android.settings.APPLICATION_DETAILS_SETTINGS");
            localIntent.setData(Uri.fromParts("package", getContext().getPackageName(), null));
        } else if (Build.VERSION.SDK_INT <= 8) {
            localIntent.setAction(Intent.ACTION_VIEW);
            localIntent.setClassName("com.android.settings", "com.android.settings.InstalledAppDetails");
            localIntent.putExtra("com.android.settings.ApplicationPkgName", getContext().getPackageName());
        }
        startActivity(localIntent);
    }

    /*弹出当前页面*/
    public void popBackFragment() {
        if (getBaseActivity() != null)
            getBaseActivity().onBackPressed();
    }


    public void post(int code, String name, Object obj) {
        EventBus.getDefault().post(new BusEvent(code, name, obj));
    }

    public void start(BaseFragment toFragment) {

        getSupportDelegate().start(toFragment);
    }

    public void start(Class<? extends BaseFragment> fragment) {
        BaseFragment fra = null;
        try {
            fra = fragment.newInstance();
        } catch (java.lang.InstantiationException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
        start(fra);
    }

    public void start(Class<? extends BaseFragment> fragment, Bundle bundle) {
        BaseFragment fra = null;
        try {
            fra = fragment.newInstance();
        } catch (java.lang.InstantiationException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
        if (bundle != null) {
            //参数
            fra.setArguments(bundle);
        }
        start(fra);
    }

    public void startWithPop(Class<? extends BaseFragment> fragment, Bundle bundle) {
        BaseFragment fra = null;
        try {
            fra = fragment.newInstance();
        } catch (java.lang.InstantiationException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
        if (bundle != null) {
            //参数
            fra.setArguments(bundle);
        }
        startWithPop(fra);
    }

    public void toMainFragment() {
        //popTo(MainFragment.class, false);
        Fragment mainFragment = getBaseActivity().getSupportFragmentManager().findFragmentByTag(MainFragment.class.getName());
        if (mainFragment != null) {
            getBaseActivity().getSupportFragmentManager().popBackStackImmediate(MainFragment.class.getName(), 0);
        } else {
            getBaseActivity().getSupportFragmentManager().popBackStackImmediate(null, 1);
            ((BaseActivity) _mActivity).loadRootFragment(R.id.mhoneframeLayout, MainFragment.newInstance());
        }
    }

    public void toHomeWalletFragment() {
        Fragment homeWalletFragment = getBaseActivity().getSupportFragmentManager().findFragmentByTag(HomeWalletFragment.class.getName());
        if (homeWalletFragment != null) {
            getBaseActivity().getSupportFragmentManager().popBackStackImmediate(HomeWalletFragment.class.getName(), 0);
        } else {
            getBaseActivity().getSupportFragmentManager().popBackStackImmediate(null, 1);
            ((BaseActivity) _mActivity).loadRootFragment(R.id.mhoneframeLayout, HomeWalletFragment.newInstance());
        }
    }

    public void popToTagetFragment(Class<BaseFragment> aClass) {
        Fragment fragment = getBaseActivity().getSupportFragmentManager().findFragmentByTag(aClass.getName());
        if (fragment != null) {
            getBaseActivity().getSupportFragmentManager().popBackStackImmediate(aClass.getName(), 0);
        } else {
            getBaseActivity().getSupportFragmentManager().popBackStackImmediate(null, 1);
            ((BaseActivity) _mActivity).loadRootFragment(R.id.mhoneframeLayout, MainFragment.newInstance());
        }
    }

    public void registReceiver() {
        if (!EventBus.getDefault().isRegistered(this))
            EventBus.getDefault().register(this);
    }

    protected void initClassicsFooter() {
        ClassicsFooter.REFRESH_FOOTER_RELEASE = getString(R.string.srl_footer_release);//"释放立即加载";
        ClassicsFooter.REFRESH_FOOTER_REFRESHING = getString(R.string.srl_footer_refreshing);//"正在刷新...";
        ClassicsFooter.REFRESH_FOOTER_LOADING = getString(R.string.srl_footer_loading);//"正在加载...";
        ClassicsFooter.REFRESH_FOOTER_FINISH = getString(R.string.srl_footer_finish);//"加载完成";
        ClassicsFooter.REFRESH_FOOTER_FAILED = getString(R.string.srl_footer_failed);//"加载失败";
    }

    protected void initClassicsHeader() {
        ClassicsHeader.REFRESH_HEADER_PULLDOWN = getString(R.string.srl_header_pulling);//"下拉可以刷新";
        ClassicsHeader.REFRESH_HEADER_REFRESHING = getString(R.string.srl_header_refreshing);//"正在刷新...";
        ClassicsHeader.REFRESH_HEADER_LOADING = getString(R.string.srl_header_loading);//"正在加载...";
        ClassicsHeader.REFRESH_HEADER_RELEASE = getString(R.string.srl_header_release);//"释放立即刷新";
        ClassicsHeader.REFRESH_HEADER_FINISH = getString(R.string.srl_header_finish);//"刷新完成";
        ClassicsHeader.REFRESH_HEADER_FAILED = getString(R.string.srl_header_failed);//"刷新失败";
        ClassicsHeader.REFRESH_HEADER_LASTTIME = getString(R.string.srl_header_update);//"上次更新 M-d HH:mm";
    }

    SmartRefreshLayout smartRefreshLayout;

    /*请求异常回调*/
    protected void onErrorRefreshLayout(SmartRefreshLayout refreshLayout) {
        this.smartRefreshLayout = refreshLayout;
        getBaseActivity().onErrorRefreshLayout(refreshLayout);
    }

    private void onDestroyRefreshLayout() {
        getBaseActivity().onDestroyRefreshLayout();
    }

    private static final long WAIT_TIME = 2000L;
    private long TOUCH_TIME = 0;

    public boolean closeApp() {
        if (smartRefreshLayout != null) {
            if (smartRefreshLayout.isRefreshing()) {
                smartRefreshLayout.finishRefresh();
            }
            if (smartRefreshLayout.isLoading()) {
                smartRefreshLayout.finishLoadMore();
            }
        }
        if (System.currentTimeMillis() - TOUCH_TIME < WAIT_TIME) {
            _mActivity.finish();
            System.exit(0);
        } else {
            TOUCH_TIME = System.currentTimeMillis();
            Toast.makeText(_mActivity, getString(R.string.press_exit_again), Toast.LENGTH_SHORT).show();
        }
        return true;
    }

    public void toDIDListFragment() {
        Fragment fragment = getBaseActivity().getSupportFragmentManager().findFragmentByTag(DIDListFragment.class.getName());
        if (fragment != null) {
            popTo(DIDListFragment.class, false);
        } else {
            startWithPopTo(new DIDListFragment(), MainFragment.class, false);

        }
    }

    public void toDIDDetailFragment() {
        Fragment fragment = getBaseActivity().getSupportFragmentManager().findFragmentByTag(DidDetailFragment.class.getName());
        if (fragment != null) {
            popTo(DidDetailFragment.class, false);
        } else {
            startWithPopTo(new DIDListFragment(), WalletManageFragment.class, false);

        }
    }

    public void showFragmentMethod() {
        Log.d("???111getTopFragment", getTopFragment().getClass().getSimpleName());
        Log.d("???111getPreFragment", getPreFragment().getClass().getSimpleName());
        Log.d("???111findFragment", findFragment(ProposalFragment.class).getClass().getSimpleName());
        for (Fragment fragment : getFragmentManager().getFragments()) {
            Log.d("???111getTag", fragment.getTag());
            Log.d("???111getSimpleName", fragment.getClass().getSimpleName());
            Log.d("???111getId", fragment.getId() + "");
        }
    }

}
