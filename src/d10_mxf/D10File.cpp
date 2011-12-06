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

#define __STDC_FORMAT_MACROS

#include <cstdio>
#include <cstring>

#include <im/d10_mxf/D10File.h>
#include <im/mxf_helper/MXFDescriptorHelper.h>
#include <im/MXFUtils.h>
#include <im/Version.h>
#include <im/IMException.h>
#include <im/Logging.h>

using namespace std;
using namespace im;
using namespace mxfpp;



static const uint32_t KAG_SIZE = 0x200;

static const uint32_t INDEX_SID = 1;
static const uint32_t BODY_SID  = 2;

static const mxfRational AUDIO_SAMPLING_RATE = {48000, 1};



typedef struct
{
    EssenceType essence_type;
    mxfRational frame_rate;
    mxfUL ec_label;
} EssenceContainerULTable;

static const EssenceContainerULTable ESS_CONTAINER_UL_TABLE[] =
{
    {D10_30,   {25, 1},        MXF_EC_L(D10_30_625_50_defined_template)},
    {D10_30,   {30000, 1001},  MXF_EC_L(D10_30_525_60_defined_template)},
    {D10_40,   {25, 1},        MXF_EC_L(D10_40_625_50_defined_template)},
    {D10_40,   {30000, 1001},  MXF_EC_L(D10_40_525_60_defined_template)},
    {D10_50,   {25, 1},        MXF_EC_L(D10_50_625_50_defined_template)},
    {D10_50,   {30000, 1001},  MXF_EC_L(D10_50_525_60_defined_template)},
};



static mxfUL get_essence_container_ul(EssenceType essence_type, mxfRational frame_rate)
{
    size_t i;
    for (i = 0; i < ARRAY_SIZE(ESS_CONTAINER_UL_TABLE); i++) {
        if (ESS_CONTAINER_UL_TABLE[i].essence_type == essence_type &&
            ESS_CONTAINER_UL_TABLE[i].frame_rate == frame_rate)
        {
            return ESS_CONTAINER_UL_TABLE[i].ec_label;
        }
    }

    IM_ASSERT(false);
    return ESS_CONTAINER_UL_TABLE[0].ec_label;
}



D10File* D10File::OpenNew(string filename, mxfRational frame_rate)
{
    return new D10File(File::openNew(filename), frame_rate);
}

D10File::D10File(mxfpp::File *mxf_file, mxfRational frame_rate)
{
    IM_CHECK((frame_rate.numerator == 25    && frame_rate.denominator == 1) ||
             (frame_rate.numerator == 30000 && frame_rate.denominator == 1001));

    mMXFFile = mxf_file;
    mFrameRate = frame_rate;
    mStartTimecode = Timecode(frame_rate, false);
    mCompanyName = get_im_company_name();
    mProductName = get_im_library_name();
    mProductVersion = get_im_mxf_product_version();
    mVersionString = get_im_version_string();
    mProductUID = get_im_product_uid();
    mReserveMinBytes = 8192;
    mxf_get_timestamp_now(&mCreationDate);
    mxf_generate_uuid(&mGenerationUID);
    mxf_generate_umid(&mMaterialPackageUID);
    mxf_generate_umid(&mFileSourcePackageUID);
    mPictureTrack = 0;
    mFirstSoundTrack = 0;
    mSoundTrackCount = 0;
    mMaxSoundOutputTrackNumber = 0;
    memset(&mEssenceContainerUL, 0, sizeof(mEssenceContainerUL));
    mDataModel = 0;
    mHeaderMetadata = 0;
    mHeaderMetadataStartPos = 0;
    mHeaderMetadataEndPos = 0;
    mCBEIndexTableStartPos = 0;
    mMaterialPackage = 0;
    mFileSourcePackage = 0;
    mCPManager = new D10ContentPackageManager(frame_rate);
    mIndexSegment = 0;
}

D10File::~D10File()
{
    size_t i;
    for (i = 0; i < mTracks.size(); i++)
        delete mTracks[i];

    delete mMXFFile;
    delete mDataModel;
    delete mHeaderMetadata;
    delete mCPManager;
    delete mIndexSegment;
}

