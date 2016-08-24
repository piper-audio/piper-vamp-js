
#include <iostream>
#include <dlfcn.h>

using namespace std;

int main(int argc, char **argv)
{
    string example = "./example.so";
    
    void *lib = dlopen(example.c_str(), RTLD_LAZY | RTLD_LOCAL);
    if (!lib) {
	cerr << "failed to open " + example + ": " << dlerror() << endl;
	return 1;
    }

    typedef const char *(*RequestFn)(const char *);
    RequestFn reqFn = (RequestFn)dlsym(lib, "vampipeRequestJson");
    if (!reqFn) {
	cerr << "failed to find request function in " +
	    example + ": " << dlerror() << endl;
	return 1;
    }
    
    typedef void (*FreeFn)(const char *);
    FreeFn freeFn = (FreeFn)dlsym(lib, "vampipeFreeJson");
    if (!freeFn) {
	cerr << "failed to find free function in " +
	    example + ": " << dlerror() << endl;
	return 1;
    }

    string listRequest = "{\"type\": \"list\"}";
    const char *listResponse = reqFn(listRequest.c_str());
    cout << listResponse << endl;
    freeFn(listResponse);

    string loadRequest = "{\"type\":\"load\",\"content\": {\"pluginKey\":\"vamp-example-plugins:zerocrossing\",\"inputSampleRate\":44100,\"adapterFlags\":[\"AdaptAllSafe\"]}}";
    const char *loadResponse = reqFn(loadRequest.c_str());
    cout << loadResponse << endl;
    freeFn(loadResponse);
}

