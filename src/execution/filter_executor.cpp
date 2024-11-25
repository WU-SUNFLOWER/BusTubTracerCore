#include "execution/executors/filter_executor.h"
#include "common/exception.h"
#include "type/value_factory.h"

namespace bustub {

FilterExecutor::FilterExecutor(ExecutorContext *exec_ctx, const FilterPlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {}

void FilterExecutor::Init(ProcessRecordContext *ptx) {
  // Initialize the child executor
  child_executor_->Init(ptx);
}

auto FilterExecutor::Next(Tuple *tuple, RID *rid, ProcessRecordContext *ptx) -> bool {
  auto filter_expr = plan_->GetPredicate();

  while (true) {
    // Get the next tuple
    const auto status = child_executor_->Next(tuple, rid, ptx);

    if (!status) {
      return false;
    }

    auto value = filter_expr->Evaluate(tuple, child_executor_->GetOutputSchema());
    if (!value.IsNull() && value.GetAs<bool>()) {
      if (ptx) ptx->AddToExecRecorder(plan_, *tuple);
      return true;
    }
  }
}

}  // namespace bustub
