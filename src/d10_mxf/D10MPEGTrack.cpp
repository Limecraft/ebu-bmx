/*
 * Copyright (C) 2011, British Broadcasting Corporation
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <bmx/d10_mxf/D10MPEGTrack.h>
#include <bmx/Utils.h>
#include <bmx/BMXException.h>
#include <bmx/Logging.h>

using namespace std;
using namespace bmx;
using namespace mxfpp;



D10MPEGTrack::D10MPEGTrack(D10File *file, uint32_t track_index, mxfRational frame_rate, EssenceType essence_type)
: D10Track(file, track_index, frame_rate, essence_type)
{
    mD10DescriptorHelper = dynamic_cast<D10MXFDescriptorHelper*>(mDescriptorHelper);
    BMX_ASSERT(mD10DescriptorHelper);

    mWriterHelper.SetOutputMaxSampleSize(true);

    mD10DescriptorHelper->SetAspectRatio(ASPECT_RATIO_16_9);
}

D10MPEGTrack::~D10MPEGTrack()
{
}

void D10MPEGTrack::SetOutputTrackNumber(uint32_t track_number)
{
    (void)track_number;

    log_warn("Ignoring attempt to set D10 MPEG output track number\n");
}

void D10MPEGTrack::SetAspectRatio(mxfRational aspect_ratio)
{
    mD10DescriptorHelper->SetAspectRatio(aspect_ratio);
}

void D10MPEGTrack::SetAFD(uint8_t afd)
{
    mD10DescriptorHelper->SetAFD(afd);
}

FileDescriptor* D10MPEGTrack::CreateFileDescriptor(HeaderMetadata *header_metadata)
{
    return mD10DescriptorHelper->CreateFileDescriptor(header_metadata);
}

void D10MPEGTrack::PrepareWrite()
{
    uint32_t max_sample_size = mD10DescriptorHelper->GetMaxSampleSize();
    mWriterHelper.SetMaxSampleSize(max_sample_size);
    mD10DescriptorHelper->SetSampleSize(max_sample_size);

    mCPManager->RegisterMPEGTrackElement(mTrackIndex, max_sample_size);
}

void D10MPEGTrack::WriteSamplesInt(const unsigned char *data, uint32_t size, uint32_t num_samples)
{
    BMX_CHECK(size > 0 && num_samples > 0);

    const CDataBuffer *data_array;
    uint32_t data_array_size;
    uint32_t sample_size = size / num_samples;
    uint32_t i;
    for (i = 0; i < num_samples; i++) {
        mWriterHelper.ProcessFrame(&data[i * sample_size], sample_size, &data_array, &data_array_size);
        D10Track::WriteSampleInt(data_array, data_array_size);
    }
}

