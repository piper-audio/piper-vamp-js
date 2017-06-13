/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Piper Vamp JSON Adapter

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

#ifndef PIPER_PLUGIN_LIBRARY_H
#define PIPER_PLUGIN_LIBRARY_H

#include "vamp-support/CountingPluginHandleMapper.h"
#include "vamp-support/PluginStaticData.h"
#include "vamp-support/RequestResponse.h"

#include <vector>
#include <string>
#include <cstring>

namespace piper_vamp_js {

class PiperAdapterInterface;

class PiperPluginLibrary
{
public:
    PiperPluginLibrary(std::vector<PiperAdapterInterface *> pp);
    
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

    piper_vamp::ListResponse listPluginData(piper_vamp::ListRequest r) const;
    piper_vamp::LoadResponse loadPlugin(piper_vamp::LoadRequest r,
                                        std::string &err) const;
    piper_vamp::ConfigurationResponse
    configurePlugin(piper_vamp::ConfigurationRequest r,
                    const piper_vamp::PluginStaticData &psd,
                    std::string &err)
        const;

    // map from pluginKey -> adapter
    std::map<std::string, PiperAdapterInterface *> m_adapters;

    // map from plugin handle -> plugin static data
    std::map<piper_vamp::PluginHandleMapper::Handle,
             piper_vamp::PluginStaticData> m_pluginStaticData;
        
    piper_vamp::CountingPluginHandleMapper m_mapper;
    bool m_useBase64;
};

}

#endif
