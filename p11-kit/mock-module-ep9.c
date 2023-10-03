/*
 * Copyright (c) 2023, Red Hat Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above
 *       copyright notice, this list of conditions and the
 *       following disclaimer.
 *     * Redistributions in binary form must reproduce the
 *       above copyright notice, this list of conditions and
 *       the following disclaimer in the documentation and/or
 *       other materials provided with the distribution.
 *     * The names of contributors to this software may not be
 *       used to endorse or promote products derived from this
 *       software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * Author: Zoltan Fridrich <zfridric@redhat.com>
 */

#include "config.h"

#define CRYPTOKI_EXPORTS 1
#include "pkcs11.h"

#include "attrs.h"
#include "mock.h"

#include <string.h>

static CK_RV
override_initialize (CK_VOID_PTR init_args)
{
	CK_OBJECT_CLASS cert_klass = CKO_CERTIFICATE;
	char *cert_label = "TEST CERTIFICATE";
	CK_CERTIFICATE_TYPE cert_type = CKC_X_509;
	CK_BBOOL ck_true = CK_TRUE;
	char cert_data[] = {
		0x30, 0x82, 0x01, 0x6a, 0x30, 0x82, 0x01, 0x14, 0xa0, 0x03,
		0x02, 0x01, 0x02, 0x02, 0x02, 0x03, 0xe7, 0x30, 0x0d, 0x06,
		0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05,
		0x05, 0x00, 0x30, 0x28, 0x31, 0x26, 0x30, 0x24, 0x06, 0x03,
		0x55, 0x04, 0x03, 0x13, 0x1d, 0x66, 0x61, 0x72, 0x2d, 0x69,
		0x6e, 0x2d, 0x74, 0x68, 0x65, 0x2d, 0x66, 0x75, 0x74, 0x75,
		0x72, 0x65, 0x2e, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65,
		0x2e, 0x63, 0x6f, 0x6d, 0x30, 0x20, 0x17, 0x0d, 0x31, 0x33,
		0x30, 0x33, 0x32, 0x37, 0x31, 0x36, 0x34, 0x39, 0x33, 0x33,
		0x5a, 0x18, 0x0f, 0x32, 0x30, 0x36, 0x37, 0x31, 0x32, 0x32,
		0x39, 0x31, 0x36, 0x34, 0x39, 0x33, 0x33, 0x5a, 0x30, 0x28,
		0x31, 0x26, 0x30, 0x24, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13,
		0x1d, 0x66, 0x61, 0x72, 0x2d, 0x69, 0x6e, 0x2d, 0x74, 0x68,
		0x65, 0x2d, 0x66, 0x75, 0x74, 0x75, 0x72, 0x65, 0x2e, 0x65,
		0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d,
		0x30, 0x5c, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
		0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x4b, 0x00,
		0x30, 0x48, 0x02, 0x41, 0x00, 0xe2, 0x2d, 0x35, 0x70, 0x75,
		0xc0, 0x07, 0x56, 0x40, 0x7d, 0x63, 0xbc, 0xd2, 0x60, 0xb3,
		0xcf, 0xb8, 0x3d, 0x27, 0x6e, 0x10, 0xcd, 0x42, 0x50, 0x51,
		0x9d, 0x79, 0x30, 0x79, 0x5a, 0xe3, 0xc3, 0x51, 0x38, 0x85,
		0x4c, 0xb4, 0x91, 0xd9, 0xe6, 0x8d, 0x69, 0x6a, 0xd4, 0x9c,
		0x1c, 0x49, 0xc2, 0x25, 0x2a, 0xc9, 0x2b, 0xf2, 0xf4, 0x8e,
		0x8a, 0x3f, 0x8b, 0x4c, 0x97, 0xc3, 0x16, 0x96, 0x99, 0x02,
		0x03, 0x01, 0x00, 0x01, 0xa3, 0x26, 0x30, 0x24, 0x30, 0x22,
		0x06, 0x03, 0x55, 0x1d, 0x25, 0x04, 0x1b, 0x30, 0x19, 0x06,
		0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x02, 0x06,
		0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x04, 0x06,
		0x03, 0x2a, 0x03, 0x04, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86,
		0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05, 0x00, 0x03,
		0x41, 0x00, 0xc2, 0x83, 0x27, 0x32, 0x80, 0x74, 0x73, 0xe2,
		0xa3, 0x92, 0xaa, 0x7c, 0xd8, 0x50, 0xf4, 0x61, 0x50, 0xb1,
		0x63, 0x9e, 0x29, 0xef, 0x38, 0x1d, 0xc0, 0x55, 0x20, 0x0f,
		0x7e, 0xe9, 0x1f, 0xa1, 0x54, 0x1a, 0x5f, 0x8c, 0x26, 0x1b,
		0x66, 0x96, 0x0e, 0x64, 0x52, 0x1c, 0x00, 0x96, 0xfb, 0x81,
		0x77, 0xa2, 0x3a, 0x1d, 0x49, 0x0c, 0x03, 0xd5, 0x19, 0xf2,
		0x6a, 0x01, 0x29, 0x31, 0xfb, 0xf5
	};
	CK_ATTRIBUTE cert_attrs[] = {
		{ CKA_CLASS, &cert_klass, sizeof (cert_klass) },
		{ CKA_LABEL, cert_label, strlen (cert_label) },
		{ CKA_TRUSTED, &ck_true, sizeof (ck_true) },
		{ CKA_COPYABLE, &ck_true, sizeof (ck_true) },
		{ CKA_DESTROYABLE, &ck_true, sizeof (ck_true) },
		{ CKA_CERTIFICATE_TYPE, &cert_type, sizeof (cert_type) },
		{ CKA_VALUE, cert_data, sizeof (cert_data) },
		{ CKA_INVALID, NULL, 0 },
	};

	CK_OBJECT_CLASS pubkey_klass = CKO_PUBLIC_KEY;
	char *pubkey_label = "TEST PUBLIC KEY";
	CK_KEY_TYPE pubkey_type = CKK_EC;
	char pubkey_data[] = {
		0x30, 0x76, 0x30, 0x10, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce,
		0x3d, 0x02, 0x01, 0x06, 0x05, 0x2b, 0x81, 0x04, 0x00, 0x22,
		0x03, 0x62, 0x00, 0x04, 0x9f, 0x52, 0xe5, 0xc0, 0xb3, 0x7f,
		0x28, 0x16, 0x10, 0x45, 0x51, 0xfa, 0x1d, 0xf2, 0x0c, 0x4f,
		0x37, 0xc4, 0xa8, 0x93, 0x95, 0xce, 0xd2, 0xde, 0x90, 0xb7,
		0x21, 0xa7, 0x68, 0x62, 0xef, 0xc7, 0x02, 0x68, 0xc6, 0x3c,
		0xd4, 0x50, 0x65, 0x62, 0xcf, 0x09, 0xf6, 0x5e, 0xe4, 0xad,
		0xcf, 0x8c, 0xe1, 0xa0, 0x5e, 0x08, 0x66, 0x05, 0x8d, 0xb6,
		0xbe, 0x86, 0x25, 0xed, 0xb4, 0x95, 0x8f, 0x2f, 0xbc, 0x9d,
		0x94, 0x4f, 0xb9, 0x50, 0x6e, 0x14, 0x36, 0x49, 0xf7, 0x12,
		0x8b, 0x3c, 0x12, 0x26, 0x41, 0xca, 0x2f, 0x43, 0x56, 0xcc,
		0x9f, 0xcb, 0xd7, 0x9e, 0x8e, 0x1f, 0xbc, 0x01, 0x78, 0x29
	};
	CK_ATTRIBUTE pubkey_attrs[] = {
		{ CKA_CLASS, &pubkey_klass, sizeof (pubkey_klass) },
		{ CKA_LABEL, pubkey_label, strlen (pubkey_label) },
		{ CKA_KEY_TYPE, &pubkey_type, sizeof (pubkey_type) },
		{ CKA_PUBLIC_KEY_INFO, pubkey_data, sizeof (pubkey_data) },
		{ CKA_INVALID, NULL, 0 },
	};

	CK_RV rv = mock_C_Initialize (init_args);
	mock_module_add_object (MOCK_SLOT_ONE_ID, cert_attrs);
	mock_module_add_object (MOCK_SLOT_ONE_ID, pubkey_attrs);
	return rv;
}

#ifdef OS_WIN32
__declspec(dllexport)
#endif
CK_RV
C_GetFunctionList (CK_FUNCTION_LIST_PTR_PTR list)
{
	mock_module_init ();
	mock_module.C_GetFunctionList = C_GetFunctionList;
	mock_module.C_Initialize = override_initialize;
	if (list == NULL)
		return CKR_ARGUMENTS_BAD;
	*list = &mock_module;
	return CKR_OK;
}