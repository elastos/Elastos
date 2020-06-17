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

package org.elastos.wallet.ela.ui.Assets;


import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Intent;
import android.graphics.BitmapFactory;
import android.os.Build;
import android.os.Bundle;
import android.os.Parcelable;
import android.support.v4.app.NotificationCompat;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.google.gson.Gson;
import com.scwang.smartrefresh.layout.SmartRefreshLayout;
import com.scwang.smartrefresh.layout.api.RefreshLayout;
import com.scwang.smartrefresh.layout.listener.OnRefreshListener;

import org.elastos.did.DIDDocument;
import org.elastos.did.DIDStore;
import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.DIDStoreException;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.SubWallet;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.activity.TransferActivity;
import org.elastos.wallet.ela.ui.Assets.adapter.AssetskAdapter;
import org.elastos.wallet.ela.ui.Assets.bean.BalanceEntity;
import org.elastos.wallet.ela.ui.Assets.bean.qr.RecieveJwtEntity;
import org.elastos.wallet.ela.ui.Assets.bean.qr.proposal.RecieveProcessJwtEntity;
import org.elastos.wallet.ela.ui.Assets.bean.qr.proposal.RecieveProposalFatherJwtEntity;
import org.elastos.wallet.ela.ui.Assets.bean.qr.proposal.RecieveProposalJwtEntity;
import org.elastos.wallet.ela.ui.Assets.bean.qr.proposal.RecievePublishedVoteJwtEntity;
import org.elastos.wallet.ela.ui.Assets.bean.qr.proposal.RecieveReviewJwtEntity;
import org.elastos.wallet.ela.ui.Assets.bean.qr.proposal.RecieveWithdrawJwtEntity;
import org.elastos.wallet.ela.ui.Assets.fragment.AddAssetFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.AssetDetailsFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.CreateSignReadOnlyWalletFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.TransferFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.WalletListFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.WalletManageFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.mulsignwallet.CreateMulWalletFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.transfer.SignFragment;
import org.elastos.wallet.ela.ui.Assets.listener.ISubWalletListener;
import org.elastos.wallet.ela.ui.Assets.presenter.AssetsPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonGetBalancePresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.PwdPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.TransferPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.WalletManagePresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.mulwallet.CreatMulWalletPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.AssetsViewData;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonBalanceViewData;
import org.elastos.wallet.ela.ui.committee.bean.CtDetailBean;
import org.elastos.wallet.ela.ui.committee.bean.CtListBean;
import org.elastos.wallet.ela.ui.committee.presenter.CtListPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonObjEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringWithiMethNameEntity;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener1;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.crvote.presenter.CRlistPresenter;
import org.elastos.wallet.ela.ui.did.fragment.AuthorizationFragment;
import org.elastos.wallet.ela.ui.main.MainActivity;
import org.elastos.wallet.ela.ui.mine.bean.MessageEntity;
import org.elastos.wallet.ela.ui.mine.fragment.MessageListFragment;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalSearchEntity;
import org.elastos.wallet.ela.ui.proposal.fragment.SuggestionsInfoFragment;
import org.elastos.wallet.ela.ui.proposal.presenter.ProposalDetailPresenter;
import org.elastos.wallet.ela.ui.proposal.presenter.ProposalPresenter;
import org.elastos.wallet.ela.ui.proposal.presenter.bean.ProposalProcessPayLoad;
import org.elastos.wallet.ela.ui.proposal.presenter.bean.ProposalReviewPayLoad;
import org.elastos.wallet.ela.ui.proposal.presenter.bean.ProposalWithdrawPayLoad;
import org.elastos.wallet.ela.ui.vote.ElectoralAffairs.VoteListPresenter;
import org.elastos.wallet.ela.ui.vote.activity.VoteActivity;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.JwtUtils;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.MyUtil;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.QrBean;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.utils.ScanQRcodeUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.math.BigDecimal;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import butterknife.BindView;
import butterknife.OnClick;

import static android.content.Context.NOTIFICATION_SERVICE;


/**
 * 资产首页
 */
public class AssetskFragment extends BaseFragment implements AssetsViewData, CommonRvListener1, ISubWalletListener, OnRefreshListener, CommonBalanceViewData, CommmonStringWithMethNameViewData, NewBaseViewData {

