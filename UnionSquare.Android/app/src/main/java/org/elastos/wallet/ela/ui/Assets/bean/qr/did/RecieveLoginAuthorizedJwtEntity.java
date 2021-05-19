/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package org.elastos.wallet.ela.ui.Assets.bean.qr.did;

import org.elastos.wallet.ela.ui.Assets.bean.qr.RecieveJwtEntity;

public class RecieveLoginAuthorizedJwtEntity extends RecieveJwtEntity {
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


    private String userId;

    private ClaimsBean claims;


    public String getUserId() {
        return userId;
    }

    public void setUserId(String userId) {
        this.userId = userId;
    }


    public ClaimsBean getClaims() {
        return claims;
    }

    public void setClaims(ClaimsBean claims) {
        this.claims = claims;
    }


    public static class ClaimsBean {
    }
}
