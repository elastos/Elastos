package org.elastos.wallet.ela.ui.Assets.bean;

public class BalanceEntity {
    private String chainId;
    private String balance;
    private String masterWalletId;

    public BalanceEntity(String chainId, String balance,String masterWalletId) {
        this.chainId = chainId;
        this.balance = balance;
        this.masterWalletId = masterWalletId;
    }

    public String getMasterWalletId() {
        return masterWalletId;
    }

    public void setMasterWalletId(String masterWalletId) {
        this.masterWalletId = masterWalletId;
    }

    public String getChainId() {
        return chainId;
    }

    public void setChainId(String chainId) {
        this.chainId = chainId;
    }

    public String getBalance() {
        return balance;
    }

    public void setBalance(String balance) {
        this.balance = balance;
    }

    @Override
    public String toString() {
        return "BalanceEntity{" +
                "chainId='" + chainId + '\'' +
                ", balance='" + balance + '\'' +
                '}';
    }
}
