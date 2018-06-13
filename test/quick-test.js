"use strict";

function note(blah) {
    document.getElementById("test-result").innerHTML += blah + "<br>";
}

function comment(blah) {
    note("<br><i>" + blah + "</i>");
}

function test() {

    VampExamplePluginsModule().then(function(exampleModule) {
        
        // It is possible to declare both parameters and return values
        // as "string", in which case Emscripten will take care of
        // conversions. But it's not clear how one would manage memory
        // for newly-constructed returned C strings -- the returned
        // pointer from piperRequestJson would appear (?) to be thrown
        // away by the Emscripten string converter if we declare it as
        // returning a string, so we have no opportunity to pass it to
        // piperFreeJson, which suggests this would leak memory if the
        // string isn't static. Not wholly sure though. Anyway,
        // passing and returning pointers (as numbers) means we can
        // manage the Emscripten heap memory however we want in our
        // request wrapper function below.

        var piperRequestJson = exampleModule.cwrap(
            'piperRequestJson', 'number', ['number']
        );
        
        var piperFreeJson = exampleModule.cwrap(
            'piperFreeJson', 'void', ['number']
        );

        function request(jsonStr) {
            note("Request JSON = " + jsonStr);
            var m = exampleModule;
            // Inspection reveals that intArrayFromString converts the
            // string from utf16 to utf8, which is what we want
            // (though the docs don't mention this). Note the *Cstr
            // values are Emscripten heap pointers
            var inCstr = m.allocate(m.intArrayFromString(jsonStr), 'i8', m.ALLOC_NORMAL);
            var outCstr = piperRequestJson(inCstr);
            m._free(inCstr);
            var result = m.Pointer_stringify(outCstr);
            piperFreeJson(outCstr);
            note("Returned JSON = " + result);
            return result;
        }

        comment("Querying plugin list...");
        var result = request('{"method": "list"}');

        comment("Loading zero crossings plugin...");
        result = request('{"method":"load","params": {"key":"vamp-example-plugins:powerspectrum","inputSampleRate":16,"adapterFlags":["AdaptAllSafe"]}}');

        comment("I'm now assuming that the load succeeded and the returned handle was 1. I haven't bothered to parse the JSON. If those assumptions are wrong, this obviously isn't going to work. Configuring the plugin...");
        result = request('{"method":"configure","params":{"handle":1,"configuration":{"framing": { "blockSize": 8, "stepSize": 8 }, "channelCount": 1 }}}');

        comment("If I try to configure it again, it should fail because it's already configured... but this doesn't change anything, and subsequent processing should work fine. Just an example of a failure call. NB this only works if Emscripten has exception catching enabled -- it's off by default in opt builds, which would just end the script here. Wonder what the performance penalty is like.");
        result = request('{"method":"configure","params":{"handle":1,"configuration":{"framing": { "blockSize": 8, "stepSize": 8 }, "channelCount": 1 }}}');

        comment("Now processing a couple of blocks of data, on the same assumptions...");
        result = request('{"method":"process","params":{"handle":1,"processInput":{"timestamp":{"s":0,"n":0},"inputBuffers":[[0,1,-1,0,1,-1,0,1]]}}}');
        result = request('{"method":"process","params":{"handle":1,"processInput":{"timestamp":{"s":0,"n":500000000},"inputBuffers":[[0,1,-1,0,1,-1,0,1]]}}}');

        comment("Cleaning up the plugin and getting any remaining features...");
        result = request('{"method":"finish","params":{"handle":1}}');

        comment("A process call should now fail, as the plugin has been cleaned up.");
        result = request('{"method":"process","params":{"handle":1,"processInput":{"timestamp":{"s":0,"n":1000000000},"inputBuffers":[{"values":[0,1,-1,0,1,-1,0,1]}]}}}');
    });
}

window.onload = function() {
    test();
}
