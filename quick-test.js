"use strict";

var exampleModule = ExampleModule();

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

var vampipeFreeJson = exampleModule.cwrap(
    'vampipeFreeJson', 'void', ['number']
);

function note(blah) {
    document.getElementById("test-result").innerHTML += blah + "<br>";
}

function comment(blah) {
    note("<br><i>" + blah + "</i>");
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

function test() {

    comment("Querying plugin list...");
    var result = request('{"type": "list"}');

    comment("Loading zero crossings plugin...");
    result = request('{"type":"load","content": {"pluginKey":"vamp-example-plugins:powerspectrum","inputSampleRate":16,"adapterFlags":["AdaptAllSafe"]}}');

    comment("I'm now assuming that the load succeeded and the returned pluginHandle was 1. I haven't bothered to parse the JSON. If those assumptions are wrong, this obviously isn't going to work. Configuring the plugin...");
    result = request('{"type":"configure","content":{"pluginHandle":1,"configuration":{"blockSize": 8, "channelCount": 1, "stepSize": 8}}}');

    comment("If I try to configure it again, it should fail because it's already configured... but this doesn't change anything, and subsequent processing should work fine. Just an example of a failure call. NB this only works if Emscripten has exception catching enabled -- it's off by default in opt builds, which would just end the script here. Wonder what the performance penalty is like.");
    result = request('{"type":"configure","content":{"pluginHandle":1,"configuration":{"blockSize": 8, "channelCount": 1, "stepSize": 8}}}');

    comment("Now processing a couple of blocks of data, on the same assumptions...");
    result = request('{"type":"process","content":{"pluginHandle":1,"processInput":{"timestamp":{"s":0,"n":0},"inputBuffers":[{"values":[0,1,-1,0,1,-1,0,1]}]}}}');
    result = request('{"type":"process","content":{"pluginHandle":1,"processInput":{"timestamp":{"s":0,"n":500000000},"inputBuffers":[{"values":[0,1,-1,0,1,-1,0,1]}]}}}');

    comment("Cleaning up the plugin and getting any remaining features...");
    result = request('{"type":"finish","content":{"pluginHandle":1}}');

    comment("A process call should now fail, as the plugin has been cleaned up.");
    result = request('{"type":"process","content":{"pluginHandle":1,"processInput":{"timestamp":{"s":0,"n":1000000000},"inputBuffers":[{"values":[0,1,-1,0,1,-1,0,1]}]}}}');
}

window.onload = function() {
    test();
}