void D10File::SetClipName(string name)
{
    mClipName = name;
}

void D10File::SetStartTimecode(Timecode start_timecode)
{
    mStartTimecode = start_timecode;
}

void D10File::SetHaveInputUserTimecode(bool enable)
{
    mCPManager->SetHaveInputUserTimecode(enable);
}

void D10File::SetSoundSequenceOffset(uint8_t offset)
{
    mCPManager->SetSoundSequenceOffset(offset);
}

void D10File::SetProductInfo(string company_name, string product_name, mxfProductVersion product_version,
                             string version, mxfUUID product_uid)
{
    mCompanyName = company_name;
    mProductName = product_name;
    mProductVersion = product_version;
    mVersionString = version;
    mProductUID = product_uid;
}

void D10File::SetCreationDate(mxfTimestamp creation_date)
{
    mCreationDate = creation_date;
}

void D10File::SetGenerationUID(mxfUUID generation_uid)
{
    mGenerationUID = generation_uid;
}

void D10File::SetMaterialPackageUID(mxfUMID package_uid)
{
    mMaterialPackageUID = package_uid;
}

void D10File::SetFileSourcePackageUID(mxfUMID package_uid)
{
    mFileSourcePackageUID = package_uid;
}

void D10File::ReserveHeaderMetadataSpace(uint32_t min_bytes)
{
    mReserveMinBytes = min_bytes;
}

D10Track* D10File::CreateTrack(EssenceType essence_type)
{
    uint32_t track_index = mTracks.size();
    mTracks.push_back(D10Track::Create(this, track_index, mFrameRate, essence_type));
    mTrackMap[track_index] = mTracks.back();

    if (mTracks.back()->IsPicture()) {
        IM_CHECK(!mPictureTrack);
        mPictureTrack = dynamic_cast<D10MPEGTrack*>(mTracks.back());
    } else if (!mFirstSoundTrack) {
        mFirstSoundTrack = dynamic_cast<D10PCMTrack*>(mTracks.back());
    }

    return mTracks.back();
}

void D10File::PrepareHeaderMetadata()
{
    if (mHeaderMetadata)
        return;

    IM_CHECK(mPictureTrack);

    mEssenceContainerUL = get_essence_container_ul(mPictureTrack->GetEssenceType(), mFrameRate);

    uint32_t last_sound_track_number = 0;
    size_t i;
    for (i = 0; i < mTracks.size(); i++) {
        if (!mTracks[i]->IsPicture()) {
            if (!mTracks[i]->IsOutputTrackNumberSet())
                mTracks[i]->SetOutputTrackNumber(last_sound_track_number + 1);
            last_sound_track_number = mTracks[i]->GetOutputTrackNumber();

            if (mTracks[i]->GetOutputTrackNumber() > mMaxSoundOutputTrackNumber)
                mMaxSoundOutputTrackNumber = mTracks[i]->GetOutputTrackNumber();
        }
    }

    for (i = 0; i < mTracks.size(); i++)
        mTracks[i]->PrepareWrite();

    mCPManager->SetEssenceContainerUL(mEssenceContainerUL);
    mCPManager->PrepareWrite();

    CreateHeaderMetadata();
}

void D10File::PrepareWrite()
{
    if (!mHeaderMetadata)
        PrepareHeaderMetadata();

    CreateFile();
}

void D10File::WriteUserTimecode(Timecode user_timecode)
{
    mCPManager->WriteUserTimecode(user_timecode);

    while (mCPManager->HaveContentPackage())
        mCPManager->WriteNextContentPackage(mMXFFile);
}

void D10File::WriteSamples(uint32_t track_index, const unsigned char *data, uint32_t size, uint32_t num_samples)
{
    if (!data || size == 0)
        return;
    IM_CHECK(data && size && num_samples);

    GetTrack(track_index)->WriteSamplesInt(data, size, num_samples);

    while (mCPManager->HaveContentPackage())
        mCPManager->WriteNextContentPackage(mMXFFile);
}

