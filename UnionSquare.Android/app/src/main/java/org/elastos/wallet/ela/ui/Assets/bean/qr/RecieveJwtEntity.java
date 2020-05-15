package org.elastos.wallet.ela.ui.Assets.bean.qr;

/**
 * 所有网站二维码的父类
 */
public class RecieveJwtEntity {
    private String iss;
    private String callbackurl;
    private long iat;
    private long exp;
    private WebsiteBean website;


    public String getIss() {
        return iss;
    }

    public void setIss(String iss) {
        this.iss = iss;
    }

    public String getCallbackurl() {
        return callbackurl;
    }

    public void setCallbackurl(String callbackurl) {
        this.callbackurl = callbackurl;
    }

    public long getIat() {
        return iat;
    }

    public void setIat(long iat) {
        this.iat = iat;
    }

    public long getExp() {
        return exp;
    }

    public void setExp(long exp) {
        this.exp = exp;
    }

    public WebsiteBean getWebsite() {
        return website;
    }

    public void setWebsite(WebsiteBean website) {
        this.website = website;
    }

    public static class WebsiteBean {
        /**
         * domain : https://staging.cyberrepublic.org
         * logo : https://staging.cyberrepublic.org/assets/images/logo.svg
         */

        private String domain;
        private String logo;

        public String getDomain() {
            return domain;
        }

        public void setDomain(String domain) {
            this.domain = domain;
        }

        public String getLogo() {
            return logo;
        }

        public void setLogo(String logo) {
            this.logo = logo;
        }

    }

    public RecieveJwtEntity() {
    }

}
