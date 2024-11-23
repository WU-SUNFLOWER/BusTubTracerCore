#ifndef API_IMPLEMENTATION_H_
#define API_IMPLEMENTATION_H_

#include <string>
#include <functional>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "binder/binder.h"
#include "common/bustub_instance.h"
#include "common/exception.h"
#include "common/util/string_util.h"
#include "concurrency/transaction_manager.h"

extern std::unique_ptr<bustub::BustubInstance> kBusbubInstance;

namespace myapi {

struct ApiContext {
    rapidjson::Value &req_data;
    rapidjson::Value &resp_data;
    std::string &err_msg;
    rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> &resp_allocator;
    bustub::Transaction *txn;
};

using ApiFuncType = std::function<bool(ApiContext &)>;

bool SubmitSqlCommand(ApiContext &);

bool QueryTableByName(ApiContext &);

bool GetAllTables(ApiContext &);

bool GetBufferPoolInfo(ApiContext &);

}  // namespace myapi

auto DispatchRequest(const std::string &request) -> std::string;

#endif