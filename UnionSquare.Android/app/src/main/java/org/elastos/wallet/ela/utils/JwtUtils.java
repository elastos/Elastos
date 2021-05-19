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

package org.elastos.wallet.ela.utils;

import android.util.Base64;

import org.elastos.did.DIDDocument;

import java.security.KeyFactory;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.SecureRandom;
import java.security.Signature;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.spec.X509EncodedKeySpec;

import javax.crypto.KeyAgreement;

public class JwtUtils {

    /**
     * 字节数组转成16进制字符串
     **/
    public static String byte2hex(byte[] b) { // 一个字节的数，
        StringBuffer sb = new StringBuffer(b.length * 2);
        String tmp = "";
        for (int n = 0; n < b.length; n++) {
            // 整数转成十六进制表示
            tmp = (java.lang.Integer.toHexString(b[n] & 0XFF));
            if (tmp.length() == 1) {
                sb.append("0");
            }
            sb.append(tmp);
        }
        return sb.toString(); // 转成大写
    }


    /**
     * 将hex字符串转换成字节数组
     **/
    public static byte[] hex2byte(String inputString) {
        if (inputString == null || inputString.length() < 2) {
            return new byte[0];
        }
        inputString = inputString.toLowerCase();
        int l = inputString.length() / 2;
        byte[] result = new byte[l];
        for (int i = 0; i < l; ++i) {
            String tmp = inputString.substring(2 * i, 2 * i + 2);
            result[i] = (byte) (Integer.parseInt(tmp, 16) & 0xFF);
        }
        return result;
    }

    public static byte[] hex2byteBe(String inputString) {
        if (inputString == null || inputString.length() < 2) {
            return new byte[0];
        }
        inputString = inputString.toLowerCase();
        int l = inputString.length() / 2;
        byte[] result = new byte[l];
        for (int i = 0; i < l; ++i) {
            String tmp = inputString.substring(2 * i, 2 * i + 2);
            result[l-1-i] = (byte) (Integer.parseInt(tmp, 16) & 0xFF);
        }
        return result;
    }

    /**
     * bytes[]换成16进制字符串
     *
     * @param src
     * @return
     */
    public static String bytesToHexString(byte[] src) {
        StringBuilder stringBuilder = new StringBuilder("");
        if (src == null || src.length <= 0) {
            return null;
        }
        for (int i = 0; i < src.length; i++) {
            int v = src[i] & 0xFF;
            String hv = Integer.toHexString(v);
            if (hv.length() < 2) {
                stringBuilder.append(0);
            }
            stringBuilder.append(hv);
        }
        return stringBuilder.toString();
    }

