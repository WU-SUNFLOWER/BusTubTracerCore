#include <type_traits>
#include "execution/plans/update_plan.h"
#include "fmt/format.h"
#include "fmt/ranges.h"

#include "common/util/string_util.h"
#include "execution/expressions/abstract_expression.h"
#include "execution/plans/abstract_plan.h"
#include "execution/plans/aggregation_plan.h"
#include "execution/plans/limit_plan.h"
#include "execution/plans/projection_plan.h"
#include "execution/plans/sort_plan.h"
#include "execution/plans/topn_plan.h"

namespace bustub {

auto AbstractPlanNode::ChildrenToString(int indent, bool with_schema) const -> std::string {
  if (children_.empty()) {
    return "";
  }
  std::vector<std::string> children_str;
  children_str.reserve(children_.size());
  auto indent_str = StringUtil::Indent(indent);
  for (const auto &child : children_) {
    auto child_str = child->ToString(with_schema);
    auto lines = StringUtil::Split(child_str, '\n');
    for (auto &line : lines) {
      children_str.push_back(fmt::format("{}{}", indent_str, line));
    }
  }
  return fmt::format("\n{}", fmt::join(children_str, "\n"));
}

// PlanNodeToString
auto AggregationPlanNode::PlanNodeToString() const -> std::string {
  return fmt::format("Agg {{ types={}, aggregates={}, group_by={} }}", agg_types_, aggregates_, group_bys_);
}

auto ProjectionPlanNode::PlanNodeToString() const -> std::string {
  return fmt::format("Projection {{ exprs={} }}", expressions_);
}

auto UpdatePlanNode::PlanNodeToString() const -> std::string {
  return fmt::format("Update {{ table_oid={}, target_exprs={} }}", table_oid_, target_expressions_);
}

auto SortPlanNode::PlanNodeToString() const -> std::string {
  return fmt::format("Sort {{ order_bys={} }}", order_bys_);
}

auto LimitPlanNode::PlanNodeToString() const -> std::string { return fmt::format("Limit {{ limit={} }}", limit_); }

auto TopNPlanNode::PlanNodeToString() const -> std::string {
  return fmt::format("TopN {{ n={}, order_bys={}}}", n_, order_bys_);
}

// PlanNodeToJSON
void AggregationPlanNode::PlanNodeToJSON(rapidjson::Value &json_attr, rapidjson_allocator_t &json_alloc) const {

  json_attr.AddMember("types", rapidjson::Value(fmt::format("{}", agg_types_).c_str(), json_alloc), json_alloc);
  json_attr.AddMember("aggregates", rapidjson::Value(fmt::format("{}", aggregates_).c_str(), json_alloc), json_alloc);
  json_attr.AddMember("group_by", rapidjson::Value(fmt::format("{}", group_bys_).c_str(), json_alloc), json_alloc);
  
}

void ProjectionPlanNode::PlanNodeToJSON(rapidjson::Value &json_attr, rapidjson_allocator_t &json_alloc) const {
  json_attr.AddMember(
    "expressions", 
    rapidjson::Value(fmt::format("{}", expressions_).c_str(), json_alloc),
    json_alloc
  );
}

void UpdatePlanNode::PlanNodeToJSON(rapidjson::Value &json_attr, rapidjson_allocator_t &json_alloc) const {
  json_attr.AddMember("table_oid", table_oid_, json_alloc);
  json_attr.AddMember("target_exprs", rapidjson::Value(fmt::format("{}", target_expressions_).c_str(), json_alloc), json_alloc);
}

void SortPlanNode::PlanNodeToJSON(rapidjson::Value &json_attr, rapidjson_allocator_t &json_alloc) const {
  json_attr.AddMember("order_bys", rapidjson::Value(fmt::format("{}", order_bys_).c_str(), json_alloc), json_alloc);
}

void LimitPlanNode::PlanNodeToJSON(rapidjson::Value &json_attr, rapidjson_allocator_t &json_alloc) const {
  json_attr.AddMember("limit", limit_, json_alloc);
}

void TopNPlanNode::PlanNodeToJSON(rapidjson::Value &json_attr, rapidjson_allocator_t &json_alloc) const {
  json_attr.AddMember("n", n_, json_alloc);
  json_attr.AddMember("order_bys", rapidjson::Value(fmt::format("{}", order_bys_).c_str(), json_alloc), json_alloc);
}

}  // namespace bustub
