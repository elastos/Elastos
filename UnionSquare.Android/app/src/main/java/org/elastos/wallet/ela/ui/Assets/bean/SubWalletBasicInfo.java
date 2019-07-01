package org.elastos.wallet.ela.ui.Assets.bean;

public class SubWalletBasicInfo {

    /**
     * Info : {"Account":{"M":1,"N":1,"Readonly":false,"SingleAddress":false,"Type":"Standard"},"CoinIndex":2}
     * ChainID : TokenChain
     */

    private InfoBean Info;
    private String ChainID;

    public InfoBean getInfo() {
        return Info;
    }

    public void setInfo(InfoBean Info) {
        this.Info = Info;
    }

    public String getChainID() {
        return ChainID;
    }

    public void setChainID(String ChainID) {
        this.ChainID = ChainID;
    }

    public static class InfoBean {
        /**
         * Account : {"M":1,"N":1,"Readonly":false,"SingleAddress":false,"Type":"Standard"}
         * CoinIndex : 2
         */

        private AccountBean Account;
        private int CoinIndex;

        public AccountBean getAccount() {
            return Account;
        }

        public void setAccount(AccountBean Account) {
            this.Account = Account;
        }

        public int getCoinIndex() {
            return CoinIndex;
        }

        public void setCoinIndex(int CoinIndex) {
            this.CoinIndex = CoinIndex;
        }

        public static class AccountBean {
            /**
             * M : 1
             * N : 1
             * Readonly : false
             * SingleAddress : false
             * Type : Standard
             */

            private int M;
            private int N;
            private boolean Readonly;
            private boolean SingleAddress;
            private String Type;

            public int getM() {
                return M;
            }

            public void setM(int M) {
                this.M = M;
            }

            public int getN() {
                return N;
            }

            public void setN(int N) {
                this.N = N;
            }

            public boolean isReadonly() {
                return Readonly;
            }

            public void setReadonly(boolean Readonly) {
                this.Readonly = Readonly;
            }

            public boolean isSingleAddress() {
                return SingleAddress;
            }

            public void setSingleAddress(boolean SingleAddress) {
                this.SingleAddress = SingleAddress;
            }

            public String getType() {
                return Type;
            }

            public void setType(String Type) {
                this.Type = Type;
            }
        }
    }
}
