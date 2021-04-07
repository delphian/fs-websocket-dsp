
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
                    let data_len = (e.data.slice(5, 9)).readInt32LE();
                    let container = {
                        "_version": e.data[0],
                        "id": (e.data.slice(1, 5)).readInt32LE(),
                        "data_len": data_len,
                        "data": e.data.slice(9, 9 + data_len)
                    }
                    console.debug(container);
                    delete wsMessage.messages[container.id];
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
     * @param {Object}      args          - Generic argument object.
     * @param {Uint8Array}  args.message  - Message to send.
     * @param {function}    args.callback - Execute callback when response is recieved.
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
            "callback": args.callback
        };
        let msg_id   = new Uint8Array((new Uint32Array([id])).buffer);
        let msg_data = new Uint8Array(args.message.buffer);
        let msg_len  = new Uint8Array((new Uint32Array([args.message.byteLength])).buffer);
        let ptr      = 0;
        let msg = new Uint8Array(msg_id.byteLength + msg_len.byteLength + msg_data.byteLength);
            msg.set(msg_id, ptr);
            msg.set(msg_len, ptr += msg_id.byteLength);
            msg.set(msg_data, ptr += msg_len.byteLength);
        this.ws.send(msg);
    }
}

export { Client }