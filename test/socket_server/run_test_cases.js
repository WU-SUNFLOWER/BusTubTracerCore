import BusTubCore from './bustub_core.js';
import {assert, sendJsonMessage, executeSQL} from './util.js';
import test_case_1 from './test_cases/test_case_1.js';
import test_case_2 from './test_cases/test_case_2.js';
import test_case_3 from './test_cases/test_case_3.js';

const testCases = [test_case_1, test_case_2, test_case_3];

const runTestCases = async () => {
    await BusTubCore.init();

    for (let i = 0; i < testCases.length; ++i) {
        try {
            await testCases[i]();
        } catch (err) {
            console.error(`\x1b[31m${err.stack}\x1b[0m`);
            BusTubCore.exit();
            process.exit(1);
        }
        console.log(`\x1b[32mTest Case ${i} Passed!\x1b[0m`);
    }
    
    console.log(`\x1b[32mAll The Test Cases Passed!\x1b[0m`);
    BusTubCore.exit();
    process.exit(0);
}

runTestCases();