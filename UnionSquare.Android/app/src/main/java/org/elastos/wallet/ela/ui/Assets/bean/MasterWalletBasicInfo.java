package org.elastos.wallet.ela.ui.Assets.bean;

public class MasterWalletBasicInfo {

    /**
     * M : 1
     * N : 1
     * Readonly : false
     * SingleAddress : false
     * Type : Standard
     */

    private int M;
    private int N;
    private boolean Readonly;
    private boolean SingleAddress;
    private String Type;

    public int getM() {
        return M;
    }

    public void setM(int M) {
        this.M = M;
    }

    public int getN() {
        return N;
    }

    public void setN(int N) {
        this.N = N;
    }

    public boolean isReadonly() {
        return Readonly;
    }

    public void setReadonly(boolean Readonly) {
        this.Readonly = Readonly;
    }

    public boolean isSingleAddress() {
        return SingleAddress;
    }

    public void setSingleAddress(boolean SingleAddress) {
        this.SingleAddress = SingleAddress;
    }

    public String getType() {
        return Type;
    }

    public void setType(String Type) {
        this.Type = Type;
    }
}
