/*
    Test Case 4
    To verify '/query_b_plus_tree' interface with generated tables.
*/
import BusTubCore from '../bustub_core.js';
import {assert, sendJsonMessage, executeSQL} from '../util.js';

async function test_case_4() {
    await executeSQL("create index idx on test_table_1(colA)");
    await executeSQL("create index idx on test_table_2(colA)");
    await executeSQL("create index idx on test_table_3(colA)");

    let result = await executeSQL("select * from test_table_1 order by colA");
    let processInfo = result?.['process_info'];
    assert(processInfo !== void 0);
    let executorNode = processInfo?.['optimized_planner_tree'];
    assert(executorNode?.['planner_node_tag'] === "IndexScan");
    let outputTable = processInfo?.['executor_tree']?.[0]?.['output_table'];
    assert(outputTable !== void 0);
    for (let i = 1; i < outputTable.length; ++i) {
        assert(i - 1 === Number.parseInt(outputTable[i][0]));
    }

    result = await executeSQL("select * from test_table_2 order by colA");
    processInfo = result?.['process_info'];
    assert(processInfo !== void 0);
    executorNode = processInfo?.['optimized_planner_tree'];
    assert(executorNode?.['planner_node_tag'] === "IndexScan");
    outputTable = processInfo?.['executor_tree']?.[0]?.['output_table'];
    assert(outputTable !== void 0);
    for (let i = 1; i < outputTable.length; ++i) {
        assert(i - 1 === Number.parseInt(outputTable[i][0]));
    }

    result = await executeSQL("select * from test_table_3 order by colA");
    processInfo = result?.['process_info'];
    assert(processInfo !== void 0);
    executorNode = processInfo?.['optimized_planner_tree'];
    assert(executorNode?.['planner_node_tag'] === "IndexScan");
    outputTable = processInfo?.['executor_tree']?.[0]?.['output_table'];
    assert(outputTable !== void 0);
    for (let i = 1; i < outputTable.length; ++i) {
        assert(i - 1 === Number.parseInt(outputTable[i][0]));
    }
}

export {test_case_4 as default};