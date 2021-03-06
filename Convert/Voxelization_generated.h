// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_VOXELIZATION_LOADER_VOXELIZATION_H_
#define FLATBUFFERS_GENERATED_VOXELIZATION_LOADER_VOXELIZATION_H_

#include "flatbuffers/flatbuffers.h"

namespace Loader {
namespace Voxelization {

struct uint2;

struct float3;

struct Brick;

struct VoxelPart;

struct Voxelization;

MANUALLY_ALIGNED_STRUCT(4) uint2 FLATBUFFERS_FINAL_CLASS {
 private:
  uint32_t x_;
  uint32_t y_;

 public:
  uint2(uint32_t _x, uint32_t _y)
    : x_(flatbuffers::EndianScalar(_x)), y_(flatbuffers::EndianScalar(_y)) { }

  uint32_t x() const { return flatbuffers::EndianScalar(x_); }
  uint32_t y() const { return flatbuffers::EndianScalar(y_); }
};
STRUCT_END(uint2, 8);

MANUALLY_ALIGNED_STRUCT(4) float3 FLATBUFFERS_FINAL_CLASS {
 private:
  float x_;
  float y_;
  float z_;

 public:
  float3(float _x, float _y, float _z)
    : x_(flatbuffers::EndianScalar(_x)), y_(flatbuffers::EndianScalar(_y)), z_(flatbuffers::EndianScalar(_z)) { }

  float x() const { return flatbuffers::EndianScalar(x_); }
  float y() const { return flatbuffers::EndianScalar(y_); }
  float z() const { return flatbuffers::EndianScalar(z_); }
};
STRUCT_END(float3, 12);

MANUALLY_ALIGNED_STRUCT(4) Brick FLATBUFFERS_FINAL_CLASS {
 private:
  uint2 Data_;
  uint32_t Position_;

 public:
  Brick(const uint2 &_Data, uint32_t _Position)
    : Data_(_Data), Position_(flatbuffers::EndianScalar(_Position)) { }

  const uint2 &Data() const { return Data_; }
  uint32_t Position() const { return flatbuffers::EndianScalar(Position_); }
};
STRUCT_END(Brick, 12);

struct VoxelPart FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_BRICKS = 4
  };
  const flatbuffers::Vector<const Brick *> *Bricks() const { return GetPointer<const flatbuffers::Vector<const Brick *> *>(VT_BRICKS); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_BRICKS) &&
           verifier.Verify(Bricks()) &&
           verifier.EndTable();
  }
};

struct VoxelPartBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_Bricks(flatbuffers::Offset<flatbuffers::Vector<const Brick *>> Bricks) { fbb_.AddOffset(VoxelPart::VT_BRICKS, Bricks); }
  VoxelPartBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  VoxelPartBuilder &operator=(const VoxelPartBuilder &);
  flatbuffers::Offset<VoxelPart> Finish() {
    auto o = flatbuffers::Offset<VoxelPart>(fbb_.EndTable(start_, 1));
    return o;
  }
};

inline flatbuffers::Offset<VoxelPart> CreateVoxelPart(flatbuffers::FlatBufferBuilder &_fbb,
   flatbuffers::Offset<flatbuffers::Vector<const Brick *>> Bricks = 0) {
  VoxelPartBuilder builder_(_fbb);
  builder_.add_Bricks(Bricks);
  return builder_.Finish();
}

struct Voxelization FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_RESOLUTION = 4,
    VT_POSITION = 6,
    VT_SIZE = 8,
    VT_VOXELPARTS = 10
  };
  uint32_t Resolution() const { return GetField<uint32_t>(VT_RESOLUTION, 0); }
  const float3 *Position() const { return GetStruct<const float3 *>(VT_POSITION); }
  const float3 *Size() const { return GetStruct<const float3 *>(VT_SIZE); }
  const flatbuffers::Vector<flatbuffers::Offset<VoxelPart>> *VoxelParts() const { return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<VoxelPart>> *>(VT_VOXELPARTS); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint32_t>(verifier, VT_RESOLUTION) &&
           VerifyField<float3>(verifier, VT_POSITION) &&
           VerifyField<float3>(verifier, VT_SIZE) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_VOXELPARTS) &&
           verifier.Verify(VoxelParts()) &&
           verifier.VerifyVectorOfTables(VoxelParts()) &&
           verifier.EndTable();
  }
};

struct VoxelizationBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_Resolution(uint32_t Resolution) { fbb_.AddElement<uint32_t>(Voxelization::VT_RESOLUTION, Resolution, 0); }
  void add_Position(const float3 *Position) { fbb_.AddStruct(Voxelization::VT_POSITION, Position); }
  void add_Size(const float3 *Size) { fbb_.AddStruct(Voxelization::VT_SIZE, Size); }
  void add_VoxelParts(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<VoxelPart>>> VoxelParts) { fbb_.AddOffset(Voxelization::VT_VOXELPARTS, VoxelParts); }
  VoxelizationBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  VoxelizationBuilder &operator=(const VoxelizationBuilder &);
  flatbuffers::Offset<Voxelization> Finish() {
    auto o = flatbuffers::Offset<Voxelization>(fbb_.EndTable(start_, 4));
    return o;
  }
};

inline flatbuffers::Offset<Voxelization> CreateVoxelization(flatbuffers::FlatBufferBuilder &_fbb,
   uint32_t Resolution = 0,
   const float3 *Position = 0,
   const float3 *Size = 0,
   flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<VoxelPart>>> VoxelParts = 0) {
  VoxelizationBuilder builder_(_fbb);
  builder_.add_VoxelParts(VoxelParts);
  builder_.add_Size(Size);
  builder_.add_Position(Position);
  builder_.add_Resolution(Resolution);
  return builder_.Finish();
}

inline const Loader::Voxelization::Voxelization *GetVoxelization(const void *buf) { return flatbuffers::GetRoot<Loader::Voxelization::Voxelization>(buf); }

inline bool VerifyVoxelizationBuffer(flatbuffers::Verifier &verifier) { return verifier.VerifyBuffer<Loader::Voxelization::Voxelization>(); }

inline void FinishVoxelizationBuffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<Loader::Voxelization::Voxelization> root) { fbb.Finish(root); }

}  // namespace Voxelization
}  // namespace Loader

#endif  // FLATBUFFERS_GENERATED_VOXELIZATION_LOADER_VOXELIZATION_H_
