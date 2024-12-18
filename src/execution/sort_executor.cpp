#include "execution/executors/sort_executor.h"

namespace bustub {

SortExecutor::SortExecutor(ExecutorContext *exec_ctx, const SortPlanNode *plan,
                           std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {}

void SortExecutor::Init(ProcessRecordContext *ptx) {
  Tuple tuple;
  RID rid;
  child_executor_->Init(ptx);
  while (child_executor_->Next(&tuple, &rid, ptx)) {
    sorted_tuples_.push_back(tuple);
  }
  std::sort(sorted_tuples_.begin(), sorted_tuples_.end(), [this](const Tuple &a, const Tuple &b) {
    for (auto [order_by_type, expr] : plan_->GetOrderBy()) {
      bool default_order_by = (order_by_type == OrderByType::DEFAULT || order_by_type == OrderByType::ASC);
      if (expr->Evaluate(&a, child_executor_->GetOutputSchema())
              .CompareLessThan(expr->Evaluate(&b, child_executor_->GetOutputSchema())) == CmpBool::CmpTrue) {
        return default_order_by;
      }
      if (expr->Evaluate(&a, child_executor_->GetOutputSchema())
              .CompareGreaterThan(expr->Evaluate(&b, child_executor_->GetOutputSchema())) == CmpBool::CmpTrue) {
        return !default_order_by;
      }
    }
    // Reference: https://blog.csdn.net/code_debugger/article/details/43833231
    return false;
  });
  iterator_ = sorted_tuples_.begin();
}

auto SortExecutor::Next(Tuple *tuple, RID *rid, ProcessRecordContext *ptx) -> bool {
  if (iterator_ != sorted_tuples_.end()) {

    *tuple = *iterator_;
    if (ptx) ptx->AddToExecRecorder(plan_, *tuple);

    iterator_++;
    return true;
  }
  return false;
}

}  // namespace bustub