void D10File::CompleteWrite()
{
    IM_ASSERT(mMXFFile);

    // write remaining content packages if duration < sound sample sequence size

    mCPManager->FinalWrite(mMXFFile);


    // write the footer partition pack

    Partition &footer_partition = mMXFFile->createPartition();
    footer_partition.setKey(&MXF_PP_K(ClosedComplete, Footer));
    footer_partition.setIndexSID(0);
    footer_partition.setBodySID(0);
    footer_partition.write(mMXFFile);
    footer_partition.fillToKag(mMXFFile);


    // update metadata sets with duration

    UpdatePackageMetadata();


    // re-write the header metadata in the header partition

    mMXFFile->seek(mHeaderMetadataStartPos, SEEK_SET);
    PositionFillerWriter pos_filler_writer(mHeaderMetadataEndPos);
    mHeaderMetadata->write(mMXFFile, &mMXFFile->getPartition(0), &pos_filler_writer);


    // update and re-write the index table segment

    mIndexSegment->setIndexDuration(GetDuration());
    KAGFillerWriter kag_filler_writer(&mMXFFile->getPartition(0));
    mIndexSegment->write(mMXFFile, &mMXFFile->getPartition(0), &kag_filler_writer);


    // update and re-write the partition packs

    mMXFFile->getPartition(0).setKey(&MXF_PP_K(ClosedComplete, Header));
    mMXFFile->updatePartitions();


    // done with the file
    delete mMXFFile;
    mMXFFile = 0;
}

D10Track* D10File::GetTrack(uint32_t track_index)
{
    IM_ASSERT(track_index < mTracks.size());
    return mTrackMap[track_index];
}

