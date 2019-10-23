package org.elastos.wallet.ela.ui.did.entity;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.util.List;

public class AllPkEntity extends BaseEntity {
    /**
     * MaxCount : 110
     * PublicKeys : ["03a3a59e3bc3ddd9a048119d5cee1e8266484e1c1b7c5d529f5d7681066066495f"]
     */

    private int MaxCount;
    private List<String> PublicKeys;

    public int getMaxCount() {
        return MaxCount;
    }

    public void setMaxCount(int MaxCount) {
        this.MaxCount = MaxCount;
    }

    public List<String> getPublicKeys() {
        return PublicKeys;
    }

    public void setPublicKeys(List<String> PublicKeys) {
        this.PublicKeys = PublicKeys;
    }
}
