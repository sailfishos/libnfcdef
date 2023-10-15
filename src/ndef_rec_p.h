/*
 * Copyright (C) 2018-2023 Slava Monich <slava@monich.com>
 * Copyright (C) 2018-2020 Jolla Ltd.
 * Copyright (C) 2018 Bogdan Pankovsky <b.pankovsky@omprussia.ru>
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

#ifndef NDEF_REC_PRIVATE_H
#define NDEF_REC_PRIVATE_H

#include "ndef_types.h"
#include "ndef_rec.h"

typedef struct ndef_rec_class {
    GObjectClass parent;
} NdefRecClass;

/* Pre-parsed NDEF record */
typedef struct ndef_data {
    GUtilData rec;
    guint type_offset;
    guint type_length;
    guint id_length;
    guint payload_length;
} NdefData;

#define NDEF_HDR_MB       (0x80)
#define NDEF_HDR_ME       (0x40)
#define NDEF_HDR_CF       (0x20)
#define NDEF_HDR_SR       (0x10)
#define NDEF_HDR_IL       (0x08)
#define NDEF_HDR_TNF_MASK (0x07)

extern const GUtilData ndef_rec_type_u G_GNUC_INTERNAL; /* "U" */
extern const GUtilData ndef_rec_type_t G_GNUC_INTERNAL; /* "T" */
extern const GUtilData ndef_rec_type_sp G_GNUC_INTERNAL; /* "Sp" */

gboolean
ndef_type(
    const NdefData* data,
    GUtilData* type)
    G_GNUC_INTERNAL;

gboolean
ndef_payload(
    const NdefData* data,
    GUtilData* payload)
    G_GNUC_INTERNAL;

NdefRec*
ndef_rec_initialize(
    NdefRec* rec,
    NDEF_RTD rtd,
    const NdefData* ndef)
    G_GNUC_INTERNAL;

void
ndef_rec_clear_flags(
    NdefRec* rec,
    NDEF_REC_FLAGS flags)
    G_GNUC_INTERNAL;

NdefRec*
ndef_rec_new_well_known(
    GType gtype,
    NDEF_RTD rtd,
    const GUtilData* type,
    const GUtilData* payload)
    G_GNUC_INTERNAL;

NdefRecU*
ndef_rec_u_new_from_data(
    const NdefData* ndef)
    G_GNUC_INTERNAL;

char*
ndef_rec_u_steal_uri(
    NdefRecU* ndef)
    G_GNUC_INTERNAL;

NdefRecT*
ndef_rec_t_new_from_data(
    const NdefData* ndef)
    G_GNUC_INTERNAL;

char*
ndef_rec_t_steal_lang(
    NdefRecT* self)
    G_GNUC_INTERNAL;

char*
ndef_rec_t_steal_text(
    NdefRecT* self)
    G_GNUC_INTERNAL;

NdefRecSp*
ndef_rec_sp_new_from_data(
    const NdefData* ndef)
    G_GNUC_INTERNAL;

#endif /* NDEF_REC_PRIVATE_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
