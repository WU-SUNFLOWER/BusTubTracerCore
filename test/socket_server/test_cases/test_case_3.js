/*
    Test Case 3
    To verify '/submit_sql_command' interface.
*/
import BusTubCore from '../bustub_core.js';
import {assert, sendJsonMessage, executeSQL} from '../util.js';

async function test_case_3() {
    // Check Point 3.1
    // Can BusTubCore execute `create table` command correctly?
    let result = await executeSQL("create table department (id int, name varchar(128))");
    assert(result["can_show_process"] === false);
    result = await executeSQL("create table teacher (id int, name varchar(128), did int)");
    assert(result["can_show_process"] === false);
    result = await executeSQL("create table course (id int, name varchar(128), credit int)");
    assert(result["can_show_process"] === false);
    result = await executeSQL("create table course_teacher (cid int, tid int)");
    assert(result["can_show_process"] === false);

    // Check Point 3.2
    // Let's insert tuples into `department` table.
    let departments = [
        "Computer Science", "Software Engineering", 
        "Information Security", "Teaching and Research Section"
    ];
    let values = "";
    for (let i = 0; i < departments.length; ++i) {
        values += `(${i}, '${departments[i]}')`;
        if (i !== departments.length - 1) values += ', ';
    }
    result = await executeSQL(`insert into department values ${values}`);
    // Check the result of execution.
    assert(result["can_show_process"] === true);
    assert(result["process_info"] !== void 0);
    let rootId = result?.process_info?.optimized_planner_tree?.planner_node_id;
    assert(rootId !== void 0);
    let rootRecord = result?.process_info?.executor_tree.find(record => record?.bound_planner_node_id === rootId);
    assert(rootRecord?.output_table !== void 0);
    let [_, [count]] = rootRecord?.output_table;
    assert(Number.parseInt(count) === departments.length);

    // Check Point 3.3
    // Let's insert tuples into `teachere` table.
    let teacheres = [
        ['CZX', 3], ['HT', 0], ['LBL', 1], ['HPY', 3], 
        ['FJ', 0], ['SC', 0], ['XB', 1], ['LCX', 0],
        ['BCZ', 0], ['DXY', 0], ['CWG', 0], ['JZY', 0],
        ['SJ', 2], ['HBH', 2], ['ZDH', 2], ['YT', 2],
        ['WQ', 0], ['XGL', 0], ['KDK', 0], ['DJF', 1]
    ];
    values = '';
    for (let i = 0; i < teacheres.length; ++i) {
        let [name, did] = teacheres[i];
        values += `(${i}, '${name}', ${did})`;
        if (i < teacheres.length - 1) values += ', ';
    }
    result = await executeSQL(`insert into teacher values ${values}`);
    // Check the result of execution.
    assert(result["can_show_process"] === true);
    assert(result["process_info"] !== void 0);
    rootId = result?.process_info?.optimized_planner_tree?.planner_node_id;
    assert(rootId !== void 0);
    rootRecord = result?.process_info?.executor_tree.find(record => record?.bound_planner_node_id === rootId);
    assert(rootRecord?.output_table !== void 0);
    [_, [count]] = rootRecord?.output_table;
    assert(Number.parseInt(count) === teacheres.length);

    // Check Point 3.4
    // Let's insert tuples into `course` table
    let courses = [
        ['C Language Programming', 4], ['Discrete Mathematics', 3],
        ['Linux Basics', 2], ['Principles of Database', 3],
        ['Digital Logic', 1], ['C++ Language Programming', 2],
        ['Database Course Design', 1], ['Computer Architecture', 3],
        ['Data Science', 1], ['Digital Image Processing', 2],
        ['Introduction to AI', 2], ['Software Development Practice', 3]
    ];
    values = '';
    for (let i = 0; i < courses.length; ++i) {
        let [name, credits] = courses[i];
        values += `(${i}, '${name}', ${credits})`;
        if (i !== courses.length - 1) values += ', ';
    }
    result = await executeSQL(`insert into course values ${values}`);
    // Check the result of execution.
    assert(result["can_show_process"] === true);
    assert(result["process_info"] !== void 0);
    rootId = result?.process_info?.optimized_planner_tree?.planner_node_id;
    assert(rootId !== void 0);
    rootRecord = result?.process_info?.executor_tree.find(record => record?.bound_planner_node_id === rootId);
    assert(rootRecord?.output_table !== void 0);
    [_, [count]] = rootRecord?.output_table;
    assert(Number.parseInt(count) === courses.length);

    // Check Point 3.5
    // Let's insert tuples into `course_teacher` table.
    // (cid int, tid int)
    let course_teacher = [
        [0, 7], [1], [2], [3, 14], [4],
        [5, 11], [6], [8], [9], [10],
        [16, 17, 9], [18]   
    ];
    assert(course_teacher.length === courses.length);
    values = '';
    let recordCount = 0;
    for (let cid = 0; cid < course_teacher.length; ++cid) {
        for (let tid of course_teacher[cid]) {
            values += `(${cid}, ${tid}),`;
            ++recordCount;
        }
    }
    values = values.slice(0, values.length - 1);
    result = await executeSQL(`insert into course_teacher values ${values}`);
    assert(result["can_show_process"] === true);
    assert(result["process_info"] !== void 0);
    rootId = result?.process_info?.optimized_planner_tree?.planner_node_id;
    assert(rootId !== void 0);
    rootRecord = result?.process_info?.executor_tree.find(record => record?.bound_planner_node_id === rootId);
    assert(rootRecord?.output_table !== void 0);
    [_, [count]] = rootRecord?.output_table;
    assert(Number.parseInt(count) === recordCount);

    // Check Point 3.6
    // Let's try to select all the courses with credit more than 2
    result = await executeSQL("select * from course where credit > 2");
    assert(result?.can_show_process === true && result?.process_info !== void 0);
    let {optimized_planner_tree, executor_tree} = result.process_info;
    rootId = optimized_planner_tree?.planner_node_id;
    assert(rootId !== void 0);
    rootRecord = result?.process_info?.executor_tree.find(record => record?.bound_planner_node_id === rootId);
    assert(rootRecord?.output_table !== void 0);
    assert(rootRecord.output_table.length - 1 === courses.reduce((count, [_, credit]) => {
        return credit > 2 ? (count + 1) : count;
    }, 0));

    // Check Point 3.7
    // Let's try to query the teacher's name with the courses they teaches.
    result =  await executeSQL("select course.name, teacher.name from course, teacher, course_teacher where course.id = course_teacher.cid and teacher.id = course_teacher.tid");
    assert(result?.process_info?.optimized_planner_tree?.planner_node_tag === "Projection");
    assert(result?.process_info?.optimized_planner_tree?.children[0]?.planner_node_tag == "NestedLoopJoin");
    assert(result?.process_info?.optimized_planner_tree?.children[0]?.children[0]?.planner_node_tag === "NestedLoopJoin");
    assert(result?.process_info?.optimized_planner_tree?.children[0]?.children[1]?.planner_node_tag === "SeqScan");
}

export {test_case_3 as default};