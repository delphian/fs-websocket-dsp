# Websocket Server for Digital Signal Processing
Websocket server written in C to handle digital signal processing requests.

## Prerequisites
* [Liquid DSP](https://github.com/jgaeddert/liquid-dsp)
    * [Install instructions](https://liquidsdr.org/doc/installation/)
* [libwebsockets](https://libwebsockets.org/)
    * [Install instructions](https://libwebsockets.org/lws-api-doc-main/html/md_READMEs_README_build.html)
* [ws](https://www.npmjs.com/package/ws/v/7.4.3) - (For nodejs client)

## Installation
#### Build and run the server
```
git clone https://github.com/delphian/fs-websocket-dsp.git
cd fs-websocket-dsp
./rebuild.sh
/build/ws_server
```
#### Include node client and call server
```
import WS from 'ws';
import { Client as WsDspClient } from '_path-to_/fs-websocket-dsp/src/dsp_client_nodejs/index.js';

let wsDspClient = new WsDspClient({ ws: new WS('ws://localhost:7681') });

setInterval(() => {
    wsDspClient.sendBinary({
        "message": new Uint8Array([5, 23, 42]),
        "callback": (container) => { },
        "error": (error) => { }
    });
}, 10000);

```