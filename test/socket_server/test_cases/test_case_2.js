/*
    Test Case 2
    To verify '/get_buffer_pool_info' interface.
*/

import BusTubCore from '../bustub_core.js';
import {assert, sendJsonMessage, executeSQL} from '../util.js';

async function test_case_2() {
    let message = {
        'api': '/get_buffer_pool_info',
        'data': {},
    };
    let result = await sendJsonMessage(message);
    assert(result.hasOwnProperty('buffer_pool_info'));
    let bufferPoolInfo = result["buffer_pool_info"];
    assert(bufferPoolInfo.length === 128);
    for (let pageInfo of bufferPoolInfo) {
        assert(pageInfo.hasOwnProperty("frame_id"));
        assert(pageInfo.hasOwnProperty("page_id"));
        assert(pageInfo.hasOwnProperty("is_dirty"));
        assert(pageInfo.hasOwnProperty("pin_count"));
        assert(pageInfo.hasOwnProperty("is_free"));
        // Free page must be undirty, or dirty page must be unfree.
        assert((pageInfo["is_dirty"] ^ pageInfo["is_free"]) === 1);
        // At first, allocated page should have the same frame_id and page_id.
        if (!pageInfo["is_free"]) {
            assert(pageInfo["frame_id"] === pageInfo["page_id"]);
        }
        // At first, free page should have -1 as its page_id
        else {
            assert(pageInfo["page_id"] === -1);
        }
    }
}

export {test_case_2 as default};