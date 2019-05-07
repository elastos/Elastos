package org.elastos.wallet.ela.ui.vote.bean;

import java.io.Serializable;

public class VoteBean implements Serializable {

    private String etdotname;
    private String publickey;
    private String net;
    private String area;

    public String getWalletkey() {
        return walletkey;
    }

    public void setWalletkey(String walletkey) {
        this.walletkey = walletkey;
    }

    private String url;
    private String walletkey;

    public String getEtdotname() {
        return etdotname;
    }

    public void setEtdotname(String etdotname) {
        this.etdotname = etdotname;
    }

    public String getPublickey() {
        return publickey;
    }

    public void setPublickey(String publickey) {
        this.publickey = publickey;
    }

    public String getNet() {
        return net;
    }

    public void setNet(String net) {
        this.net = net;
    }

    public String getArea() {
        return area;
    }

    public void setArea(String area) {
        this.area = area;
    }

    public String getUrl() {
        return url;
    }

    public void setUrl(String url) {
        this.url = url;
    }
}
