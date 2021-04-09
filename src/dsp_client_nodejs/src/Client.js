import atob from 'atob';

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
     * @param {Uint8Array} args.message  - Message to send.
     * @param {Object}     args.options  - (Optional) Options to pass into websocket send.
     * @param {function}   args.callback - Execute callback when response is recieved.
     */
     sendBinary(args) {
        if (!args)
            throw `${_CLASS}: Parameter object is required`;
        if (!args.message)
            throw `${_CLASS}: Parameter is required: 'message'`;
        if (!args.callback)
            throw `${_CLASS}: Parameter is required: 'callback'`;
        let id = Math.floor(Math.random() * 4294967295);
        this.messages[id] = {
            "type": "binary",
            "callback": args.callback,
            "debug": true
        };
        let msg_version = new Uint8Array([1]);
        let msg_id      = new Uint8Array((new Uint32Array([id])).buffer);
        let msg_data    = new Uint8Array(args.message.buffer);
        let msg_len     = new Uint8Array((new Uint32Array([args.message.byteLength])).buffer);
        let ptr         = 0;
        let msg = new Uint8Array(msg_version.byteLength + msg_id.byteLength + msg_len.byteLength + msg_data.byteLength);
            msg.set(msg_version, ptr);
            msg.set(msg_id,      ptr += msg_version.byteLength);
            msg.set(msg_len,     ptr += msg_id.byteLength);
            msg.set(msg_data,    ptr += msg_len.byteLength);
        if (args.debug)
            console.debug(`sending ${id}:`, msg);    
        this.ws.send(msg, args.options);
    }
    /**
     * Send test message to server which will be echoed back.
     * @param {TypedArray} data - Test data to send
     * @returns timer for canceling interval.
     */
    testBinary(data, options) {
        let timer = setInterval(() => {
            this.sendBinary({
                "message": data,
                "options": options,
                "callback": (container) => { },
                "error": (error) => { },
                "debug": true
            });
        }, 15000);
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