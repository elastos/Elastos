package org.elastos.wallet.ela.ui.vote.bean;

public class ElectoralAffairsBean {


    /**
     * Address : 172.31.32.36:25339
     * Location : 374
     * NickName : adr_us12
     * NodePublicKey : 022fca820ec28b4f69b4896ecc7b62b454a93e93b047df6a7c1599e6dcbeb5c16f
     * OwnerPublicKey : 022fca820ec28b4f69b4896ecc7b62b454a93e93b047df6a7c1599e6dcbeb5c16f
     * URL : www.adr_us1212.com
     */

    private String Address;
    private int Location;
    private String NickName;
    private String NodePublicKey;
    private String OwnerPublicKey;
    private String URL;

    public String getAddress() {
        return Address;
    }

    public void setAddress(String Address) {
        this.Address = Address;
    }

    public int getLocation() {
        return Location;
    }

    public void setLocation(int Location) {
        this.Location = Location;
    }

    public String getNickName() {
        return NickName;
    }

    public void setNickName(String NickName) {
        this.NickName = NickName;
    }

    public String getNodePublicKey() {
        return NodePublicKey;
    }

    public void setNodePublicKey(String NodePublicKey) {
        this.NodePublicKey = NodePublicKey;
    }

    public String getOwnerPublicKey() {
        return OwnerPublicKey;
    }

    public void setOwnerPublicKey(String OwnerPublicKey) {
        this.OwnerPublicKey = OwnerPublicKey;
    }

    public String getURL() {
        return URL;
    }

    public void setURL(String URL) {
        this.URL = URL;
    }
}
