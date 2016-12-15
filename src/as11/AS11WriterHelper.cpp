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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define __STDC_FORMAT_MACROS

#include <bmx/as11/AS11WriterHelper.h>
#include <bmx/as11/AS11SegmentationFramework.h>
#include <bmx/as11/AS11DMS.h>
#include <bmx/as11/AS11Info.h>
#include <bmx/as11/UKDPPDMS.h>
#include <bmx/MXFUtils.h>
#include <bmx/Utils.h>
#include <bmx/BMXException.h>
#include <bmx/Logging.h>

using namespace std;
using namespace bmx;
using namespace mxfpp;


static const MCALabelEntry AS11_MCA_LABELS[] =
{
    {AUDIO_CHANNEL_LABEL,               "chADSSdc",   "AD Studio Signal Data Channel",
        {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x08, 0x01, 0x01, 0x01, 0x01, 0x00}},
    {SOUNDFIELD_GROUP_LABEL,            "sgADSS",     "AD Studio Signal",
        {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x08, 0x01, 0x01, 0x02, 0x01, 0x00}},
    {GROUP_OF_SOUNDFIELD_GROUP_LABEL,   "ggAPg",      "Alternative Program",
        {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x08, 0x01, 0x01, 0x03, 0x01, 0x00}},
    {GROUP_OF_SOUNDFIELD_GROUP_LABEL,   "ggADPgMx",   "Audio Description Program Mix",
        {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x08, 0x01, 0x01, 0x03, 0x02, 0x00}},
    {GROUP_OF_SOUNDFIELD_GROUP_LABEL,   "ggAD",       "Audio Description",
        {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x08, 0x01, 0x01, 0x03, 0x03, 0x00}},
    {GROUP_OF_SOUNDFIELD_GROUP_LABEL,   "ggME",       "Music and Effects",
        {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x08, 0x01, 0x01, 0x03, 0x04, 0x00}},
    {GROUP_OF_SOUNDFIELD_GROUP_LABEL,   "ggUnAu",      "Unused Audio",
        {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x01, 0x0d, 0x01, 0x08, 0x01, 0x01, 0x03, 0x05, 0x00}},
    {AUDIO_CHANNEL_LABEL,               "chL",        "Left",
        {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x0d, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00}},
    {AUDIO_CHANNEL_LABEL,               "chR",        "Right",
        {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x0d, 0x03, 0x02, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00}},
    {AUDIO_CHANNEL_LABEL,               "chC",        "Center",
        {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x0d, 0x03, 0x02, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00}},
    {AUDIO_CHANNEL_LABEL,               "chLFE",      "LFE",
        {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x0d, 0x03, 0x02, 0x01, 0x04, 0x00, 0x00, 0x00, 0x00}},
    {AUDIO_CHANNEL_LABEL,               "chLs",       "Left Surround",
        {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x0d, 0x03, 0x02, 0x01, 0x05, 0x00, 0x00, 0x00, 0x00}},
    {AUDIO_CHANNEL_LABEL,               "chRs",       "Right Surround",
        {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x0d, 0x03, 0x02, 0x01, 0x06, 0x00, 0x00, 0x00, 0x00}},
    {AUDIO_CHANNEL_LABEL,               "chVIN",      "Visually Impaired Narrative",
        {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x0d, 0x03, 0x02, 0x01, 0x0f, 0x00, 0x00, 0x00, 0x00}},
    {SOUNDFIELD_GROUP_LABEL,            "sg51",       "5.1",
        {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x0d, 0x03, 0x02, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00}},
    {SOUNDFIELD_GROUP_LABEL,            "sgST",       "Standard Stereo",
        {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x0d, 0x03, 0x02, 0x02, 0x20, 0x01, 0x00, 0x00, 0x00}},
    {GROUP_OF_SOUNDFIELD_GROUP_LABEL,   "ggMPg",      "Main Program",
        {0x06, 0x0e, 0x2b, 0x34, 0x04, 0x01, 0x01, 0x0d, 0x03, 0x02, 0x03, 0x20, 0x01, 0x00, 0x00, 0x00}},
};

static const char *AS11_CORE_TRACK_NAME         = "AS_11_Core";
static const char *AS11_SEGMENTATION_TRACK_NAME = "AS_11_Segmentation";
static const char *AS11_UKDPP_TRACK_NAME        = "AS_11_UKDPP";


