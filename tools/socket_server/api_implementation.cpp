#include "fmt/format.h"
#include "fmt/ranges.h"

#include "buffer/buffer_pool_manager_instance.h"

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

    rapidjson::Value resp_indices(rapidjson::kArrayType);
    std::vector<bustub::IndexInfo *> table_indexes = 
        kBusbubInstance->catalog_->GetTableIndexes(table_name);
    for (auto &index_info : table_indexes) {
        rapidjson::Value resp_index_info(rapidjson::kObjectType);
        resp_index_info.AddMember(
            "index_oid", 
            rapidjson::Value(index_info->index_oid_), 
            ctx.resp_allocator
        );
        resp_index_info.AddMember(
            "index_name",
            rapidjson::Value(index_info->name_.c_str(), ctx.resp_allocator),
            ctx.resp_allocator
        );
        resp_index_info.AddMember(
            "key_schema",
            rapidjson::Value(index_info->key_schema_.ToString().c_str(), ctx.resp_allocator),
            ctx.resp_allocator
        );
        resp_index_info.AddMember(
            "key_size",
            rapidjson::Value(index_info->key_size_),
            ctx.resp_allocator
        );
        resp_indices.PushBack(resp_index_info, ctx.resp_allocator);
    }
    ctx.resp_data.AddMember("indices", resp_indices, ctx.resp_allocator);

    return true;
}

bool myapi::GetAllTables(ApiContext &ctx) {
    
    if (ctx.req_data.MemberCount() != 0) {
        ctx.err_msg = "This api hasn't any parameter.";
        return false;
    }

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
    
    if (ctx.req_data.MemberCount() != 0) {
        ctx.err_msg = "This api hasn't any parameter.";
        return false;
    }
    
    bustub::BufferPoolManagerInstance *buffer_pool = 
        dynamic_cast<bustub::BufferPoolManagerInstance*>(kBusbubInstance->buffer_pool_manager_);

    if (buffer_pool == nullptr) {
        ctx.err_msg = "Fail to access buffer pool of BusTub.";
        return false;
    }

    std::unordered_set<bustub::frame_id_t> free_frame_ids;
    for (auto &frame_id : buffer_pool->GetFreeList()) {
        free_frame_ids.emplace(frame_id);
    }

    rapidjson::Value resp_info(rapidjson::kArrayType);
    bustub::Page *pages = buffer_pool->GetPages();
    for (size_t i = 0; i < buffer_pool->GetPoolSize(); ++i) {
        bustub::Page &page = pages[i];
        rapidjson::Value resp_page_info(rapidjson::kObjectType);
        resp_page_info.AddMember(
            "frame_id", 
            rapidjson::Value(i), 
            ctx.resp_allocator
        );
        resp_page_info.AddMember(
            "page_id", 
            rapidjson::Value(page.GetPageId()), 
            ctx.resp_allocator
        );
        resp_page_info.AddMember(
            "is_dirty",
            rapidjson::Value(page.IsDirty()),
            ctx.resp_allocator
        );
        resp_page_info.AddMember(
            "pin_count",
            rapidjson::Value(page.GetPinCount()),
            ctx.resp_allocator
        );
        resp_page_info.AddMember(
            "is_free",
            rapidjson::Value(free_frame_ids.find(i) != free_frame_ids.end()),
            ctx.resp_allocator
        );
        resp_info.PushBack(resp_page_info, ctx.resp_allocator);
    }

    ctx.resp_data.AddMember("buffer_pool_info", resp_info, ctx.resp_allocator);
    return true;
}

bool myapi::GetTableHeapInfo(ApiContext &ctx) {

    if (!ctx.req_data.HasMember("table_oid") || !ctx.req_data["table_oid"].IsNumber()) {
        ctx.err_msg = "Missing or invalid 'table_oid' field";
        return false;
    }

    bustub::TableInfo *table_info = 
        kBusbubInstance->catalog_->GetTable(ctx.req_data["table_oid"].GetUint());
    bustub::TableHeap *table_heap = table_info->table_.get();
    bustub::page_id_t first_page_id = table_heap->GetFirstPageId();

    rapidjson::Value resp_page_ids(rapidjson::kArrayType);
    bustub::page_id_t cur_page_id = first_page_id;
    while (cur_page_id != bustub::INVALID_PAGE_ID) {
        bustub::TablePage *cur_page = reinterpret_cast<bustub::TablePage *>
            (kBusbubInstance->buffer_pool_manager_->FetchPage(cur_page_id));

        if (cur_page->GetPageId() != cur_page_id) {
            ctx.err_msg = fmt::format("Illegal table page id={}", cur_page_id);
            return false;
        }

        resp_page_ids.PushBack(rapidjson::Value(cur_page_id), ctx.resp_allocator);
        cur_page_id = cur_page->GetNextPageId();
    }

    ctx.resp_data.AddMember("table_page_ids",resp_page_ids,ctx.resp_allocator);
    return true;
}