void D10File::CreateHeaderMetadata()
{
    IM_ASSERT(!mHeaderMetadata);


    // create the header metadata
    mDataModel = new DataModel();
    mHeaderMetadata = new HeaderMetadata(mDataModel);


    // Preface
    Preface *preface = new Preface(mHeaderMetadata);
    preface->setLastModifiedDate(mCreationDate);
    preface->setVersion(258);   // v1.2 - smpte 377-2004
    preface->setOperationalPattern(MXF_OP_L(1a, MultiTrack_Stream_Internal));
    preface->appendEssenceContainers(mEssenceContainerUL);
    preface->setDMSchemes(vector<mxfUL>());

    // Preface - Identification
    Identification *ident = new Identification(mHeaderMetadata);
    preface->appendIdentifications(ident);
    ident->initialise(mCompanyName, mProductName, mVersionString, mProductUID);
    ident->setProductVersion(mProductVersion);
    ident->setModificationDate(mCreationDate);
    ident->setThisGenerationUID(mGenerationUID);

    // Preface - ContentStorage
    ContentStorage* content_storage = new ContentStorage(mHeaderMetadata);
    preface->setContentStorage(content_storage);

    // Preface - ContentStorage - EssenceContainerData
    EssenceContainerData *ess_container_data = new EssenceContainerData(mHeaderMetadata);
    content_storage->appendEssenceContainerData(ess_container_data);
    ess_container_data->setLinkedPackageUID(mFileSourcePackageUID);
    ess_container_data->setIndexSID(INDEX_SID);
    ess_container_data->setBodySID(BODY_SID);

    // Preface - ContentStorage - MaterialPackage
    mMaterialPackage = new MaterialPackage(mHeaderMetadata);
    content_storage->appendPackages(mMaterialPackage);
    mMaterialPackage->setPackageUID(mMaterialPackageUID);
    mMaterialPackage->setPackageCreationDate(mCreationDate);
    mMaterialPackage->setPackageModifiedDate(mCreationDate);
    if (!mClipName.empty())
        mMaterialPackage->setName(mClipName);

    // Preface - ContentStorage - MaterialPackage - Timecode Track
    Track *timecode_track = new Track(mHeaderMetadata);
    mMaterialPackage->appendTracks(timecode_track);
    timecode_track->setTrackName("TC1");
    timecode_track->setTrackID(1);
    timecode_track->setTrackNumber(0);
    timecode_track->setEditRate(mFrameRate);
    timecode_track->setOrigin(0);

    // Preface - ContentStorage - MaterialPackage - Timecode Track - Sequence
    Sequence *sequence = new Sequence(mHeaderMetadata);
    timecode_track->setSequence(sequence);
    sequence->setDataDefinition(MXF_DDEF_L(Timecode));
    sequence->setDuration(-1); // updated when writing completed

    // Preface - ContentStorage - MaterialPackage - Timecode Track - TimecodeComponent
    TimecodeComponent *timecode_component = new TimecodeComponent(mHeaderMetadata);
    sequence->appendStructuralComponents(timecode_component);
    timecode_component->setDataDefinition(MXF_DDEF_L(Timecode));
    timecode_component->setDuration(-1); // updated when writing completed
    timecode_component->setRoundedTimecodeBase(mStartTimecode.GetRoundedTCBase());
    timecode_component->setDropFrame(mStartTimecode.IsDropFrame());
    timecode_component->setStartTimecode(mStartTimecode.GetOffset());

    // Preface - ContentStorage - SourcePackage
    mFileSourcePackage = new SourcePackage(mHeaderMetadata);
    content_storage->appendPackages(mFileSourcePackage);
    mFileSourcePackage->setPackageUID(mFileSourcePackageUID);
    mFileSourcePackage->setPackageCreationDate(mCreationDate);
    mFileSourcePackage->setPackageModifiedDate(mCreationDate);

    // Preface - ContentStorage - SourcePackage - Timecode Track
    timecode_track = new Track(mHeaderMetadata);
    mFileSourcePackage->appendTracks(timecode_track);
    timecode_track->setTrackName("TC1");
    timecode_track->setTrackID(1);
    timecode_track->setTrackNumber(0);
    timecode_track->setEditRate(mFrameRate);
    timecode_track->setOrigin(0);

    // Preface - ContentStorage - SourcePackage - Timecode Track - Sequence
    sequence = new Sequence(mHeaderMetadata);
    timecode_track->setSequence(sequence);
    sequence->setDataDefinition(MXF_DDEF_L(Timecode));
    sequence->setDuration(-1); // updated when writing completed

    // Preface - ContentStorage - SourcePackage - Timecode Track - TimecodeComponent
    timecode_component = new TimecodeComponent(mHeaderMetadata);
    sequence->appendStructuralComponents(timecode_component);
    timecode_component->setDataDefinition(MXF_DDEF_L(Timecode));
    timecode_component->setDuration(-1); // updated when writing completed
    timecode_component->setRoundedTimecodeBase(mStartTimecode.GetRoundedTCBase());
    timecode_component->setDropFrame(mStartTimecode.IsDropFrame());
    timecode_component->setStartTimecode(mStartTimecode.GetOffset());

    // Preface - ContentStorage - MaterialPackage/SourcePackage - Timeline Tracks
    // picture track followed by sound track
    uint32_t i;
    for (i = 0; i < 2; i++)
    {
        bool is_picture = (i == 0);
        mxfUL data_def = (is_picture ? MXF_DDEF_L(Picture) : MXF_DDEF_L(Sound));
        string track_name = get_track_name(is_picture, is_picture ? 1 : i);

        // Preface - ContentStorage - MaterialPackage - Timeline Track
        Track *track = new Track(mHeaderMetadata);
        mMaterialPackage->appendTracks(track);
        track->setTrackName(track_name);
        track->setTrackID(i + 2);
        track->setTrackNumber(0);
        track->setEditRate(mFrameRate);
        track->setOrigin(0);

        // Preface - ContentStorage - MaterialPackage - Timeline Track - Sequence
        Sequence *sequence = new Sequence(mHeaderMetadata);
        track->setSequence(sequence);
        sequence->setDataDefinition(data_def);
        sequence->setDuration(-1); // updated when writing completed

        // Preface - ContentStorage - MaterialPackage - Timeline Track - Sequence - SourceClip
        SourceClip *source_clip = new SourceClip(mHeaderMetadata);
        sequence->appendStructuralComponents(source_clip);
        source_clip->setDataDefinition(data_def);
        source_clip->setDuration(-1); // updated when writing completed
        source_clip->setStartPosition(0);
        source_clip->setSourcePackageID(mFileSourcePackageUID);
        source_clip->setSourceTrackID(i + 2);

        // Preface - ContentStorage - SourcePackage - Timeline Track
        track = new Track(mHeaderMetadata);
        mFileSourcePackage->appendTracks(track);
        track->setTrackName(track_name);
        track->setTrackID(i + 2);
        track->setTrackNumber(is_picture ? MXF_D10_PICTURE_TRACK_NUM(0x00) : MXF_D10_SOUND_TRACK_NUM(0x00));
        track->setEditRate(mFrameRate);
        track->setOrigin(0);
    
        // Preface - ContentStorage - SourcePackage - Timeline Track - Sequence
        sequence = new Sequence(mHeaderMetadata);
        track->setSequence(sequence);
        sequence->setDataDefinition(data_def);
        sequence->setDuration(-1); // updated when writing completed

        // Preface - ContentStorage - SourcePackage - Timeline Track - Sequence - SourceClip
        source_clip = new SourceClip(mHeaderMetadata);
        sequence->appendStructuralComponents(source_clip);
        source_clip->setDataDefinition(data_def);
        source_clip->setDuration(-1); // updated when writing completed
        source_clip->setStartPosition(0);
        source_clip->setSourceTrackID(0);
        source_clip->setSourcePackageID(g_Null_UMID);
    }

    // Preface - ContentStorage - SourcePackage - MultipleDescriptor
    MultipleDescriptor *mult_descriptor = new MultipleDescriptor(mHeaderMetadata);
    mFileSourcePackage->setDescriptor(mult_descriptor);
    mult_descriptor->setSampleRate(mFrameRate);
    mult_descriptor->setEssenceContainer(mEssenceContainerUL);

    // Preface - ContentStorage - SourcePackage - MultipleDescriptor - CDCIEssenceDescriptor
    CDCIEssenceDescriptor *cdci_descriptor =
        dynamic_cast<CDCIEssenceDescriptor*>(mPictureTrack->CreateFileDescriptor(mHeaderMetadata));
    mult_descriptor->appendSubDescriptorUIDs(cdci_descriptor);
    cdci_descriptor->setLinkedTrackID(2);
    cdci_descriptor->setEssenceContainer(mEssenceContainerUL);

    // Preface - ContentStorage - SourcePackage - MultipleDescriptor - GenericSoundEssenceDescriptor
    GenericSoundEssenceDescriptor *sound_descriptor = new GenericSoundEssenceDescriptor(mHeaderMetadata);
    mult_descriptor->appendSubDescriptorUIDs(sound_descriptor);
    sound_descriptor->setLinkedTrackID(3);
    sound_descriptor->setSampleRate(mFrameRate);
    sound_descriptor->setEssenceContainer(mEssenceContainerUL);
    if (mFirstSoundTrack) {
        sound_descriptor->setAudioSamplingRate(mFirstSoundTrack->GetSamplingRate());
        sound_descriptor->setChannelCount(mMaxSoundOutputTrackNumber);
        sound_descriptor->setQuantizationBits(mFirstSoundTrack->GetQuantizationBits());
        if (mFirstSoundTrack->HaveSetLocked())
            sound_descriptor->setLocked(mFirstSoundTrack->GetLocked());
        if (mFirstSoundTrack->HaveSetAudioRefLevel())
            sound_descriptor->setAudioRefLevel(mFirstSoundTrack->GetAudioRefLevel());
    } else {
        sound_descriptor->setAudioSamplingRate(AUDIO_SAMPLING_RATE);
        sound_descriptor->setChannelCount(0);
        sound_descriptor->setQuantizationBits(16);
    }
}

