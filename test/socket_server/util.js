import BusTubCore from './bustub_core.js';

const assert = (expr, tip = '') => {
    if (expr !== true) {
        throw new Error(`Assert Failed! ${tip}`);
    }
};

const respAssert = (resp) => {
    assert(resp.hasOwnProperty("data") && !resp.hasOwnProperty("err_msg"), resp["err_msg"]);
};

const sendJsonMessage = async (message) => {
    let result = await BusTubCore.sendMessage(JSON.stringify(message));
    let resultJson = JSON.parse(result);
    respAssert(resultJson);
    return resultJson.data;
};

const executeSQL = async (sql) => {
    let message = {
        'api': '/submit_sql_command',
        'data': { sql },
    };
    return await sendJsonMessage(message);
};

export {assert, sendJsonMessage, executeSQL};