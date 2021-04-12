
const _CLASS = '@FaintSignals/dsp-client-nodejs/Command';

const COMMAND_FN = {
    ECHO: 1 
};

class Command {
    type = null;
    paramsLen = null;
    params = null;
    /**
     * Constructor.
     * @param {Object}     args            - Generic argument object.
     * @param {number}     args.type       - Should be constant COMMAND_FN. 
     * @param {number}     args.paramsLen  - Byte length of parameters.
     * @param {Uint8Array} args.params     - (Optional) Byte stream of parameter data.
     */
    constructor(args) {
        if (!args)
            throw `${_CLASS}: Parameter object is required`;
        if (!args.type)
            throw `${_CLASS}: Parameter is required: 'type'`;
        if (typeof(args.paramsLen) == 'undefined')
            throw `${_CLASS}: Parameter is required: 'paramsLen'`;
        if (args.paramsLen && !args.params)
            throw `${_CLASS}: Parameter is required: 'params'`;
        this.type = args.type;
        this.paramsLen = args.paramsLen;
        this.params = (args.params) ? args.params : null;
    }
    /**
     * Parse a character stream (byte array) into Command class.
     * @param {Object}      args        - Generic argument object.
     * @param {Uint8Array}  args.stream - Byte array of processing command.
     * @returns Processing command object.
     */
    static parse(args) {
        if (!args)
            throw `${_CLASS}: Parameter object is required`;
        if (!args.stream)
            throw `${_CLASS}: Parameter is required: 'stream'`;
        let dView = new DataView(args.stream);
        let command = new Command({ 
            type: dView.getUint32(0),
            pramsLen: dView.getUint32(4),
            params: args.stream.slice(8)
        });
        return command;
    }
    /**
     * Get byte stream of command object.
     * @returns Uint8Array - Processing command byte stream.
     */
    serialize() {
        let type      = new Uint8Array((new Uint32Array([this.type])).buffer);
        let paramsLen = new Uint8Array((new Uint32Array([this.paramsLen])).buffer);
        let stream = new Uint8Array(
            type.byteLength +
            paramsLen.byteLength +
            ((this.params) ? this.params.byteLength : 0));
        let dst = 0;
        stream.set(type,        dst);      dst += type.byteLength;
        stream.set(paramsLen,   dst);      dst += paramsLen.byteLength;
        if (this.paramsLen) {
            stream.set(this.params, dst);  dst += this.params.byteLength;
        }
        return stream;
    }
}

export { COMMAND_FN, Command }
