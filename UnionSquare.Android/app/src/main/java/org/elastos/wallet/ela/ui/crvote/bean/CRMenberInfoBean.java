package org.elastos.wallet.ela.ui.crvote.bean;

public class CRMenberInfoBean {

    /**
     * CROwnerPublicKey : 02775B47CCB0808BA70EA16800385DBA2737FDA090BB0EBAE948DD16FF658CA74D
     * CROwnerDID : 02848A8F1880408C4186ED31768331BC9296E1B0C3EC7AE6F11E9069B16013A9C5
     * NickName : hello nickname
     * URL : www.google.com
     * Location : 86
     */

    private String CROwnerPublicKey;
    private String CROwnerDID;
    private String NickName;
    private String URL;
    private int Location;

    public String getCROwnerPublicKey() {
        return CROwnerPublicKey;
    }

    public void setCROwnerPublicKey(String CROwnerPublicKey) {
        this.CROwnerPublicKey = CROwnerPublicKey;
    }

    public String getCROwnerDID() {
        return CROwnerDID;
    }

    public void setCROwnerDID(String CROwnerDID) {
        this.CROwnerDID = CROwnerDID;
    }

    public String getNickName() {
        return NickName;
    }

    public void setNickName(String NickName) {
        this.NickName = NickName;
    }

    public String getURL() {
        return URL;
    }

    public void setURL(String URL) {
        this.URL = URL;
    }

    public int getLocation() {
        return Location;
    }

    public void setLocation(int Location) {
        this.Location = Location;
    }
}