    //加签
    public static String signECDSA(PrivateKey privateKey, String message) {
        String result = "";
        try {
            //执行签名
            Signature signature = Signature.getInstance("SHA1withECDSA");
            signature.initSign(privateKey);
            signature.update(message.getBytes());
            byte[] sign = signature.sign();
            return byte2hex(sign);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return result;
    }

    //验签
    public static boolean verifyECDSA(PublicKey publicKey, String signed, String message) {
        try {
            //验证签名
            Signature signature = Signature.getInstance("SHA1withECDSA");
            signature.initVerify(publicKey);
            signature.update(message.getBytes());

            byte[] hex = hex2byte(signed);
            boolean bool = signature.verify(hex);
            System.out.println("验证：" + bool);
            return bool;
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    /**
     * 从string转private key
     */
    public static PrivateKey getPrivateKey(String key) throws Exception {
        byte[] bytes = hex2byte(key);

        PKCS8EncodedKeySpec keySpec = new PKCS8EncodedKeySpec(bytes);
        KeyFactory keyFactory = KeyFactory.getInstance("EC");
        return keyFactory.generatePrivate(keySpec);
    }

    /**
     * 从string转public key
     */
    public static PublicKey getPublicKey(String key) throws Exception {
        byte[] bytes = hex2byte(key);

        X509EncodedKeySpec keySpec = new X509EncodedKeySpec(bytes);
        KeyFactory keyFactory = KeyFactory.getInstance("EC");
        return keyFactory.generatePublic(keySpec);
    }

    public static final char[] DIGITS_UPPER = {'0', '1', '2', '3', '4', '5',
            '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    public static String byte2HexString(final byte[] data) {
        final int l = data.length;
        final char[] out = new char[l << 1];
        // two characters form the hex value.
        for (int i = 0, j = 0; i < l; i++) {
            out[j++] = DIGITS_UPPER[(0xF0 & data[i]) >>> 4];
            out[j++] = DIGITS_UPPER[0x0F & data[i]];
        }
        return new String(out);
    }


    /**
     * 生成 share key
     *
     * @param publicStr  公钥字符串
     * @param privateStr 私钥字符串
     * @return
     */
    public static String genSharedKey(String publicStr, String privateStr) {
        try {
            return genSharedKey(getPublicKey(publicStr), getPrivateKey(privateStr));
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     * 生成 share key
     *
     * @param publicKey  公钥
     * @param privateKey 私钥
     * @return
     */
    public static String genSharedKey(PublicKey publicKey, PrivateKey privateKey) {
        String sharedKey = "";
        try {
            KeyAgreement ka = KeyAgreement.getInstance("ECDH");
            ka.init(privateKey);
            ka.doPhase(publicKey, true);
            sharedKey = bytesToHexString(ka.generateSecret());
        } catch (Exception e) {
            e.printStackTrace();
        }
        return sharedKey;
    }

    //生成KeyPair
    public static KeyPair getKeyPair() throws Exception {
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("EC");
        SecureRandom random = SecureRandom.getInstance("SHA1PRNG");
        keyGen.initialize(256, random);
        return keyGen.generateKeyPair();
    }

    public static String getJwtPayload(String jwt) {
        String[] jwtParts = jwt.split("\\.");
        // String base64Header = jwtParts[0];
        String base64Payload = jwtParts[1];
        // String unSignature = base64Header + "." + base64Payload;
        //  String signature = jwtParts[2];
        //  String header = new String(org.elastos.did.util.Base64.decode(base64Header));
        return new String(Base64.decode(base64Payload, Base64.URL_SAFE | Base64.NO_PADDING | Base64.NO_WRAP));
    }

    public static String getJwtHeader(String jwt) {
        String[] jwtParts = jwt.split("\\.");
        //String header = jwtParts[0];
        return jwtParts[0];
    }

    public static boolean verifyJwt(String result, DIDDocument didDocument) {
        if (didDocument == null) {
            return false;
        }
        String[] jwtParts = result.split("\\.");
        String unSignature = jwtParts[0] + "." + jwtParts[1];
        String signature = jwtParts[2];
        return didDocument.verify(signature, unSignature.getBytes());
    }
/*
    public static class MySigningKeyResolver extends SigningKeyResolverAdapter {

        public String getPubk() {
            return pubk;
        }

        public void setPubk(String pubk) {
            this.pubk = pubk;
        }

        private String pubk;

        @Override
        public Key resolveSigningKey(JwsHeader jwsHeader, Claims claims) {
            byte[] pkby = JwtUtils.hex2byte(pubk);
            X509EncodedKeySpec keySpec = new X509EncodedKeySpec(pkby);
            KeyFactory keyFactory = null;
            try {
                keyFactory = KeyFactory.getInstance("EC");
                return keyFactory.generatePublic(keySpec);
            } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
                e.printStackTrace();
            }
            return null;
        }
    }*/

   /* private void decodeJwt(String result) {
        result = result.replace("//credaccess/", "");
                         *//*result = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiJkaWQ6ZWxhc3RvczppV3NLM1g4YUJwTHFGVlhBZDJLVXBNcEZHSllFWXlmalVpIiwiaWF0IjoxNTY2MzUyMjEzLCJleHAiOjE1ODA2MDcwODksImNhbGxiYWNrdXJsIjoiaHR0cHM6Ly93d3cubnVjbGV1c2NvbnNvbGUuY29tL2RpZF9jYWxsYmFja19lbGFzdG9zIiwiY2xhaW1zIjp7fX0.nBDAz8vcfcVufHtGzD31fSoGabGHwkmAPPsHi8o0l74";
                        String secretString = "iWsK3X8aBpLqFVXAd2KUpMpFGJYEYyfjUiiWsK3X8aBpLqFVXAd2KUpMpFGJYEYyfjUi";
                        SecretKey key = Keys.hmacShaKeyFor(Decoders.BASE64.decode(secretString));
                        Jwt jwt = Jwts.parserBuilder()
                                .setSigningKey(key)
                                .build()
                                .parseClaimsJws(result);*//*
        //   new SecretKeySpec(keyBytes, alg.getJcaName());
        KeyPair keyPair = Keys.keyPairFor(SignatureAlgorithm.ES256);

        String jws = Jwts.builder() // (1)
                .setSubject("Bob")      // (2)
                .signWith(keyPair.getPrivate())          // (3)
                .compact();
        Log.i("?????结果", jws);
        String prik = JwtUtils.byte2hex(keyPair.getPrivate().getEncoded());
        String pubk = JwtUtils.byte2hex(keyPair.getPublic().getEncoded());
        Log.i("???公钥16进制string", prik + "////\n" + pubk + "///\n" + JwtUtils.byte2HexString(keyPair.getPublic().getEncoded()));
        Log.i("???Base64string", Base64.encodeToString(keyPair.getPrivate().getEncoded()) + "////" + Base64.encodeToString(keyPair.getPublic().getEncoded()));
        JwtUtils.MySigningKeyResolver signingKeyResolver = new JwtUtils.MySigningKeyResolver();
        signingKeyResolver.setPubk(pubk);
        try {
            Jws<Claims> jws1 = Jwts.parserBuilder()
                    .setSigningKeyResolver(signingKeyResolver)
                    .build()
                    .parseClaimsJws(jws);
            Log.i("???解密成功", jws1.getBody().toString() + "\n" + jws1.toString() + "\n"
                    + jws1.getHeader() + "\n"
            );
            Bundle bundle = new Bundle();
            bundle.putString("JWT", jws1.toString());
            //  ((BaseFragment) getParentFragment()).start(AuthorizationFragment.class, bundle);
        } catch (Exception e) {
            // toErroScan(result);
        }


    }*/
}


