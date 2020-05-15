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

package org.elastos.wallet.ela.ui.proposal.bean;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

public class SuggestBean extends BaseEntity {


    /**
     * code : 1
     * data : {"title":"test05141849","createdAt":1589453858,"id":"5ebd2422c4a7fe0078148b0c","abs":"这是一个测试建议11\n这是一个测试建议11这是一个测试建议11\n这是一个测试建议11这是一个测试建议11这是一个测试建议11\n这是一个测试建议11这是一个测试建议11这是一个测试建议11这是一个测试建议11\n这是一个测试建议11这是一个测试建议11这是一个测试建议11这是一个测试建议11这是一个测试建议11\n这是一个测试建议11这是一个测试建议11这是一个测试建议11这是一个测试建议11这是一个测试建议11这是一个测试建议11\n","address":"https://staging.cyberrepublic.org/suggestion/5ebd2422c4a7fe0078148b0c","didName":"undefined undefined"}
     */


    private DataBean data;

    public DataBean getData() {
        return data;
    }

    public void setData(DataBean data) {
        this.data = data;
    }

    public static class DataBean {
        /**
         * title : test05141849
         * createdAt : 1589453858
         * id : 5ebd2422c4a7fe0078148b0c
         * abs : 这是一个测试建议11
         * 这是一个测试建议11这是一个测试建议11
         * 这是一个测试建议11这是一个测试建议11这是一个测试建议11
         * 这是一个测试建议11这是一个测试建议11这是一个测试建议11这是一个测试建议11
         * 这是一个测试建议11这是一个测试建议11这是一个测试建议11这是一个测试建议11这是一个测试建议11
         * 这是一个测试建议11这是一个测试建议11这是一个测试建议11这是一个测试建议11这是一个测试建议11这是一个测试建议11
         * address : https://staging.cyberrepublic.org/suggestion/5ebd2422c4a7fe0078148b0c
         * didName : undefined undefined
         */

        private String title;
        private int createdAt;
        private String id;
        private String did;
        private String abs;
        private String address;
        private String didName;

        public String getDid() {
            return did;
        }

        public void setDid(String did) {
            this.did = did;
        }

        public String getTitle() {
            return title;
        }

        public void setTitle(String title) {
            this.title = title;
        }

        public int getCreatedAt() {
            return createdAt;
        }

        public void setCreatedAt(int createdAt) {
            this.createdAt = createdAt;
        }

        public String getId() {
            return id;
        }

        public void setId(String id) {
            this.id = id;
        }

        public String getAbs() {
            return abs;
        }

        public void setAbs(String abs) {
            this.abs = abs;
        }

        public String getAddress() {
            return address;
        }

        public void setAddress(String address) {
            this.address = address;
        }

        public String getDidName() {
            return didName;
        }

        public void setDidName(String didName) {
            this.didName = didName;
        }
    }
}
