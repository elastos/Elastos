/*
 *  BIP32 library, a Java implementation of BIP32
 *  Copyright (C) 2017-2019 Alan Evans, NovaCrypto
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  Original source: https://github.com/NovaCrypto/BIP32
 *  You can contact the authors via github issues.
 */

package io.github.novacrypto.bip32;

import io.github.novacrypto.bip32.derivation.CkdFunction;
import io.github.novacrypto.bip32.derivation.CkdFunctionDerive;
import io.github.novacrypto.bip32.derivation.Derivation;
import io.github.novacrypto.bip32.derivation.Derive;
import io.github.novacrypto.toruntime.CheckedExceptionToRuntime;

import java.math.BigInteger;
import java.util.Arrays;

import static io.github.novacrypto.base58.Base58.base58Encode;
import static io.github.novacrypto.bip32.BigIntegerUtils.parse256;
import static io.github.novacrypto.bip32.BigIntegerUtils.ser256;
import static io.github.novacrypto.bip32.ByteArrayWriter.head32;
import static io.github.novacrypto.bip32.ByteArrayWriter.tail32;
import static io.github.novacrypto.bip32.HmacSha512.hmacSha512;
import static io.github.novacrypto.bip32.Index.isHardened;
import static io.github.novacrypto.bip32.Secp256r1SC.n;
import static io.github.novacrypto.bip32.derivation.CkdFunctionResultCacheDecorator.newCacheOf;
import static io.github.novacrypto.toruntime.CheckedExceptionToRuntime.toRuntime;

/**
 * A BIP32 private key
 */
public final class ExtendedPrivateKey implements
        Derive<ExtendedPrivateKey>,
        CKDpriv,
        CKDpub,
        ExtendedKey {

    public static Deserializer<ExtendedPrivateKey> deserializer() {
        return ExtendedPrivateKeyDeserializer.DEFAULT;
    }

    public static Deserializer<ExtendedPrivateKey> deserializer(final Networks networks) {
        return new ExtendedPrivateKeyDeserializer(networks);
    }

    private static final CkdFunction<ExtendedPrivateKey> CKD_FUNCTION = new CkdFunction<ExtendedPrivateKey>() {
        @Override
        public ExtendedPrivateKey deriveChildKey(final ExtendedPrivateKey parent, final int childIndex) {
            return parent.cKDpriv(childIndex);
        }
    };

    private static final byte[] BITCOIN_SEED = getBytes("Bitcoin seed");

    private final HdKey hdKey;

    private ExtendedPrivateKey(final Network network, final byte[] key, final byte[] chainCode) {
        this(new HdKey.Builder()
                // .network(network)
                // .neutered(false)
                .key(key)
                .chainCode(chainCode)
                // .depth(0)
                // .childNumber(0)
                // .parentFingerprint(0)
                .build());
    }

    ExtendedPrivateKey(final HdKey hdKey) {
        this.hdKey = hdKey;
    }

    public static ExtendedPrivateKey fromSeed(final byte[] seed, final Network network) {
        final byte[] I = hmacSha512(BITCOIN_SEED, seed);

        final byte[] Il = head32(I);
        final byte[] Ir = tail32(I);

        return new ExtendedPrivateKey(network, Il, Ir);
    }

    private static byte[] getBytes(final String seed) {
        return toRuntime(new CheckedExceptionToRuntime.Func<byte[]>() {
            @Override
            public byte[] run() throws Exception {
                return seed.getBytes("UTF-8");
            }
        });
    }

    @Override
    public byte[] extendedKeyByteArray() {
        return hdKey.serialize();
    }

    public byte[] getKeyBytes() {
    	return hdKey.getKey();
    }

    @Override
    public ExtendedPrivateKey toNetwork(final Network otherNetwork) {
        // if (otherNetwork == network())
        //     return this;
        return new ExtendedPrivateKey(
                hdKey.toBuilder()
                        // .network(otherNetwork)
                        .build());
    }

    @Override
    public String extendedBase58() {
        return base58Encode(extendedKeyByteArray());
    }

    @Override
    public ExtendedPrivateKey cKDpriv(final int index) {
        final byte[] data = new byte[37];
        final ByteArrayWriter writer = new ByteArrayWriter(data);

        if (isHardened(index)) {
            writer.concat((byte) 0);
            writer.concat(hdKey.getKey(), 32);
        } else {
            writer.concat(hdKey.getPoint());
        }
        writer.concatSer32(index);

        final byte[] I = hmacSha512(hdKey.getChainCode(), data);
        Arrays.fill(data, (byte) 0);

        final byte[] Il = head32(I);
        final byte[] Ir = tail32(I);

        final byte[] key = hdKey.getKey();
        final BigInteger parse256_Il = parse256(Il);
        final BigInteger ki = parse256_Il.add(parse256(key)).mod(n());

        if (parse256_Il.compareTo(n()) >= 0 || ki.equals(BigInteger.ZERO)) {
            return cKDpriv(index + 1);
        }

        ser256(Il, ki);

        return new ExtendedPrivateKey(new HdKey.Builder()
                // .network(hdKey.getNetwork())
                // .neutered(false)
                .key(Il)
                .chainCode(Ir)
                // .depth(hdKey.depth() + 1)
                .childNumber(index)
                // .parentFingerprint(hdKey.calculateFingerPrint())
                .build());
    }

    @Override
    public ExtendedPublicKey cKDpub(final int index) {
        return cKDpriv(index).neuter();
    }

    public ExtendedPublicKey neuter() {
        return ExtendedPublicKey.from(hdKey);
    }

    public Derive<ExtendedPrivateKey> derive() {
        return derive(CKD_FUNCTION);
    }

    public Derive<ExtendedPrivateKey> deriveWithCache() {
        return derive(newCacheOf(CKD_FUNCTION));
    }

    @Override
    public ExtendedPrivateKey derive(final CharSequence derivationPath) {
        return derive().derive(derivationPath);
    }

    @Override
    public <Path> ExtendedPrivateKey derive(final Path derivationPath, final Derivation<Path> derivation) {
        return derive().derive(derivationPath, derivation);
    }

    private Derive<ExtendedPrivateKey> derive(final CkdFunction<ExtendedPrivateKey> ckdFunction) {
        return new CkdFunctionDerive<>(ckdFunction, this);
    }

    // @Override
    // public Network network() {
    //     return hdKey.getNetwork();
    // }

    @Override
    public int depth() {
        return hdKey.depth();
    }

    @Override
    public int childNumber() {
        return hdKey.getChildNumber();
    }
}