package org.elastos.wallet.ela.ui.Assets.bean;

public class RecorderAddressEntity {
    private String address;
    private String amount;

    public RecorderAddressEntity(String address, String amount) {
        this.address = address;
        this.amount = amount;
    }

    public String getAddress() {
        return address;
    }

    public void setAddress(String address) {
        this.address = address;
    }

    public String getAmount() {
        return amount;
    }

    public void setAmount(String amount) {
        this.amount = amount;
    }

    @Override
    public String toString() {
        return "RecorderAddressEntity{" +
                "address='" + address + '\'' +
                ", amount='" + amount + '\'' +
                '}';
    }
}
