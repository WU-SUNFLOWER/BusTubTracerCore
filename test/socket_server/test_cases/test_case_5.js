/*
    Test Case 5
    To verify `/query_table_by_name`, `/get_table_heap_info`, 
    `/get_table_page_info` and `/get_tuple_info` interface.
*/
import BusTubCore from '../bustub_core.js';
import {assert, sendJsonMessage, executeSQL} from '../util.js';

async function test_case_5() {
    let msg = {
        'api': '/query_table_by_name',
        'data': {
            'table_name': 'test_table_3'
        }
    };
    let result = await sendJsonMessage(msg);
    let tableOid = result['table_oid'];
    let totalTuple = result['tuples'].length;

    msg = {
        'api': '/get_table_heap_info',
        'data': {
            'table_oid': tableOid
        }
    }
    let {table_page_ids: pageIds} = await sendJsonMessage(msg);
    
    let tupleCheckCount = 0;
    for (let pageId of pageIds) {
        let msg = {
            'api': '/get_table_page_info',
            'data': {
                'page_id': pageId
            }
        };
        let result = await sendJsonMessage(msg);
        let { tuple_count: tupleCount } = await sendJsonMessage(msg);
        for (let i = 0; i < tupleCount; ++i) {
            let msg = {
                'api': '/get_tuple_info',
                'data': {
                    "table_oid": tableOid,
                    "page_id":pageId,
                    "slot_num": i                
                }
            };
            let { values } = await sendJsonMessage(msg);
            assert(tupleCheckCount++ === Number.parseInt(values[0].value));
        }
    }
    assert(tupleCheckCount === totalTuple);
}

export {test_case_5 as default};