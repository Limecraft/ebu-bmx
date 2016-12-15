/*
 * Copyright (C) 2013, British Broadcasting Corporation
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

#ifndef BMX_AS11_WRITER_HELPER_H_
#define BMX_AS11_WRITER_HELPER_H_


#include <bmx/clip_writer/ClipWriter.h>
#include <bmx/as11/AS11Labels.h>
#include <bmx/as11/AS11CoreFramework.h>
#include <bmx/as11/UKDPPFramework.h>
#include <bmx/apps/AppMCALabelHelper.h>



namespace bmx
{


typedef struct
{
    int64_t start;
    int64_t duration;
    uint16_t part_number;
    uint16_t part_total;
} AS11PosSegment;

typedef struct
{
    Timecode start;
    int64_t duration;
    uint16_t part_number;
    uint16_t part_total;
} AS11TCSegment;


class AS11WriterHelper
{
public:
    static bool IndexAS11MCALabels(AppMCALabelHelper *labels_helper);

public:
    AS11WriterHelper(ClipWriter *clip);
    ~AS11WriterHelper();

    void SetSpecificationId(AS11SpecificationId spec_id);

    void InsertAS11CoreFramework(AS11CoreFramework *framework);
    void InsertUKDPPFramework(UKDPPFramework *framework);

    void InsertPosSegmentation(std::vector<AS11PosSegment> segments);
    void InsertTCSegmentation(std::vector<AS11TCSegment> segments);
    void CompleteSegmentation(bool with_filler);
    uint16_t GetTotalSegments();
    int64_t GetTotalSegmentDuration();

    ClipWriter* GetClip() const { return mClip; }

private:
    void AppendDMSLabel(mxfUL scheme_label);
    void InsertFramework(uint32_t track_id, std::string track_name, mxfpp::DMFramework *framework);

private:
    ClipWriter *mClip;
    mxfpp::Sequence *mSegmentationSequence;
};


};



#endif

