/*
 * Copyright (c) 2020 Elastos Foundation
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

#ifndef __ELASTOS_SDK_SECP256K1_NAME_FIX_H__
#define __ELASTOS_SDK_SECP256K1_NAME_FIX_H__

#define secp256k1_context_create		SPV_secp256k1_context_create
#define secp256k1_context_clone		SPV_secp256k1_context_clone
#define secp256k1_context_destroy		SPV_secp256k1_context_destroy
#define secp256k1_context_set_illegal_callback		SPV_secp256k1_context_set_illegal_callback
#define secp256k1_context_set_error_callback		SPV_secp256k1_context_set_error_callback
#define secp256k1_scratch_space_create		SPV_secp256k1_scratch_space_create
#define secp256k1_scratch_space_destroy		SPV_secp256k1_scratch_space_destroy
#define secp256k1_ec_pubkey_parse		SPV_secp256k1_ec_pubkey_parse
#define secp256k1_ec_pubkey_serialize		SPV_secp256k1_ec_pubkey_serialize
#define secp256k1_ecdsa_signature_parse_compact		SPV_secp256k1_ecdsa_signature_parse_compact
#define secp256k1_ecdsa_signature_parse_der		SPV_secp256k1_ecdsa_signature_parse_der
#define secp256k1_ecdsa_signature_serialize_der		SPV_secp256k1_ecdsa_signature_serialize_der
#define secp256k1_ecdsa_signature_serialize_compact		SPV_secp256k1_ecdsa_signature_serialize_compact
#define secp256k1_ecdsa_verify		SPV_secp256k1_ecdsa_verify
#define secp256k1_ecdsa_signature_normalize		SPV_secp256k1_ecdsa_signature_normalize
#define secp256k1_ecdsa_sign		SPV_secp256k1_ecdsa_sign
#define secp256k1_ec_seckey_verify		SPV_secp256k1_ec_seckey_verify
#define secp256k1_ec_pubkey_create		SPV_secp256k1_ec_pubkey_create
#define secp256k1_ec_privkey_negate		SPV_secp256k1_ec_privkey_negate
#define secp256k1_ec_pubkey_negate		SPV_secp256k1_ec_pubkey_negate
#define secp256k1_ec_privkey_tweak_add		SPV_secp256k1_ec_privkey_tweak_add
#define secp256k1_ec_pubkey_tweak_add		SPV_secp256k1_ec_pubkey_tweak_add
#define secp256k1_ec_privkey_tweak_mul		SPV_secp256k1_ec_privkey_tweak_mul
#define secp256k1_ec_pubkey_tweak_mul		SPV_secp256k1_ec_pubkey_tweak_mul
#define secp256k1_context_randomize		SPV_secp256k1_context_randomize
#define secp256k1_ec_pubkey_combine		SPV_secp256k1_ec_pubkey_combine
#define secp256k1_ecdh		SPV_secp256k1_ecdh
#define secp256k1_ecdsa_recoverable_signature_parse_compact		SPV_secp256k1_ecdsa_recoverable_signature_parse_compact
#define secp256k1_ecdsa_recoverable_signature_convert		SPV_secp256k1_ecdsa_recoverable_signature_convert
#define secp256k1_ecdsa_recoverable_signature_serialize_compact		SPV_secp256k1_ecdsa_recoverable_signature_serialize_compact
#define secp256k1_ecdsa_sign_recoverable		SPV_secp256k1_ecdsa_sign_recoverable
#define secp256k1_ecdsa_recover		SPV_secp256k1_ecdsa_recover

#endif
