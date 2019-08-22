package org.elastos.wallet.ela.utils;

public class QrBean {

    /**
     * version : 0
     * name : 0
     * total : 1.0
     * index : 1.0
     * data : ads
     * md5 : 2deb000b57bfac9d72c14d4ed967b572
     * extra : {"Type":1,"SubWallet":"ELA"}
     */

    private int version;
    private int name;
    private int total;
    private int index;
    private String data;
    private String md5;
    private ExtraBean extra;

    public int getVersion() {
        return version;
    }

    public void setVersion(int version) {
        this.version = version;
    }

    public int getName() {
        return name;
    }

    public void setName(int name) {
        this.name = name;
    }

    public int getTotal() {
        return total;
    }

    public void setTotal(int total) {
        this.total = total;
    }

    public int getIndex() {
        return index;
    }

    public void setIndex(int index) {
        this.index = index;
    }

    public String getData() {
        return data;
    }

    public void setData(String data) {
        this.data = data;
    }

    public String getMd5() {
        return md5;
    }

    public void setMd5(String md5) {
        this.md5 = md5;
    }

    public ExtraBean getExtra() {
        return extra;
    }

    public void setExtra(ExtraBean extra) {
        this.extra = extra;
    }

    public static class ExtraBean {
        /**
         * Type : 1
         * SubWallet : ELA
         */

        private int Type;
        private String SubWallet;

        public int getType() {
            return Type;
        }

        public void setType(int Type) {
            this.Type = Type;
        }

        public String getSubWallet() {
            return SubWallet;
        }

        public void setSubWallet(String SubWallet) {
            this.SubWallet = SubWallet;
        }
    }
}
