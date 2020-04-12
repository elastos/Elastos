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
