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

package org.elastos.wallet.ela.ui.Assets.fragment;

import android.content.Intent;
import android.os.Bundle;
import android.os.Parcelable;
import android.text.TextUtils;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.table.SubWallet;
import org.elastos.wallet.ela.ui.Assets.activity.IPAddressActivity;
import org.elastos.wallet.ela.ui.Assets.presenter.AssetsPresenter;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.utils.RxEnum;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.math.BigDecimal;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class NodeConnectSetDetailFragment extends BaseFragment implements CommmonStringWithMethNameViewData {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_status)
    TextView tvStatus;
    @BindView(R.id.tv_ipaddress)
    TextView tvIpaddress;
    @BindView(R.id.tv_speed)
    TextView tvSpeed;
    @BindView(R.id.iv_autoconnect)
    ImageView ivAutoconnect;
    @BindView(R.id.tv_change)
    TextView tvChange;
    Unbinder unbinder;
    private SubWallet subWallet;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_nodeconnectsetdetail;
    }

    @Override
    protected void setExtraData(Bundle data) {
        subWallet = data.getParcelable("subWallet");


    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(subWallet.getChainId()+" "+getString(R.string.nodeconnectsetting));
        if (subWallet.getFiled1().equals("Connected") && !TextUtils.isEmpty(subWallet.getDownloadPeer())) {
            putData(subWallet);

        }

        ivAutoconnect.setSelected(true);
        registReceiver();

    }

    private void putData(SubWallet subWallet) {
        tvStatus.setText(getString(R.string.connected));
        tvIpaddress.setText(subWallet.getDownloadPeer());
        BigDecimal bigDecimal = new BigDecimal(subWallet.getBytesPerSecond()).divide(new BigDecimal(1024), 2, BigDecimal.ROUND_DOWN);
        if (bigDecimal.compareTo(new BigDecimal(1024)) < 0) {
            tvSpeed.setText(bigDecimal.toPlainString() + "KB/s");
        } else {
            bigDecimal = bigDecimal.divide(new BigDecimal(1024), 2, BigDecimal.ROUND_DOWN);
            tvSpeed.setText(bigDecimal.toPlainString() + "MB/s");
        }
    }


    @OnClick({R.id.tv_change, R.id.iv_autoconnect})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.iv_autoconnect:
                //点击自动连接按钮
                if (!ivAutoconnect.isSelected()) {
                    ivAutoconnect.setSelected(true);
                    tvChange.setText(getString(R.string.autoconnect));
                    randomChange();
                } else {
                    Intent intent = new Intent(getBaseActivity(), IPAddressActivity.class);
                    intent.putExtra("subWallet", (Parcelable) subWallet);
                    startActivity(intent);
                }
                break;
            case R.id.tv_change:
                if (ivAutoconnect.isSelected()) {
                    //随机切换
                    if (TextUtils.isEmpty(subWallet.getDownloadPeer())) {
                        showToast(getString(R.string.connecting));
                    } else {
                        randomChange();
                    }
                } else {
                    //手动输入
                    Intent intent = new Intent(getBaseActivity(), IPAddressActivity.class);
                    intent.putExtra("subWallet", (Parcelable) subWallet);
                    startActivity(intent);
                }
                break;
        }
    }

    private void initData() {
        tvStatus.setText("--");
        tvIpaddress.setText("--");
        tvSpeed.setText("--");
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.UPDATAPROGRESS.ordinal()) {
            if (subWallet.getFiled1().equals("Connected") && !TextUtils.isEmpty(subWallet.getDownloadPeer())) {
                putData(subWallet);
            } else {
                initData();
            }

        }
        if (integer == RxEnum.IPVALID.ordinal()) {
            ivAutoconnect.setSelected(false);
            tvChange.setText(getString(R.string.handinput));
        }
    }


    @Override
    public void onGetCommonData(String methodname, String data) {

    }


    private void randomChange() {
        subWallet.setDownloadPeer(null);
        initData();
        new AssetsPresenter().syncStop(subWallet.getBelongId(), subWallet.getChainId(), this);
        new AssetsPresenter().syncStart(subWallet.getBelongId(), subWallet.getChainId(), this);

    }


}
