/*
    Test Case 4
    To verify '/query_b_plus_tree' interface.
*/
import BusTubCore from '../bustub_core.js';
import {assert, sendJsonMessage, executeSQL} from '../util.js';

async function test_case_4() {
    await executeSQL("create index idx_dep on department(id)");
    await executeSQL("create index idx_tid on teacher(id)");
    await executeSQL("create index idx_cid on course(id)");

    let message = {
        'api': '/query_table_by_name',
        'data': {
            'table_name': 'course'
        }
    };
    let result = await sendJsonMessage(message);
    assert(result['indices'] !== void 0);
    assert(result['indices'].length === 1);
    let indexInfo = result['indices'][0];
    assert(indexInfo['index_name'] === 'idx_cid');
    let indexOid = indexInfo['index_oid'];
    const tuples = result['tuples'];
    const tupleCount = tuples.length;

    message = {
        'api': '/query_b_plus_tree',
        'data': {
            'index_oid': indexOid,
        }
    };
    result = await sendJsonMessage(message);

    let nodeMap = new Map();
    let rootNodeInfo = result['root'];
    let { header: {page_id: rootId}, key_value: rootKV } = rootNodeInfo;
    nodeMap.set(rootId, rootNodeInfo);
    
    let nodes = result['nodes'];
    for (let nodeInfo of nodes) {
        let { header: {page_id: id} } = nodeInfo;
        nodeMap.set(id, nodeInfo);
    }
    let { page_id: nextPageID } = rootKV[0];
    let nextPageInfo;
    while ((nextPageInfo = nodeMap.get(nextPageID)).page_type === "internal_page") {
        nextPageID = nextPageInfo['key_value'][0]['page_id'];
    }
    let leaveRecord = new Map();
    while (nextPageID > 0) {
        let nodeInfo = nodeMap.get(nextPageID);
        let {header: {page_type: nodeType}, key_value: nodeKV} = nodeInfo;
        assert(nodeType === 'leaf_page');
        for (let { index, rid } of nodeKV) {
            leaveRecord.set(index, rid);
        }
        nextPageID = nodeInfo['header']['next_page_id'];
    }

    assert(leaveRecord.size === tupleCount);
    for (let tuple of tuples) {
        let { rid: tupleRid, columns: tupleCol} = tuple;
        let tupleIndex = Number.parseInt(tupleCol[0]);
        let leaveRid = leaveRecord.get(tupleIndex);
        assert(tupleRid.page_id === leaveRid.page_id);
        assert(tupleRid.slot_num === leaveRid.slot_num);
    }
}

export {test_case_4 as default};