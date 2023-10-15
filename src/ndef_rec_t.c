/*
 * Copyright (C) 2019-2023 Slava Monich <slava@monich.com>
 * Copyright (C) 2019-2020 Jolla Ltd.
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

#include "ndef_rec_p.h"
#include "ndef_util_p.h"
#include "ndef_log.h"

#include <gutil_misc.h>

/* NFCForum-TS-RTD_TEXT_1.0 */

struct nfc_ndef_rec_t_priv {
    char* lang;
    char* text;
};

#define THIS(obj) NDEF_REC_T(obj)
#define THIS_TYPE NDEF_TYPE_REC_T
#define PARENT_TYPE NDEF_TYPE_REC
#define PARENT_CLASS ndef_rec_t_parent_class

typedef NdefRecClass NdefRecTClass;
G_DEFINE_TYPE(NdefRecT, ndef_rec_t, PARENT_TYPE)

const GUtilData ndef_rec_type_t = { (const guint8*) "T", 1 };

#define STATUS_LANG_LEN_MASK (0x3f)
#define STATUS_ENC_UTF16 (0x80) /* Otherwise UTF-8 */

static const char ENC_UTF8[] = "UTF-8";
static const char ENC_UTF16_LE[] = "UTF-16LE";
static const char ENC_UTF16_BE[] = "UTF-16BE";

/* UTF-16 Byte Order Marks */
static const guint8 UTF16_BOM_LE[] = {0xff, 0xfe};
static const guint8 UTF16_BOM_BE[] = {0xfe, 0xff};

static
GBytes*
ndef_rec_t_build(
    const char* text,
    const char* lang,
    NDEF_REC_T_ENC enc)
{
    const guint8 lang_len = strlen(lang);
    const gsize text_len = strlen(text);
    const guint8 status_byte = (lang_len & STATUS_LANG_LEN_MASK) |
        ((enc == NDEF_REC_T_ENC_UTF8) ? 0 : STATUS_ENC_UTF16);
    const guint8* bom = NULL;
    const void* enc_text = NULL;
    void* enc_text_tmp = NULL;
    gsize enc_text_len;
    gsize bom_len = 0;
    GError* err = NULL;

    switch (enc) {
    case NDEF_REC_T_ENC_UTF8:
        enc_text = text;
        enc_text_len = text_len;
        break;
    case NDEF_REC_T_ENC_UTF16BE:
         enc_text = enc_text_tmp = g_convert(text, text_len, ENC_UTF16_BE,
            ENC_UTF8, NULL, &enc_text_len, &err);
         break;
    case NDEF_REC_T_ENC_UTF16LE:
        bom = UTF16_BOM_LE;
        bom_len = sizeof(UTF16_BOM_LE);
        enc_text = enc_text_tmp = g_convert(text, text_len, ENC_UTF16_LE,
            ENC_UTF8, NULL, &enc_text_len, &err);
        break;
    }

    if (enc_text) {
        GByteArray* buf = g_byte_array_sized_new(1 + lang_len + bom_len +
            enc_text_len);

        g_byte_array_append(buf, &status_byte, 1);
        g_byte_array_append(buf, (const guint8*)lang, lang_len);
        if (bom) g_byte_array_append(buf, bom, bom_len);
        g_byte_array_append(buf, enc_text, enc_text_len);
        g_free(enc_text_tmp);
        return g_byte_array_free_to_bytes(buf);
    } else {
        if (err) {
            GWARN("Failed to encode Text record: %s", err->message);
            g_error_free(err);
        } else {
            GWARN("Failed to encode Text record");
        }
        return NULL;
    }
}

/*==========================================================================*
 * Interface
 *==========================================================================*/

