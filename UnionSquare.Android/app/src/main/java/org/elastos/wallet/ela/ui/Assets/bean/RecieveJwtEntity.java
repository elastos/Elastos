package org.elastos.wallet.ela.ui.Assets.bean;

import android.os.Parcelable;

public class RecieveJwtEntity  {
    //{
    // "iss": "did:elastos:iYpQMwheDxySqivocSJaoprcoDTqQsDYAu",
    // "userId": "5e60cbc4d408a4606c1476de",
    // "callbackurl": "https://staging-api.cyberrepublic.org/api/user/did-callback-ela",
    // "claims": {},
    // "website": {
    //  "domain": "https://staging.cyberrepublic.org",
    //  "logo": "https://staging.cyberrepublic.org/assets/images/logo.svg"
    // },
    // "iat": 1583834166,
    // "exp": 1584438966
    //}

    /**
     * iss : did:elastos:iYpQMwheDxySqivocSJaoprcoDTqQsDYAu
     * userId : 5e60cbc4d408a4606c1476de
     * callbackurl : https://staging-api.cyberrepublic.org/api/user/did-callback-ela
     * claims : {}
     * website : {"domain":"https://staging.cyberrepublic.org","logo":"https://staging.cyberrepublic.org/assets/images/logo.svg"}
     * iat : 1583834166
     * exp : 1584438966
     */

    private String iss;
    private String userId;
    private String callbackurl;
    private ClaimsBean claims;
    private WebsiteBean website;
    private long iat;
    private long exp;

    public String getIss() {
        return iss;
    }

    public void setIss(String iss) {
        this.iss = iss;
    }

    public String getUserId() {
        return userId;
    }

    public void setUserId(String userId) {
        this.userId = userId;
    }

    public String getCallbackurl() {
        return callbackurl;
    }

    public void setCallbackurl(String callbackurl) {
        this.callbackurl = callbackurl;
    }

    public ClaimsBean getClaims() {
        return claims;
    }

    public void setClaims(ClaimsBean claims) {
        this.claims = claims;
    }

    public WebsiteBean getWebsite() {
        return website;
    }

    public void setWebsite(WebsiteBean website) {
        this.website = website;
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

    public static class ClaimsBean {
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
}
