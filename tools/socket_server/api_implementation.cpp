#include "fmt/format.h"
#include "fmt/ranges.h"

#include "api_implementation.h"

bool myapi::SubmitSqlCommand(ApiContext &ctx) 
{
    if (!ctx.req_data.HasMember("sql") || !ctx.req_data["sql"].IsString()) {
        ctx.err_msg = "Missing or invalid 'sql' field";
        return false;
    }

    std::string sql_command = ctx.req_data["sql"].GetString();
    std::string sql_result;

    try {
        auto writer = bustub::FortTableWriter();
        kBusbubInstance->ExecuteSqlTxn(sql_command, writer, ctx.txn);
        for (const auto &table : writer.tables_) {
            sql_result += table;
        }
    } catch (const bustub::Exception &ex) {
        ctx.err_msg = ex.what();
        return false;
    } catch (const std::exception &ex) {
        ctx.err_msg = ex.what();
        return false;
    } catch (...) {
        ctx.err_msg = "An unknown error occurred.";
        return false;
    }

    ctx.resp_data.AddMember(
        "raw_result",
        rapidjson::Value(sql_result.c_str(), ctx.resp_allocator),
        ctx.resp_allocator
    );

    return true;
}

bool myapi::QueryTableByName(ApiContext &ctx) 
{

    if (!ctx.req_data.HasMember("table_name") || !ctx.req_data["table_name"].IsString()) {
        ctx.err_msg = "Missing or invalid 'table_name' field";
        return false;
    }

    std::string table_name = ctx.req_data["table_name"].GetString();
    bustub::TableInfo* table_info = kBusbubInstance->catalog_->GetTable(table_name);
    if (table_info == bustub::Catalog::NULL_TABLE_INFO) {
        ctx.err_msg = "Can't find table matched with 'table_name' field";
        return false;
    }

    // set table id filed
    ctx.resp_data.AddMember("table_oid", rapidjson::Value(table_info->oid_), ctx.resp_allocator);
    // set table name filed
    ctx.resp_data.AddMember(
        "table_name", 
        rapidjson::Value(table_info->name_.c_str(), ctx.resp_allocator),
        ctx.resp_allocator
    );

    // set column names field
    rapidjson::Value resp_column_names(rapidjson::kArrayType);
    bustub::Schema &table_schema = table_info->schema_;
    const std::vector<bustub::Column> &table_columns = table_schema.GetColumns();
    for (const auto &column : table_columns) {
        resp_column_names.PushBack(
            rapidjson::Value(column.GetName().c_str(), ctx.resp_allocator),
            ctx.resp_allocator
        );
    }
    ctx.resp_data.AddMember("column_names", resp_column_names, ctx.resp_allocator);

    rapidjson::Value resp_tuples(rapidjson::kArrayType);

    bustub::TableHeap *table_heap = table_info->table_.get();
    bustub::TableIterator table_iter = table_heap->Begin(ctx.txn);
    while (table_iter != table_heap->End()) {
        const bustub::Tuple &tuple = *table_iter;
        bustub::RID rid = tuple.GetRid();

        rapidjson::Value resp_tuple_object(rapidjson::kObjectType);

        rapidjson::Value resp_rid_object(rapidjson::kObjectType);
        resp_rid_object.AddMember("page_id", rapidjson::Value(rid.GetPageId()), ctx.resp_allocator);
        resp_rid_object.AddMember("slot_num", rapidjson::Value(rid.GetSlotNum()), ctx.resp_allocator);
        resp_tuple_object.AddMember("rid", resp_rid_object, ctx.resp_allocator);

        rapidjson::Value resp_tuple_columns(rapidjson::kArrayType);
        for (size_t i = 0; i < table_columns.size(); ++i) {
            bustub::Value value = tuple.GetValue(&table_schema, i);
            resp_tuple_columns.PushBack(
                rapidjson::Value(value.ToString().c_str(), ctx.resp_allocator), 
                ctx.resp_allocator
            );
        }
        resp_tuple_object.AddMember("columns", resp_tuple_columns, ctx.resp_allocator);

        resp_tuples.PushBack(resp_tuple_object, ctx.resp_allocator);

        ++table_iter;
    }
    ctx.resp_data.AddMember("tuples", resp_tuples, ctx.resp_allocator);

    return true;
}

bool myapi::GetAllTables(ApiContext &ctx) {
    
    std::vector<std::string> table_names = kBusbubInstance->catalog_->GetTableNames();

    rapidjson::Value resp_tables(rapidjson::kArrayType);

    for (const std::string &name : table_names) {
        bustub::TableInfo *table = kBusbubInstance->catalog_->GetTable(name);

        rapidjson::Value resp_table_info(rapidjson::kObjectType);
        resp_table_info.AddMember(
            "table_oid", 
            rapidjson::Value(table->oid_), 
            ctx.resp_allocator
        );
        resp_table_info.AddMember(
            "table_name",
            rapidjson::Value(table->name_.c_str(), ctx.resp_allocator),
            ctx.resp_allocator
        );
        resp_tables.PushBack(resp_table_info, ctx.resp_allocator);
    }

    ctx.resp_data.AddMember("tables", resp_tables, ctx.resp_allocator);
    return true;
}

bool myapi::GetBufferPoolInfo(ApiContext &ctx) {

}

static const std::unordered_map<std::string, myapi::ApiFuncType> kApiMap = {
    {"/submit_sql_command", &myapi::SubmitSqlCommand},
    {"/query_table_by_name", &myapi::QueryTableByName},
    {"/get_all_tables", &myapi::GetAllTables},
    {"/get_buffer_pool_info", &myapi::GetBufferPoolInfo},
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
        myapi::ApiFuncType api_func = api_iter->second;
        bustub::Transaction *txn = kBusbubInstance->txn_manager_->Begin();

        // transcation start
        myapi::ApiContext api_context = 
            {req_data, resp_data, err_msg, resp_allocator, txn};
        bool result = api_func(api_context);
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