void D10File::CreateFile()
{
    IM_ASSERT(mHeaderMetadata);


    // set minimum llen

    mMXFFile->setMinLLen(4);


    // write the header metadata

    Partition &header_partition = mMXFFile->createPartition();
    header_partition.setKey(&MXF_PP_K(OpenIncomplete, Header));
    header_partition.setVersion(1, 2);  // v1.2 - smpte 377-2004
    header_partition.setIndexSID(INDEX_SID);
    header_partition.setBodySID(BODY_SID);
    header_partition.setKagSize(KAG_SIZE);
    header_partition.setOperationalPattern(&MXF_OP_L(1a, MultiTrack_Stream_Internal));
    header_partition.addEssenceContainer(&mEssenceContainerUL);
    header_partition.write(mMXFFile);
    header_partition.fillToKag(mMXFFile);

    mHeaderMetadataStartPos = mMXFFile->tell(); // need this position when we re-write the header metadata
    KAGFillerWriter reserve_filler_writer(&header_partition, mReserveMinBytes);
    mHeaderMetadata->write(mMXFFile, &header_partition, &reserve_filler_writer);
    mHeaderMetadataEndPos = mMXFFile->tell();  // need this position when we re-write the header metadata


    // write cbe index table

    mxfUUID uuid;
    mxf_generate_uuid(&uuid);

    mIndexSegment = new IndexTableSegment();
    mIndexSegment->setInstanceUID(uuid);
    mIndexSegment->setIndexEditRate(mFrameRate);
    mIndexSegment->setIndexDuration(0); // will be updated when writing is completed
    mIndexSegment->setIndexSID(INDEX_SID);
    mIndexSegment->setBodySID(BODY_SID);

    const std::vector<uint32_t> &ext_delta_entries = mCPManager->GetExtDeltaEntryArray();
    size_t i;
    for (i = 0; i < ext_delta_entries.size() - 1; i++)
        mIndexSegment->appendDeltaEntry(0, 0, ext_delta_entries[i]);
    mIndexSegment->setEditUnitByteCount(ext_delta_entries[i]);

    KAGFillerWriter kag_filler_writer(&mMXFFile->getPartition(0));
    mIndexSegment->write(mMXFFile, &mMXFFile->getPartition(0), &kag_filler_writer);
}

