#pragma once

#include <vector>
#include <string>
#include <utility>
#include <unordered_map>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "storage/table/tuple.h"
#include "catalog/catalog.h"

namespace bustub {

using plan_node_id_t = uint32_t;

class AbstractPlanNode;

}

using rapidjson_allocator_t = rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>;

class ProcessRecordContext {

private:

    bool can_record_;
    rapidjson::Value planner_tree_record_;
    rapidjson::Value opt_planner_tree_record_;
    rapidjson::Value exec_tree_record_;
    rapidjson_allocator_t &allocator_;

    std::unordered_map<bustub::plan_node_id_t, std::pair<std::vector<bustub::Tuple>, const bustub::Schema *>> exec_recorder_ {};

public:

    ProcessRecordContext(rapidjson_allocator_t &allocator)
    : can_record_(false), planner_tree_record_(rapidjson::kObjectType)
    , opt_planner_tree_record_(rapidjson::kObjectType), exec_tree_record_(rapidjson::kArrayType)
    , allocator_(allocator) {}
    
    auto CanRecord() -> bool {
        return can_record_;
    }

    void SetCanRecord() {
        can_record_ = true;
    }

    rapidjson::Value & GetPlannerTreeRecord() {
        return planner_tree_record_;
    }

    rapidjson::Value & GetOptPlannerTreeRecord() {
        return opt_planner_tree_record_;
    }

    rapidjson_allocator_t & GetAllocator() {
        return allocator_;
    }

    void Save(rapidjson::Value &wrapper);

    void AddToExecRecorder(const bustub::AbstractPlanNode *plan_node, const bustub::Tuple &tuple);

    void SaveExecutionRecord();
};