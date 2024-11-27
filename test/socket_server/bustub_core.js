import { fileURLToPath } from 'url'
import path, { resolve, dirname } from 'path';
import net from 'net';
import process from 'process';
import { spawn } from 'child_process';

const BusTubCore = {
    
    ELF_PATH: path.join("build/bin/socket_server"),

    SOCKET_PATH: "/tmp/bustub_core_socket",

    DATAGRAM_HEADER_STR: "BTC",
    get DATAGRAM_HEADER_STR_SIZE() {
        return this.DATAGRAM_HEADER_STR.length;
    },
    DATAGRAM_OPTION_SIZE: 1,
    DATAGRAM_PAYLOAD_LEN_SIZE: 4,
    get DATAGRAM_HEADER_SIZE() {
        return this.DATAGRAM_HEADER_STR_SIZE 
                + this.DATAGRAM_OPTION_SIZE 
                + this.DATAGRAM_PAYLOAD_LEN_SIZE;
    },
    get DATAGRAM_OPTION_OFFSET() {
        return this.DATAGRAM_HEADER_STR_SIZE;
    },
    get DATAGRAM_PAYLOAD_LEN_OFFSET() {
        return this.DATAGRAM_OPTION_OFFSET + this.DATAGRAM_OPTION_SIZE;
    },
    get DATAGRAM_PAYLOAD_OFFSET() {
        return this.DATAGRAM_PAYLOAD_LEN_OFFSET + this.DATAGRAM_PAYLOAD_LEN_SIZE;
    },

    // type=0, client->server
    // type=1, server->client
    OPTIONS_TYPE_MASK: 0x01,  

    process: null,

    connection: null,

    async init() {
        const __filename = fileURLToPath(import.meta.url);
        const __dirname = dirname(__filename);
        
        this.ELF_PATH = path.join(__dirname, '../', '../', this.ELF_PATH);

        if (this.process != null || this.connection != null) {
            console.log("connection to BusTubCore has already been built!");
        }

        this.process = await this.initProcess();
        console.log("sucessfully started BusTubCore.");
        this.connection = await this.initConnection();
        console.log("successfully established connection with BusTubCore.");

    },

    exit() {
        ;
        this.closeConnection();
        console.log("The connection to BusTubCore is closed.");
        
        if (this.exitProcess()) {
            console.log("The BusTubCore Process is exited.");
        } else {
            console.error("Fail to kill BusTubCore Process.");
        }
        
    },

    initProcess() {
        return new Promise((resolve, reject) => {
            const bustubProcess = spawn(this.ELF_PATH);
            bustubProcess.stdout.on("data", (data) => {
                console.log(`BusTubCore output: ${data}`);
            });
            bustubProcess.stderr.on("data", (data) => {
                console.error(`BusTubCore error: ${data}`);
            });
            bustubProcess.on("close", (code) => {
                console.log(`BusTubCore error: ${code}`);
            });
            process.on("SIGUSR1", function handler() {
                process.removeListener("SIGUSR1", handler);
                resolve(bustubProcess);
            });
        });
    },

    exitProcess() {
        return this.process.kill();
    },

    initConnection() {
        return new Promise((resolve, reject) => {
            const conn = net.createConnection({ path: this.SOCKET_PATH }, () => {
                resolve(conn);
            });
            conn.on("error", (err) => {
                console.error(`error from BusTubCore: ${err}`);
            });
        });
    },

    closeConnection() {
        this.connection.destroy();
    },

    packDatagram(payload) {
        const payloadLength = Buffer.byteLength(payload, "utf8");
        // alloc buffer for datagram
        const datagram = Buffer.alloc(this.DATAGRAM_HEADER_SIZE + payloadLength);
        // set the header string of datagram
        datagram.write(this.DATAGRAM_HEADER_STR, 0, this.DATAGRAM_HEADER_STR_SIZE, "ascii");
        // set the options of datagram
        const options = 0;
        datagram.writeUInt8(options, this.DATAGRAM_OPTION_OFFSET);
        // set the payload length of datagram
        datagram.writeUint32BE(payloadLength, this.DATAGRAM_PAYLOAD_LEN_OFFSET);
        // write our payload!
        datagram.write(payload, this.DATAGRAM_PAYLOAD_OFFSET, payloadLength, "utf8");
        return datagram;
    },

    recvRespond() {
        const self = this;
        let respondBuffer = Buffer.alloc(0);
        return new Promise((resolve, reject) => {
            self.connection.on("data", function handler(data) {
                respondBuffer = Buffer.concat([respondBuffer, data]);
                
                // if we haven't got the completed header,
                // just do no thing but wait!
                if (respondBuffer.length < self.DATAGRAM_HEADER_SIZE) {
                    return;
                }

                const headerString = respondBuffer.subarray(0, 3).toString("ascii");
                const options = respondBuffer.readUInt8(3);
                const payloadLength = respondBuffer.readUInt32BE(4);
                const totalLength = self.DATAGRAM_HEADER_SIZE + payloadLength;

                if (headerString != self.DATAGRAM_HEADER_STR) {
                    console.error("illegal datagram header");
                    return reject();
                }

                if ((options & self.OPTIONS_TYPE_MASK) != 1) {
                    console.error("illegal datagram header");
                    return reject();
                }

                // if there is still data that hasn't been received,
                // we just do nothing but continue to wait.
                if (respondBuffer.length < totalLength) {
                    return;
                }

                if (respondBuffer.length > totalLength) {
                    console.error("find more bytes than payload length!");
                    respondBuffer = respondBuffer.subarray(0, totalLength);
                }

                // ok, yet we have got the completed datagram.
                // just return the payload data as string~
                self.connection.removeListener('data', handler);
                resolve(respondBuffer.subarray(self.DATAGRAM_HEADER_SIZE).toString("utf8"));
            });
        });
    },

    async sendMessage(message) {

        const self = this;
        if (self.connection == null) {
            console.error("connection to BusTubCore hasn't been built!");
            return;
        }

        // pack datagram
        const datagram = this.packDatagram(message);
        // send datagram through stream socket
        if (!self.connection.write(datagram)) {
            console.error("fail to send datagram");
            return;
        }
        // wait server to respond
        let respond = await self.recvRespond();
        return respond;
    },

};

Object.defineProperty(BusTubCore, 'ELF_PATH', {
    configurable: false
});

Object.defineProperty(BusTubCore, 'SOCKET_PATH', {
    configurable: false
});

export { BusTubCore as default };