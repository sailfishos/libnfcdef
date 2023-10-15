/*
 * Copyright (C) 2023 Slava Monich <slava@monich.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer
 *     in the documentation and/or other materials provided with the
 *     distribution.
 *
 *  3. Neither the names of the copyright holders nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * any official policies, either expressed or implied.
 */

#include "test_common.h"

#include "ndef_tlv.h"

static TestOpt test_opt;

#define TLV_TEST (0x04)

/*==========================================================================*
 * tlv_null
 *==========================================================================*/

static
void
test_tlv_null(
    void)
{
    /* ndef_tlv_check is NULL tolerant */
    g_assert_cmpuint(ndef_tlv_check(NULL), == ,0);
}

/*==========================================================================*
 * tlv_none
 *==========================================================================*/

typedef struct test_tlv_none_data {
    const char* name;
    GUtilData data;
    gsize consume;
    guint check;
} TestTlvNone;

static const guint8 tlv_broken1[] = { TLV_TEST };
static const guint8 tlv_broken2[] = { TLV_NULL };
static const guint8 tlv_broken3[] = {
    TLV_TEST, 0xff, 0x00 /* Truncated length */
};
static const guint8 tlv_broken4[] = {
    TLV_TEST, 0x01, /* Missing data */
};
static const guint8 tlv_empty1[] = {
    TLV_TERMINATOR    /* Terminator record */
};
static const guint8 tlv_empty2[] = {
    TLV_NULL,         /* No length or value */
    TLV_TERMINATOR    /* Terminator record */
};

static TestTlvNone tlv_none_tests[] = {
    { "no_data", { NULL, 0}, 0, 0 },
    { "broken1", { TEST_ARRAY_AND_SIZE(tlv_broken1) }, 0, 0 },
    { "broken2", { TEST_ARRAY_AND_SIZE(tlv_broken2) },
      sizeof(tlv_broken2), 0},
    { "broken3", { TEST_ARRAY_AND_SIZE(tlv_broken3) }, 0, 0},
    { "broken4", { TEST_ARRAY_AND_SIZE(tlv_broken4) }, 0, 0},
    { "empty1", { TEST_ARRAY_AND_SIZE(tlv_empty1) },
      sizeof(tlv_empty1), sizeof(tlv_empty1) },
    { "empty2", { TEST_ARRAY_AND_SIZE(tlv_empty2) },
      sizeof(tlv_empty2), sizeof(tlv_empty2) }
};

static
void
test_tlv_none(
    gconstpointer data)
{
    const TestTlvNone* test = data;
    GUtilData value, buf = test->data;

    g_assert_cmpuint(ndef_tlv_next(&buf, &value), == ,0);
    g_assert_cmpuint(buf.bytes - test->data.bytes, == ,test->consume);
    g_assert_cmpuint(ndef_tlv_check(&test->data), == ,test->check);
}

/*==========================================================================*
 * tlv_ndef
 *==========================================================================*/

typedef struct test_tlv_ndef_data {
    const char* name;
    GUtilData data;
} TestTlvNdef;

static const guint8 tlv_ndef_empty[] = {
    TLV_NDEF_MESSAGE, /* Value type */
    0x00,             /* Value length */
    TLV_TERMINATOR    /* Terminator record */
};
static const guint8 tlv_ndef_non_empty[] = {
    TLV_NULL,         /* Ignored */
    TLV_NDEF_MESSAGE, /* Value type */
    0x04,             /* Value length */
    0x91,                 /* NDEF record header (MB,SR,TNF=0x01) */
    0x01,                 /* Length of the record type */
    0x00,                 /* Length of the record payload */
    'x',                  /* Record type: 'x' */
    TLV_TERMINATOR    /* Terminator record */
};

static const guint8 tlv_ndef_long[] = {
    TLV_NDEF_MESSAGE, /* Value type */
    0xff, 0x01, 0x03, /* Value length */
    0xc1,             /* NDEF record header (MB,ME,TNF=0x01) */
    0x01,             /* Length of the record type */
    0xff,             /* Length of the record payload */
    'T',              /* Record type: 'T' (TEXT) */
    0x02,             /* encoding UTF-8 language length 2 */
    'e', 'n',         /* language */
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a', 
    'a', 'a', 'a', 'a', 'a', 'a', 'a', 'a',
    'a', 'a', 'a', 'a',
    TLV_TERMINATOR    /* Terminator record */
};

static TestTlvNdef tlv_ndef_tests[] = {
    { "empty", { TEST_ARRAY_AND_SIZE(tlv_ndef_empty) } },
    { "non_empty", { TEST_ARRAY_AND_SIZE(tlv_ndef_non_empty) } },
    { "long", { TEST_ARRAY_AND_SIZE(tlv_ndef_long) } },
};

static
void
test_tlv_ndef(
    gconstpointer data)
{
    const TestTlvNdef* test = data;
    GUtilData buf = test->data, value;

    g_assert_cmpuint(ndef_tlv_check(&buf), == ,buf.size);
    g_assert_cmpuint(ndef_tlv_next(&buf, &value), == ,TLV_NDEF_MESSAGE);
    g_assert(value.bytes == (buf.bytes - value.size));
    g_assert_cmpuint(ndef_tlv_next(&buf, &value), == ,0);
}

/*==========================================================================*
 * Common
 *==========================================================================*/

#define TEST_(name) "/ndef_tlv/" name

int main(int argc, char* argv[])
{
    guint i;

    g_test_init(&argc, &argv, NULL);
    g_test_add_func(TEST_("null"), test_tlv_null);
    for (i = 0; i < G_N_ELEMENTS(tlv_none_tests); i++) {
        const TestTlvNone* test = tlv_none_tests + i;
        char* path = g_strconcat(TEST_("none/"), test->name, NULL);

        g_test_add_data_func(path, test, test_tlv_none);
        g_free(path);
    }
    for (i = 0; i < G_N_ELEMENTS(tlv_ndef_tests); i++) {
        const TestTlvNdef* test = tlv_ndef_tests + i;
        char* path = g_strconcat(TEST_("ndef/"), test->name, NULL);

        g_test_add_data_func(path, test, test_tlv_ndef);
        g_free(path);
    }
    test_init(&test_opt, argc, argv);
    return g_test_run();
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
