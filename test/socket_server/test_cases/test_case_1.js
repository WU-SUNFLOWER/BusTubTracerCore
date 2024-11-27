/*
    Test Case 1
    To verify '/get_all_tables' interface.
*/

import BusTubCore from '../bustub_core.js';
import {assert, sendJsonMessage, executeSQL} from '../util.js';

async function test_case_1() {
    let message = {
        'api': '/get_all_tables',
        'data': {},
    };
    let result = await sendJsonMessage(message);
    let tables = result["tables"];
    assert(result.hasOwnProperty("tables"));
    assert(tables.length === 3);
    for (let table of tables) {
        assert(table.hasOwnProperty("table_name"));
        assert(table.hasOwnProperty("table_oid"));
        assert(table["table_name"] === `test_table_${table["table_oid"] + 1}`);
    }
}

export {test_case_1 as default};