static int64_t get_offset(uint16_t to_tc_base, uint16_t from_tc_base, int64_t from_offset)
{
    return convert_position(from_offset, to_tc_base, from_tc_base, ROUND_AUTO);
}



bool AS11WriterHelper::IndexAS11MCALabels(AppMCALabelHelper *labels_helper)
{
    return labels_helper->IndexLabels(AS11_MCA_LABELS, BMX_ARRAY_SIZE(AS11_MCA_LABELS));
}

AS11WriterHelper::AS11WriterHelper(ClipWriter *clip)
{
    mClip = clip;
    mSegmentationSequence = 0;

    AS11Info::RegisterExtensions(clip->GetHeaderMetadata());

    UniqueIdHelper *track_id_helper = clip->GetTrackIdHelper();
    BMX_ASSERT(track_id_helper);
    track_id_helper->SetId(AS11_CORE_TRACK_NAME,         5001);
    track_id_helper->SetId(AS11_SEGMENTATION_TRACK_NAME, 5002);
    track_id_helper->SetId(AS11_UKDPP_TRACK_NAME,        5101);
}

AS11WriterHelper::~AS11WriterHelper()
{
}

void AS11WriterHelper::SetSpecificationId(AS11SpecificationId spec_id)
{
    HeaderMetadata *header_metadata = mClip->GetHeaderMetadata();
    BMX_ASSERT(header_metadata);

    Preface *preface = header_metadata->getPreface();
    switch (spec_id)
    {
        case AS11_X1_SPEC:
            preface->appendULArrayItem(&MXF_ITEM_K(Preface, SpecificationIdentifiers), AS11_BLOCKS_FF_0);
            preface->appendULArrayItem(&MXF_ITEM_K(Preface, SpecificationIdentifiers), AS11_BLOCKS_FF_1_WIP);
            break;
        case AS11_X2_SPEC:
            preface->appendULArrayItem(&MXF_ITEM_K(Preface, SpecificationIdentifiers), AS11_BLOCKS_FF_0);
            preface->appendULArrayItem(&MXF_ITEM_K(Preface, SpecificationIdentifiers), AS11_BLOCKS_FF_2);
            break;
        case AS11_X3_SPEC:
            preface->appendULArrayItem(&MXF_ITEM_K(Preface, SpecificationIdentifiers), AS11_BLOCKS_FF_0);
            preface->appendULArrayItem(&MXF_ITEM_K(Preface, SpecificationIdentifiers), AS11_BLOCKS_FF_5_WIP);
            break;
        case AS11_X4_SPEC:
            preface->appendULArrayItem(&MXF_ITEM_K(Preface, SpecificationIdentifiers), AS11_BLOCKS_FF_0);
            preface->appendULArrayItem(&MXF_ITEM_K(Preface, SpecificationIdentifiers), AS11_BLOCKS_FF_6_WIP);
            break;
        case AS11_X7_SPEC:
            preface->appendULArrayItem(&MXF_ITEM_K(Preface, SpecificationIdentifiers), AS11_BLOCKS_FF_7);
            break;
        case AS11_X8_SPEC:
            preface->appendULArrayItem(&MXF_ITEM_K(Preface, SpecificationIdentifiers), AS11_BLOCKS_FF_9_WIP);
            break;
        case UNKNOWN_AS11_SPEC:
            break;
    }
}

void AS11WriterHelper::InsertAS11CoreFramework(AS11CoreFramework *framework)
{
    AppendDMSLabel(MXF_DM_L(AS11CoreDescriptiveScheme));
    InsertFramework(mClip->GetTrackIdHelper()->GetId(AS11_CORE_TRACK_NAME), AS11_CORE_TRACK_NAME, framework);
}

void AS11WriterHelper::InsertUKDPPFramework(UKDPPFramework *framework)
{
    AppendDMSLabel(MXF_DM_L(UKDPPDescriptiveScheme));
    InsertFramework(mClip->GetTrackIdHelper()->GetId(AS11_UKDPP_TRACK_NAME), AS11_UKDPP_TRACK_NAME, framework);
}

