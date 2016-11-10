"use strict";

function note(blah) {
    console.log(blah);
}

if (process.argv.length < 4) {
    note("\nUsage: " + process.argv[0] + " <LibraryPath> <pluginKey>");
    note("e.g. " + process.argv[0] + " ./VampExamplePlugins.js vamp-example-plugins:zerocrossing");
    throw "Wrong number of command-line args (2 expected)"
}

var libraryPath = process.argv[2];
var pluginKey = process.argv[3];

var base64 = require("./base64");

note("Loading library \"" + libraryPath + "\"...");
var extractor = require(libraryPath);
var extractorModule = extractor();

var piperRequestJson = extractorModule.cwrap(
    'piperRequestJson', 'number', ['number']
);

var piperProcessRaw = extractorModule.cwrap(
    "piperProcessRaw", "number", ["number", "number", "number", "number"]
);

var piperFreeJson = extractorModule.cwrap(
    'piperFreeJson', 'void', ['number']
);

function processRaw(request) {
    
    const nChannels = request.processInput.inputBuffers.length;
    const nFrames = request.processInput.inputBuffers[0].length;

    const buffersPtr = extractorModule._malloc(nChannels * 4);
    const buffers = new Uint32Array(
        extractorModule.HEAPU8.buffer, buffersPtr, nChannels);

    for (let i = 0; i < nChannels; ++i) {
        const framesPtr = extractorModule._malloc(nFrames * 4);
        const frames = new Float32Array(
            extractorModule.HEAPU8.buffer, framesPtr, nFrames);
        frames.set(request.processInput.inputBuffers[i]);
        buffers[i] = framesPtr;
    }
    
    const responseJson = piperProcessRaw(
        request.handle,
        buffersPtr,
        request.processInput.timestamp.s,
        request.processInput.timestamp.n);
    
    for (let i = 0; i < nChannels; ++i) {
        extractorModule._free(buffers[i]);
    }
    extractorModule._free(buffersPtr);

    const responseJstr = extractorModule.Pointer_stringify(responseJson);
    const response = JSON.parse(responseJstr);
    
    piperFreeJson(responseJson);
    
    return response;
}

function makeTimestamp(seconds) {
    if (seconds >= 0.0) {
        return {
            s: Math.floor(seconds),
            n: Math.floor((seconds - Math.floor(seconds)) * 1e9 + 0.5)
        };
    } else {
        const { s, n } = makeTimestamp(-seconds);
        return { s: -s, n: -n };
    }
}

function frame2timestamp(frame, rate) {
    return makeTimestamp(frame / rate);
}

function request(req) {
    var jsonStr = JSON.stringify(req);
    var m = extractorModule;
    // Inspection reveals that intArrayFromString converts the string
    // from utf16 to utf8, which is what we want (though the docs
    // don't mention this). Note the *Cstr values are Emscripten heap
    // pointers
    let inCstr = m.allocate(m.intArrayFromString(jsonStr), 'i8', m.ALLOC_NORMAL);
    let outCstr = piperRequestJson(inCstr);
    m._free(inCstr);
    const responseJstr = m.Pointer_stringify(outCstr);
    const response = JSON.parse(responseJstr);
    piperFreeJson(outCstr);
    return response;
}

function myFromBase64(b64) {
    while (b64.length % 4 > 0) { b64 += "="; }
    let conv = new Float32Array(base64.toByteArray(b64).buffer);
    return conv;
}

function convertWireFeature(wfeature) {
    let out = {};
    if (wfeature.timestamp != null) {
        out.timestamp = wfeature.timestamp;
    }
    if (wfeature.duration != null) {
        out.duration = wfeature.duration;
    }
    if (wfeature.label != null) {
        out.label = wfeature.label;
    }
    const vv = wfeature.featureValues;
    if (vv != null) {
        if (typeof vv === "string") {
            out.featureValues = myFromBase64(vv);
        } else {
            out.featureValues = new Float32Array(vv);
        }
    }
    return out;
}

function convertWireFeatureList(wfeatures) {
    return wfeatures.map(convertWireFeature);
}

function responseToFeatureSet(response) {
    const features = new Map();
    const processResponse = response.result;
    const wireFeatures = processResponse.features;
    Object.keys(wireFeatures).forEach(key => {
        return features.set(key, convertWireFeatureList(wireFeatures[key]));
    });
    return features;
}

function test() {

    const rate = 44100;
    
    note("Loading plugin \"" + pluginKey + "\"...");
    let response = request({
        method: "load",
        params: {
            key: pluginKey,
            inputSampleRate: rate,
            adapterFlags: ["AdaptAllSafe"]
        }
    });

    const blockSize = 1024;

    response = request({
        method: "configure",
        params: {
            handle: 1,
            configuration: {
                blockSize: blockSize,
                channelCount: 1,
                stepSize: blockSize
            }
        }
    });

    const nblocks = 1000;

    const makeBlock = (n => { 
        return {
            timestamp : frame2timestamp(n * blockSize, rate),
            inputBuffers : [
                new Float32Array(Array.from(Array(blockSize).keys(),
                                            n => n / blockSize))
            ],
        }
    });
    
    const blocks = Array.from(Array(nblocks).keys(), makeBlock);
    
    note("Now processing " + nblocks + " blocks of 1024 samples each...");

    let featureCount = 0;
    
    for (let i = 0; i < nblocks; ++i) {
	response = processRaw({
	    "handle": 1,
	    "processInput": blocks[i]
	});
        let features = responseToFeatureSet(response);
        for (let featureList of features.values()) {
            featureCount += featureList.length;
        }
        console.log(i);
    }

    note("Cleaning up the plugin and getting any remaining features...");
    response = request({
        method: "finish",
        params: {
            handle: 1
        }
    });

    let features = responseToFeatureSet(response);
    for (let featureList of features.values()) {
        featureCount += featureList.length;
    }
    
    note("Done, total number of features across all outputs = " + featureCount);
}

test();

