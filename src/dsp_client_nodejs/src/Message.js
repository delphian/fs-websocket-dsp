
const _CLASS = '@FaintSignals/dsp-client-nodejs/Message';

class Message {
    version = null;
    id = null;
    commands = null;
    data = null;
    /**
     * Constructor.
     * @param {Object}      args            - Generic argument object.
     * @param {number}      args.version    - Message version. Each version defines subsequent properties.
     * @param {number}      args.id         - Unique message identifier.
     * @param {Command[]}   args.commands   - Array of @FaintSignals/dsp-client-nodejs/Command.
     * @param {Uint8Array}  args.data       - Byte array of binary data.
     */
    constructor(args) {
        if (!args)
            throw `${_CLASS}: Parameter object is required`;
        if (!args.id)
            throw `${_CLASS}: Parameter is required: 'id'`;
        if (!args.commands)
            throw `${_CLASS}: Parameter is required: 'commands'`;
        if (!args.data)
            throw `${_CLASS}: Parameter is required: 'data'`;
        this.version  = args.version;
        this.id       = args.id;
        this.commands = args.commands;
        this.data     = args.data;
    }
    /**
     * Parse a byte array into Message object.
     * @param {Object}      args        - Generic argument object.
     * @param {Uint8Array}  args.stream - Byte array of processing command.
     * @returns Message object.
     */
    static parse(args) {
        if (!args)
            throw `${_CLASS}: Parameter object is required`;
        if (!args.stream)
            throw `${_CLASS}: Parameter is required: 'stream'`;
        let dView = new DataView(args.stream);
        // Calculate properies from stream.
        let src = 0;
        let version       = dView.getUint8(src);    src += 1;
        let id            = dView.getUint32(src);   src += 4;
        let commandsCount = dView.getUint32(src);   src += 4;
        let commands = [];
        for (let i = 0; i < commandsCount; i++) {
            let command = Command.parse({ stream: args.stream.slice(src) })
            commands.push(command);
            src += command.serialize().byteLength;
        }
        let dataLen       = dView.getUint32(src);   src += 4;
        let data          = args.stream.slice(src);
        // Compile properties into object.
        let message = new Message({
            version: version,
            id: id,
            commands: commands,
            data: data
        });
        return message;
    }
    /**
     * Get byte stream of command object.
     * @returns Uint8Array - Processing command byte stream.
     */
    serialize() {
        // Calculate byte array size.
        let version       = new Uint8Array([this.version]);
        let id            = new Uint32Array([this.id]);
        let commandsCount = new Uint32Array([this.commands.length]);
        let commands      = new Uint8Array();
        let dst = 0;
        this.commands.forEach((command) => {
            let cmdStream = command.serialize();
            let newCmds = new Uint8Array(commands.byteLength + cmdStream.byteLength);
            newCmds.set(commands, 0);
            newCmds.set(cmdStream, commands.byteLength);
            commands = newCmds;
        });
        let dataLen = new Uint32Array([(this.data) ? this.data.byteLength : 0]);
        // Compile byte array size.
        let stream = new Uint8Array(
            version.byteLength +
            id.byteLength +
            commandsCount.byteLength +
            commands.byteLength +
            dataLen.byteLength +
            ((this.data) ? this.data.byteLength : 0)
        );
        // Compile byte array.
        stream.set(version,       dst);   dst += version.byteLength;
        stream.set(id,            dst);   dst += id.byteLength;
        stream.set(commandsCount, dst);   dst += commandsCount.byteLength;
        stream.set(commands,      dst);   dst += commands.byteLength;
        stream.set(dataLen,       dst);   dst += dataLen.byteLength;
        stream.set(this.data,     dst);   dst += this.data.byteLength;
        return stream;
    }
}

export { Message }
