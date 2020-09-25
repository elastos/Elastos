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

package org.elastos.wallet.ela.ui.main;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;

import com.chaychan.library.BottomBarItem;
import com.chaychan.library.BottomBarLayout;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.WalletNet;
import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.SupportFragment;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.Assets.AssetskFragment;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonObjectWithMethNameViewData;
import org.elastos.wallet.ela.ui.find.FindFragment;
import org.elastos.wallet.ela.ui.main.presenter.MainPresenter;
import org.elastos.wallet.ela.ui.mine.MineFragment;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.utils.certificate.CertificationUtil;

import java.util.Calendar;

import butterknife.BindView;

/**
 *
 */

public class MainFragment extends BaseFragment implements CommmonObjectWithMethNameViewData {

    @BindView(R.id.bottombar)
    BottomBarLayout mbottomBarLayout;
    @BindView(R.id.bottombaritem)
    BottomBarItem bottombaritem;
    @BindView(R.id.bottombaritem1)
    BottomBarItem bottombaritem1;
    @BindView(R.id.bottombaritem2)
    BottomBarItem bottombaritem2;
    private SupportFragment[] mFragments = new SupportFragment[4];
    private SPUtil sp;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_main;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void setExtraData(Bundle data) {

    }

    @Override
    protected void initView(View view) {
        sp = new SPUtil(getContext());
        SupportFragment homeFragment = findFragment(AssetskFragment.class);
        if (homeFragment == null) {
            AssetskFragment assetskFragment = new AssetskFragment();
            assetskFragment.setArguments(getArguments());
            mFragments[0] = assetskFragment;
            mFragments[1] = FindFragment.newInstance();
            mFragments[2] = MineFragment.newInstance();
            // mFragments[3] = MineFragment.newInstance();\
            loadMultipleRootFragment(R.id.layout_fragment, 0,
                    mFragments[0],
                    mFragments[1],
                    mFragments[2]
                    //  mFragments[3]
            );
        } else {
            // 这里库已经做了Fragment恢复,所有不需要额外的处理了, 不会出现重叠问题

            // 这里我们需要拿到mFragments的引用
            mFragments[0] = homeFragment;
            mFragments[1] = findFragment(FindFragment.class);
            mFragments[2] = findFragment(MineFragment.class);
            // mFragments[3] = findFragment(MineFragment.class);


        }


        mbottomBarLayout.setOnItemSelectedListener(new BottomBarLayout.OnItemSelectedListener() {
            @Override
            public void onItemSelected(BottomBarItem bottomBarItem, int before, int current) {
                showHideFragment(mFragments[current], mFragments[before]);
                switch (current) {
                    case 0:
                        bottombaritem.setBackgroundResource(R.color.mainitembg);
                        bottombaritem1.setBackgroundResource(R.color.black1);
                        bottombaritem2.setBackgroundResource(R.color.black1);
                        break;
                    case 1:
                        bottombaritem.setBackgroundResource(R.color.black1);
                        bottombaritem1.setBackgroundResource(R.color.mainitembg);
                        bottombaritem2.setBackgroundResource(R.color.black1);
                        FindFragment findFragment = (FindFragment) mFragments[1];
                        findFragment.refreshView();
                        break;
                    case 2:
                        bottombaritem.setBackgroundResource(R.color.black1);
                        bottombaritem1.setBackgroundResource(R.color.black1);
                        bottombaritem2.setBackgroundResource(R.color.mainitembg);
                        break;
                }
            }
        });

        initCache();
        initServer();
    }


    private void initCache() {
        if (sp.isRefreshCache()) {
            sp.setRefreshCache();
            intAreaCache();
        }

    }

    private void intAreaCache() {
        CacheUtil.removeArea();
        AppUtlis.getArea(getContext(), null);
    }

    private void initServer() {

        if (MyApplication.currentWalletNet == WalletNet.MAINNET
                || MyApplication.currentWalletNet == WalletNet.ALPHAMAINNET) {
            //主网才有高可用
            new MainPresenter().getServerList(this);
        }

    }


    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == CertificationUtil.REQUEST_CODE_CREDENTIALS) {

            if (resultCode == RESULT_OK) {
                //系统密码识别失败 打开重新验证页面
                CertificationUtil.pwdCertificateStatus = 2;

            } else {
                CertificationUtil.pwdCertificateStatus = 3;
            }
        }
    }


    @Override
    public void onStart() {
        super.onStart();
        //安全验证
        // Log.d("???", "onStart");
        certificate();

    }

    private void certificate() {
        if (!sp.isOpenCertificate()) {
            return;
        }
        if (CertificationUtil.fingerCertificating || CertificationUtil.pwdCertificateStatus == 1) {
            return;
        }
        if (CertificationUtil.pwdCertificateStatus == 2) {
            CertificationUtil.pwdCertificateStatus = 0;
            return;
        }
        if (Calendar.getInstance().getTimeInMillis() - stop60 > 60000) {
            CertificationUtil.isOpenCertificate(this, CertificationUtil.REQUEST_CODE_CREDENTIALS);
        }
    }

    long stop60 = 0;

    @Override
    public void onStop() {
        super.onStop();
        if (sp.isOpenCertificate()) {
            stop60 = Calendar.getInstance().getTimeInMillis();
        }
    }

    public static MainFragment newInstance() {

        Bundle args = new Bundle();
        MainFragment fragment = new MainFragment();
        fragment.setArguments(args);
        return fragment;
    }


    @Override
    public void onGetCommonData(String methodname, Object data) {

        switch (methodname) {
            case "ping":
                String address = (String) data;
                if (!address.equals(MyApplication.REQUEST_BASE_URL)) {
                    sp.setDefaultServer(address);
                    //通过比较差异 sp和MyApplication.REQUEST_BASE_URL判断是否更新
                }
                break;
        }
    }
}
