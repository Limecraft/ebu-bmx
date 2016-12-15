/*
 * Copyright (C) 2016, British Broadcasting Corporation
 * All Rights Reserved.
 *
 * Author: Philip de Nier
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the British Broadcasting Corporation nor the names
 *       of its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef AS11_LABELS_H_
#define AS11_LABELS_H_


#include <bmx/BMXTypes.h>

#if !defined(MXF_ITEM_K)
#define MXF_ITEM_K(setname, name)   g_##setname##_##name##_item_key
#endif


namespace bmx
{


typedef enum
{
    UNKNOWN_AS11_SPEC,
    AS11_X1_SPEC,
    AS11_X2_SPEC,
    AS11_X3_SPEC,
    AS11_X4_SPEC,
    AS11_X7_SPEC,
    AS11_X8_SPEC,
} AS11SpecificationId;



static const UL AS11_DM_XML_Document =
    {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x08, 0x01, 0x04, 0x01, 0x00, 0x00};


// File format specification identification labels

static const UL AS11_BLOCKS_FF_0 =
    {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x08, 0x01, 0x05, 0x01, 0x00, 0x00};

static const UL AS11_BLOCKS_FF_1_WIP =
    {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x08, 0x01, 0x03, 0x02, 0x00, 0x00};

static const UL AS11_BLOCKS_FF_2 =
    {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x08, 0x01, 0x05, 0x03, 0x00, 0x00};

static const UL AS11_BLOCKS_FF_5_WIP =
    {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x08, 0x01, 0x03, 0x06, 0x00, 0x00};

static const UL AS11_BLOCKS_FF_6_WIP =
    {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x08, 0x01, 0x03, 0x07, 0x00, 0x00};

static const UL AS11_BLOCKS_FF_7 =
    {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x08, 0x01, 0x05, 0x08, 0x00, 0x00};

static const UL AS11_BLOCKS_FF_9_WIP =
    {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x08, 0x01, 0x03, 0x0a, 0x00, 0x00};


static const UL MXF_ITEM_K(Preface, SpecificationIdentifiers) =
    {0x06, 0x0e, 0x2b, 0x34, 0x01, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x08, 0x01, 0x01, 0x01, 0x00, 0x00};


// Channel assignment labels

static const UL CONSTRAINED_MCA_LABEL_FRAMEWORK =
    {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x08, 0x01, 0x02, 0x01, 0x00, 0x00};

static const UL CONSTRAINED_MCA_LABEL_WITH_DEFAULT_LAYOUT_A =
    {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x08, 0x01, 0x02, 0x02, 0x00, 0x00};

static const UL DEFAULT_LAYOUT_A_WITHOUT_MCA_LABEL =
    {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x08, 0x01, 0x02, 0x03, 0x00, 0x00};


};


#endif
