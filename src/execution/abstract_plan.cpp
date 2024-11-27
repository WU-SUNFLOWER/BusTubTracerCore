#include "execution/plans/abstract_plan.h"

std::atomic<bustub::plan_node_id_t> bustub::AbstractPlanNode::next_plan_node_id_ = 0;