void AS11WriterHelper::InsertPosSegmentation(vector<AS11PosSegment> segments)
{
    BMX_ASSERT(!mSegmentationSequence);

    AppendDMSLabel(MXF_DM_L(AS11SegmentationDescriptiveScheme));

    HeaderMetadata *header_metadata = mClip->GetHeaderMetadata();
    BMX_ASSERT(header_metadata);

    MaterialPackage *material_package = header_metadata->getPreface()->findMaterialPackage();
    BMX_ASSERT(material_package);

    // Preface - ContentStorage - Package - DM Track
    Track *dm_track = new Track(header_metadata);
    material_package->appendTracks(dm_track);
    dm_track->setTrackName(AS11_SEGMENTATION_TRACK_NAME);
    dm_track->setTrackID(mClip->GetTrackIdHelper()->GetId(AS11_SEGMENTATION_TRACK_NAME));
    dm_track->setTrackNumber(0);
    dm_track->setEditRate(mClip->GetFrameRate());
    dm_track->setOrigin(0);

    // Preface - ContentStorage - Package - DM Track - Sequence
    mSegmentationSequence = new Sequence(header_metadata);
    dm_track->setSequence(mSegmentationSequence);
    mSegmentationSequence->setDataDefinition(MXF_DDEF_L(DescriptiveMetadata));
    // duration is set below

    int64_t next_start = 0;
    size_t i;
    for (i = 0; i < segments.size(); i++) {
        BMX_CHECK_M(segments[i].start >= next_start,
                   ("AS11 segment starts (%" PRId64 ") before end of last segment (%" PRId64 ")",
                    segments[i].start, next_start - 1));

        if (segments[i].start > next_start) {
            // Preface - ContentStorage - Package - DM Track - Sequence - Filler
            StructuralComponent *component = dynamic_cast<StructuralComponent*>(
                header_metadata->createAndWrap(&MXF_SET_K(Filler)));
            mSegmentationSequence->appendStructuralComponents(component);
            component->setDataDefinition(MXF_DDEF_L(DescriptiveMetadata));
            component->setDuration(segments[i].start - next_start);
        }

        // Preface - ContentStorage - Package - DM Track - Sequence - DMSegment
        DMSegment *dm_segment = new DMSegment(header_metadata);
        mSegmentationSequence->appendStructuralComponents(dm_segment);
        dm_segment->setDataDefinition(MXF_DDEF_L(DescriptiveMetadata));
        dm_segment->setDuration(segments[i].duration);

        // Preface - ContentStorage - Package - DM Track - Sequence - DMSegment - DMFramework
        AS11SegmentationFramework *framework = new AS11SegmentationFramework(header_metadata);
        framework->SetPartNumber(segments[i].part_number);
        framework->SetPartTotal(segments[i].part_total);
        dm_segment->setDMFramework(framework);

        next_start = segments[i].start + segments[i].duration;
    }

    mSegmentationSequence->setDuration(next_start);
}

void AS11WriterHelper::InsertTCSegmentation(vector<AS11TCSegment> segments)
{
    vector<AS11PosSegment> pos_segments;

    Timecode start_timecode = mClip->GetStartTimecode();

    uint16_t to_tc_base = get_rounded_tc_base(mClip->GetFrameRate());
    int64_t start_offset = get_offset(to_tc_base, start_timecode.GetRoundedTCBase(), start_timecode.GetOffset());
    int64_t offset;
    size_t i;
    for (i = 0; i < segments.size(); i++) {
        offset = get_offset(to_tc_base, segments[i].start.GetRoundedTCBase(), segments[i].start.GetOffset());
        if (offset < start_offset) {
            // assume passed midnight
            offset = get_offset(to_tc_base, segments[i].start.GetRoundedTCBase(),
                                segments[i].start.GetOffset() + segments[i].start.GetMaxOffset());
        }

        AS11PosSegment pos_segment;
        pos_segment.start = offset - start_offset;
        pos_segment.duration = segments[i].duration;
        pos_segment.part_number = segments[i].part_number;
        pos_segment.part_total = segments[i].part_total;
        pos_segments.push_back(pos_segment);
    }

    InsertPosSegmentation(pos_segments);
}

