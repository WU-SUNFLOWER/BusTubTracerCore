#include "execution/executors/projection_executor.h"
#include "storage/table/tuple.h"

namespace bustub {

ProjectionExecutor::ProjectionExecutor(ExecutorContext *exec_ctx, const ProjectionPlanNode *plan,
                                       std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {}

void ProjectionExecutor::Init(ProcessRecordContext *ptx) {
  // Initialize the child executor
  child_executor_->Init(ptx);
}

auto ProjectionExecutor::Next(Tuple *tuple, RID *rid, ProcessRecordContext *ptx) -> bool {
  Tuple child_tuple{};

  // Get the next tuple
  const auto status = child_executor_->Next(&child_tuple, rid, ptx);

  if (!status) {
    return false;
  }

  // Compute expressions
  std::vector<Value> values{};
  values.reserve(GetOutputSchema().GetColumnCount());
  for (const auto &expr : plan_->GetExpressions()) {
    values.push_back(expr->Evaluate(&child_tuple, child_executor_->GetOutputSchema()));
  }

  *tuple = Tuple{values, &GetOutputSchema()};
  
  if (ptx) ptx->AddToExecRecorder(plan_, *tuple);

  return true;
}
}  // namespace bustub
