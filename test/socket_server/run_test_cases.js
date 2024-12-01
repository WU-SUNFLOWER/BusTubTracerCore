import BusTubCore from './bustub_core.js';
import test_case_1 from './test_cases/test_case_1.js';
import test_case_2 from './test_cases/test_case_2.js';
import test_case_3 from './test_cases/test_case_3.js';
import test_case_4 from './test_cases/test_case_4.js';
import test_case_5 from './test_cases/test_case_5.js';
import test_case_6 from './test_cases/test_case_6.js';

const testCases = [test_case_1, test_case_2, test_case_3, test_case_4, test_case_5, test_case_6];

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
        console.log(`\x1b[32mTest Case ${i + 1} Passed!\x1b[0m`);
    }
    
    console.log(`\x1b[32mAll The Test Cases Passed!\x1b[0m`);
    BusTubCore.exit();
    process.exit(0);
}

runTestCases();