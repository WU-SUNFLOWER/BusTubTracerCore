#include "execution/executors/values_executor.h"

namespace bustub {

ValuesExecutor::ValuesExecutor(ExecutorContext *exec_ctx, const ValuesPlanNode *plan)
    : AbstractExecutor(exec_ctx), plan_(plan), dummy_schema_(Schema({})) {}

void ValuesExecutor::Init(ProcessRecordContext *ptx) { 
  cursor_ = 0; 
}

auto ValuesExecutor::Next(Tuple *tuple, RID *rid, ProcessRecordContext *ptx) -> bool {
  if (cursor_ >= plan_->GetValues().size()) {
    return false;
  }

  std::vector<Value> values{};
  values.reserve(GetOutputSchema().GetColumnCount());

  const auto &row_expr = plan_->GetValues()[cursor_];
  for (const auto &col : row_expr) {
    values.push_back(col->Evaluate(nullptr, dummy_schema_));
  }

  *tuple = Tuple{values, &GetOutputSchema()};
  if (ptx) ptx->AddToExecRecorder(plan_, *tuple);

  cursor_ += 1;

  return true;
}

}  // namespace bustub
