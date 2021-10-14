/*
Copyright 2020 The OneFlow Authors. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef ONEFLOW_CORE_VM_VPU_INSTRUCTION__H_
#define ONEFLOW_CORE_VM_VPU_INSTRUCTION__H_

#include <cstring>
#include <mutex>
#include "oneflow/core/job/parallel_desc.h"
#include "oneflow/core/intrusive/flat_msg.h"
#include "oneflow/core/intrusive/intrusive.h"
#include "oneflow/core/vm/stream_desc.h"
#include "oneflow/core/vm/vm_object.h"
#include "oneflow/core/vm/stream_type.h"
#include "oneflow/core/vm/instr_type_id.h"
#include "oneflow/core/vm/id_util.h"
#include "oneflow/core/vm/interpret_type.h"
#include "oneflow/core/vm/instruction_operand.h"
#include "oneflow/core/vm/instruction.pb.h"
#include "oneflow/core/vm/instruction.cfg.h"

namespace oneflow {
namespace vm {

// clang-format off
INTRUSIVE_BEGIN(InstructionOperandList);
 public:
  void __Init__() {}
  // Getters
  const std::vector<FlatMsg<InstructionOperand>>& operand() const { return operand_; }
  // Setters
  std::vector<FlatMsg<InstructionOperand>>* mut_operand() { return &operand_; }

 private:
  friend class intrusive::Ref;
  intrusive::Ref* mut_intrusive_ref() { return &intrusive_ref_; }

  InstructionOperandList() : intrusive_ref_(), operand_() {}
  INTRUSIVE_DEFINE_FIELD(intrusive::Ref, intrusive_ref_);
  INTRUSIVE_DEFINE_FIELD(std::vector<FlatMsg<InstructionOperand>>, operand_);
INTRUSIVE_END(InstructionOperandList);

INTRUSIVE_BEGIN(InstructionMsg);
 public:
  // Getters
  bool has_parallel_desc_symbol_id() const { return 0 != parallel_desc_symbol_id_; }
  int64_t parallel_desc_symbol_id() const { return parallel_desc_symbol_id_; }
  const InstructionOperandList& operand_list() const {
    if (operand_list_) { return operand_list_.Get(); }
    static const auto default_val = intrusive::make_shared<InstructionOperandList>();
    return default_val.Get();
  }
  const std::string& instr_type_name() const { return instr_type_name_; }
  const InstrTypeId& instr_type_id() const { return instr_type_id_; }
  const std::shared_ptr<const ParallelDesc>& parallel_desc() const { return parallel_desc_; }
  const std::shared_ptr<PhyInstrOperand>& phy_instr_operand() const { return phy_instr_operand_; }
  // Setters
  void set_parallel_desc_symbol_id(int64_t val) { parallel_desc_symbol_id_ = val; }
  InstructionOperandList* mut_operand_list() {
    if (!operand_list_) { operand_list_ = intrusive::make_shared<InstructionOperandList>(); }
    return operand_list_.Mutable();
  }
  void reset_operand_list(const InstructionOperandList& other) {
    operand_list_.Reset(const_cast<InstructionOperandList*>(&other));
  }
  std::string* mut_instr_type_name() { return &instr_type_name_; }
  InstrTypeId* mut_instr_type_id() { return &instr_type_id_; }
  std::shared_ptr<const ParallelDesc>* mut_parallel_desc() { return &parallel_desc_; }
  std::shared_ptr<PhyInstrOperand>* mut_phy_instr_operand() { return &phy_instr_operand_; }

  // methods
  void __Init__();
  void __Init__(const std::string& instr_type_name);
  void __Init__(const InstructionProto& proto);
  void __Init__(const cfg::InstructionProto& proto); 
  void __Init__(const InstructionMsg& instr_msg);

  void ToProto(InstructionProto* proto) const;
  intrusive::shared_ptr<InstructionMsg> add_parallel_desc(int64_t symbol_id);
  intrusive::shared_ptr<InstructionMsg> add_double_operand(double double_operand);
  intrusive::shared_ptr<InstructionMsg> add_int64_operand(int64_t int64_operand);
  intrusive::shared_ptr<InstructionMsg> add_uint64_operand(uint64_t uint64_operand);
  intrusive::shared_ptr<InstructionMsg> add_bool_operand(bool bool_operand);
  intrusive::shared_ptr<InstructionMsg> add_separator();
  intrusive::shared_ptr<InstructionMsg> add_const_operand(ObjectId logical_object_id);
  intrusive::shared_ptr<InstructionMsg> add_const_operand(ObjectId logical_object_id, const SoleMirroredObject&);
  intrusive::shared_ptr<InstructionMsg> add_const_operand(ObjectId logical_object_id, const AllMirroredObject&);
  intrusive::shared_ptr<InstructionMsg> add_symbol_operand(ObjectId logical_object_id);
  intrusive::shared_ptr<InstructionMsg> add_mut_operand(ObjectId logical_object_id);
  intrusive::shared_ptr<InstructionMsg> add_mut_operand(ObjectId logical_object_id, const SoleMirroredObject&);
  intrusive::shared_ptr<InstructionMsg> add_mut_operand(ObjectId logical_object_id, const AllMirroredObject&);
  intrusive::shared_ptr<InstructionMsg> add_init_symbol_operand(ObjectId logical_object_id);
  intrusive::shared_ptr<InstructionMsg> add_mut2_operand(ObjectId logical_object_id);
  intrusive::shared_ptr<InstructionMsg> add_mut2_operand(ObjectId logical_object_id, const SoleMirroredObject&);
  intrusive::shared_ptr<InstructionMsg> add_mut2_operand(ObjectId logical_object_id, const AllMirroredObject&);
  intrusive::shared_ptr<InstructionMsg> add_del_operand(ObjectId logical_object_id);
  const std::vector<FlatMsg<InstructionOperand>>& operand() const {
    return operand_list().operand();
  }
  std::vector<FlatMsg<InstructionOperand>>* mut_operand() {
    return mut_operand_list()->mut_operand();
  }
  intrusive::shared_ptr<InstructionMsg> Clone() const;
  intrusive::shared_ptr<InstructionMsg> MakeInferInstrMsg() const;

 private:
  InstructionOperand* add_instr_operand();
  friend class intrusive::Ref;
  intrusive::Ref* mut_intrusive_ref() { return &intrusive_ref_; }

  InstructionMsg() : intrusive_ref_(), instr_type_id_(), instr_type_name_(), parallel_desc_symbol_id_(), parallel_desc_(), operand_list_(), phy_instr_operand_(), instr_msg_entry_() {}
  INTRUSIVE_DEFINE_FIELD(intrusive::Ref, intrusive_ref_);
  // fields
  INTRUSIVE_DEFINE_FIELD(InstrTypeId, instr_type_id_);
  // instr_type_name is a necessary reduandant field for method ToProto
  INTRUSIVE_DEFINE_FIELD(std::string, instr_type_name_);
  INTRUSIVE_DEFINE_FIELD(int64_t, parallel_desc_symbol_id_);
  INTRUSIVE_DEFINE_FIELD(std::shared_ptr<const ParallelDesc>, parallel_desc_);
  INTRUSIVE_DEFINE_FIELD(intrusive::shared_ptr<InstructionOperandList>, operand_list_);
  INTRUSIVE_DEFINE_FIELD(std::shared_ptr<PhyInstrOperand>, phy_instr_operand_);
  // list entries
  INTRUSIVE_DEFINE_FIELD(intrusive::ListEntry, instr_msg_entry_);
INTRUSIVE_END(InstructionMsg);
// clang-format on

using InstructionMsgList = intrusive::List<INTRUSIVE_FIELD(InstructionMsg, instr_msg_entry_)>;

template<OperandMemZoneModifier mem_zone_modifier>
void CheckOperand(const Operand& operand);

static const int kInstructionStatusBufferBytes = 32;

// clang-format off
FLAT_MSG_BEGIN(InstructionDeleted);
FLAT_MSG_END(InstructionDeleted);

FLAT_MSG_BEGIN(InstructionStatusBuffer);
  FLAT_MSG_DEFINE_OPTIONAL(InstructionDeleted, instruction_deleted);
  FLAT_MSG_DEFINE_REPEATED(char, buffer, kInstructionStatusBufferBytes);
FLAT_MSG_END(InstructionStatusBuffer);
// clang-format on

struct Instruction;
// clang-format off
INTRUSIVE_BEGIN(InstructionEdge);
 public:
  void __Init__() {
    clear_src_instruction();
    clear_dst_instruction();
  }
  // Getters
  bool has_src_instruction() const { return src_instruction_ != nullptr; } 
  bool has_dst_instruction() const { return dst_instruction_ != nullptr; } 
  const Instruction& src_instruction() const { return *src_instruction_; }
  const Instruction& dst_instruction() const { return *dst_instruction_; } 
  // Setters
  void set_src_instruction(Instruction* val) { src_instruction_ = val; } 
  void set_dst_instruction(Instruction* val) { dst_instruction_ = val; } 
  void clear_src_instruction() { src_instruction_ = nullptr; } 
  void clear_dst_instruction() { dst_instruction_ = nullptr; } 
  Instruction* mut_src_instruction() { return src_instruction_; } 
  Instruction* mut_dst_instruction() { return dst_instruction_; } 
  // methods
  void __Init__(Instruction* src_instruction, Instruction* dst_instruction) {
    __Init__();
    set_src_instruction(src_instruction);
    set_dst_instruction(dst_instruction);
  }

 private:
  friend class intrusive::Ref;
  intrusive::Ref* mut_intrusive_ref() { return &intrusive_ref_; }

  InstructionEdge() : intrusive_ref_(), src_instruction_(), dst_instruction_(), in_edge_entry_(), out_edge_entry_() {}
  INTRUSIVE_DEFINE_FIELD(intrusive::Ref, intrusive_ref_);
  // fields
  INTRUSIVE_DEFINE_FIELD(Instruction*, src_instruction_); 
  INTRUSIVE_DEFINE_FIELD(Instruction*, dst_instruction_); 
  // list entries
  INTRUSIVE_DEFINE_FIELD(intrusive::ListEntry, in_edge_entry_);
  INTRUSIVE_DEFINE_FIELD(intrusive::ListEntry, out_edge_entry_);
INTRUSIVE_END(InstructionEdge);
// clang-format on

struct Stream;
// clang-format off
INTRUSIVE_BEGIN(Instruction);
 public:
  // types
  using InEdgeList = intrusive::List<INTRUSIVE_FIELD(InstructionEdge, in_edge_entry_)>;
  using OutEdgeList = intrusive::List<INTRUSIVE_FIELD(InstructionEdge, out_edge_entry_)>;
  using RwMutexedObjectAccessList =
      intrusive::List<INTRUSIVE_FIELD(RwMutexedObjectAccess, instruction_access_entry_)>;
  using MirroredObjectId2RwMutexedObjectAccess =
      intrusive::SkipList<INTRUSIVE_FIELD(RwMutexedObjectAccess, mirrored_object_id_)>;

  // Getters
  void __Init__() { clear_stream(); }
  bool has_stream() const { return stream_ != nullptr;  }
  const Stream& stream() const { return *stream_;  }
  const InstructionMsg& instr_msg() const {
    if (instr_msg_) { return instr_msg_.Get(); }
    static const auto default_val = intrusive::make_shared<InstructionMsg>();
    return default_val.Get();
  }
  const std::shared_ptr<const ParallelDesc>& parallel_desc() const { return parallel_desc_; }
  const InstructionStatusBuffer& status_buffer() const { return status_buffer_.Get(); }
  bool is_instruction_entry_empty() const { return instruction_entry_.empty(); }
  bool is_vm_stat_running_instruction_entry_empty() const { return vm_stat_running_instruction_entry_.empty(); }
  bool is_pending_instruction_entry_empty() const { return pending_instruction_entry_.empty(); }
  bool is_front_seq_compute_instr_entry_empty() const { return front_seq_compute_instr_entry_.empty(); }
  const InEdgeList& in_edges() const { return in_edges_; }
  const OutEdgeList& out_edges() const { return out_edges_; }
  const RwMutexedObjectAccessList& access_list() const { return access_list_; }
  const MirroredObjectId2RwMutexedObjectAccess& mirrored_object_id2access() const {
      return mirrored_object_id2access_; }

  // Setters
  void set_stream(Stream* val) { stream_ = val; }
  void clear_stream() { stream_ = nullptr; }
  Stream* mut_stream() { return stream_; }
  InstructionMsg* mut_instr_msg() {
    if (!instr_msg_) { instr_msg_ = intrusive::make_shared<InstructionMsg>(); }
    return instr_msg_.Mutable();
  }
  void reset_instr_msg(InstructionMsg* instr_msg) { instr_msg_.Reset(instr_msg); }
  void clear_instr_msg() { instr_msg_.Reset(); }
  std::shared_ptr<const ParallelDesc>* mut_parallel_desc() { return &parallel_desc_; }
  InstructionStatusBuffer* mut_status_buffer() { return status_buffer_.Mutable(); }
  InEdgeList* mut_in_edges() { return &in_edges_; }
  OutEdgeList* mut_out_edges() { return &out_edges_; }
  RwMutexedObjectAccessList* mut_access_list() { return &access_list_; }
  MirroredObjectId2RwMutexedObjectAccess* mut_mirrored_object_id2access() {
      return &mirrored_object_id2access_;
  }

  // methods
  void __Init__(InstructionMsg* instr_msg, Stream* stream, const std::shared_ptr<const ParallelDesc>& parallel_desc);
  void __Delete__();
  bool Done() const;
  void set_has_event_record(bool val);
  const StreamType& stream_type() const;
  template<OperandMemZoneModifier mem_zone_modifier>
      const RwMutexedObject* operand_type(const Operand& operand) const {
    CheckOperand<mem_zone_modifier>(operand);
    return operand_type(operand, GetOperandDefaultGlobalDeviceId());
  }
  template<OperandMemZoneModifier mem_zone_modifier>
      const RwMutexedObject* operand_value(const Operand& operand) const {
    CheckOperand<mem_zone_modifier>(operand);
    return operand_value(operand, GetOperandDefaultGlobalDeviceId());
  }
  template<OperandMemZoneModifier mem_zone_modifier>
      RwMutexedObject* mut_operand_type(const Operand& operand) {
    CheckOperand<mem_zone_modifier>(operand);
    return mut_operand_type(operand, GetOperandDefaultGlobalDeviceId());
  }
  template<OperandMemZoneModifier mem_zone_modifier>
      RwMutexedObject* mut_operand_value(const Operand& operand) {
    CheckOperand<mem_zone_modifier>(operand);
    return mut_operand_value(operand, GetOperandDefaultGlobalDeviceId());
  }
  template<OperandAccessModifier access_modifier, OperandMemZoneModifier mem_zone_modifier>
  const RwMutexedObject* operand_type(
      const ModifiedOperand<access_modifier, mem_zone_modifier>& operand) const {
    return operand_type<mem_zone_modifier>(operand.operand());
  }
  template<OperandAccessModifier access_modifier, OperandMemZoneModifier mem_zone_modifier>
  const RwMutexedObject* operand_value(
      const ModifiedOperand<access_modifier, mem_zone_modifier>& operand) const {
    return operand_value<mem_zone_modifier>(operand.operand());
  }
  template<OperandAccessModifier access_modifier, OperandMemZoneModifier mem_zone_modifier>
  RwMutexedObject* mut_operand_type(
      const ModifiedOperand<access_modifier, mem_zone_modifier>& operand) {
    return mut_operand_type<mem_zone_modifier>(operand.operand());
  }
  template<OperandAccessModifier access_modifier, OperandMemZoneModifier mem_zone_modifier>
  RwMutexedObject* mut_operand_value(
      const ModifiedOperand<access_modifier, mem_zone_modifier>& operand) {
    return mut_operand_value<mem_zone_modifier>(operand.operand());
  }
  template<InterpretType interpret_type>
         MirroredObject* MutMirroredObject(const MutOperand& mut_operand) {
    return MirroredObjectUtil<interpret_type>::Mut(this, mut_operand);
  }
  template<InterpretType interpret_type>
         const MirroredObject* GetMirroredObject(const ConstOperand& const_operand) const {
    return MirroredObjectUtil<interpret_type>::Get(*this, const_operand);
  }
  MirroredObject* mut_type_mirrored_object(const MutOperand& mut_operand);
  MirroredObject* mut_value_mirrored_object(const MutOperand& mut_operand);

  intrusive::Ref::RefCntType ref_cnt() const { return intrusive_ref_.ref_cnt(); }

 private:
  template<int64_t(*TransformLogicalObjectId)(int64_t)>
          MirroredObject* MutMirroredObject(const Operand& operand,
                                            int64_t default_global_device_id);
  template<int64_t(*TransformLogicalObjectId)(int64_t)>
          const MirroredObject* GetMirroredObject(const Operand& operand,
                                                  int64_t default_global_device_id) const;
  const RwMutexedObject* operand_type(const Operand& operand,
                                              int64_t default_global_device_id) const;
  const RwMutexedObject* operand_value(const Operand& operand,
                                               int64_t default_global_device_id) const;
  RwMutexedObject* mut_operand_type(const Operand& operand,
                                            int64_t default_global_device_id);
  RwMutexedObject* mut_operand_value(const Operand& operand,  
                                             int64_t default_global_device_id);
  MirroredObject* MutMirroredObject(const Operand& operand,
                                                     int64_t default_global_device_id) {
    return MutMirroredObject<&IdUtil::GetValueId>(operand, default_global_device_id);
  }
  int64_t GetOperandDefaultGlobalDeviceId() const;
  template<InterpretType interpret_type>
  struct MirroredObjectUtil {
    static const MirroredObject* Get(const Instruction&, const ConstOperand&);
    static MirroredObject* Mut(Instruction*, const MutOperand&);
  };

  friend class intrusive::Ref;
  intrusive::Ref* mut_intrusive_ref() { return &intrusive_ref_; }

  Instruction() : intrusive_ref_(), status_buffer_(), instr_msg_(), parallel_desc_(), stream_(), instruction_entry_(), vm_stat_running_instruction_entry_(), pending_instruction_entry_(), front_seq_infer_instr_entry_(), front_seq_compute_instr_entry_(), mirrored_object_id2access_(), access_list_(), in_edges_(), out_edges_() {}
  INTRUSIVE_DEFINE_FIELD(intrusive::Ref, intrusive_ref_);
  // fields
  INTRUSIVE_DEFINE_FIELD(FlatMsg<InstructionStatusBuffer>, status_buffer_);
  INTRUSIVE_DEFINE_FIELD(intrusive::shared_ptr<InstructionMsg>, instr_msg_);
  INTRUSIVE_DEFINE_FIELD(std::shared_ptr<const ParallelDesc>, parallel_desc_);
  INTRUSIVE_DEFINE_FIELD(Stream*, stream_); 
  // list entries
  INTRUSIVE_DEFINE_FIELD(intrusive::ListEntry, instruction_entry_);
  // `vm_stat_running_instruction_entry` valid from instruction ready to instruction done 
  INTRUSIVE_DEFINE_FIELD(intrusive::ListEntry, vm_stat_running_instruction_entry_);
  INTRUSIVE_DEFINE_FIELD(intrusive::ListEntry, pending_instruction_entry_);
  INTRUSIVE_DEFINE_FIELD(intrusive::ListEntry, front_seq_infer_instr_entry_);
  INTRUSIVE_DEFINE_FIELD(intrusive::ListEntry, front_seq_compute_instr_entry_);
  // maps
  INTRUSIVE_DEFINE_FIELD(MirroredObjectId2RwMutexedObjectAccess, mirrored_object_id2access_);
  // lists
  INTRUSIVE_DEFINE_FIELD(RwMutexedObjectAccessList, access_list_);
  INTRUSIVE_DEFINE_FIELD(InEdgeList, in_edges_);
  INTRUSIVE_DEFINE_FIELD(OutEdgeList, out_edges_);
INTRUSIVE_END(Instruction);
// clang-format on

}  // namespace vm
}  // namespace oneflow

#endif  // ONEFLOW_CORE_VM_VPU_INSTRUCTION__H_