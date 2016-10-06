
#include "VamPipeAdapter.h"
#include "VamPipePluginLibrary.h"

#include "Silvet.h"

using vampipe::VamPipeAdapter;
using vampipe::VamPipePluginLibrary;

static std::string soname("silvet");

static VamPipeAdapter<Silvet> silvetAdapter(soname);

static VamPipePluginLibrary library({
    &silvetAdapter
});

extern "C" {

const char *vampipeRequestJson(const char *request) {
    return library.requestJson(request);
}

const char *vampipeProcessRaw(int handle,
                              const float *const *inputBuffers,
                              int sec,
                              int nsec) {
    return library.processRaw(handle, inputBuffers, sec, nsec);
}
    
void vampipeFreeJson(const char *json) {
    return library.freeJson(json);
}

}