void AS11WriterHelper::CompleteSegmentation(bool with_filler)
{
    if (!mSegmentationSequence)
        return;

    int64_t clip_duration = mClip->GetInputDuration();
    if (clip_duration < 0)
        clip_duration = mClip->GetDuration();

    BMX_CHECK_M(mSegmentationSequence->getDuration() <= clip_duration,
                ("AS-11 segmentation duration (%" PRId64 ") exceeds package duration (%" PRId64 ")",
                 mSegmentationSequence->getDuration(), clip_duration));
    if (mSegmentationSequence->getDuration() == clip_duration)
        return;

    HeaderMetadata *header_metadata = mClip->GetHeaderMetadata();
    BMX_ASSERT(header_metadata);

    vector<StructuralComponent*> components = mSegmentationSequence->getStructuralComponents();

    if (with_filler || components.empty()) {
        // Preface - ContentStorage - Package - DM Track - Sequence - Filler
        StructuralComponent *component = dynamic_cast<StructuralComponent*>(
            header_metadata->createAndWrap(&MXF_SET_K(Filler)));
        mSegmentationSequence->appendStructuralComponents(component);
        component->setDataDefinition(MXF_DDEF_L(DescriptiveMetadata));
        component->setDuration(clip_duration - mSegmentationSequence->getDuration());
    } else {
        components.back()->setDuration(clip_duration - mSegmentationSequence->getDuration());
    }

    mSegmentationSequence->setDuration(clip_duration);
}

uint16_t AS11WriterHelper::GetTotalSegments()
{
    if (!mSegmentationSequence)
        return 0;

    uint16_t total_segments = 0;
    vector<StructuralComponent*> components = mSegmentationSequence->getStructuralComponents();
    size_t i;
    for (i = 0; i < components.size(); i++) {
        if (*components[i]->getKey() != MXF_SET_K(Filler))
            total_segments++;
    }

    return total_segments;
}

int64_t AS11WriterHelper::GetTotalSegmentDuration()
{
    if (!mSegmentationSequence)
        return 0;

    int64_t total_duration = 0;
    vector<StructuralComponent*> components = mSegmentationSequence->getStructuralComponents();
    size_t i;
    for (i = 0; i < components.size(); i++) {
        if (components[i]->haveDuration() && *components[i]->getKey() != MXF_SET_K(Filler))
            total_duration += components[i]->getDuration();
    }

    return total_duration;
}

void AS11WriterHelper::AppendDMSLabel(mxfUL scheme_label)
{
    HeaderMetadata *header_metadata = mClip->GetHeaderMetadata();
    BMX_ASSERT(header_metadata);

    vector<mxfUL> dm_schemes = header_metadata->getPreface()->getDMSchemes();
    size_t i;
    for (i = 0; i < dm_schemes.size(); i++) {
        if (mxf_equals_ul_mod_regver(&dm_schemes[i], &scheme_label))
            break;
    }
    if (i >= dm_schemes.size())
        header_metadata->getPreface()->appendDMSchemes(scheme_label);
}

void AS11WriterHelper::InsertFramework(uint32_t track_id, string track_name, DMFramework *framework)
{
    HeaderMetadata *header_metadata = mClip->GetHeaderMetadata();
    BMX_ASSERT(header_metadata);

    MaterialPackage *material_package = header_metadata->getPreface()->findMaterialPackage();
    BMX_ASSERT(material_package);

    // Preface - ContentStorage - Package - DM Track
    StaticTrack *dm_track = new StaticTrack(header_metadata);
    material_package->appendTracks(dm_track);
    dm_track->setTrackName(track_name);
    dm_track->setTrackID(track_id);
    dm_track->setTrackNumber(0);

    // Preface - ContentStorage - Package - DM Track - Sequence
    Sequence *sequence = new Sequence(header_metadata);
    dm_track->setSequence(sequence);
    sequence->setDataDefinition(MXF_DDEF_L(DescriptiveMetadata));

    // Preface - ContentStorage - Package - DM Track - Sequence - DMSegment
    DMSegment *dm_segment = new DMSegment(header_metadata);
    sequence->appendStructuralComponents(dm_segment);
    dm_segment->setDataDefinition(MXF_DDEF_L(DescriptiveMetadata));

    // move the framework set after the dm degment set
    mxf_remove_set(header_metadata->getCHeaderMetadata(), framework->getCMetadataSet());
    BMX_CHECK(mxf_add_set(header_metadata->getCHeaderMetadata(), framework->getCMetadataSet()));

    // Preface - ContentStorage - Package - DM Track - Sequence - DMSegment - DMFramework
    dm_segment->setDMFramework(framework);
}

