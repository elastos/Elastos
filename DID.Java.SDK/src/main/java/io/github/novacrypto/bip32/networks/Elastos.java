package io.github.novacrypto.bip32.networks;

import io.github.novacrypto.bip32.Network;

public enum Elastos implements Network {
    MAIN_NET {
        @Override
        public int getPrivateVersion() {
            return 0;
        }

        @Override
        public int getPublicVersion() {
            return 0;
        }

        @Override
        public byte p2pkhVersion() {
            return 0;
        }

        @Override
        public byte p2shVersion() {
            return 0x05;
        }
    },

    TEST_NET {
        @Override
        public int getPrivateVersion() {
            return 0;
        }

        @Override
        public int getPublicVersion() {
            return 0;
        }

        @Override
        public byte p2pkhVersion() {
            return 0;
        }

        @Override
        public byte p2shVersion() {
            return (byte) 0;
        }
    }
}