    @BindView(R.id.iv_title_left)
    ImageView ivTitleLeft;
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.iv_add)
    ImageView ivAdd;
    @BindView(R.id.recyclerview)
    RecyclerView recyclerview;
    @BindView(R.id.srl)
    SmartRefreshLayout srl;
    private AssetskAdapter assetskAdapter;
    private AssetsPresenter assetsPresenter;
    private RealmUtil realmUtil;
    private Wallet wallet;
    private CommonGetBalancePresenter commonGetBalancePresenter;
    private Map<String, List<org.elastos.wallet.ela.db.table.SubWallet>> listMap;
    private Map<String, String> transactionMap;
    private String scanResult;
    private String payPasswd;
    private ProposalPresenter proposalPresenter;
    private ProposalDetailPresenter proposalDetailPresenter;
    private RecieveProposalFatherJwtEntity curentJwtEntity;
    private String voteNum;
    private List<ProposalSearchEntity.DataBean.ListBean> searchBeanList;
    private List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> crList;
    private List<VoteListBean.DataBean.ResultBean.ProducersBean> depositList;
    private List<CtListBean.Council> councilList;

    @Override
    protected int getLayoutId() {
        initClassicsHeader();
        return R.layout.fragment_assetsk;
    }

    @Override

    public void onSaveInstanceState(Bundle outState) {
        realmUtil.updateSubWalletDetial(listMap);
        CacheUtil.setUnReadMessage(messageList);
        super.onSaveInstanceState(outState);
    }

    @Override
    protected void setExtraData(Bundle data) {
        String toValue = data.getString("toValue");
        if ("notice".equals(toValue)) {
            ((BaseFragment) getParentFragment()).start(MessageListFragment.class);
        }

    }

    @Override
    protected void initView(View view) {
        srl.setOnRefreshListener(this);
        assetsPresenter = new AssetsPresenter();
        commonGetBalancePresenter = new CommonGetBalancePresenter();
        realmUtil = new RealmUtil();
        wallet = realmUtil.queryDefauleWallet();
        getMyDID().init(wallet.getWalletId());//初始化mydid
        tvTitle.setText(wallet.getWalletName());
        setWalletViewNew(wallet);
        // List<Wallet> wallets = realmUtil.queryUserAllWallet();
        listMap = new HashMap<>();
        transactionMap = new HashMap<>();
     /*   for (Wallet wallet : wallets) {
            assetsPresenter.getAllSubWallets(wallet.getWalletId(), this);
        }*/
        // assetsPresenter.getAllSubWallets(wallet.getWalletId(), this);
        registReceiver();
    }

    @OnClick({R.id.iv_title_left, R.id.iv_title_right, R.id.iv_add, R.id.tv_title, R.id.iv_scan})
    public void onViewClicked(View view) {
        Bundle bundle;
        switch (view.getId()) {
            case R.id.iv_title_left:
            case R.id.tv_title:
                //钱包列表
                ((BaseFragment) getParentFragment()).start(new WalletListFragment());
                break;
            case R.id.iv_title_right:
                //钱包管理
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                bundle.putParcelableArrayList("subWallets", (ArrayList<? extends Parcelable>) listMap.get(wallet.getWalletId()));

                ((BaseFragment) getParentFragment()).start(WalletManageFragment.class, bundle);
                break;
            case R.id.iv_scan:
                //扫一扫
                requstManifestPermission(getString(R.string.needpermission));

                break;
            case R.id.iv_add:
                bundle = new Bundle();
                bundle.putString("walletId", wallet.getWalletId());
                ArrayList<String> chainIds = new ArrayList<>();
                List<org.elastos.wallet.ela.db.table.SubWallet> assetList = listMap.get(wallet.getWalletId());
                for (org.elastos.wallet.ela.db.table.SubWallet iSubWallet : assetList) {
                    chainIds.add(iSubWallet.getChainId());
                }
                bundle.putStringArrayList("chainIds", chainIds);
                ((BaseFragment) getParentFragment()).start(AddAssetFragment.class, bundle);
                break;

        }
    }

    public static AssetskFragment newInstance() {
        Bundle args = new Bundle();
        AssetskFragment fragment = new AssetskFragment();
        fragment.setArguments(args);
        return fragment;
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
            restoreScanData();
            scanResult = data.getStringExtra("result");//&& matcherUtil.isMatcherAddr(result)
            if (!TextUtils.isEmpty(scanResult) /*&& matcherUtil.isMatcherAddr(result)*/) {
                if (scanResult.startsWith("elastos:")) {
                    //兼容elastos:
                    scanElastos(scanResult);
                } else {
                    scanQrBean(scanResult);
                }

            }
        }

    }

    private void scanQrBean(String result) {
        try {
            QrBean qrBean = JSON.parseObject(result, QrBean.class);
            int type = qrBean.getExtra().getType();
            Bundle bundle = new Bundle();

            switch (type) {
                case Constant.TRANSFER:
                    //扫描联系人到转账页面
                    bundle.putParcelable("wallet", wallet);
                    bundle.putString("ChainID", qrBean.getExtra().getSubWallet());
                    bundle.putString("address", qrBean.getData());
                    ((BaseFragment) getParentFragment()).start(TransferFragment.class, bundle);
                    break;
                case Constant.CREATEREADONLY:
                    //创建只读钱包
                    bundle.putString("result", result);
                    ((BaseFragment) getParentFragment()).start(CreateSignReadOnlyWalletFragment.class, bundle);
                    break;
                case Constant.CREATEMUL:
                    //创建多签钱包
                    bundle.putString("result", result);
                    ((BaseFragment) getParentFragment()).start(CreateMulWalletFragment.class, bundle);
                    break;
                case Constant.SIGN:
                    //去签名
                    //数据完整后跳转//如果是其他数据  用新的数据
                    String attribute = getData(qrBean, Constant.SIGN);
                    if (!TextUtils.isEmpty(attribute)) {
                        bundle.putParcelable("wallet", wallet);
                        bundle.putInt("transType", qrBean.getExtra().getTransType());
                        bundle.putString("attributes", attribute);
                        ((BaseFragment) getParentFragment()).start(SignFragment.class, bundle);
                    }
                    break;
                default:
                    toErroScan(result);
                    break;
            }
        } catch (Exception e) {
            toErroScan(result);
        }
    }

    /**
     * 所有elastos 的第一级分发
     *
     * @param result
     */
    private void scanElastos(String result) {
        if (result.startsWith("elastos://credaccess/")) {
            //did
            ////0 普通单签 1单签只读 2普通多签 3多签只读
            if (wallet.getType() != 0) {
                toErroScan(scanResult);

            }
            decodeWebJwt("elastos://credaccess/", result);

        } else if (result.startsWith("elastos://crproposal/")) {
            //社区提案的二维码
            ////0 普通单签 1单签只读 2普通多签 3多签只读
            if (wallet.getType() != 0) {
                toErroScan(scanResult);
                return;
            }
            decodeWebJwt("elastos://crproposal/", result);

        } else {
            //钱包转账地址情况 elastos:EJQcgWDazveSy436TauPJ3R8PCYpifp6HA?amount=6666.00000000
            result = result.replace("elastos:", "");
            String[] parts = result.split("\\?");
            diposeElastosCaode(new TransferPresenter().analyzeElastosData(parts, wallet.getWalletId(), this), parts);

        }
    }


    private void decodeWebJwt(String tag, String result) {
        try {
            result = result.replace(tag, "");
            String payload = JwtUtils.getJwtPayload(result);
            RecieveJwtEntity recieveJwtEntity = JSON.parseObject(payload, RecieveJwtEntity.class);
            String elaString = recieveJwtEntity.getIss();
            elaString = elaString.contains("did:elastos:") ? elaString : "did:elastos:" + elaString;
            // Log.e("Base64", "Base64---->" + header + "\n" + payload + "\n" + signature);.0
            new WalletManagePresenter().forceDIDResolve(elaString, this, tag);
        } catch (Exception e) {
            toErroScan(scanResult);
        }


    }


    private void setWalletView(Wallet wallet) {
        tvTitle.setText(wallet.getWalletName());
        switch (wallet.getType()) {
            //0 普通单签 1单签只读 2普通多签 3多签只读
            case 0:
                ivTitleLeft.setImageResource(R.mipmap.single_wallet);
                break;
            case 1:
                ivTitleLeft.setImageResource(R.mipmap.single_walllet_readonly);
                break;
            case 2:
                ivTitleLeft.setImageResource(R.mipmap.multi_wallet);
                break;
            case 3:
                ivTitleLeft.setImageResource(R.mipmap.multi_wallet_readonly);
                break;
        }
    }

    /**
     * 这里调用getAllSubWallets会刷新recycleview
     *
     * @param wallet
     */
    private void setWalletViewNew(Wallet wallet) {
        setWalletView(wallet);
        assetsPresenter.getAllSubWallets(wallet.getWalletId(), this);
    }


    private void setRecycleView() {
        List<org.elastos.wallet.ela.db.table.SubWallet> assetList = listMap.get(wallet.getWalletId());
        if (assetList == null || assetList.size() == 0) {
            return;
        }
        if (assetskAdapter == null) {
            recyclerview.setAdapter(assetskAdapter = new AssetskAdapter(getContext(), assetList));
            recyclerview.setLayoutManager(new LinearLayoutManager(getContext()));
            recyclerview.setHasFixedSize(true);
            recyclerview.setNestedScrollingEnabled(false);
            recyclerview.setFocusableInTouchMode(false);
            assetskAdapter.setCommonRvListener(this);
        } else {
            assetskAdapter.setData(assetList);
            assetskAdapter.notifyDataSetChanged();
        }
    }

    @Override
    public synchronized void onGetAllSubWallets(List<SubWallet> data, int type) {
        if (data == null || data.size() == 0) {
            return;
        }
        String currentBelongId = data.get(0).getBelongId();
        List<SubWallet> assetList = listMap.get(currentBelongId);//原来的数据
        listMap.put(currentBelongId, data);
        for (SubWallet newSubWallet : data) {

            First:
            {
                if (assetList != null && assetList.size() != 0) {
                    for (SubWallet oldSubWallet : assetList) {
                        if (newSubWallet.equals(oldSubWallet)) {
                            //原有的数据保留
                            newSubWallet.setProgress(oldSubWallet.getProgress());
                            newSubWallet.setSyncTime(oldSubWallet.getSyncTime());
                            break First;

                        }

                    }
                    assetsPresenter.syncStart(currentBelongId, newSubWallet.getChainId(), this);
                    SubWallet subWallet = realmUtil.querySubWallet(newSubWallet.getBelongId(), newSubWallet.getChainId());
                    newSubWallet.setProgress(subWallet.getProgress());
                    newSubWallet.setSyncTime(subWallet.getSyncTime());
                    assetsPresenter.registerWalletListener(currentBelongId, newSubWallet.getChainId(), this);
                } else {
                    assetsPresenter.syncStart(currentBelongId, newSubWallet.getChainId(), this);
                    SubWallet subWallet = realmUtil.querySubWallet(newSubWallet.getBelongId(), newSubWallet.getChainId());
                    newSubWallet.setProgress(subWallet.getProgress());
                    newSubWallet.setSyncTime(subWallet.getSyncTime());
                    assetsPresenter.registerWalletListener(currentBelongId, newSubWallet.getChainId(), this);
                }
            }

        }


        if (wallet.getWalletId().equals(currentBelongId)) {
            setRecycleView();
        }
    }

    @Override
    public void onBalance(BalanceEntity data) {

        List<org.elastos.wallet.ela.db.table.SubWallet> assetList = listMap.get(data.getMasterWalletId());
        for (org.elastos.wallet.ela.db.table.SubWallet assetsItemEntity : assetList) {
            if (assetsItemEntity.getChainId().equals(data.getChainId())) {
                assetsItemEntity.setBalance(data.getBalance());
                if (wallet.getWalletId().equals(data.getMasterWalletId())) {
                    post(RxEnum.BALANCECHANGE.ordinal(), null, assetsItemEntity);
                }
            }

        }
    }

    private void toVoteActivity(BalanceEntity data) {
        Intent intent = new Intent(getContext(), VoteActivity.class);
        BigDecimal balance = Arith.div(Arith.sub(data.getBalance(), 1000000), MyWallet.RATE_S, 8);
        String maxBalance = NumberiUtil.removeZero(balance.toPlainString());
        //小于1 huo 0
        if ((balance.compareTo(new BigDecimal(0)) <= 0)) {
            intent.putExtra("maxBalance", "0");
        } else {
            intent.putExtra("maxBalance", maxBalance);
        }
        intent.putExtra("openType", getClass().getSimpleName());
        intent.putExtra("type", Constant.PROPOSALPUBLISHED);
        startActivity(intent);
    }


    @Override
    public void onRvItemClick(View view, int position, Object o) {
        Bundle bundle = new Bundle();
        bundle.putParcelable("wallet", wallet);
        for (SubWallet subWallet : listMap.get(wallet.getWalletId())) {
            if (subWallet.getChainId().equals((String) o)) {
                bundle.putParcelable("subWallet", subWallet);
            }
        }

        ((BaseFragment) getParentFragment()).start(AssetDetailsFragment.class, bundle);
    }

    /***************ISubWalletListener子钱包同步的监听*****************/
    @Override
    public void OnTransactionStatusChanged(JSONObject jsonObject) {
        try {
            int confirms = jsonObject.getInt("confirms");
            String hash = jsonObject.getString("txId");
            if (confirms == 0) {
                //did发布和更新
                if (hash.equals(getMyDID().getMyDIDAdapter().getTxId())) {
                    getMyDID().getMyDIDAdapter().getCallback().accept(hash, 0, null);
                }

                return;
            }
            String transferType = transactionMap.get(hash);
            if (TextUtils.isEmpty(transferType)) {
                return;
            }
            //1002是投票覆盖其他投票的特殊投票类型  在此成功时候给出提醒
            String reason;
            if ("1002".equals(transferType)) {
                reason = String.format(getString(R.string.specialvotesucessreason), getString(R.string.crcvote));

            } else if ("1003".equals(transferType)) {
                reason = String.format(getString(R.string.specialvotesucessreason), getString(R.string.supernode_election));

            } else {
                return;
            }
            //只需要一次成功的提醒
            transactionMap.remove(hash);

            String chainId = jsonObject.getString("ChainID");
            String masterWalletID = jsonObject.getString("MasterWalletID");
            String transferTypeDes = getTransferDes(transferType, chainId);
            String walleName = realmUtil.queryUserWallet(masterWalletID).getWalletName();
            showNotification(reason, transferTypeDes, walleName);
            addToMessageCenter(hash, transferType, chainId, walleName, reason);

        } catch (Exception e) {
            Log.i(getClass().getSimpleName(), e.getMessage());
        }

    }

    @Override
    public void OnBlockSyncStarted(JSONObject jsonObject) {

        //   Log.e("???", jsonObject.toString());
    }

    @Override
    public synchronized void OnBlockSyncProgress(JSONObject jsonObject) {

        try {
            long BytesPerSecond = jsonObject.getLong("BytesPerSecond");
            String DownloadPeer = jsonObject.getString("DownloadPeer");
            String MasterWalletID = jsonObject.getString("MasterWalletID");
            long lastBlockTime = jsonObject.getLong("LastBlockTime");
            String ChainID = jsonObject.getString("ChainID");
            //同步进行中
            int progress = jsonObject.getInt("Progress");
            List<org.elastos.wallet.ela.db.table.SubWallet> assetList = listMap.get(MasterWalletID);
            if (assetList == null) {
                return;
            }
            for (org.elastos.wallet.ela.db.table.SubWallet subWallet : assetList) {
                if (ChainID.equals(subWallet.getChainId())) {
                    subWallet.setBytesPerSecond(BytesPerSecond);
                    subWallet.setDownloadPeer(DownloadPeer);
                    subWallet.setProgress(progress);
                    if (lastBlockTime != 0) {
                        // String curentTime = DateUtil.time(lastBlockTime);
                        subWallet.setSyncTime(lastBlockTime + "");
                        subWallet.setFiled1("Connected");
                    }
                    subWallet.setProgress(progress);
                    if (progress == 100) {
                        subWallet.setFiled2("true");
                    }
                    if (wallet.getWalletId().equals(MasterWalletID)) {
                        post(RxEnum.UPDATAPROGRESS.ordinal(), null, subWallet);
                    }
                }
            }


        } catch (JSONException e) {
            e.printStackTrace();
        }

    }

    @Override
    public void OnBlockSyncStopped(JSONObject jsonObject) {
        //同步完成  记录时间并更新
        // Log.e("???", jsonObject.toString());
    }

    @Override
    public synchronized void OnBalanceChanged(JSONObject jsonObject) {

        try {
            String MasterWalletID = jsonObject.getString("MasterWalletID");
            String ChainID = jsonObject.getString("ChainID");
            long balance = jsonObject.getLong("Balance");
            List<org.elastos.wallet.ela.db.table.SubWallet> assetList = listMap.get(MasterWalletID);
            if (assetList == null) {
                return;
            }
            for (org.elastos.wallet.ela.db.table.SubWallet assetsItemEntity : assetList) {
                if (ChainID.equals(assetsItemEntity.getChainId())) {
                    assetsItemEntity.setBalance(balance + "");
                    if (wallet.getWalletId().equals(MasterWalletID)) {
                        post(RxEnum.BALANCECHANGE.ordinal(), null, assetsItemEntity);
                    }

                }
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }

    }

    public static List<MessageEntity> messageList;

    @Override
    public synchronized void OnTxPublished(JSONObject jsonObject) {

        try {
            String hash = jsonObject.getString("hash");
            String transferType = transactionMap.get(hash);
            if (TextUtils.isEmpty(transferType)) {
                return;
            }
            String resultString = jsonObject.getString("result");
            JSONObject result = new JSONObject(resultString);
            int code = result.getInt("Code");
            String reason = result.getString("Reason");
            //只捕捉失败的回调
            if (code == 0 || (code == 18 && reason.contains("uplicate"))) {
                return;
            }
            if (hash.equals(getMyDID().getMyDIDAdapter().getTxId())) {
                getMyDID().getMyDIDAdapter().getCallback().accept(hash, code, reason);
            }
            //只需要捕捉一次失败
            transactionMap.remove(hash);
            String chainId = jsonObject.getString("ChainID");
            String masterWalletID = jsonObject.getString("MasterWalletID");
            String transferTypeDes = getTransferDes(transferType, chainId);
            String walleName = realmUtil.queryUserWallet(masterWalletID).getWalletName();
            reason = getString(R.string.commonfailereason);
            showNotification(reason, transferTypeDes, walleName);
            addToMessageCenter(hash, transferType, chainId, walleName, reason);

        } catch (Exception e) {
            Log.i(getClass().getSimpleName(), e.getMessage());
        }
    }

    private String getTransferDes(String transferType, String chainId) {
        String transferTypeDes = getString(R.string.transfertype13);
        try {
            if (chainId.equals(MyWallet.IDChain)) {
                transferTypeDes = getString(getContext().getResources().getIdentifier("sidetransfertype" + transferType, "string",
                        getContext().getPackageName()));
            } else {
                transferTypeDes = getString(getContext().getResources().getIdentifier("transfertype" + transferType, "string",
                        getContext().getPackageName()));
            }
        } catch (Exception e) {
            Log.i("transferTypeDes", e.getMessage());
        }
        return transferTypeDes;
    }

    @Override
    public void OnTxDeleted(JSONObject jsonObject) {
        //  Log.e("???", jsonObject.toString());
    }

    @Override
    public void OnAssetRegistered(JSONObject jsonObject) {

    }

    @Override
    public synchronized void OnConnectStatusChanged(JSONObject jsonObject) {
        //"Connecting", "Connected", "Disconnected"
        // jsonObject.put("status", status);
        //            jsonObject.put("MasterWalletID", mMasterWalletID);
        //            jsonObject.put("ChainID", mSubWalletID);
        //            jsonObject.put("Action", "OnConnectStatusChanged");
        try {
            String MasterWalletID = jsonObject.getString("MasterWalletID");
            String ChainID = jsonObject.getString("ChainID");
            String status = jsonObject.getString("status");
            List<org.elastos.wallet.ela.db.table.SubWallet> assetList = listMap.get(MasterWalletID);
            if (assetList == null) {
                return;
            }
            for (org.elastos.wallet.ela.db.table.SubWallet subWallet : assetList) {
                if (ChainID.equals(subWallet.getChainId())) {
                    subWallet.setFiled1(status);
                    if (wallet.getWalletId().equals(MasterWalletID)) {
                        post(RxEnum.UPDATAPROGRESS.ordinal(), null, subWallet);
                    }
                }
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }

    }

    @Override
    public void onRefresh(RefreshLayout refreshLayout) {
        onErrorRefreshLayout(srl);
        List<org.elastos.wallet.ela.db.table.SubWallet> assetList = listMap.get(wallet.getWalletId());
        for (org.elastos.wallet.ela.db.table.SubWallet assetsItemEntity : assetList) {
            commonGetBalancePresenter.getBalance(wallet.getWalletId(), assetsItemEntity.getChainId(), 2, this);
            assetsPresenter.syncStart(wallet.getWalletId(), assetsItemEntity.getChainId(), this);
        }

    }

    /**
     * 处理回退事件
     *
     * @return
     */
    @Override
    public boolean onBackPressedSupport() {
        return closeApp();
    }

    /**
     * eventbus监听
     *
     * @param result
     */
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.WALLETUPDATE.ordinal()) {
            //切换钱包
            Wallet temp = (Wallet) result.getObj();
            if (!wallet.getWalletId().equals(temp.getWalletId())) {
                wallet = temp;
                getMyDID().init(wallet.getWalletId());//初始化mydid
                setWalletViewNew(wallet);
                setRecycleView();
                if (dataMap != null && dataMap.size() > 0) {
                    dataMap.clear();
                }
            }

        }
        if (integer == RxEnum.ONE.ordinal()) {
            //创建钱包
            Wallet temp = (Wallet) result.getObj();
            if (!wallet.getWalletId().equals(temp.getWalletId())) {
                wallet = temp;
                setWalletViewNew(wallet);
            }

        }
        if (integer == RxEnum.UPDATA_WALLET_NAME.ordinal()) {
            String walletId = (String) result.getObj();
            if (wallet.getWalletId().equals(walletId)) {
                wallet.setWalletName(result.getName());
                tvTitle.setText(result.getName());
            }
        }
        if (integer == RxEnum.DELETE.ordinal()) {
            //删除
            String walletId = (String) result.getObj();
            listMap.remove(walletId);
            if (wallet.getWalletId().equals(walletId)) {
                wallet = realmUtil.queryDefauleWallet();
                getMyDID().init(wallet.getWalletId());//初始化mydid
                setWalletViewNew(wallet);
                setRecycleView();
            }
        }
        if (integer == RxEnum.UPDATAPROPERTY.ordinal()) {
            //子钱包改变  创建或删除
            setWalletViewNew(wallet);
        }
        if (integer == RxEnum.UPDATAPROGRESS.ordinal()) {
            //progress改变
            setRecycleView();
        }
        if (integer == RxEnum.BALANCECHANGE.ordinal()) {
            //资产改变
            setRecycleView();
        }
        if (integer == RxEnum.TRANSFERSUCESS.ordinal()) {
            //交易发生出去 首页收到不会弹成功的提示 只为了消息记录
            //TRANSACTIONSUCCESSMESSAGE弹出提示
            transactionMap.put((String) result.getObj(), result.getName());
            restoreScanData();
        }
        if (integer == RxEnum.VERTIFYPAYPASS.ordinal()) {
            //验证密码成功
            payPasswd = (String) result.getObj();
            initDid();
        }
        if (integer == RxEnum.JUSTSHOWFEE.ordinal()) {
            //展示手续费后  再去验证密码
            if (getClass().getSimpleName().equals(result.getName())) {
                proposalPresenter.toVertifyPwdActivity(wallet, this);
            }
        }
        if (integer == RxEnum.SCANDATATOASSETPAGE.ordinal()) {
            //其他页面接收的二维码数据
            restoreScanData();
            scanResult = (String) result.getObj();
            scanElastos(scanResult);

        }
        if (integer == RxEnum.TRANSACTIONSUCCESSMESSAGE.ordinal()) {
            //只为通知特定页面加的补丁 弹提示框
            if (getClass().getSimpleName().equals(result.getName())) {
                showTransSucessTip(null);
            }
        }
        if (integer == RxEnum.VOTETRANSFERACTIVITY.ordinal()) {
            if (getClass().getSimpleName().equals(result.getName())) {
                voteNum = (String) result.getObj();
                //点击下一步 获得金额后
                if (proposalDetailPresenter == null) {
                    proposalDetailPresenter = new ProposalDetailPresenter();
                }
                proposalDetailPresenter.getVoteInfo(wallet.getWalletId(), "", this);
            }
        }

    }

    private void initDid() {
        try {
            DIDStore store = getMyDID().getDidStore();
            if (store.containsPrivateIdentity()) {
                getDigset();
            } else {
                //获得私钥用于初始化did
                new CreatMulWalletPresenter().exportxPrivateKey(wallet.getWalletId(), payPasswd, this);
            }
        } catch (DIDException e) {
            e.printStackTrace();
        }

    }

    private void getDigset() {
        getMyDID().initDID(payPasswd);
        if (curentJwtEntity instanceof RecieveReviewJwtEntity) {
            getReviewDigest();
        }
        if (curentJwtEntity instanceof RecieveProcessJwtEntity) {
            getProcessDigest();
        }
        if (curentJwtEntity instanceof RecieveWithdrawJwtEntity) {
            getWithDrawDigest();
        }

    }

    private void getReviewDigest() {

        RecieveReviewJwtEntity entity = (RecieveReviewJwtEntity) curentJwtEntity;
        ProposalReviewPayLoad payLoad = converReviewPayLoad(entity.getData());
        proposalPresenter.proposalReviewDigest(wallet.getWalletId(), new Gson().toJson(payLoad), this, payLoad);
    }

    private void getWithDrawDigest() {

        RecieveWithdrawJwtEntity entity = (RecieveWithdrawJwtEntity) curentJwtEntity;
        ProposalWithdrawPayLoad payLoad = converWithDrawPayLoad(entity.getData());
        proposalPresenter.proposalWithdrawDigest(wallet.getWalletId(), new Gson().toJson(payLoad), this, payLoad);
    }

    private void getProcessDigest() {


        RecieveProcessJwtEntity entity = (RecieveProcessJwtEntity) curentJwtEntity;
        ProposalProcessPayLoad payLoad = converRrocessPayLoad(entity.getData());

        if ("reviewmilestone".equals(entity.getCommand())) {
            payLoad.setOwnerSignature(entity.getData().getOwnersignature());
            payLoad.setNewOwnerSignature(entity.getData().getNewownersignature() == null ? "" : entity.getData().getNewownersignature());
            switch (entity.getData().getProposaltrackingtype().toLowerCase()) {
                case "common":
                    payLoad.setType(0);
                    break;
                case "progress":
                    payLoad.setType(1);
                    break;
                case "rejected":
                    payLoad.setType(2);
                    break;
                case "terminated":
                    payLoad.setType(3);
                    break;
                case "changeowner":
                    payLoad.setType(4);
                    break;
                case "finalized":
                    payLoad.setType(5);
                    break;
            }

            payLoad.setSecretaryGeneralOpinionHash(entity.getData().getSecretaryopinionhash());
            Log.i("///", new Gson().toJson(payLoad));
            proposalPresenter.proposalTrackingSecretaryDigest(wallet.getWalletId(), new Gson().toJson(payLoad), this, payLoad);

        } else if ("updatemilestone".equals(entity.getCommand())) {
            proposalPresenter.proposalTrackingOwnerDigest(wallet.getWalletId(), new Gson().toJson(payLoad), this, payLoad);
        }
    }

    @Override
    public void onDestroy() {
        //存储所有同步状态
        realmUtil.updateSubWalletDetial(listMap);
        CacheUtil.setUnReadMessage(messageList);
        super.onDestroy();
    }


    private int currentType = -1;
    private Map<Integer, String> dataMap;//用于存储二维码信息

    private String getData(QrBean qrBean, int type) {
        if (dataMap == null) {
            dataMap = new TreeMap<>();
        }
        if (type != currentType) {
            currentType = type;
            dataMap.clear();
        }

        String mydata = qrBean.getData();
        int max = qrBean.getTotal();
        int current = qrBean.getIndex();
        dataMap.put(current, mydata);
        if (dataMap.size() == max) {
            StringBuilder signData = new StringBuilder();
            for (String s : dataMap.values()) {
                signData.append(s);
            }
            currentType = -1;
            dataMap.clear();
            return signData.toString();
        }
        String msg = String.format(getContext().getString(R.string.scanprocess), dataMap.size() + "/" + max);
        showToast(msg);
        requstManifestPermission(getString(R.string.needpermission));

        return null;
    }

    private void toErroScan(String result) {
        restoreScanData();
        if (dataMap != null) {
            dataMap.clear();
        }
        currentType = -1;
        Bundle bundle = new Bundle();
        bundle.putString("result", result);
        ((BaseFragment) getParentFragment()).start(ErrorScanFragment.class, bundle);
    }

    private void diposeElastosCaode(int analyzeElastosData, String[] parts) {
        Bundle bundle = new Bundle();
        switch (analyzeElastosData) {
            case 0:
                toErroScan("elastos:" + parts[0] + (parts.length == 1 ? "" : ("?" + parts[1])));
                break;
            case 2:
                bundle.putString("amount", parts[1].replace("amount=", ""));
            case 1:
                bundle.putParcelable("wallet", wallet);
                bundle.putString("ChainID", MyWallet.ELA);
                bundle.putString("address", parts[0]);
                ((BaseFragment) getParentFragment()).start(TransferFragment.class, bundle);
                break;
        }
    }

    private void addToMessageCenter(String hash, String transferType, String chainId, String walleName, String reason) {
        if (messageList == null) {
            messageList = new ArrayList<>();
        }
        MessageEntity messageEntity = new MessageEntity();
        messageEntity.setReason(reason);
        messageEntity.setTransferType(transferType);
        messageEntity.setWalletName(walleName);
        messageEntity.setTime(new Date().getTime() / 1000);
        messageEntity.setHash(hash);
        messageEntity.setChainId(chainId);
        messageList.remove(messageEntity);
        messageList.add(messageEntity);
        post(RxEnum.NOTICE.ordinal(), null, null);
    }

    private void showNotification(String reason, String transferTypeDes, String walleName) {
        SPUtil sp = new SPUtil(getContext());
        if (!sp.isOpenSendMsg()) {
            return;
        }
        Intent intent = new Intent(getContext(),
                MainActivity.class);//代表fragment所绑定的activity，这个需要写全路径
        intent.putExtra("toValue", "notice");//传递参数，然后根据参数进行判断需要跳转的fragment界面

        PendingIntent pIntent = PendingIntent.getActivity(getContext(), 0, intent,
                PendingIntent.FLAG_UPDATE_CURRENT);
        NotificationManager manager = (NotificationManager) getContext().getSystemService(NOTIFICATION_SERVICE);
        //需添加的代码
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            String channelId = "default";
            String channelName = "默认通知";
            manager.createNotificationChannel(new NotificationChannel(channelId, channelName, NotificationManager.IMPORTANCE_HIGH));
        }
        Notification notification = new NotificationCompat.Builder(getContext(), "default")
                .setContentTitle(MyUtil.getAppName(getContext()))
                .setContentText("【" + walleName + getString(R.string.wallet) + "】" + transferTypeDes + " - " + getString(R.string.transactionfinish) + ", " + reason + ".")
                .setWhen(System.currentTimeMillis())
                .setContentIntent(pIntent)
                .setAutoCancel(true)
                .setSmallIcon(R.mipmap.icon_ela)
                .setLargeIcon(BitmapFactory.decodeResource(getResources(), R.mipmap.icon_ela))
                .build();
        manager.notify((int) System.currentTimeMillis(), notification);
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getVoteInfo":
                //剔除非公示期的
                String voteInfo = ((CommmonStringEntity) baseEntity).getData();
                JSONArray otherUnActiveVote = proposalDetailPresenter.conversUnactiveVote("CRCProposal", voteInfo, depositList, crList, searchBeanList, councilList);
                try {
                    JSONObject voteJson = proposalDetailPresenter.conversVote(voteInfo, "CRCProposal");//key value
                    //点击下一步 获得上次的投票后筛选数据
                    String amount = Arith.mulRemoveZero(voteNum, MyWallet.RATE_S).toPlainString();
                    JSONObject newVotes = proposalDetailPresenter.getPublishDataFromLastVote(voteJson, amount, searchBeanList);
                    newVotes.put(((RecievePublishedVoteJwtEntity) curentJwtEntity).getData().getProposalhash(), amount);
                    proposalDetailPresenter.createVoteCRCProposalTransaction(wallet.getWalletId(), newVotes.toString(), otherUnActiveVote.toString(), this);
                } catch (Exception e) {
                    e.printStackTrace();
                }
                break;
            case "createVoteCRCProposalTransaction":
                //签名发交易
                goTransferActivity(((CommmonStringEntity) baseEntity).getData());
                break;
            case "proposalSearch":
                searchBeanList = ((ProposalSearchEntity) baseEntity).getData().getList();
                break;
            case "getCouncilList":
                councilList = ((CtListBean) baseEntity).getData().getCouncil();
                break;
            case "getCRlist":
                crList = ((CRListBean) baseEntity).getData().getResult().getCrcandidatesinfo();

                break;
            case "getDepositVoteList":
                depositList = ((VoteListBean) baseEntity).getData().getResult().getProducers();
                break;
            case "getBalance":

                BalanceEntity balanceEntity = (BalanceEntity) ((CommmonObjEntity) baseEntity).getData();
                List<org.elastos.wallet.ela.db.table.SubWallet> assetList = listMap.get(balanceEntity.getMasterWalletId());
                for (org.elastos.wallet.ela.db.table.SubWallet assetsItemEntity : assetList) {
                    if (assetsItemEntity.getChainId().equals(balanceEntity.getChainId())) {
                        assetsItemEntity.setBalance(balanceEntity.getBalance());
                        if (wallet.getWalletId().equals(balanceEntity.getMasterWalletId())) {
                            post(RxEnum.BALANCECHANGE.ordinal(), null, assetsItemEntity);
                        }
                    }

                }
                toVoteActivity(balanceEntity);
                break;
            case "getCurrentCouncilInfo":
                CtDetailBean ctDetailBean = (CtDetailBean) baseEntity;
                switch (curentJwtEntity.getCommand().toLowerCase()) {
                    case "reviewmilestone":
                        if ("SecretaryGeneral".equals(ctDetailBean.getData().getType())) {
                            proposalPresenter.showFeePage(wallet, Constant.PROPOSALSECRET, 39, this, ((RecieveProcessJwtEntity) curentJwtEntity).getData());
                        } else {
                            restoreScanData();
                            showToast(getString(R.string.didnotsame));
                        }
                        break;
                    case "createproposal":
                        if ("CouncilMember".equals(ctDetailBean.getData().getType())) {
                            //目前只有建议转提案调用他
                            toSuggest(curentJwtEntity.getCommand());
                        } else {
                            restoreScanData();
                            showToast(getString(R.string.didnotsame));
                        }
                        break;
                }


                break;
            case "postData":
                String command = curentJwtEntity.getCommand();
                String des = "";
                if ("createsuggestion".equals(command) || "updatemilestone".equals(command)) {
                    des = getString(R.string.signsendsuccess);

                }
                showTransSucessTip(des);
                break;
            case "newPublishTransaction":
                String hash = "";
                int transfertype = -1;
                try {
                    JSONObject pulishdata = new JSONObject(((CommmonStringWithiMethNameEntity) baseEntity).getData());
                    hash = pulishdata.getString("TxHash");
                    //String callBacktype = "";

                    if ("reviewproposal".equals(curentJwtEntity.getCommand())) {
                        transfertype = 38;
                        //  callBacktype = "txidproposalreview";
                        //proposalPresenter.backProposalJwt(callBacktype, scanResult, hash, payPasswd, this);
                        showTransSucessTip(null);

                    } else if ("reviewmilestone".equals(curentJwtEntity.getCommand())) {
                        transfertype = 39;
                        showTransSucessTip(null);
                        //callBacktype = "txid";
                        //proposalPresenter.backProposalJwt(callBacktype, scanResult, hash, payPasswd, this);
                    } else if ("withdraw".equals(curentJwtEntity.getCommand())) {
                        transfertype = 41;
                        showTransSucessTip(null);
                    }
                    post(RxEnum.TRANSFERSUCESS.ordinal(), transfertype + "", hash);
                } catch (JSONException e) {
                    e.printStackTrace();
                }

                break;
            case "signTransaction":
            case "createProposalWithdrawTransaction":
                new PwdPresenter().newPublishTransaction(wallet.getWalletId(), MyWallet.ELA, ((CommmonStringEntity) baseEntity).getData(), this);
                break;
            case "createProposalTrackingTransaction":
            case "createProposalReviewTransaction":
                new PwdPresenter().signTransaction(wallet.getWalletId(), MyWallet.ELA, ((CommmonStringEntity) baseEntity).getData(), payPasswd, this);
                break;
            case "proposalTrackingOwnerDigest":
                String signDigest1 = proposalPresenter.getSignDigist(payPasswd, ((CommmonStringEntity) baseEntity).getData(), this);
                proposalPresenter.backProposalJwt("signature", scanResult, signDigest1, payPasswd, this);

                break;
            case "proposalTrackingSecretaryDigest":
                String signDigest2 = proposalPresenter.getSignDigist(payPasswd, ((CommmonStringEntity) baseEntity).getData(), this);
                ProposalProcessPayLoad processPayLoad = (ProposalProcessPayLoad) o;
                processPayLoad.setSecretaryGeneralSignature(signDigest2);
                proposalPresenter.createProposalTrackingTransaction(wallet.getWalletId(), new Gson().toJson(processPayLoad), this);


                break;
            case "proposalReviewDigest":
                String signDigest = proposalPresenter.getSignDigist(payPasswd, ((CommmonStringEntity) baseEntity).getData(), this);
                ProposalReviewPayLoad proposalReviewPayLoad = (ProposalReviewPayLoad) o;
                proposalReviewPayLoad.setSignature(signDigest);
                proposalPresenter.createProposalReviewTransaction(wallet.getWalletId(), new Gson().toJson(proposalReviewPayLoad), this);
                break;
            case "proposalWithdrawDigest":
                String signDigest3 = proposalPresenter.getSignDigist(payPasswd, ((CommmonStringEntity) baseEntity).getData(), this);
                ProposalWithdrawPayLoad withdrawPayLoad = (ProposalWithdrawPayLoad) o;
                withdrawPayLoad.setSignature(signDigest3);
                RecieveWithdrawJwtEntity.DataBean dataBean = ((RecieveWithdrawJwtEntity) curentJwtEntity).getData();
                JSONArray utxos = converWithDrawPayLoadUtxo(dataBean.getUtxos());
                proposalPresenter.createProposalWithdrawTransaction(wallet.getWalletId(), dataBean.getRecipient(), dataBean.getAmount(), utxos.toString(), new Gson().toJson(withdrawPayLoad), this);
                break;
            case "forceDIDResolve":
                //未传递paypas网站提供的did验签
                verifyDID((DIDDocument) ((CommmonObjEntity) baseEntity).getData(), (String) o);
                break;
            case "DIDResolveWithTip":
                curentHasDID((DIDDocument) ((CommmonObjEntity) baseEntity).getData(), (String) o);
                break;
            case "exportxPrivateKey":
                String privateKey = ((CommmonStringEntity) baseEntity).getData();
                try {
                    getMyDID().getDidStore().initPrivateIdentity(privateKey, payPasswd);
                    getDigset();
                } catch (DIDException e) {
                    e.printStackTrace();
                    showToast(getString(R.string.didinitfaile));
                }
                break;
        }
    }

    private void showTransSucessTip(String des) {
        new DialogUtil().showTransferSucess(des, getBaseActivity(), new WarmPromptListener() {
            @Override
            public void affireBtnClick(View view) {
                restoreScanData();
            }
        });
    }

    private void goTransferActivity(String attributesJson) {
        Intent intent = new Intent(getActivity(), TransferActivity.class);
        intent.putExtra("amount", voteNum);
        intent.putExtra("wallet", wallet);
        intent.putExtra("openType", getClass().getSimpleName());
        intent.putExtra("chainId", MyWallet.ELA);
        intent.putExtra("attributes", attributesJson);
        intent.putExtra("type", Constant.PROPOSALPUBLISHED);
        intent.putExtra("extra", ((RecievePublishedVoteJwtEntity) curentJwtEntity).getData().getProposalhash());
        intent.putExtra("transType", 1004);
        startActivity(intent);
    }

    private void verifyDID(DIDDocument didDocument, String tag) {
        String result = scanResult.replace(tag, "");
        if (JwtUtils.verifyJwt(result, didDocument)) {
            String name = getMyDID().getName(didDocument);
            new WalletManagePresenter().DIDResolveWithTip(wallet.getDid(), this, name);
            return;
        }
        //验签失败
        toErroScan(scanResult);

    }

    private void curentHasDID(DIDDocument didDocument, String name) {
        if (didDocument == null) {
            showToast(getString(R.string.notcreatedid));
            return;
        }
        if (getMyDID().getExpires(didDocument).before(new Date())) {
            //did过期
            showToast(getString(R.string.didoutofdate));
            return;

        }
        try {
            getMyDID().getDidStore().storeDid(didDocument);//存储本地
        } catch (DIDStoreException e) {
            e.printStackTrace();
        }
        if (scanResult.startsWith("elastos://credaccess/")) {
            //did
            toAuthorization(name);

        } else if (scanResult.startsWith("elastos://crproposal/")) {
            //社区提案的二维码
            String result = scanResult.replace("elastos://crproposal/", "");
            String payload = JwtUtils.getJwtPayload(result);
            RecieveProposalFatherJwtEntity entity = JSON.parseObject(payload, RecieveProposalFatherJwtEntity.class);
            String command = entity.getCommand().toLowerCase();
            if (proposalPresenter == null) {
                proposalPresenter = new ProposalPresenter();
            }
            switch (command) {
                case "voteforproposal"://voteforproposal
                    //公示期谁都可以投票
                    curentJwtEntity = JSON.parseObject(payload, RecievePublishedVoteJwtEntity.class);
                    proposalPresenter.proposalSearch(-1, -1, "ALL", null, this);
                    new VoteListPresenter().getDepositVoteList("1", "all", this, false);
                    new CRlistPresenter().getCRlist(-1, -1, "all", this, false);
                    new CtListPresenter().getCouncilList(this, String.valueOf(1));
                    commonGetBalancePresenter.getBalance(wallet.getWalletId(), MyWallet.ELA, this);


                    break;
                case "withdraw":
                    //提现  判断是否本人
                    curentJwtEntity = JSON.parseObject(payload, RecieveWithdrawJwtEntity.class);
                    RecieveWithdrawJwtEntity.DataBean procesData2 = ((RecieveWithdrawJwtEntity) curentJwtEntity).getData();

                    if (procesData2.getOwnerpublickey().equals(getMyDID().getDidPublicKey(didDocument))) {
                        proposalPresenter.showFeePage(wallet, Constant.PROPOSALWITHDRAW, 41, this, procesData2);
                    } else {
                        restoreScanData();
                        showToast(getString(R.string.didnotsame));
                    }
                    break;
                case "reviewmilestone":
                    //执行期  秘书长签名 发送交易
                    //判断身份  网站判断是秘书长  这里判断是本
                    curentJwtEntity = JSON.parseObject(payload, RecieveProcessJwtEntity.class);
                    proposalPresenter.getCurrentCouncilInfo(wallet.getDid().replace("did:elastos:", ""), this);
                    break;
                case "updatemilestone":
                    //执行期  委员自己执行反馈 网站确认是委员  这里确认本人 只签名
                    curentJwtEntity = JSON.parseObject(payload, RecieveProcessJwtEntity.class);
                    RecieveProcessJwtEntity.DataBean procesData1 = ((RecieveProcessJwtEntity) curentJwtEntity).getData();
                    if (getMyDID().getDidPublicKey(didDocument).equals(procesData1.getOwnerpubkey())) {
                        proposalPresenter.showFeePage(wallet, Constant.PROPOSALPROCESS, 39, this, procesData1);
                    } else {
                        restoreScanData();
                        showToast(getString(R.string.didnotsame));
                    }
                    break;
                case "createsuggestion":
                    //发建议 任何人 本人 只签名
                    curentJwtEntity = JSON.parseObject(payload, RecieveProposalJwtEntity.class);
                    RecieveProposalJwtEntity.DataBean suggestData = ((RecieveProposalJwtEntity) curentJwtEntity).getData();
                    if (suggestData.getOwnerpublickey().equals(getMyDID().getDidPublicKey(didDocument)))
                        toSuggest(command);
                    else {
                        restoreScanData();
                        showToast(getString(R.string.didnotsame));
                    }
                    break;
                case "createproposal":
                    //把建议->提案 判断身份  任意委员但是要当前网站登陆者对用用户 发交易
                    curentJwtEntity = JSON.parseObject(payload, RecieveProposalJwtEntity.class);
                    RecieveProposalJwtEntity.DataBean suggestData1 = ((RecieveProposalJwtEntity) curentJwtEntity).getData();
                    if (suggestData1.getDid().equals(wallet.getDid()))
                        proposalPresenter.getCurrentCouncilInfo(wallet.getDid().replace("did:elastos:", ""), this);
                    else {
                        restoreScanData();
                        showToast(getString(R.string.didnotsame));
                    }
                    break;

                case "reviewproposal":
                    //评议期  身份必须是本委员(委员身份网站已经确认   本人) 发交易
                    curentJwtEntity = JSON.parseObject(payload, RecieveReviewJwtEntity.class);
                    RecieveReviewJwtEntity.DataBean reviewData = ((RecieveReviewJwtEntity) curentJwtEntity).getData();
                    if (reviewData.getDID().equals(wallet.getDid())) {
                        proposalPresenter.showFeePage(wallet, Constant.PROPOSALREVIEW, 38, this, reviewData);
                    } else {
                        restoreScanData();
                        showToast(getString(R.string.didnotsame));
                    }

                    break;
            }

        }


    }

    private JSONArray converWithDrawPayLoadUtxo(List<RecieveWithdrawJwtEntity.DataBean.UtxosBean> lists) {
        JSONArray utxos = new JSONArray();
        for (RecieveWithdrawJwtEntity.DataBean.UtxosBean bean : lists) {


            try {
                JSONObject utxo = new JSONObject();
                utxo.put("Hash", bean.getTxid());
                utxo.put("Index", bean.getVout());
                utxo.put("Amount", bean.getAmount());
                utxos.put(utxo);
            } catch (JSONException e) {
                e.printStackTrace();
            }
        }

        return utxos;
    }

    private ProposalWithdrawPayLoad converWithDrawPayLoad(RecieveWithdrawJwtEntity.DataBean origin) {
        ProposalWithdrawPayLoad payload = new ProposalWithdrawPayLoad();
        payload.setProposalHash(origin.getProposalhash());
        payload.setOwnerPublicKey(origin.getOwnerpublickey());
        return payload;
    }

    private ProposalReviewPayLoad converReviewPayLoad(RecieveReviewJwtEntity.DataBean origin) {
        ProposalReviewPayLoad payload = new ProposalReviewPayLoad();
        payload.setProposalHash(origin.getProposalhash());
        switch (origin.getVoteresult().toLowerCase()) {
            case "approve":
                payload.setVoteResult(0);
                break;
            case "reject":
                payload.setVoteResult(1);
                break;
            case "abstain":
                payload.setVoteResult(2);
                break;
        }
        payload.setOpinionHash(origin.getOpinionhash());
        payload.setDID(origin.getDID().replace("did:elastos:", ""));
        return payload;
    }

    private ProposalProcessPayLoad converRrocessPayLoad(RecieveProcessJwtEntity.DataBean origin) {
        ProposalProcessPayLoad payload = new ProposalProcessPayLoad();
        payload.setProposalHash(origin.getProposalhash());
        payload.setMessageHash(origin.getMessagehash());
        payload.setStage(origin.getStage());
        payload.setOwnerPublicKey(origin.getOwnerpubkey());
        payload.setNewOwnerPublicKey(origin.getNewownerpubkey() == null ? "" : origin.getNewownerpubkey());
        return payload;
    }

    private void toSuggest(String command) {
        restoreScanData();
        Bundle bundle = new Bundle();
        bundle.putParcelable("wallet", wallet);
        bundle.putString("command", command);
        bundle.putString("scanResult", scanResult);
        ((BaseFragment) getParentFragment()).start(SuggestionsInfoFragment.class, bundle);
    }

    private void toAuthorization(String webName) {
        restoreScanData();
        Bundle bundle = new Bundle();
        bundle.putString("scanResult", scanResult);
        bundle.putParcelable("wallet", wallet);
        bundle.putString("webName", webName);// 网站did的name
        ((BaseFragment) getParentFragment()).start(AuthorizationFragment.class, bundle);
    }

    @Override
    public void onGetCommonData(String methodname, String data) {

    }

    private void restoreScanData() {
        payPasswd = null;
        curentJwtEntity = null;
        crList = null;
        searchBeanList = null;
        depositList = null;
        councilList = null;
        voteNum = null;
    }
}
