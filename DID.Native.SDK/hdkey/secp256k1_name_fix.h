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

#ifndef __SECP256K1_NAMEFIX_H__
#define __SECP256K1_NAMEFIX_H__

#define secp256k1_context_create		DID_secp256k1_context_create
#define secp256k1_context_clone		DID_secp256k1_context_clone
#define secp256k1_context_destroy		DID_secp256k1_context_destroy
#define secp256k1_context_set_illegal_callback		DID_secp256k1_context_set_illegal_callback
#define secp256k1_context_set_error_callback		DID_secp256k1_context_set_error_callback
#define secp256k1_scratch_space_create		DID_secp256k1_scratch_space_create
#define secp256k1_scratch_space_destroy		DID_secp256k1_scratch_space_destroy
#define secp256k1_ec_pubkey_parse		DID_secp256k1_ec_pubkey_parse
#define secp256k1_ec_pubkey_serialize		DID_secp256k1_ec_pubkey_serialize
#define secp256k1_ecdsa_signature_parse_compact		DID_secp256k1_ecdsa_signature_parse_compact
#define secp256k1_ecdsa_signature_parse_der		DID_secp256k1_ecdsa_signature_parse_der
#define secp256k1_ecdsa_signature_serialize_der		DID_secp256k1_ecdsa_signature_serialize_der
#define secp256k1_ecdsa_signature_serialize_compact		DID_secp256k1_ecdsa_signature_serialize_compact
#define secp256k1_ecdsa_verify		DID_secp256k1_ecdsa_verify
#define secp256k1_ecdsa_signature_normalize		DID_secp256k1_ecdsa_signature_normalize
#define secp256k1_ecdsa_sign		DID_secp256k1_ecdsa_sign
#define secp256k1_ec_seckey_verify		DID_secp256k1_ec_seckey_verify
#define secp256k1_ec_pubkey_create		DID_secp256k1_ec_pubkey_create
#define secp256k1_ec_privkey_negate		DID_secp256k1_ec_privkey_negate
#define secp256k1_ec_pubkey_negate		DID_secp256k1_ec_pubkey_negate
#define secp256k1_ec_privkey_tweak_add		DID_secp256k1_ec_privkey_tweak_add
#define secp256k1_ec_pubkey_tweak_add		DID_secp256k1_ec_pubkey_tweak_add
#define secp256k1_ec_privkey_tweak_mul		DID_secp256k1_ec_privkey_tweak_mul
#define secp256k1_ec_pubkey_tweak_mul		DID_secp256k1_ec_pubkey_tweak_mul
#define secp256k1_context_randomize		DID_secp256k1_context_randomize
#define secp256k1_ec_pubkey_combine		DID_secp256k1_ec_pubkey_combine
#define secp256k1_ecdh		DID_secp256k1_ecdh
#define secp256k1_ecdsa_recoverable_signature_parse_compact		DID_secp256k1_ecdsa_recoverable_signature_parse_compact
#define secp256k1_ecdsa_recoverable_signature_convert		DID_secp256k1_ecdsa_recoverable_signature_convert
#define secp256k1_ecdsa_recoverable_signature_serialize_compact		DID_secp256k1_ecdsa_recoverable_signature_serialize_compact
#define secp256k1_ecdsa_sign_recoverable		DID_secp256k1_ecdsa_sign_recoverable
#define secp256k1_ecdsa_recover		DID_secp256k1_ecdsa_recover

#endif /* __SECP256K1_NAMEFIX_H__ */
