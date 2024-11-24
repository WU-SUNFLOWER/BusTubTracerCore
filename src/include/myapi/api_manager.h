#pragma once

#include <string>
#include <functional>

#include "binder/binder.h"
#include "common/bustub_instance.h"
#include "common/exception.h"
#include "common/util/string_util.h"
#include "concurrency/transaction_manager.h"

#include "myapi/process_record_context.h"

class ApiManager {

private:

    struct ApiContext {
        rapidjson::Value &req_data;
        rapidjson::Value &resp_data;
        std::string &err_msg;
        rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> &resp_allocator;
        bustub::Transaction *txn;
    };

    using ApiFuncType = std::function<bool(ApiContext &)>;

    bustub::BustubInstance * const kBustubInstance_;

    std::unordered_map<std::string, ApiFuncType> kApiMap_;

public:

    ApiManager(bustub::BustubInstance *bustub_instance);

    bool SubmitSqlCommand(ApiContext &);

    bool QueryTableByName(ApiContext &);

    bool GetAllTables(ApiContext &);

    bool GetBufferPoolInfo(ApiContext &);

    bool GetTableHeapInfo(ApiContext &); 

    bool GetTablePageInfo(ApiContext &); 

    bool GetTupleInfo(ApiContext &);

    auto DispatchRequest(const std::string &request) -> std::string;

};