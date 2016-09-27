"use strict";

var VampExamplePlugins = require("./VampExamplePlugins");
var base64 = require("./base64");
var exampleModule = VampExamplePlugins();

// It is possible to declare both parameters and return values as
// "string", in which case Emscripten will take care of
// conversions. But it's not clear how one would manage memory for
// newly-constructed returned C strings -- the returned pointer from
// vampipeRequestJson would appear (?) to be thrown away by the
// Emscripten string converter if we declare it as returning a string,
// so we have no opportunity to pass it to vampipeFreeJson, which
// suggests this would leak memory if the string isn't static. Not
// wholly sure though. Anyway, passing and returning pointers (as
// numbers) means we can manage the Emscripten heap memory however we
// want in our request wrapper function below.

var vampipeRequestJson = exampleModule.cwrap(
    'vampipeRequestJson', 'number', ['number']
);

var vampipeProcessRaw = exampleModule.cwrap(
    "vampipeProcessRaw", "number", ["number", "number", "number", "number"]
);

var vampipeFreeJson = exampleModule.cwrap(
    'vampipeFreeJson', 'void', ['number']
);

function note(blah) {
    console.log(blah);
}

function comment(blah) {
    console.log(blah);
}

function processRaw(request) {
    
    const nChannels = request.processInput.inputBuffers.length;
    const nFrames = request.processInput.inputBuffers[0].length;

    const buffersPtr = exampleModule._malloc(nChannels * 4);
    const buffers = new Uint32Array(
        exampleModule.HEAPU8.buffer, buffersPtr, nChannels);

    for (let i = 0; i < nChannels; ++i) {
        const framesPtr = exampleModule._malloc(nFrames * 4);
        const frames = new Float32Array(
            exampleModule.HEAPU8.buffer, framesPtr, nFrames);
        frames.set(request.processInput.inputBuffers[i]);
        buffers[i] = framesPtr;
    }
    
    const responseJson = vampipeProcessRaw(
        request.pluginHandle,
        buffersPtr,
        request.processInput.timestamp.s,
        request.processInput.timestamp.n);
    
    for (let i = 0; i < nChannels; ++i) {
        exampleModule._free(buffers[i]);
    }
    exampleModule._free(buffersPtr);

    const responseJstr = exampleModule.Pointer_stringify(responseJson);
    const response = JSON.parse(responseJstr);
    
    vampipeFreeJson(responseJson);
    
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

function request(jsonStr) {
    note("Request JSON = " + jsonStr);
    var m = exampleModule;
    // Inspection reveals that intArrayFromString converts the string
    // from utf16 to utf8, which is what we want (though the docs
    // don't mention this). Note the *Cstr values are Emscripten heap
    // pointers
    var inCstr = m.allocate(m.intArrayFromString(jsonStr), 'i8', m.ALLOC_NORMAL);
    var outCstr = vampipeRequestJson(inCstr);
    m._free(inCstr);
    var result = m.Pointer_stringify(outCstr);
    vampipeFreeJson(outCstr);
    note("Returned JSON = " + result);
    return result;
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
    const processResponse = response.content;
    const wireFeatures = processResponse.features;
    Object.keys(wireFeatures).forEach(key => {
        return features.set(key, convertWireFeatureList(wireFeatures[key]));
    });
    return features;
}

function test() {

    const rate = 44100;
    
    comment("Loading zero crossings plugin...");
    let result = request('{"type":"load","content": {"pluginKey":"vamp-example-plugins:zerocrossing","inputSampleRate":' + rate + ',"adapterFlags":["AdaptAllSafe"]}}');

    const blockSize = 1024;

    result = request('{"type":"configure","content":{"pluginHandle":1,"configuration":{"blockSize": ' + blockSize + ', "channelCount": 1, "stepSize": ' + blockSize + '}}}');

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
    
    comment("Now processing " + nblocks + " blocks of 1024 samples each...");

    let total = 0;
    
    let start = (new Date()).getTime();
    comment("Start at " + start);
    
    for (let i = 0; i < nblocks; ++i) {
	result = processRaw({
	    "pluginHandle": 1,
	    "processInput": blocks[i]
	});
        let features = responseToFeatureSet(result);
        let count = features.get("counts")[0].featureValues[0];
        total += count;
    }

    let finish = (new Date()).getTime();
    comment("Finish at " + finish + " for a time of " + (finish - start) + " ms");

    comment("Total = " + total);

    comment("Again...");

    total = 0;
    
    start = (new Date()).getTime();
    comment("Start at " + start);
    
    for (let i = 0; i < nblocks; ++i) {
	result = processRaw({
	    "pluginHandle": 1,
	    "processInput": blocks[i]
	});
        let features = responseToFeatureSet(result);
        let count = features.get("counts")[0].featureValues[0];
        total += count;
    }

    finish = (new Date()).getTime();
    comment("Finish at " + finish + " for a time of " + (finish - start) + " ms");
    
    comment("Total = " + total);
    
    comment("Cleaning up the plugin and getting any remaining features...");
    result = request('{"type":"finish","content":{"pluginHandle":1}}');
}

test();

