importScripts("a.out.js");

self.addEventListener('message', function(e) {
    var file = e.data;
    process(file);
}, false);

function process(file) {

    console.log("in worker process", file);

    var fr = new FileReader();

    console.log("worker", file);

    fr.readAsArrayBuffer(file);

    fr.onloadend = function (e){
        var data = fr.result; // Uint8 for emscripten
        console.log("worker", data)

        var numBytes = data.byteLength; // 1 bytes per element
        var data_Uint8 = new Uint8Array(data);

        var ptr = Module._malloc(numBytes);
        var heapBytes = new Uint8Array(Module.HEAPU8.buffer, ptr, numBytes);
        for (let i=0;i<numBytes;i++)
            heapBytes[i] = data_Uint8[i]

        for (let i=0;i<100;i++)
            console.log(heapBytes[i])

        console.log("byteoffset ", numBytes);

        Module.ccall("make_bin", // c function name
                undefined, // return
                ["number"], // param
                [heapBytes.byteOffset]
        );

        Module._free(heapBytes.byteOffset)

        const out_data = Module['FS_readFile']('data.txt', { encoding: 'utf8'});
        const out_bin = Module['FS_readFile']('out.bin');
        self.postMessage({out_bin, out_data});
    }
}

