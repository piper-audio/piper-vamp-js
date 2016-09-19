/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    VamPipe

    Centre for Digital Music, Queen Mary, University of London.
    Copyright 2015-2016 QMUL.
  
    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
    ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    Except as contained in this notice, the names of the Centre for
    Digital Music; Queen Mary, University of London; and Chris Cannam
    shall not be used in advertising or otherwise to promote the sale,
    use or other dealings in this Software without prior written
    authorization.
*/

#ifndef VAMPIPE_PLUGIN_LIBRARY_H
#define VAMPIPE_PLUGIN_LIBRARY_H

#include "bits/CountingPluginHandleMapper.h"

#include <vamp-hostsdk/PluginStaticData.h>
#include <vamp-hostsdk/RequestResponse.h>

#include <vector>
#include <string>
#include <cstring>

namespace vampipe {

class VamPipeAdapterInterface;

class VamPipePluginLibrary
{
public:
    VamPipePluginLibrary(std::vector<VamPipeAdapterInterface *> pp);
    
    const char *requestJson(const char *request) {
	return strdup(requestJsonImpl(request).c_str());
    }

    const char *processRaw(int handle, const float *const *inputBuffers,
                           int sec, int nsec) {
        return strdup(processRawImpl(handle, inputBuffers, sec, nsec).c_str());
    }
    
    void freeJson(const char *json) {
	free(const_cast<char *>(json));
    }
    
private:
    std::string requestJsonImpl(std::string req);
    std::string processRawImpl(int, const float *const *, int, int);

    Vamp::HostExt::ListResponse listPluginData() const;
    Vamp::HostExt::LoadResponse loadPlugin(Vamp::HostExt::LoadRequest r) const;
    Vamp::HostExt::ConfigurationResponse configurePlugin(Vamp::HostExt::
							 ConfigurationRequest r)
        const;

    // map from pluginKey -> adapter
    std::map<std::string, VamPipeAdapterInterface *> m_adapters;
    CountingPluginHandleMapper m_mapper;
    bool m_useBase64;
};

}

#endif
