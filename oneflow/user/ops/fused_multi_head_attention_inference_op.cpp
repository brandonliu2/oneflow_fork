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
#include "oneflow/core/framework/framework.h"
#include "oneflow/core/framework/op_generated.h"

namespace oneflow {

/*static*/ auto FusedMultiHeadAttentionInferenceOp::InferDataType(user_op::InferContext* ctx)
    -> Maybe<void> {
  DataType query_type = ctx->InputDType("query", 0);
  DataType key_type = ctx->InputDType("key", 0);
  DataType value_type = ctx->InputDType("value", 0);
  CHECK_EQ_OR_RETURN(key_type, query_type);
  CHECK_EQ_OR_RETURN(value_type, query_type);
  if (ctx->has_input("attn_bias", 0)) {
    CHECK_EQ_OR_RETURN(ctx->InputDType("attn_bias", 0), query_type);
  }
  ctx->SetOutputDType("out", 0, query_type);
  return Maybe<void>::Ok();
}

/*static*/ auto FusedMultiHeadAttentionInferenceOp::InferLogicalTensorDesc(
    user_op::InferContext* ctx) -> Maybe<void> {
  const int64_t query_head_size = ctx->Attr<int64_t>("query_head_size");
  CHECK_GE_OR_RETURN(query_head_size, 1);

  const auto ParseDims = [](const Shape& shape, const std::string& layout,
                            const Optional<int64_t>& num_heads, const Optional<int64_t>& head_size,
                            int64_t* b, int64_t* m, int64_t* h, int64_t* k) -> Maybe<void> {
    if (shape.NumAxes() == 3) {
      if (layout == "BM(HK)" || layout == "MB(HK)" || layout == "BM(H2K)" || layout == "MB(H2K)"
          || layout == "BM(H3K)" || layout == "MB(H3K)") {
        int64_t packed_n = 0;
        if (layout == "BM(HK)") {
          *b = shape.At(0);
          *m = shape.At(1);
          packed_n = 1;
        } else if (layout == "MB(HK)") {
          *b = shape.At(1);
          *m = shape.At(0);
          packed_n = 1;
        } else if (layout == "BM(H2K)") {
          *b = shape.At(0);
          *m = shape.At(1);
          packed_n = 2;
        } else if (layout == "MB(H2K)") {
          *b = shape.At(1);
          *m = shape.At(0);
          packed_n = 2;
        } else if (layout == "BM(H3K)") {
          *b = shape.At(0);
          *m = shape.At(1);
          packed_n = 3;
        } else if (layout == "MB(H3K)") {
          *b = shape.At(1);
          *m = shape.At(0);
          packed_n = 3;
        } else {
          UNIMPLEMENTED_THEN_RETURN();
        }
        const int64_t hidden_size = shape.At(2);
        if (num_heads) {
          const int64_t expected_h = JUST(num_heads);
          const int64_t packed_h = packed_n * expected_h;
          CHECK_EQ_OR_RETURN(hidden_size % packed_h, 0);
          *h = expected_h;
          *k = hidden_size / packed_h;
        } else if (head_size) {
          const int64_t expected_k = JUST(head_size);
          const int64_t packed_k = packed_n * expected_k;
          CHECK_EQ_OR_RETURN(hidden_size % packed_k, 0);
          *h = hidden_size / packed_k;
          *k = expected_k;
        } else {
          UNIMPLEMENTED_THEN_RETURN();
        }
      } else {
        UNIMPLEMENTED_THEN_RETURN();
      }
    } else if (shape.NumAxes() == 4) {
      if (layout == "BMHK") {
        *b = shape.At(0);
        *m = shape.At(1);
        *h = shape.At(2);
        *k = shape.At(3);
      } else if (layout == "BHMK") {
        *b = shape.At(0);
        *m = shape.At(2);
        *h = shape.At(1);
        *k = shape.At(3);
      } else if (layout == "MBHK") {
        *b = shape.At(1);
        *m = shape.At(0);
        *h = shape.At(2);
        *k = shape.At(3);
      } else {
        UNIMPLEMENTED_THEN_RETURN();
      }
      if (num_heads) {
        const int64_t expected_h = JUST(num_heads);
        CHECK_EQ_OR_RETURN(*h, expected_h);
      }
      if (head_size) {
        const int64_t expected_k = JUST(head_size);
        CHECK_EQ_OR_RETURN(*k, expected_k);
      }
    } else {
      UNIMPLEMENTED_THEN_RETURN();
    };
    return Maybe<void>::Ok();
  };

  const Shape& query_shape = ctx->InputShape("query", 0);
  const std::string& query_layout = ctx->Attr<std::string>("query_layout");
  int64_t q_b = 0;
  int64_t q_m = 0;
  int64_t q_h = 0;
  int64_t q_k = 0;
  JUST(ParseDims(query_shape, query_layout, Optional<int64_t>(), query_head_size, &q_b, &q_m, &q_h,
                 &q_k));

  const Shape& key_shape = ctx->InputShape("key", 0);
  const std::string& key_layout = ctx->Attr<std::string>("key_layout");
  int64_t k_b = 0;
  int64_t k_m = 0;
  int64_t k_h = 0;
  int64_t k_k = 0;
  JUST(ParseDims(key_shape, key_layout, Optional<int64_t>(), q_k, &k_b, &k_m, &k_h, &k_k));
  CHECK_EQ_OR_RETURN(k_b, q_b);
  CHECK_EQ_OR_RETURN(k_h, q_h);

  const Shape& value_shape = ctx->InputShape("value", 0);
  const std::string& value_layout = ctx->Attr<std::string>("value_layout");
  int64_t v_b = 0;
  int64_t v_m = 0;
  int64_t v_h = 0;
  int64_t v_k = 0;
  JUST(ParseDims(value_shape, value_layout, q_h, Optional<int64_t>(), &v_b, &v_m, &v_h, &v_k));
  CHECK_EQ_OR_RETURN(v_b, q_b);
  CHECK_EQ_OR_RETURN(v_m, k_m);

  if (ctx->has_input("attn_bias", 0)) {
    const Shape& attn_bias_shape = ctx->InputShape("attn_bias", 0);
    const int64_t num_attn_bias_axes = attn_bias_shape.NumAxes();
    CHECK_GE_OR_RETURN(num_attn_bias_axes, 1);
    CHECK_LE_OR_RETURN(num_attn_bias_axes, 4);
    DimVector padded_attn_bias_shape;
    for (int i = 0; i < 4 - num_attn_bias_axes; ++i) { padded_attn_bias_shape.push_back(1); }
    for (int i = 0; i < num_attn_bias_axes; ++i) {
      padded_attn_bias_shape.push_back(attn_bias_shape.At(i));
    }
    CHECK_OR_RETURN(padded_attn_bias_shape.at(0) == 1 || padded_attn_bias_shape.at(0) == q_b);
    CHECK_OR_RETURN(padded_attn_bias_shape.at(1) == 1 || padded_attn_bias_shape.at(1) == q_h);
    CHECK_OR_RETURN(padded_attn_bias_shape.at(2) == 1 || padded_attn_bias_shape.at(2) >= q_m);
    CHECK_OR_RETURN(padded_attn_bias_shape.at(3) >= k_m);
  }
  const std::string& output_layout = ctx->Attr<std::string>("output_layout");
  if (output_layout == "BM(HK)") {
    ctx->SetOutputShape("out", 0, Shape({q_b, q_m, q_h * v_k}));
  } else if (output_layout == "MB(HK)") {
    ctx->SetOutputShape("out", 0, Shape({q_m, q_b, q_h * v_k}));
  } else {
    UNIMPLEMENTED_THEN_RETURN();
  }
  return Maybe<void>::Ok();
}
/*static*/ auto FusedMultiHeadAttentionInferenceOp::InferPhysicalTensorDesc(
    user_op::InferContext* ctx) -> Maybe<void> {
  return FusedMultiHeadAttentionInferenceOp::InferLogicalTensorDesc(ctx);
}
/*static*/ auto FusedMultiHeadAttentionInferenceOp::GetSbp(user_op::SbpContext* ctx)
    -> Maybe<void> {
  const int64_t query_head_size = ctx->user_op_conf().attr<int64_t>("query_head_size");
  const std::string& query_layout = ctx->user_op_conf().attr<std::string>("query_layout");
  const std::string& key_layout = ctx->user_op_conf().attr<std::string>("key_layout");
  const std::string& value_layout = ctx->user_op_conf().attr<std::string>("value_layout");
  const std::string& output_layout = ctx->user_op_conf().attr<std::string>("output_layout");
  int64_t num_heads = 0;
  const user_op::TensorDesc& query = ctx->LogicalTensorDesc4InputArgNameAndIndex("query", 0);
  if (query.shape().NumAxes() == 3) {
    if (query_layout == "BM(HK)" || query_layout == "MB(HK)") {
      CHECK_EQ_OR_RETURN(query.shape().At(2) % query_head_size, 0);
      num_heads = query.shape().At(2) / query_head_size;
    } else if (query_layout == "BM(H3K)" || query_layout == "MB(H3K)") {
      CHECK_EQ_OR_RETURN(query.shape().At(2) % (query_head_size * 3), 0);
      num_heads = query.shape().At(2) / (query_head_size * 3);
    } else {
      UNIMPLEMENTED_THEN_RETURN();
    }
  } else if (query.shape().NumAxes() == 4) {
    if (query_layout == "BMHK") {
      num_heads = query.shape().At(2);
    } else if (query_layout == "BHMK") {
      num_heads = query.shape().At(1);
    } else {
      UNIMPLEMENTED_THEN_RETURN();
    }
  }
  const bool can_hk_split = num_heads % ctx->parallel_num() == 0;
  const auto ParseSplitAxis = [ctx, can_hk_split](const std::string& layout, int64_t* b_split_axis,
                                                  int64_t* h_split_axis) -> Maybe<void> {
    if (layout == "BM(HK)" || layout == "BM(H2K)" || layout == "BM(H3K)") {
      *b_split_axis = 0;
      if (can_hk_split) {
        *h_split_axis = 2;
      } else {
        *h_split_axis = -1;
      }
    } else if (layout == "MB(HK)" || layout == "MB(H2K)" || layout == "MB(H3K)") {
      *b_split_axis = 1;
      if (can_hk_split) {
        *h_split_axis = 2;
      } else {
        *h_split_axis = -1;
      }
    } else if (layout == "BMHK") {
      *b_split_axis = 0;
      *h_split_axis = 2;
    } else if (layout == "BHMK") {
      *b_split_axis = 0;
      *h_split_axis = 1;
    } else if (layout == "MBHK") {
      *b_split_axis = 1;
      *h_split_axis = 2;
    } else {
      UNIMPLEMENTED_THEN_RETURN();
    }
    return Maybe<void>::Ok();
  };

  int64_t q_b_split_axis = -1;
  int64_t q_h_split_axis = -1;
  JUST(ParseSplitAxis(query_layout, &q_b_split_axis, &q_h_split_axis));
  int64_t k_b_split_axis = -1;
  int64_t k_h_split_axis = -1;
  JUST(ParseSplitAxis(key_layout, &k_b_split_axis, &k_h_split_axis));
  int64_t v_b_split_axis = -1;
  int64_t v_h_split_axis = -1;
  JUST(ParseSplitAxis(value_layout, &v_b_split_axis, &v_h_split_axis));
  int64_t o_b_split_axis = -1;
  int64_t o_h_split_axis = -1;
  JUST(ParseSplitAxis(output_layout, &o_b_split_axis, &o_h_split_axis));

  std::vector<user_op::OpArg> attn_bias_arg;
  if (ctx->user_op_conf().has_input("attn_bias", 0)) { attn_bias_arg.emplace_back("attn_bias", 0); }
  if (q_b_split_axis >= 0 && k_b_split_axis >= 0 && v_b_split_axis >= 0 && o_b_split_axis >= 0) {
    bool broadcast_attn_bias = false;
    if (ctx->user_op_conf().has_input("attn_bias", 0)) {
      const user_op::TensorDesc& attn_bias =
          ctx->LogicalTensorDesc4InputArgNameAndIndex("attn_bias", 0);
      if (attn_bias.shape().NumAxes() < 4 || attn_bias.shape().At(0) == 1) {
        broadcast_attn_bias = true;
      }
    }
    if (broadcast_attn_bias) {
      ctx->NewBuilder()
          .Split(user_op::OpArg("query", 0), q_b_split_axis)
          .Split(user_op::OpArg("key", 0), k_b_split_axis)
          .Split(user_op::OpArg("value", 0), v_b_split_axis)
          .Broadcast(attn_bias_arg)
          .Split(ctx->outputs(), o_b_split_axis)
          .Build();

    } else {
      ctx->NewBuilder()
          .Split(user_op::OpArg("query", 0), q_b_split_axis)
          .Split(user_op::OpArg("key", 0), k_b_split_axis)
          .Split(user_op::OpArg("value", 0), v_b_split_axis)
          .Split(attn_bias_arg, 0)
          .Split(ctx->outputs(), o_b_split_axis)
          .Build();
    }
  }
  if (q_h_split_axis >= 0 && k_h_split_axis >= 0 && v_h_split_axis >= 0 && o_h_split_axis >= 0) {
    bool broadcast_attn_bias = false;
    if (ctx->user_op_conf().has_input("attn_bias", 0)) {
      const user_op::TensorDesc& attn_bias =
          ctx->LogicalTensorDesc4InputArgNameAndIndex("attn_bias", 0);
      if (attn_bias.shape().NumAxes() == 4) {
        if (attn_bias.shape().At(1) == 1) { broadcast_attn_bias = true; }
      } else if (attn_bias.shape().NumAxes() == 3) {
        if (attn_bias.shape().At(0) == 1) { broadcast_attn_bias = true; }
      } else {
        broadcast_attn_bias = true;
      }
    }
    if (broadcast_attn_bias) {
      ctx->NewBuilder()
          .Split(user_op::OpArg("query", 0), q_h_split_axis)
          .Split(user_op::OpArg("key", 0), k_h_split_axis)
          .Split(user_op::OpArg("value", 0), v_h_split_axis)
          .Broadcast(attn_bias_arg)
          .Split(ctx->outputs(), o_h_split_axis)
          .Build();

    } else {
      ctx->NewBuilder()
          .Split(user_op::OpArg("query", 0), q_h_split_axis)
          .Split(user_op::OpArg("key", 0), k_h_split_axis)
          .Split(user_op::OpArg("value", 0), v_h_split_axis)
          .Split(attn_bias_arg, 1)
          .Split(ctx->outputs(), o_h_split_axis)
          .Build();
    }
  }
  return Maybe<void>::Ok();
}

}  // namespace oneflow
