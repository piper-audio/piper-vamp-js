
#include <iostream>
#include <dlfcn.h>

using namespace std;

int main(int, char **)
{
    string example = "./example.so";
    
    void *lib = dlopen(example.c_str(), RTLD_LAZY | RTLD_LOCAL);
    if (!lib) {
	cerr << "failed to open " + example + ": " << dlerror() << endl;
	return 1;
    }

    typedef const char *(*RequestFn)(const char *);
    RequestFn reqFn = (RequestFn)dlsym(lib, "piperRequestJson");
    if (!reqFn) {
	cerr << "failed to find request function in " +
	    example + ": " << dlerror() << endl;
	return 1;
    }
    
    typedef void (*FreeFn)(const char *);
    FreeFn freeFn = (FreeFn)dlsym(lib, "piperFreeJson");
    if (!freeFn) {
	cerr << "failed to find free function in " +
	    example + ": " << dlerror() << endl;
	return 1;
    }

    string listRequest = "{\"method\": \"list\"}";
    const char *listResponse = reqFn(listRequest.c_str());
    cout << listResponse << endl;
    freeFn(listResponse);

    string loadRequest = "{\"method\":\"load\",\"params\": {\"key\":\"vamp-example-plugins:powerspectrum\",\"inputSampleRate\":44100,\"adapterFlags\":[\"AdaptAllSafe\"]}}";
    const char *loadResponse = reqFn(loadRequest.c_str());
    cout << loadResponse << endl;
    freeFn(loadResponse);

    string configRequest = "{\"method\":\"configure\",\"params\":{\"handle\":1,\"configuration\":{\"blockSize\":8,\"channelCount\":1,\"stepSize\":8}}}";
    const char *configResponse = reqFn(configRequest.c_str());
    cout << configResponse << endl;
    freeFn(configResponse);

    string processRequest = "{\"method\":\"process\",\"id\": 6,\"params\":{\"handle\":1,\"processInput\":{\"timestamp\":{\"s\":0,\"n\":0},\"inputBuffers\":[[0,1,0,-1,0,1,0,-1]]}}}";
    const char *processResponse = reqFn(processRequest.c_str());
    cout << processResponse << endl;
    freeFn(processResponse);

    string b64processRequest = "{\"method\":\"process\",\"params\":{\"handle\":1,\"processInput\":{\"timestamp\":{\"s\":0,\"n\":0},\"inputBuffers\":[\"AAAAAAAAgD8AAAAAAACAvwAAAAAAAIA/AAAAAAAAgL8\"]}}}";
    const char *b64processResponse = reqFn(b64processRequest.c_str());
    cout << b64processResponse << endl;
    freeFn(b64processResponse);
    
    string finishRequest = "{\"method\":\"finish\",\"params\":{\"handle\":1}}";
    const char *finishResponse = reqFn(finishRequest.c_str());
    cout << finishResponse << endl;
    freeFn(finishResponse);
}

