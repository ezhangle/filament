/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <image/KtxBundle.h>

#include <vector>

namespace {

using Blob = std::vector<uint8_t>;

struct KtxImpl {
    image::KtxInfo info = {};
    uint32_t nmips;
    uint32_t nlayers;
    uint32_t nfaces;
    std::vector<Blob> blobs;
};

// We flatten the three-dimensional blob index using the ordering defined by the
// the KTX spec, which says:
//
// for each mipmap_level in numberOfMipmapLevels
//     for each array_element in numberOfArrayElements
//        for each face in numberOfFaces
//             ....
//        end
//     end
// end
//
inline size_t flatten(const KtxImpl* impl, image::KtxBlobIndex index) {
    return index.cubeFace +
        index.arrayIndex * impl->nfaces +
        index.mipLevel * impl->nlayers * impl->nfaces;
}

}

namespace image  {

struct KtxBundle::SharedReference {
    std::shared_ptr<KtxImpl> impl = std::make_shared<KtxImpl>();
};

KtxBundle::~KtxBundle() {
    delete mImplRef;
}

KtxBundle::KtxBundle(uint32_t numMipLevels, uint32_t arrayLength, bool isCubemap) :
        mImplRef(new SharedReference) {
    auto self = mImplRef->impl.get();
    self->nmips = numMipLevels;
    self->nlayers = arrayLength;
    self->nfaces = isCubemap ? 6 : 1;
    self->blobs.resize(numMipLevels * arrayLength * self->nfaces);
}

KtxBundle::KtxBundle(uint8_t const* bytes, uint32_t nbytes) : mImplRef(new SharedReference) {
    // TBD
    auto self = mImplRef->impl.get();
    self->nmips = 0;
    self->nlayers = 0;
    self->nfaces = 0;
}

bool KtxBundle::serialize(uint8_t* destination, uint32_t numBytes) const {
    // TBD
    return false;
}

uint32_t KtxBundle::getSerializedLength() const {
    // TBD
    return 0;
}

KtxBundle& KtxBundle::operator=(const KtxBundle& that) {
    if (that.mImplRef) {
        mImplRef = new SharedReference();
        mImplRef->impl = that.mImplRef->impl;
    } else {
        delete mImplRef;
        mImplRef = nullptr;
    }
    return *this;
}

KtxBundle::KtxBundle(const KtxBundle& that) {
    *this = that;
}

KtxInfo const& KtxBundle::getInfo() const {
    return mImplRef->impl->info;
}

KtxInfo& KtxBundle::info() {
    return mImplRef->impl->info;
}

uint32_t KtxBundle::getNumMipLevels() const {
    return mImplRef->impl->nmips;
}

uint32_t KtxBundle::getArrayLength() const {
    return mImplRef->impl->nlayers;
}

bool KtxBundle::isCubemap() const {
    return mImplRef->impl->nfaces;
}

bool KtxBundle::getBlob(KtxBlobIndex index, uint8_t const** data, uint32_t* size) const {
    auto self = mImplRef->impl.get();
    if (index.mipLevel >= self->nmips || index.arrayIndex >= self->nlayers ||
            index.cubeFace >= self->nfaces) {
        return false;
    }
    auto& blob = self->blobs[flatten(self, index)];
    if (blob.empty()) {
        return false;
    }
    *data = blob.data();
    *size = blob.size();
    return true;
}

bool KtxBundle::setBlob(KtxBlobIndex index, uint8_t const* data, uint32_t size) {
    auto self = mImplRef->impl.get();
    if (index.mipLevel >= self->nmips || index.arrayIndex >= self->nlayers ||
            index.cubeFace >= self->nfaces) {
        return false;
    }
    auto& blob = self->blobs[flatten(self, index)];
    blob.resize(size);
    memcpy(blob.data(), data, size);
    return true;
}

}  // namespace image