NdefRecT*
ndef_rec_t_new_from_data(
    const NdefData* ndef)
{
    GUtilData payload;

    if (ndef_payload(ndef, &payload)) {
        const guint8 status_byte = payload.bytes[0];
        const guint lang_len = (status_byte & STATUS_LANG_LEN_MASK);
        const char* lang = (char*)payload.bytes + 1;

        if ((lang_len < payload.size) && /* Empty or ASCII (at least UTF-8) */
            (!lang_len || g_utf8_validate(lang, lang_len, NULL))) {
            const char* text = (char*)payload.bytes + lang_len + 1;
            const guint text_len = payload.size - lang_len - 1;
            const char* utf8;
            gsize utf8_len;
            char* utf8_buf;

            if (status_byte & STATUS_ENC_UTF16) {
                GError* err = NULL;
                if (text_len >= sizeof(UTF16_BOM_BE) &&
                    !memcmp(text, UTF16_BOM_BE, sizeof(UTF16_BOM_BE))) {
                    utf8_buf = g_convert(text + sizeof(UTF16_BOM_BE),
                        text_len - sizeof(UTF16_BOM_BE), ENC_UTF8,
                        ENC_UTF16_BE, NULL, &utf8_len, &err);
                } else if (text_len >= sizeof(UTF16_BOM_LE) &&
                    !memcmp(text, UTF16_BOM_LE, sizeof(UTF16_BOM_LE))) {
                    utf8_buf = g_convert(text + sizeof(UTF16_BOM_LE),
                        text_len - sizeof(UTF16_BOM_LE), ENC_UTF8,
                        ENC_UTF16_LE, NULL, &utf8_len, &err);
                } else {
                    /*
                     * 3.4 UTF-16 Byte Order
                     *
                     * ... If the BOM is omitted, the byte order shall be
                     * big-endian (UTF-16 BE).
                     */
                    utf8_buf = g_convert(text, text_len, ENC_UTF8,
                        ENC_UTF16_BE, NULL, &utf8_len, &err);
                }
                if (err) {
                    GWARN("Failed to decode Text record: %s", err->message);
                    g_free(utf8_buf); /* Should be NULL already */
                    g_error_free(err);
                    utf8 = NULL;
                } else {
                    utf8 = utf8_buf;
                }
            } else if (!text_len) {
                utf8 = "";
                utf8_buf = NULL;
            } else if (g_utf8_validate(text, text_len, NULL)) {
                utf8 = utf8_buf = g_strndup(text, text_len);
                utf8_len = text_len;
            } else {
                utf8 = NULL;
            }

            if (utf8) {
                NdefRecT* self = g_object_new(THIS_TYPE, NULL);
                NdefRecTPriv* priv = self->priv;

                ndef_rec_initialize(&self->rec, NDEF_RTD_TEXT, ndef);
                self->text = utf8;
                priv->text = utf8_buf;
                if (lang_len) {
                    self->lang = priv->lang = g_strndup(lang, lang_len);
                } else {
                    self->lang = "";
                }
                return self;
            }
        }
    }
    return NULL;
}

NdefRecT*
ndef_rec_t_new_enc(
    const char* text,
    const char* lang,
    NDEF_REC_T_ENC enc)
{
    GBytes* payload_bytes;
    char* lang_tmp = NULL;
    static const char lang_default[] = "en";
    static const char text_default[] = "";

    if (!lang) {
        NdefLanguage* system = ndef_system_language();

        if (system) {
            lang = lang_tmp = system->territory ?
                g_strconcat(system->language, "-", system->territory, NULL) :
                g_strdup(system->language);
            g_free(system);
            GDEBUG("System language: %s", lang);
        }
    }

    payload_bytes = ndef_rec_t_build(text ? text : text_default,
        lang ? lang : lang_default, enc);
    if (payload_bytes) {
        GUtilData payload;
        NdefRecT* self = THIS(ndef_rec_new_well_known(THIS_TYPE, NDEF_RTD_TEXT,
            &ndef_rec_type_t, gutil_data_from_bytes(&payload, payload_bytes)));
        NdefRecTPriv* priv = self->priv;

        /* Avoid unnecessary allocations */
        if (lang) {
            self->lang = priv->lang = (lang_tmp ? lang_tmp : g_strdup(lang));
        } else {
            self->lang = lang_default;
        }
        if (text) {
            self->text = priv->text = g_strdup(text);
        } else {
            self->text = text_default;
        }
        g_bytes_unref(payload_bytes);
        return self;
    }
    g_free(lang_tmp);
    return NULL;
}

