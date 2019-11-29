package org.elastos.wallet.ela.ui.Assets.presenter;

import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.ObservableListener;
import org.elastos.wallet.ela.rxjavahelp.PresenterAbstract;
import org.elastos.wallet.ela.ui.common.bean.CommmonBooleanEntity;
import org.elastos.wallet.ela.ui.common.listener.CommonBooleanListener;
import org.elastos.wallet.ela.ui.common.listener.CommonStringListner;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class TransferPresenter extends PresenterAbstract {


    public void isAddressValid(String walletId, String addr, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonBooleanListener.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().isAddressValid(walletId, addr);
            }
        });
        subscriberObservable(observer, observable);
    }


    public void createTransaction(String walletId, String chainId, String s, String address, String amount, String memo, boolean useVotedUTXO, BaseFragment baseFragment) {
        Observer observer = createObserver(CommonStringListner.class, baseFragment);
        Observable observable = createObservable(new ObservableListener() {
            @Override
            public BaseEntity subscribe() {
                return baseFragment.getMyWallet().createTransaction(walletId, chainId, s, address, amount, memo, useVotedUTXO);
            }
        });
        subscriberObservable(observer, observable);
    }

    /**
     * @param walletId
     * @param baseFragment
     * @return -1  非elastos:数据 ,diposeElastosData处理elastos数据的结果
     */
    public int analyzeElastosData(String[] parts, String walletId, BaseFragment baseFragment) {

        //兼容elastos:EJQcgWDazveSy436TauPJ3R8PCYpifp6HA?amount=6666.00000000
        String address = parts[0];
        //判断 address是否符合
        BaseEntity baseEntity = baseFragment.getMyWallet().isAddressValid(walletId, address);
        return diposeElastosData(parts, baseEntity);
    }

    /**
     * @param parts EJQcgWDazveSy436TauPJ3R8PCYpifp6HA ? amount=6666.00000000
     * @return 0 数据格式不能识别 1只有address  part[0] 2 有address part[0] amount parts[1].replace("amount=", "")
     */
    private int diposeElastosData(String[] parts, BaseEntity baseEntity) {
        if (!baseEntity.getCode().equals(MyWallet.SUCCESSCODE)) {
            return 0;
        }
        CommmonBooleanEntity commmonBooleanEntity = (CommmonBooleanEntity) baseEntity;
        boolean data = commmonBooleanEntity.getData();
        if (!data) {
            return 0;
        }

        if (parts == null) {
            return 1;
        }
        String amount = null;
        if (parts.length > 1 && parts[1].contains("amount=")) {
            amount = parts[1].replace("amount=", "");
            try {
                Double.parseDouble(amount);
            } catch (Exception e) {
                return 0;
            }
        }
        if (amount == null) {
            return 1;
        } else {
            return 2;
        }

    }
}