void D10File::UpdatePackageMetadata()
{
    int64_t output_duration = GetDuration();

    UpdateTrackMetadata(mMaterialPackage, output_duration);
    UpdateTrackMetadata(mFileSourcePackage, output_duration);


    IM_ASSERT(mFileSourcePackage->haveDescriptor());
    FileDescriptor *file_descriptor = dynamic_cast<FileDescriptor*>(mFileSourcePackage->getDescriptor());
    if (file_descriptor)
        file_descriptor->setContainerDuration(output_duration);

    MultipleDescriptor *mult_descriptor = dynamic_cast<MultipleDescriptor*>(file_descriptor);
    if (mult_descriptor) {
        vector<GenericDescriptor*> sub_descriptors = mult_descriptor->getSubDescriptorUIDs();
        size_t i;
        for (i = 0; i < sub_descriptors.size(); i++) {
            file_descriptor = dynamic_cast<FileDescriptor*>(sub_descriptors[i]);
            if (file_descriptor)
                file_descriptor->setContainerDuration(output_duration);
        }
    }
}

void D10File::UpdateTrackMetadata(GenericPackage *package, int64_t duration)
{
    vector<GenericTrack*> tracks = package->getTracks();
    size_t i;
    for (i = 0; i < tracks.size(); i++) {
        Track *track = dynamic_cast<Track*>(tracks[i]);
        if (!track)
            continue;

        Sequence *sequence = dynamic_cast<Sequence*>(track->getSequence());
        IM_ASSERT(sequence);
        if (sequence->getDuration() < 0) {
            sequence->setDuration(duration);

            vector<StructuralComponent*> components = sequence->getStructuralComponents();
            IM_CHECK(components.size() == 1);
            components[0]->setDuration(duration);
        }
    }
}

