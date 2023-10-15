/*
 * Copyright (C) 2018-2023 Slava Monich <slava@monich.com>
 * Copyright (C) 2018 Jolla Ltd.
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

#include "ndef_tlv.h"

/*
 * TLV iterator. Usage:
 *
 * guint type;
 * GUtilData buf;
 * GUtilData value;
 *
 * ... Initialize buf
 *
 * while ((type = ndef_tlv_next(&buf, &value)) > 0) {
 *   ... analize type and value
 * }
 */
guint
ndef_tlv_next(
    GUtilData* buf,
    GUtilData* value)
{
    value->bytes = NULL;
    value->size = 0;

    while (buf->size > 0) {
        switch (buf->bytes[0]) {
        case TLV_NULL:
            /* No L, no V */
            buf->bytes++;
            buf->size--;
            break;
        case TLV_TERMINATOR:
            /* No L, no V */
            buf->bytes++;
            buf->size--;
            return 0;
        default:
            if (buf->size > 1) {
                /* Assume one byte format */
                guint len = buf->bytes[1];
                guint lsize = 1;
                guint tlvsize;

                if (len == 0xff) {
                    /* Three consecutive bytes format */
                    if (buf->size > 3) {
                        /* Big endian */
                        len = (((guint)buf->bytes[2]) << 8) | buf->bytes[3];
                        lsize = 3;
                    } else {
                        return 0;
                    }
                }

                tlvsize = 1 + lsize + len;
                if (buf->size >= tlvsize) {
                    guint type = buf->bytes[0];

                    /* Valid TLV block found */
                    value->bytes = buf->bytes + (1 + lsize);
                    value->size = len;
                    buf->bytes += tlvsize;
                    buf->size -= tlvsize;
                    return type;
                }
            }
            return 0;
        }
    }
    return 0;
}

/*
 * ndef_tlv_check() returns the actual size of TLV sequence including
 * TLV_TERMINATOR, zero if the sequence is incomplete or optentially
 * broken.
 */
gsize
ndef_tlv_check(
    const GUtilData* buf)
{
    if (buf) {
        GUtilData it = *buf;
        GUtilData value;

        while (ndef_tlv_next(&it, &value) > 0);
        if (it.bytes > buf->bytes && it.bytes[-1] == TLV_TERMINATOR) {
            return it.bytes - buf->bytes;
        }
    }
    return 0;
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
