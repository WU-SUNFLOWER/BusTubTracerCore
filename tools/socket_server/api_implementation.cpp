#include <string>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "fmt/format.h"
#include "fmt/ranges.h"
#include "binder/binder.h"
#include "common/bustub_instance.h"
#include "common/exception.h"
#include "common/util/string_util.h"

static std::unique_ptr<bustub::BustubInstance> instance = nullptr;

void BustubInit() {
    std::cout << "Initialize BusTub..." << std::endl;
    auto bustub = std::make_unique<bustub::BustubInstance>("test.db");
    bustub->GenerateMockTable();

    if (bustub->buffer_pool_manager_ != nullptr) {
        bustub->GenerateTestTable();
    }

    instance = std::move(bustub);

    std::cout << "BusTub Initialized!" << std::endl;

}

auto DispatchRequest(const std::string &request) -> std::string {
    
    /* preprocess request json */
    rapidjson::Document req_json;
    if (req_json.Parse(request.c_str()).HasParseError()) {
        return "{\"err_msg\": \"Invalid JSON format\"}";
    }
    if (!req_json.HasMember("api") || !req_json["api"].IsString()) {
        return "{\"err_msg\": \"Missing or invalid 'api' field\"}";
    }
    if (!req_json.HasMember("data") || !req_json["data"].IsObject()) {
        return "{\"err_msg\": \"Missing or invalid 'data' field\"}";
    }

    std::string api = req_json["api"].GetString();
    rapidjson::Value& req_data = req_json["data"];

    /* preprocess respond_json */
    rapidjson::Document resp_json;
    resp_json.SetObject();
    auto& resp_allocator = resp_json.GetAllocator();

    resp_json.AddMember(
        "data", 
        rapidjson::Value(rapidjson::kObjectType), 
        resp_allocator
    );

    rapidjson::Value& resp_data = resp_json["data"];

    /* preprocess json_writer */
    rapidjson::StringBuffer json_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> json_writer(json_buffer);

    /* dispatch start!!! */
    if (api == "/submit_sql_command") {

        if (!req_data.HasMember("sql") || !req_data["sql"].IsString()) {
            return "{\"err_msg\": \"Missing or invalid 'sql' field\"}";
        }

        std::string sql_command = req_data["sql"].GetString();
        std::string sql_result;
        try {
            auto writer = bustub::FortTableWriter();
            instance->ExecuteSql(sql_command, writer);
            for (const auto &table : writer.tables_) {
                sql_result += table;
            }
        } catch (bustub::Exception &ex) {
            return fmt::format(R"({{ "err_msg": "{}" }})", ex.what());
        }

        resp_data.AddMember(
            "raw_result",
            rapidjson::Value(sql_result.c_str(), resp_allocator),
            resp_allocator
        );

        resp_json.Accept(json_writer);
        return json_buffer.GetString();

    }
    
    return "{\"err_msg\": \"Unknown API\"}";
}