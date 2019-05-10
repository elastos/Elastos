package org.elastos.wallet.ela.ui.common.bean;

import org.elastos.wallet.core.SubWallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.util.List;

public class ISubWalletListEntity extends BaseEntity {



    private List<SubWallet> data;


    public ISubWalletListEntity(String code, List<SubWallet> data) {
        this.setCode(code);
        this.data = data;
    }
    public List<SubWallet> getData() {
        return data;
    }

    public void setData(List<SubWallet> data) {
        this.data = data;
    }
}
