package org.elastos.wallet.ela.ui.Assets.bean;

public class AssetsItemEntity {
    private int progress;
    private String syncRime;
    private String balance;
    private String chainId;
    private String wallletId;

    public int getProgress() {
        return progress;
    }

    public void setProgress(int progress) {
        this.progress = progress;
    }

    public String getSyncRime() {
        return syncRime;
    }

    public void setSyncRime(String syncRime) {
        this.syncRime = syncRime;
    }

    public String getBalance() {
        return balance;
    }

    public void setBalance(String balance) {
        this.balance = balance;
    }

    public String getChainId() {
        return chainId;
    }

    public void setChainId(String chainId) {
        this.chainId = chainId;
    }

    public String getWallletId() {
        return wallletId;
    }

    public void setWallletId(String wallletId) {
        this.wallletId = wallletId;
    }
}
