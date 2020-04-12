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

package org.elastos.wallet.ela.ui.mine.fragment;

import android.content.Intent;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.listener.RealmTransactionAbs;
import org.elastos.wallet.ela.db.table.Contact;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.QRCodeUtils;
import org.elastos.wallet.ela.utils.QrBean;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.ScanQRcodeUtil;
import org.elastos.wallet.ela.utils.ScreenUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.UUID;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class ContactDetailFragment extends BaseFragment {


    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.et_name)
    EditText etName;
    @BindView(R.id.et_walletaddr)
    EditText etWalletaddr;
    @BindView(R.id.iv_1)
    ImageView iv1;
    @BindView(R.id.iv_2)
    ImageView iv2;
    @BindView(R.id.et_mobile)
    EditText etMobile;
    @BindView(R.id.et_email)
    EditText etEmail;
    @BindView(R.id.et_remark)
    EditText etRemark;
    @BindView(R.id.tv_add)
    TextView tvAdd;
    @BindView(R.id.tv_change)
    TextView tvChange;
    @BindView(R.id.tv_edit)
    TextView tvEdit;
    @BindView(R.id.tv_delete)
    TextView tvDelete;
    @BindView(R.id.ll_edit)
    LinearLayout llEdit;
    Unbinder unbinder;
    private RealmUtil realmUtil;
    private Contact contact;
    private String type;
    private DialogUtil dialogUtil;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_contact_detail;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        realmUtil = new RealmUtil();
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        type = data.getString("type");
        switch (type) {
            case Constant.CONTACTADD:
                //添加联系人
                tvTitle.setText(R.string.addcontact);
                break;
            case Constant.CONTACTEDIT:
                //编辑联系人
                tvTitle.setText(R.string.editcontact);
                contact = data.getParcelable("Contact");
                setEtData(contact);
                tvAdd.setVisibility(View.GONE);
                tvChange.setVisibility(View.VISIBLE);
                break;
            case Constant.CONTACTSHOW:
                //查看联系人
                iv1.setImageResource(R.mipmap.setting_adding_copy);
                iv2.setImageResource(R.mipmap.setting_adding_code);
                tvTitle.setText(R.string.showcontact);
                etEnable(false);
                contact = data.getParcelable("Contact");
                setEtData(contact);
                tvAdd.setVisibility(View.GONE);
                llEdit.setVisibility(View.VISIBLE);
                //只有查看页面才会有这些操作
                registReceiver();
                dialogUtil = new DialogUtil();
                break;
        }


    }

    private void setEtData(Contact contact) {
        if (contact == null) {
            return;
        }
        etName.setText(contact.getName());
        etWalletaddr.setText(contact.getWalletAddr());
        etMobile.setText(contact.getMobile());
        etEmail.setText(contact.getEmail());
        etRemark.setText(contact.getRemark());
    }

    private void etEnable(Boolean type) {
        etName.setEnabled(type);
        etWalletaddr.setEnabled(type);
        etMobile.setEnabled(type);
        etEmail.setEnabled(type);
        etRemark.setEnabled(type);
    }

    @OnClick({R.id.tv_add, R.id.tv_edit, R.id.tv_delete, R.id.iv_1, R.id.iv_2, R.id.tv_change})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.iv_1:
                doPicture(0);


                break;
            case R.id.iv_2:
                doPicture(1);
                break;
            case R.id.tv_add:
                addContact(null);
                break;
            case R.id.tv_change:
                addContact(contact.getId());
                break;

            case R.id.tv_edit:
                //去编辑页面
                Bundle bundle = new Bundle();
                bundle.putParcelable("Contact", contact);
                bundle.putString("type", Constant.CONTACTEDIT);
                ContactDetailFragment contactDetailFragment = new ContactDetailFragment();
                contactDetailFragment.setArguments(bundle);
                start(contactDetailFragment);
                break;
            case R.id.tv_delete:
                dialogUtil.showWarmPrompt1(getBaseActivity(), getString(R.string.deletornot), new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        realmUtil.deleteContact(contact.getId());
                        post(RxEnum.UPDATACONTACT.ordinal(), "删除", null);
                        popBackFragment();

                    }
                });

                break;
        }
    }

    private void doPicture(int i) {
        switch (type) {
            case Constant.CONTACTADD:
            case Constant.CONTACTEDIT:
                //添加联系人
                if (i == 0) {
                    requstManifestPermission(getString(R.string.needpermission));
                } else {
                    etWalletaddr.setText(ClipboardUtil.paste(getBaseActivity()));
                }
                break;
            case Constant.CONTACTSHOW:
                String walletAddr = etWalletaddr.getText().toString().trim();
                if (TextUtils.isEmpty(walletAddr)) {
                    return;
                }
                if (i == 0) {
                    ClipboardUtil.copyClipboar(getBaseActivity(), walletAddr);
                } else {
                    //显示二维码
                    Bitmap mBitmap = QRCodeUtils.createQrCodeBitmap(walletAddr, ScreenUtil.dp2px(getContext(), 240), ScreenUtil.dp2px(getContext(), 240));
                    dialogUtil.showImage(getBaseActivity(), mBitmap);
                }

                break;
        }
    }


    private void addContact(String id) {
        if (TextUtils.isEmpty(id)) {
            id = UUID.randomUUID().toString().replaceAll("-", "");
        }
        String name = etName.getText().toString().trim();
        if (TextUtils.isEmpty(name)) {
            showToast(getString(R.string.namenoempty));
            return;
        }
        String wallletAddr = etWalletaddr.getText().toString().trim();
        if (TextUtils.isEmpty(wallletAddr)) {
            showToast(getString(R.string.wallletaddrnoempty));
            return;
        }
        //(String id, String name, String walletAddr, String mobile, String email, String remark, String filed1, String filed2, String filed3)
        contact = new Contact(id,
                name, wallletAddr, getText(etMobile), getText(etEmail), getText(etRemark));
        realmUtil.insertContact(contact, new RealmTransactionAbs() {
            @Override
            public void onSuccess() {
                post(RxEnum.UPDATACONTACT.ordinal(), "增加或者修改", contact);
                popBackFragment();
            }
        });
    }

    private String getText(EditText editText) {
        return editText.getText().toString().trim();
    }

    @Override
    protected void requstPermissionOk() {
        new ScanQRcodeUtil().scanQRcode(this);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        //处理扫描结果（在界面上显示）
        if (resultCode == RESULT_OK && requestCode == ScanQRcodeUtil.SCAN_QR_REQUEST_CODE && data != null) {
            String result = data.getStringExtra("result");//&& matcherUtil.isMatcherAddr(result)
            if (!TextUtils.isEmpty(result) /*&& matcherUtil.isMatcherAddr(result)*/) {
                etWalletaddr.setText(result);
                String address = result;
                try {
                    QrBean qrBean = JSON.parseObject(result, QrBean.class);
                    int type = qrBean.getExtra().getType();
                    if (type == Constant.TRANSFER) {
                        address = qrBean.getData();
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
                etWalletaddr.setText(address);
            }
        }

    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.UPDATACONTACT.ordinal()) {
            contact = (Contact) result.getObj();
            setEtData(contact);

        }
    }

    @Override
    public void onDestroy() {
        EventBus.getDefault().unregister(this);
        super.onDestroy();
    }
}
