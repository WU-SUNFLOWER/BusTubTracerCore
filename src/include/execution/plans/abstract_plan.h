//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// abstract_plan.h
//
// Identification: src/include/execution/plans/abstract_plan.h
//
// Copyright (c) 2015-19, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <atomic>

#include "myapi/process_record_context.h"

#include "catalog/schema.h"
#include "fmt/format.h"

namespace bustub {

#define BUSTUB_PLAN_NODE_CLONE_WITH_CHILDREN(cname)                                                            \
  auto CloneWithChildren(std::vector<AbstractPlanNodeRef> children) const -> std::unique_ptr<AbstractPlanNode> \
                                                                              override {                       \
    auto plan_node = cname(*this);                                                                             \
    plan_node.children_ = children;                                                                            \
    return std::make_unique<cname>(std::move(plan_node));                                                      \
  }

/** PlanType represents the types of plans that we have in our system. */
enum class PlanType {
  SeqScan,
  IndexScan,
  Insert,
  Update,
  Delete,
  Aggregation,
  Limit,
  NestedLoopJoin,
  NestedIndexJoin,
  HashJoin,
  Filter,
  Values,
  Projection,
  Sort,
  TopN,
  MockScan
};  
  
#define PlanNodeNameMapItem(name) { PlanType::name, #name }
const std::unordered_map<PlanType, std::string> kPlanNodeNameMap = {
  PlanNodeNameMapItem(SeqScan),
  PlanNodeNameMapItem(IndexScan),
  PlanNodeNameMapItem(Insert),
  PlanNodeNameMapItem(Update),
  PlanNodeNameMapItem(Delete),
  PlanNodeNameMapItem(Aggregation),
  PlanNodeNameMapItem(Limit),
  PlanNodeNameMapItem(NestedLoopJoin),
  PlanNodeNameMapItem(NestedIndexJoin),
  PlanNodeNameMapItem(HashJoin),
  PlanNodeNameMapItem(Filter),
  PlanNodeNameMapItem(Values),
  PlanNodeNameMapItem(Projection),
  PlanNodeNameMapItem(Sort),
  PlanNodeNameMapItem(TopN),
  PlanNodeNameMapItem(MockScan)
};
#undef PlanNodeNameMapItem

class AbstractPlanNode;
using AbstractPlanNodeRef = std::shared_ptr<const AbstractPlanNode>;

using plan_node_id_t = uint32_t;

/**
 * AbstractPlanNode represents all the possible types of plan nodes in our system.
 * Plan nodes are modeled as trees, so each plan node can have a variable number of children.
 * Per the Volcano model, the plan node receives the tuples of its children.
 * The ordering of the children may matter.
 */
class AbstractPlanNode {
 public:

  static std::atomic<plan_node_id_t> next_plan_node_id_;

  /**
   * Create a new AbstractPlanNode with the specified output schema and children.
   * @param output_schema the schema for the output of this plan node
   * @param children the children of this plan node
   */
  AbstractPlanNode(SchemaRef output_schema, std::vector<AbstractPlanNodeRef> children)
      : output_schema_(std::move(output_schema)), children_(std::move(children))
      , id_(next_plan_node_id_.fetch_add(1)) {}

  /** Virtual destructor. */
  virtual ~AbstractPlanNode() = default;

  /** @return the schema for the output of this plan node */
  auto OutputSchema() const -> const Schema & { return *output_schema_; }

  /** @return the child of this plan node at index child_idx */
  auto GetChildAt(uint32_t child_idx) const -> AbstractPlanNodeRef { return children_[child_idx]; }

  /** @return the children of this plan node */
  auto GetChildren() const -> const std::vector<AbstractPlanNodeRef> & { return children_; }

  /** @return the name of this plan node */
  auto GetNodeName() const -> std::string {
    auto iter = kPlanNodeNameMap.find(GetType());
    if (iter != kPlanNodeNameMap.end()) {
      return iter->second;
    }
    return "<unknown>";
  }

  /** @return the type of this plan node */
  virtual auto GetType() const -> PlanType = 0;

  /** @return the string representation of the plan node and its children */
  auto ToString(bool with_schema = true) const -> std::string {
    if (with_schema) {
      return fmt::format("{} | {}{}", PlanNodeToString(), output_schema_, ChildrenToString(2, with_schema));
    }
    return fmt::format("{}{}", PlanNodeToString(), ChildrenToString(2, with_schema));
  }

  void ToJSON(rapidjson::Value &json_object, rapidjson_allocator_t &json_alloc) const {

    json_object.AddMember("planner_node_tag", rapidjson::Value(GetNodeName().c_str(), json_alloc), json_alloc);
    json_object.AddMember("planner_node_id", id_, json_alloc);

    rapidjson::Value json_attr(rapidjson::kObjectType);
    PlanNodeToJSON(json_attr, json_alloc);
    json_object.AddMember("planner_node_attr", json_attr, json_alloc);

    if (GetType() == PlanType::SeqScan || GetType() == PlanType::Values) return;

    rapidjson::Value children(rapidjson::kArrayType);
    for (const auto &child : children_) {
      rapidjson::Value child_json_object(rapidjson::kObjectType);
      child->ToJSON(child_json_object, json_alloc);
      children.PushBack(child_json_object, json_alloc);
    }
    json_object.AddMember("children", children, json_alloc);

  }

  /** @return the cloned plan node with new children */
  virtual auto CloneWithChildren(std::vector<AbstractPlanNodeRef> children) const
      -> std::unique_ptr<AbstractPlanNode> = 0;


  /**
   * The schema for the output of this plan node. In the volcano model, every plan node will spit out tuples,
   * and this tells you what schema this plan node's tuples will have.
   */
  SchemaRef output_schema_;

  /** The children of this plan node. */
  std::vector<AbstractPlanNodeRef> children_;

  plan_node_id_t id_;

 protected:
  /** @return the string representation of the plan node itself */
  virtual auto PlanNodeToString() const -> std::string { return "<unknown>"; }

  virtual void PlanNodeToJSON(rapidjson::Value &json_object, rapidjson_allocator_t &json_alloc) const = 0;

  /** @return the string representation of the plan node's children */
  auto ChildrenToString(int indent, bool with_schema = true) const -> std::string;

 private:
};

}  // namespace bustub

template <typename T>
struct fmt::formatter<T, std::enable_if_t<std::is_base_of<bustub::AbstractPlanNode, T>::value, char>>
    : fmt::formatter<std::string> {
  template <typename FormatCtx>
  auto format(const T &x, FormatCtx &ctx) const {
    return fmt::formatter<std::string>::format(x.ToString(), ctx);
  }
};

template <typename T>
struct fmt::formatter<std::unique_ptr<T>, std::enable_if_t<std::is_base_of<bustub::AbstractPlanNode, T>::value, char>>
    : fmt::formatter<std::string> {
  template <typename FormatCtx>
  auto format(const std::unique_ptr<T> &x, FormatCtx &ctx) const {
    return fmt::formatter<std::string>::format(x->ToString(), ctx);
  }
};
