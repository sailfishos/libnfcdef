/*
 * Copyright (C) 2018-2023 Slava Monich <slava@monich.com>
 * Copyright (C) 2018-2021 Jolla Ltd.
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

#ifndef NDEF_REC_H
#define NDEF_REC_H

#include "ndef_types.h"

#include <glib-object.h>

G_BEGIN_DECLS

typedef enum nfc_ndef_rec_flags {
    NDEF_REC_FLAGS_NONE = 0x00,
    NDEF_REC_FLAG_FIRST = 0x01,     /* MB */
    NDEF_REC_FLAG_LAST = 0x02       /* ME */
} NDEF_REC_FLAGS;

/* Known record types (RTD = Record Type Definition) */
typedef enum nfc_ndef_rtd {
    NDEF_RTD_UNKNOWN,
    NDEF_RTD_URI,                   /* "U" */
    NDEF_RTD_TEXT,                  /* "T" */
    NDEF_RTD_SMART_POSTER           /* "Sp" */
} NDEF_RTD;

/* TNF = Type name format */
typedef enum nfc_ndef_tnf {
    NDEF_TNF_EMPTY,
    NDEF_TNF_WELL_KNOWN,
    NDEF_TNF_MEDIA_TYPE,
    NDEF_TNF_ABSOLUTE_URI,
    NDEF_TNF_EXTERNAL
} NDEF_TNF;

#define NDEF_TNF_MAX NDEF_TNF_EXTERNAL

typedef struct nfc_ndef_rec_priv NdefRecPriv;

struct nfc_ndef_rec {
    GObject object;
    NdefRecPriv* priv;
    NdefRec* next;
    NDEF_TNF tnf;
    NDEF_RTD rtd;
    NDEF_REC_FLAGS flags;
    GUtilData raw;
    GUtilData type;
    GUtilData id;
    GUtilData payload;
};

GType ndef_rec_get_type(void);
#define NDEF_TYPE_REC (ndef_rec_get_type())
#define NDEF_REC(obj) (G_TYPE_CHECK_INSTANCE_CAST(obj, \
        NDEF_TYPE_REC, NdefRec))

NdefRec*
ndef_rec_new(
    const GUtilData* block);

NdefRec*
ndef_rec_new_from_tlv(
    const GUtilData* tlv);

NdefRec*
ndef_rec_new_mediatype(
    const GUtilData* type,
    const GUtilData* payload);

NdefRec*
ndef_rec_ref(
    NdefRec* rec);

void
ndef_rec_unref(
    NdefRec* rec);

/* URI */

typedef struct nfc_ndef_rec_u_priv NdefRecUPriv;

struct nfc_ndef_rec_u {
    NdefRec rec;
    NdefRecUPriv* priv;
    const char* uri;
};

GType ndef_rec_u_get_type(void);
#define NDEF_TYPE_REC_U (ndef_rec_u_get_type())
#define NDEF_REC_U(obj) (G_TYPE_CHECK_INSTANCE_CAST(obj, \
        NDEF_TYPE_REC_U, NdefRecU))
#define NDEF_IS_REC_U(obj) G_TYPE_CHECK_INSTANCE_TYPE(obj, \
        NDEF_TYPE_REC_U)

NdefRecU*
ndef_rec_u_new(
    const char* uri);

/* Text */

typedef struct nfc_ndef_rec_t_priv NdefRecTPriv;

struct nfc_ndef_rec_t {
    NdefRec rec;
    NdefRecTPriv* priv;
    const char* lang;
    const char* text;
};

GType ndef_rec_t_get_type(void);
#define NDEF_TYPE_REC_T (ndef_rec_t_get_type())
#define NDEF_REC_T(obj) (G_TYPE_CHECK_INSTANCE_CAST(obj, \
        NDEF_TYPE_REC_T, NdefRecT))
#define NDEF_IS_REC_T(obj) G_TYPE_CHECK_INSTANCE_TYPE(obj, \
        NDEF_TYPE_REC_T)

typedef enum nfc_ndef_rec_t_enc {
    NDEF_REC_T_ENC_UTF8,
    NDEF_REC_T_ENC_UTF16BE,
    NDEF_REC_T_ENC_UTF16LE
} NDEF_REC_T_ENC;

typedef enum nfc_lang_match {
    NDEF_LANG_MATCH_NONE = 0x00,
    NDEF_LANG_MATCH_TERRITORY = 0x01,
    NDEF_LANG_MATCH_LANGUAGE = 0x02,
    NDEF_LANG_MATCH_FULL = NDEF_LANG_MATCH_LANGUAGE | NDEF_LANG_MATCH_TERRITORY
} NDEF_LANG_MATCH;

NdefRecT*
ndef_rec_t_new_enc(
    const char* text,
    const char* lang,
    NDEF_REC_T_ENC enc);

#define ndef_rec_t_new(text, lang) \
    ndef_rec_t_new_enc(text, lang, NDEF_REC_T_ENC_UTF8)

NDEF_LANG_MATCH
ndef_rec_t_lang_match(
    NdefRecT* rec,
    const NdefLanguage* lang);

gint
ndef_rec_t_lang_compare(
    gconstpointer a,   /* NdefRecT* */
    gconstpointer b,   /* NdefRecT* */
    gpointer user_data /* NdefLanguage* */);

/* Smart poster */

typedef enum nfc_ndef_sp_act {
    NDEF_SP_ACT_DEFAULT = -1, /* No action record */
    NDEF_SP_ACT_OPEN,         /* Perform the action */
    NDEF_SP_ACT_SAVE,         /* Save for later */
    NDEF_SP_ACT_EDIT          /* Open for editing */
} NDEF_SP_ACT;

typedef struct nfc_ndef_rec_sp_priv NdefRecSpPriv;

typedef struct nfc_ndef_media {
    GUtilData data;
    const char* type;
} NdefMedia;

struct nfc_ndef_rec_sp {
    NdefRec rec;
    NdefRecSpPriv* priv;
    const char* uri;
    const char* title;
    const char* lang;
    const char* type;
    guint size;
    NDEF_SP_ACT act;
    const NdefMedia* icon;
};

GType ndef_rec_sp_get_type(void);
#define NDEF_TYPE_REC_SP (ndef_rec_sp_get_type())
#define NDEF_REC_SP(obj) (G_TYPE_CHECK_INSTANCE_CAST(obj, \
        NDEF_TYPE_REC_SP, NdefRecSp))
#define NDEF_IS_REC_SP(obj) G_TYPE_CHECK_INSTANCE_TYPE(obj, \
        NDEF_TYPE_REC_SP)

NdefRecSp*
ndef_rec_sp_new(
    const char* uri,
    const char* title,
    const char* lang,
    const char* type,
    guint size,
    NDEF_SP_ACT act,
    const NdefMedia* icon);

/* Utilities */

gboolean
ndef_valid_mediatype(
    const GUtilData* type,
    gboolean wildcard);

gboolean
ndef_valid_mediatype_str(
    const char* type,
    gboolean wildcard);

G_END_DECLS

#endif /* NDEF_REC_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
