#include "fmt/format.h"
#include "fmt/ranges.h"

#include "api_implementation.h"

bool myapi::SubmitSqlCommand(
    rapidjson::Value &req_data, rapidjson::Value &resp_data, std::string &err_msg,
    rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> &resp_allocator,
    bustub::Transaction *txn
) {
    if (!req_data.HasMember("sql") || !req_data["sql"].IsString()) {
        err_msg = "Missing or invalid 'sql' field";
        return false;
    }

    std::string sql_command = req_data["sql"].GetString();
    std::string sql_result;

    try {
        auto writer = bustub::FortTableWriter();
        kBusbubInstance->ExecuteSqlTxn(sql_command, writer, txn);
        for (const auto &table : writer.tables_) {
            sql_result += table;
        }
    } catch (const bustub::Exception &ex) {
        err_msg = ex.what();
        return false;
    } catch (const std::exception &ex) {
        err_msg = ex.what();
        return false;
    } catch (...) {
        err_msg = "An unknown error occurred.";
        return false;
    }

    resp_data.AddMember(
        "raw_result",
        rapidjson::Value(sql_result.c_str(), resp_allocator),
        resp_allocator
    );

    return true;
}

bool myapi::QueryTableByName(
    rapidjson::Value &req_data, rapidjson::Value &resp_data, std::string &err_msg,
    rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> &resp_allocator,
    bustub::Transaction *txn
) {

    if (!req_data.HasMember("table_name") || !req_data["table_name"].IsString()) {
        err_msg = "Missing or invalid 'table_name' field";
        return false;
    }

    std::string table_name = req_data["table_name"].GetString();
    bustub::TableInfo* table_info = kBusbubInstance->catalog_->GetTable(table_name);
    if (table_info == bustub::Catalog::NULL_TABLE_INFO) {
        err_msg = "Can't find table matched with 'table_name' field";
        return false;
    }

    // set table id filed
    resp_data.AddMember("table_id", rapidjson::Value(table_info->oid_), resp_allocator);
    // set table name filed
    resp_data.AddMember(
        "table_name", 
        rapidjson::Value(table_info->name_.c_str(), resp_allocator),
        resp_allocator
    );

    // set column names field
    rapidjson::Value resp_column_names(rapidjson::kArrayType);
    bustub::Schema &table_schema = table_info->schema_;
    const std::vector<bustub::Column> &table_columns = table_schema.GetColumns();
    for (const auto &column : table_columns) {
        resp_column_names.PushBack(
            rapidjson::Value(column.GetName().c_str(), resp_allocator),
            resp_allocator
        );
    }
    resp_data.AddMember("column_names", resp_column_names, resp_allocator);

    rapidjson::Value resp_tuples(rapidjson::kArrayType);

    bustub::TableHeap *table_heap = table_info->table_.get();
    bustub::TableIterator table_iter = table_heap->Begin(txn);
    while (table_iter != table_heap->End()) {
        const bustub::Tuple &tuple = *table_iter;
        bustub::RID rid = tuple.GetRid();

        rapidjson::Value resp_tuple_object(rapidjson::kObjectType);

        rapidjson::Value resp_rid_object(rapidjson::kObjectType);
        resp_rid_object.AddMember("page_id", rapidjson::Value(rid.GetPageId()), resp_allocator);
        resp_rid_object.AddMember("slot_num", rapidjson::Value(rid.GetSlotNum()), resp_allocator);
        resp_tuple_object.AddMember("rid", resp_rid_object, resp_allocator);

        rapidjson::Value resp_tuple_columns(rapidjson::kArrayType);
        for (size_t i = 0; i < table_columns.size(); ++i) {
            bustub::Value value = tuple.GetValue(&table_schema, i);
            resp_tuple_columns.PushBack(
                rapidjson::Value(value.ToString().c_str(), resp_allocator), 
                resp_allocator
            );
        }
        resp_tuple_object.AddMember("columns", resp_tuple_columns, resp_allocator);

        resp_tuples.PushBack(resp_tuple_object, resp_allocator);

        ++table_iter;
    }
    resp_data.AddMember("tuples", resp_tuples, resp_allocator);

    return true;
}

static const std::unordered_map<std::string, ApiFuncType> kApiMap = {
    {"/submit_sql_command", &myapi::SubmitSqlCommand},
    {"/query_table_by_name", &myapi::QueryTableByName},
};

auto DispatchRequest(const std::string &request) -> std::string {
    
    /* preprocess request json */
    rapidjson::Document req_json;
    if (req_json.Parse(request.c_str()).HasParseError()) {
        return R"({"err_msg": "Invalid JSON format"})";
    }
    if (!req_json.HasMember("api") || !req_json["api"].IsString()) {
        return R"({"err_msg": "Missing or invalid 'api' field"})";
    }
    if (!req_json.HasMember("data") || !req_json["data"].IsObject()) {
        return R"({"err_msg": "Missing or invalid 'data' field"})";
    }

    std::string api_name = req_json["api"].GetString();
    rapidjson::Value &req_data = req_json["data"];

    /* preprocess respond_json */
    rapidjson::Document resp_json;
    resp_json.SetObject();
    auto& resp_allocator = resp_json.GetAllocator();

    resp_json.AddMember(
        "data", 
        rapidjson::Value(rapidjson::kObjectType), 
        resp_allocator
    );

    rapidjson::Value &resp_data = resp_json["data"];

    /* preprocess json_writer */
    rapidjson::StringBuffer json_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> json_writer(json_buffer);

    /* preprocess err_msg */
    std::string err_msg;

    /* dispatch start!!! */

    auto api_iter = kApiMap.find(api_name);

    if (api_iter != kApiMap.end()) {
        ApiFuncType api_func = api_iter->second;
        bustub::Transaction *txn = kBusbubInstance->txn_manager_->Begin();

        // transcation start
        bool result = api_func(req_data, resp_data, err_msg, resp_allocator, txn);
        std::string result_json;
        if (result) {
            resp_json.Accept(json_writer);
            result_json = json_buffer.GetString();
        } else {
            result_json = fmt::format(R"({{ "err_msg": "{}" }})", err_msg);
        }
        // transcation end

        kBusbubInstance->txn_manager_->Commit(txn);
        delete txn;
        return result_json;
    }

    return R"({"err_msg": "Unknown API"})";
}