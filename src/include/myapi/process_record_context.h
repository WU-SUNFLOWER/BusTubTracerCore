#pragma once

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using rapidjson_allocator_t = rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>;

struct ProcessRecordContext {

    bool can_record_;
    rapidjson::Value planner_tree_record_;
    rapidjson::Value opt_planner_tree_record_;
    rapidjson::Value exec_tree_record_;
    rapidjson_allocator_t &allocator_;

    ProcessRecordContext(rapidjson_allocator_t &allocator)
    : can_record_(false), planner_tree_record_(rapidjson::kObjectType)
    , opt_planner_tree_record_(rapidjson::kObjectType), exec_tree_record_(rapidjson::kObjectType)
    , allocator_(allocator) {}
    
    auto CanRecord() -> bool {
        return can_record_;
    }

    void SetCanRecord() {
        can_record_ = true;
    }

    void Save(rapidjson::Value &wrapper) {
        wrapper.AddMember("planner_tree", planner_tree_record_, allocator_);
        wrapper.AddMember("optimized_planner_tree", opt_planner_tree_record_, allocator_);
        wrapper.AddMember("executor_tree", exec_tree_record_, allocator_);
    }
};