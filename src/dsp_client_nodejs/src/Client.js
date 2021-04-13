import atob from 'atob';
import { Command, COMMAND_FN } from './Command.js';
import { Message } from './Message.js';

const _CLASS = '@FaintSignals/ws-dsp-client/Client';

/**
 * Simple request/response message handling for websockets.
 */
class Client {
    ws = null;
    messages = null;
    /**
     * Constructor.
     * @param {Object} args    - Generic argument object.
     * @param {Object} args.ws - Open websocket connection.
     */
    constructor(args) {
        if (!args)
            throw `${_CLASS}: Parameter object is required`;
        if (!args.ws)
            throw `${_CLASS}: Parameter is required: 'ws'`;
        this.messages = [];
        this.ws = args.ws;
        this.listen();
    }
    /**
     * Listen to responses from websocket.
     * @param {Object} args - Generic argument object.
     */
    listen(args) {
        ((wsMessage) => {
            this.ws.onmessage = (e) => {
                if (e.target._binaryType == 'nodebuffer') {
                    try {
                        // Really?
                        let data_len = (new Uint32Array([(e.data.slice(5, 9)).readInt32LE()]))[0];
                        let container = {
                            "_version": e.data[0],
                            "id": (new Uint32Array([(e.data.slice(1, 5)).readInt32LE()]))[0],
                            "data_len": data_len,
                            "data": e.data.slice(9, 9 + data_len)
                        }
                        if (wsMessage.messages[container.id]) {
                            if (wsMessage.messages[container.id].debug)
                                console.debug(`receiving ${container.id}:`, container);
                            wsMessage.messages[container.id].callback(container);
                            delete wsMessage.messages[container.id];
                        } else {
                            console.error('Unsolicited message from server');
                            console.error("receiving: ", container);
                        }
                    } catch (error) {
                        console.error("Malformed message from server");
                        console.error(e.data);
                    }
                } else {
                    let container = JSON.parse(e.data);
                    if (container.id) {
                        wsMessage.messages[container.id](container);
                        if (container.responseFinal)
                            delete wsMessage.messages[container.id];
                    }    
                }
            }
            this.ws.onerror = (error) => {
                console.error(error);
            }    
        })(this);
    }
    /**
     * Send a message and register for response.
     * @param {Object}   args          - Generic argument object.
     * @param {Object}   args.message  - Message to send.
     * @param {function} args.callback - Execute callback when response is recieved.
     */
    send(args) {
        if (!args)
            throw `${_CLASS}: Parameter object is required`;
        if (!args.message)
            throw `${_CLASS}: Parameter is required: 'message'`;
        if (!args.callback)
            throw `${_CLASS}: Parameter is required: 'callback'`;
        let id = Math.floor(Math.random() * 4294967295);
        let container = {
            "id": id,
            "message": args.message
        }
        this.messages[id] = {
            "type": "text",
            "callback": args.callback
        };
        this.ws.send(JSON.stringify(container));
    }
    /**
     * Send a message and register for response.
     * @param {Object}     args          - Generic argument object.
     * @param {Message}    args.message  - @FaintSignals/dsp-client-nodejs/Message message to send.
     * @param {Object}     args.options  - (Optional) Options to pass into websocket send.
     * @param {function}   args.callback - Execute callback when response is recieved.
     * @param {bool}       args.debug    - Echo debugging information.
     */
     sendBinary(args) {
        if (!args)
            throw `${_CLASS}: Parameter object is required`;
        if (!args.message)
            throw `${_CLASS}: Parameter is required: 'message'`;
        if (!args.callback)
            throw `${_CLASS}: Parameter is required: 'callback'`;
        this.messages[args.message.id] = {
            "type": "binary",
            "callback": args.callback,
            "debug": true
        };
        if (args.debug)
            console.debug(`sending ${args.message.id}:`, args.message);
        let serialized = args.message.serialize();
        if (args.debug)
            console.debug('binary: ', serialized);
        this.ws.send(serialized, args.options);
    }
    /**
     * Send test message to server which will be echoed back.
     * @param {TypedArray} data        - Test data to send
     * @param {Object}     options     - (Optional) Options to pass into websocket.
     * @param {number}     miliseconds - (Optional) Defaults to 1000. Repeat at interval.
     * @returns timer for canceling interval.
     */
    testBinary(data, options, miliseconds) {
        miliseconds = (miliseconds) ? miliseconds : 1000;
        //let idMax = 4294967295;
        let idMax = 65535;
        let timer = setInterval(() => {
            let message = new Message({ 
                "version": 1, 
                "id": Math.floor(Math.random() * idMax),
                "commands": [
                    new Command({ "type": COMMAND_FN.ECHO, "paramsLen": 0 })
                ],
                "data": new Uint8Array(data.buffer)
            });
            this.sendBinary({
                "message": message,
                "options": options,
                "callback": (container) => { },
                "error": (error) => { },
                "debug": true
            });
        }, miliseconds);
        return timer;
    }
    /**
     * Parse IQ stream from base64 encoding
     * @param {Object} args        - Generic argument object.
     * @param {string} args.buffer - IQ data encoded with base64
     * @returns {Int8Array} Interleaved I and Q 8 bit samples.
     */
     async parseBase64IQto8(args) {
        let promise = new Promise((resolve, reject) => {
            let str = atob(args.buffer);
            let uInt8Array = new Uint8Array(str.length);
            for (let x = 0; x < str.length; x++) {
                uInt8Array[x] = str.charCodeAt(x);
            }
            let samples = new Int8Array(uInt8Array);
            resolve(samples);
        });
        return promise;
    }
    /**
     * Fast fourier transform from IQ data.
     * @param {Object}     args            - Generic argument object.
     * @param {TypedArray} args.samples    - Interleaved IQ data. Can be 8, 16, or 32 in sample size.
     * @param {number}     args.sampleRate - Sample rate in Hz.
     */
    async fftIQ(args) {
        if (!args)
            throw `${_CLASS}: Parameter object is required`;
        if (!args.samples)
            throw `${_CLASS}: Parameter is required: 'samples'`;
        if (!args.sampleRate)
            throw `${_CLASS}: Parameter is required: 'sampleRate'`;
        let promise = new Promise((resolve, reject) => {
            let sampleSize = args.samples.byteLength / args.samples.length;
            // Construct binary message.
            let msgFunction   = new Uint8Array((new Uint32Array([1])).buffer);
            let msgSampleRate = new Uint8Array((new Uint32Array([args.sampleRate])).buffer);
            let msgSampleSize = new Uint8Array((new Uint32Array([sampleSize])).buffer);
            let msgSamples    = new Uint8Array(args.samples.buffer);
            let msgSamplesLen = new Uint8Array((new Uint32Array([args.samples.byteLength])).buffer);
            let ptr = 0;
            let msg = new Uint8Array(msgFunction.byteLength + msgSampleRate.byteLength + msgSampleSize.byteLength + msgSamples.byteLength + msgSamplesLen.byteLength);
                msg.set(msgFunction,   ptr);
                msg.set(msgSampleRate, ptr += msgFunction.byteLength);
                msg.set(msgSampleSize, ptr += msgSampleRate.byteLength);
                msg.set(msgSamplesLen, ptr += msgSampleSize.byteLength);
                msg.set(msgSamples,    ptr += msgSamplesLen.byteLength);
            this.sendBinary({ "message": msg, "callback": (container) => {
console.log("ok", container);
                resolve(container);
            }, "error": (error) => { reject(error); }});    
        });
        return promise;
    }
}

export { Client }