NDEF_LANG_MATCH
ndef_rec_t_lang_match(
    NdefRecT* rec,
    const NdefLanguage* lang)
{
    NDEF_LANG_MATCH match = NDEF_LANG_MATCH_NONE;

    if (G_LIKELY(rec) && G_LIKELY(lang) && G_LIKELY(lang->language)) {
        const char* sep = strchr(rec->lang, '-');

        if (sep) {
            const gsize lang_len = sep - rec->lang;

            if (strlen(lang->language) == lang_len &&
                !g_ascii_strncasecmp(rec->lang, lang->language, lang_len)) {
                match |= NDEF_LANG_MATCH_LANGUAGE;
            }
            if (lang->territory && lang->territory[0] &&
                !g_ascii_strcasecmp(sep + 1, lang->territory)) {
                match |= NDEF_LANG_MATCH_TERRITORY;
            }
        } else {
            if (!g_ascii_strcasecmp(rec->lang, lang->language)) {
                match |= NDEF_LANG_MATCH_LANGUAGE;
            }
        }
    }
    return match;
}

gint
ndef_rec_t_lang_compare(
    gconstpointer a,
    gconstpointer b,
    gpointer user_data)
{
    /*
     * This function is passed the data from 2 elements of the GSList
     * and should return 0 if they are equal, a negative value if the
     * first element comes before the second, or a positive value if
     * the first element comes after the second.
     */
    NdefRecT* t1 = THIS(a);
    NdefRecT* t2 = THIS(b);
    const NdefLanguage* system = user_data;
    NDEF_LANG_MATCH match1 = ndef_rec_t_lang_match(t1, system);
    NDEF_LANG_MATCH match2 = ndef_rec_t_lang_match(t2, system);

    if (match1 != match2) {
        return (gint)match2 - (gint)match1;
    } else {
        NdefRec* r1 = &t1->rec;
        NdefRec* r2 = &t2->rec;

        /* Otherwise preserve the natural order */
        while (r1->next) {
            r1 = r1->next;
            if (r1 == r2) {
                /* r2 goes after r1 */
                return -1;
            }
        }

        /* r1 goes after r2 */
        return 1;
    }
}

char*
ndef_rec_t_steal_lang(
    NdefRecT* self)
{
    char* lang = NULL;

    if (G_LIKELY(self)) {
        NdefRecTPriv* priv = self->priv;

        lang = priv->lang;
        self->lang = priv->lang = NULL;
    }
    return lang;
}

char*
ndef_rec_t_steal_text(
    NdefRecT* self)
{
    char* text = NULL;

    if (G_LIKELY(self)) {
        NdefRecTPriv* priv = self->priv;

        text = priv->text;
        self->text = priv->text = NULL;
    }
    return text;
}

/*==========================================================================*
 * Internals
 *==========================================================================*/

static
void
ndef_rec_t_init(
    NdefRecT* self)
{
    self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self, THIS_TYPE, NdefRecTPriv);
}

static
void
ndef_rec_t_finalize(
    GObject* object)
{
    NdefRecT* self = THIS(object);
    NdefRecTPriv* priv = self->priv;

    g_free(priv->lang);
    g_free(priv->text);
    G_OBJECT_CLASS(PARENT_CLASS)->finalize(object);
}

static
void
ndef_rec_t_class_init(
    NdefRecTClass* klass)
{
    g_type_class_add_private(klass, sizeof(NdefRecTPriv));
    G_OBJECT_CLASS(klass)->finalize = ndef_rec_t_finalize;
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