bool myapi::GetTablePageInfo(ApiContext &ctx) {

    if (!ctx.req_data.HasMember("page_id") || !ctx.req_data["page_id"].IsNumber()) {
        ctx.err_msg = "Missing or invalid 'page_id' field";
        return false;
    }

    bustub::page_id_t page_id = ctx.req_data["page_id"].GetInt();
    if (page_id == bustub::INVALID_PAGE_ID) {
        ctx.err_msg = "Missing or invalid 'page_id' field";
        return false;
    }

    bustub::TablePage *table_page = reinterpret_cast<bustub::TablePage *>
        (kBusbubInstance->buffer_pool_manager_->FetchPage(page_id));

    if (table_page->GetTablePageId() != page_id) {
        ctx.err_msg = fmt::format("Illegal table page id {}", page_id);
        return false;
    }

    ctx.resp_data.AddMember(
        "page_id", 
        rapidjson::Value(table_page->GetTablePageId()),
        ctx.resp_allocator
    );
    ctx.resp_data.AddMember(
        "pre_page_id",
        rapidjson::Value(table_page->GetPrevPageId()),
        ctx.resp_allocator
    );
    ctx.resp_data.AddMember(
        "next_page_id",
        rapidjson::Value(table_page->GetNextPageId()),
        ctx.resp_allocator
    );
    ctx.resp_data.AddMember(
        "tuple_count",
        rapidjson::Value(table_page->GetTupleCount()),
        ctx.resp_allocator
    );
    ctx.resp_data.AddMember(
        "size_of_free_space",
        rapidjson::Value(table_page->GetFreeSpaceRemaining()),
        ctx.resp_allocator
    );
    ctx.resp_data.AddMember(
        "size_of_tuple_array",
        rapidjson::Value(bustub::BUSTUB_PAGE_SIZE - bustub::TablePage::SIZE_TABLE_PAGE_HEADER - table_page->GetFreeSpaceRemaining()),
        ctx.resp_allocator
    );

    return true;
}

bool myapi::GetTupleInfo(ApiContext &ctx) {
    // preprocess table_oid
    if (!ctx.req_data.HasMember("table_oid") || !ctx.req_data["table_oid"].IsNumber()) {
        ctx.err_msg = "Missing or invalid 'table_oid' field";
        return false;
    }
    bustub::table_oid_t table_oid = ctx.req_data["table_oid"].GetUint();
    bustub::TableInfo *table_info = kBusbubInstance->catalog_->GetTable(table_oid);
    if (table_info == bustub::Catalog::NULL_TABLE_INFO) {
        ctx.err_msg = fmt::format("Can't find table oid={}", table_oid);
        return false;
    }

    // preprocess page_id
    if (!ctx.req_data.HasMember("page_id") || !ctx.req_data["page_id"].IsNumber()) {
        ctx.err_msg = "Missing or invalid 'page_id' field";
        return false;
    }
    bustub::page_id_t page_id = ctx.req_data["page_id"].GetInt();
    if (page_id == bustub::INVALID_PAGE_ID) {
        ctx.err_msg = "Missing or invalid 'page_id' field";
        return false;
    }
    bustub::TablePage *table_page = reinterpret_cast<bustub::TablePage *>
        (kBusbubInstance->buffer_pool_manager_->FetchPage(page_id));
    if (table_page->GetPageId() != page_id) {
        ctx.err_msg = fmt::format("Illegal page id {}", page_id);
        return false;
    }

    // preprocess slot_num
    if (!ctx.req_data.HasMember("slot_num") || !ctx.req_data["slot_num"].IsNumber()) {
        ctx.err_msg = "Missing or invalid 'slot_num' field";
        return false;
    }
    uint32_t slot_num = ctx.req_data["slot_num"].GetUint();

    // get target tuple
    bustub::Tuple tuple;
    bool access_ret = table_info->table_->GetTuple({ page_id, slot_num }, &tuple, ctx.txn);
    if (!access_ret) {
        ctx.err_msg = "Fail to access tuple";
        return false;
    }

    // set "allocated" field
    ctx.resp_data.AddMember("allocated", tuple.IsAllocated(), ctx.resp_allocator);
    // set "page_id" field
    ctx.resp_data.AddMember("page_id", page_id, ctx.resp_allocator);
    // set "slot_num" filed
    ctx.resp_data.AddMember("slot_num", slot_num, ctx.resp_allocator);
    // set "size" filed
    ctx.resp_data.AddMember("size", tuple.GetLength(), ctx.resp_allocator);

    rapidjson::Value resp_values(rapidjson::kArrayType);
    bustub::Schema &schema = table_info->schema_;
    std::vector<bustub::Column> columns = schema.GetColumns();
    for (uint32_t i = 0; i < schema.GetColumnCount(); ++i) {
        bustub::Value value = tuple.GetValue(&schema, i);
        rapidjson::Value resp_value(rapidjson::kObjectType);
        
        // set "value" field
        resp_value.AddMember(
            "value",
            rapidjson::Value(value.ToString().c_str(), ctx.resp_allocator),
            ctx.resp_allocator
        );

        // set "size" field
        const bustub::Column &col = schema.GetColumn(i);
        uint32_t value_length = col.GetFixedLength();
        if (!col.IsInlined()) {
            value_length += (sizeof(uint32_t) + value.GetLength());
        }
        resp_value.AddMember("size", value_length, ctx.resp_allocator);

        // set "type" field
        resp_value.AddMember(
            "type",
            rapidjson::Value(bustub::Type::TypeIdToString(col.GetType()).c_str(), ctx.resp_allocator),
            ctx.resp_allocator
        );

        resp_values.PushBack(resp_value, ctx.resp_allocator);
    }
    // set "values" field
    ctx.resp_data.AddMember("values", resp_values, ctx.resp_allocator);

    return true;

}


static const std::unordered_map<std::string, myapi::ApiFuncType> kApiMap = {
    {"/submit_sql_command", &myapi::SubmitSqlCommand},
    {"/query_table_by_name", &myapi::QueryTableByName},
    {"/get_all_tables", &myapi::GetAllTables},
    {"/get_buffer_pool_info", &myapi::GetBufferPoolInfo},
    {"/get_table_heap_info", &myapi::GetTableHeapInfo},
    {"/get_table_page_info", &myapi::GetTablePageInfo},
    {"/get_tuple_info", &myapi::GetTupleInfo},
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