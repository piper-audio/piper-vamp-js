
#include "PiperAdapter.h"
#include "PiperPluginLibrary.h"

#include "Silvet.h"

using piper::PiperAdapter;
using piper::PiperPluginLibrary;

static std::string soname("silvet");

static PiperAdapter<Silvet> silvetAdapter(soname);

static PiperPluginLibrary library({
    &silvetAdapter
});

extern "C" {

const char *piperRequestJson(const char *request) {
    return library.requestJson(request);
}

const char *piperProcessRaw(int handle,
                              const float *const *inputBuffers,
                              int sec,
                              int nsec) {
    return library.processRaw(handle, inputBuffers, sec, nsec);
}
    
void piperFreeJson(const char *json) {
    return library.freeJson(json);
}

}

