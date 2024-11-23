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

bool SubmitSqlCommand(
    rapidjson::Value &req_data, rapidjson::Value &resp_data, std::string &err_msg,
    rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> &resp_allocator,
    bustub::Transaction *
);

bool QueryTableByName(
    rapidjson::Value &req_data, rapidjson::Value &resp_data, std::string &err_msg,
    rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> &resp_allocator,
    bustub::Transaction *
);

}  // namespace myapi

using ApiFuncType = std::function<bool(
    rapidjson::Value &, rapidjson::Value &, std::string &,
    rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> &,
    bustub::Transaction *
)>;

auto DispatchRequest(const std::string &request) -> std::string;

#endif