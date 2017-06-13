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

#include "PiperPluginLibrary.h"
#include "PiperAdapter.h"

#include "vamp-json/VampJson.h"

using namespace std;
using namespace json11;
using namespace piper_vamp;

namespace piper_vamp_js { //!!! not good

//!!! too many explicit namespaces here

//!!! dup with piper-convert
Json
convertRequestJson(string input, string &err)
{
    Json j = Json::parse(input, err);
    if (err != "") {
	err = "invalid json: " + err;
	return {};
    }
    if (!j.is_object()) {
	err = "object expected at top level";
    }
    return j;
}

PiperPluginLibrary::PiperPluginLibrary(vector<PiperAdapterInterface *> pp) :
    m_useBase64(false)
{
    for (PiperAdapterInterface *p: pp) {
	string key = p->getStaticData().pluginKey;
	m_adapters[key] = p;
    }
}

ListResponse
PiperPluginLibrary::listPluginData(ListRequest req) const
{
    bool filtered = !req.from.empty();
    ListResponse resp;
    for (auto a: m_adapters) {
        if (filtered) {
            auto n = a.second->getLibraryName();
            bool found = false;
            for (const auto &f: req.from) {
                if (f == n) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                continue;
            }
        }
	resp.available.push_back(a.second->getStaticData());
    }
    return resp;
}

LoadResponse
PiperPluginLibrary::loadPlugin(LoadRequest req, string &err) const
{
    string key = req.pluginKey;
    if (m_adapters.find(key) != m_adapters.end()) {
        auto resp = m_adapters.at(key)->loadPlugin(req);
        if (!resp.plugin) {
            // This should not actually happen -- the load call here
            // is just an object construction, not a dynamic load. But
            // report it if it does...
            err = "failed to construct plugin with key " + key;
        }
        return resp;
    } else {
	err = "no adapter for plugin key " + key;
        return {};
    }
}

ConfigurationResponse
PiperPluginLibrary::configurePlugin(ConfigurationRequest req,
                                    const PluginStaticData &psd,
                                    string &err) const
{
    for (PluginConfiguration::ParameterMap::const_iterator i =
             req.configuration.parameterValues.begin();
         i != req.configuration.parameterValues.end(); ++i) {
        req.plugin->setParameter(i->first, i->second);
    }

    if (req.configuration.currentProgram != "") {
        req.plugin->selectProgram(req.configuration.currentProgram);
    }

    ConfigurationResponse response;
    response.plugin = req.plugin;
    response.staticOutputInfo = psd.staticOutputInfo;

    Framing pluginPreferredFraming;
    pluginPreferredFraming.stepSize = req.plugin->getPreferredStepSize();
    pluginPreferredFraming.blockSize = req.plugin->getPreferredBlockSize();
    
    if (req.plugin->initialise(req.configuration.channelCount,
                               req.configuration.framing.stepSize,
                               req.configuration.framing.blockSize)) {

        response.outputs = req.plugin->getOutputDescriptors();

        // If the Vamp plugin initialise() call succeeds, then by
        // definition it is accepting the step and block size we
        // passed to it
        response.framing = req.configuration.framing;

    } else {
        
        // If initialise() fails, one reason could be that it didn't
        // like the passed-in framing (step and block size). We need
        // to check whether the passed-in framing differs from the
        // plugin's preferences; if so, then we form a working
        // supposition that initialise() failed because of this. Vamp
        // contains nothing to allow us to test this, except to try
        // initialise() again with different values. So we try again
        // with the values the plugin told us it would prefer and, if
        // that succeeds, return them in a successful response.
        // 
        // See also LoaderRequests in piper-cpp/vamp-support.
        //
        if (req.plugin->initialise(req.configuration.channelCount,
                                   pluginPreferredFraming.stepSize,
                                   pluginPreferredFraming.blockSize)) {

            response.outputs = req.plugin->getOutputDescriptors();
            response.framing = pluginPreferredFraming;

        } else {
            err = "configuration failed (wrong channel count, step size, block size?)";
        }
    }
    
    return response;
}

string
PiperPluginLibrary::processRawImpl(int handle,
                                     const float *const *inputBuffers,
                                     int sec,
                                     int nsec)
{
    Vamp::Plugin *plugin = m_mapper.handleToPlugin(handle);
    if (!plugin) {
        return VampJson::fromError("unknown plugin handle",
                                   RRType::Process, Json())
            .dump();
    }
    
    if (!m_mapper.isConfigured(handle)) {
        return VampJson::fromError("plugin has not been configured",
                                   RRType::Process, Json())
            .dump();
    }

    Vamp::RealTime timestamp(sec, nsec);

    ProcessResponse resp;
    resp.plugin = plugin;
    resp.features = plugin->process(inputBuffers, timestamp);
    
    m_useBase64 = true;
    
    return VampJson::fromRpcResponse_Process
        (resp, m_mapper,
         VampJson::BufferSerialisation::Base64,
         Json())
        .dump();
}

string
PiperPluginLibrary::requestJsonImpl(string req)
{
    string err;
    
    Json j = convertRequestJson(req, err);

    // we don't care what this is, only that it is retained in the response:
    auto id = j["id"];

    Json rj;
    if (err != "") {
        return VampJson::fromError(err, RRType::NotValid, id).dump();
    }
    
    RRType type = VampJson::getRequestResponseType(j, err);
    if (err != "") {
        return VampJson::fromError(err, RRType::NotValid, id).dump();
    }

    VampJson::BufferSerialisation serialisation =
        (m_useBase64 ?
         VampJson::BufferSerialisation::Base64 :
         VampJson::BufferSerialisation::Array);
    
    switch (type) {

    case RRType::List:
    {
        auto req = VampJson::toRpcRequest_List(j, err);
        if (err != "") {
            rj = VampJson::fromError(err, type, id);
        } else {
            rj = VampJson::fromRpcResponse_List(listPluginData(req), id);
        }
        break;
    }

    case RRType::Load:
    {
        auto req = VampJson::toRpcRequest_Load(j, err);
        if (err != "") {
            rj = VampJson::fromError(err, type, id);
        } else {
            auto resp = loadPlugin(req, err);
            if (err != "") {
                rj = VampJson::fromError(err, type, id);
            } else {
                m_mapper.addPlugin(resp.plugin);
                m_pluginStaticData[m_mapper.pluginToHandle(resp.plugin)] =
                    resp.staticData;
                rj = VampJson::fromRpcResponse_Load(resp, m_mapper, id);
            }
        }
        break;
    }

    case RRType::Configure:
    {
        auto req = VampJson::toRpcRequest_Configure(j, m_mapper, err);
        if (err != "") {
            rj = VampJson::fromError(err, type, id);
        } else {
            auto h = m_mapper.pluginToHandle(req.plugin);
            if (h == m_mapper.INVALID_HANDLE) {
                rj = VampJson::fromError
                    ("unknown or invalid plugin handle", type, id);
            } else if (m_mapper.isConfigured(h)) {
                rj = VampJson::fromError
                    ("plugin has already been configured", type, id);
            } else {
                PluginStaticData psd(m_pluginStaticData[h]);
                auto resp = configurePlugin(req, psd, err);
                if (err != "") {
                    rj = VampJson::fromError(err, type, id);
                } else {
                    m_mapper.markConfigured(h,
                                            req.configuration.channelCount,
                                            req.configuration.framing.blockSize);
                    rj = VampJson::fromRpcResponse_Configure(resp, m_mapper, id);
                }
            }
        }
        break;
    }

    case RRType::Process:
    {
        VampJson::BufferSerialisation serialisation;
            
        auto req = VampJson::toRpcRequest_Process(j, m_mapper,
                                                   serialisation, err);
        if (err != "") {
            rj = VampJson::fromError(err, type, id);
        } else {
            auto h = m_mapper.pluginToHandle(req.plugin);
            int channels = int(req.inputBuffers.size());
            if (h == m_mapper.INVALID_HANDLE) {
                rj = VampJson::fromError
                    ("unknown or invalid plugin handle", type, id);
            } else if (!m_mapper.isConfigured(h)) {
                rj = VampJson::fromError
                    ("plugin has not been configured", type, id);
            } else if (channels != m_mapper.getChannelCount(h)) {
                rj = VampJson::fromError
                    ("wrong number of channels supplied", type, id);
            } else {

                if (serialisation == VampJson::BufferSerialisation::Base64) {
                    m_useBase64 = true;
                }

                size_t blockSize = m_mapper.getBlockSize(h);
                
                const float **fbuffers = new const float *[channels];
                for (int i = 0; i < channels; ++i) {
                    if (req.inputBuffers[i].size() != blockSize) {
                        delete[] fbuffers;
                        fbuffers = 0;
                        rj = VampJson::fromError
                            ("wrong block size supplied", type, id);
                        break;
                    }
                    fbuffers[i] = req.inputBuffers[i].data();
                }

                if (fbuffers) {
                    ProcessResponse resp;
                    resp.plugin = req.plugin;
                    resp.features = req.plugin->process(fbuffers, req.timestamp);
                    delete[] fbuffers;
                    rj = VampJson::fromRpcResponse_Process
                        (resp, m_mapper, serialisation, id);
                }
            }
        }
        break;
    }

    case RRType::Finish:
    {
        auto req = VampJson::toRpcRequest_Finish(j, m_mapper, err);
        if (err != "") {
            rj = VampJson::fromError(err, type, id);
        } else {
            auto h = m_mapper.pluginToHandle(req.plugin);
            if (h == m_mapper.INVALID_HANDLE) {
                rj = VampJson::fromError
                    ("unknown or invalid plugin handle", type, id);
            } else {

                FinishResponse resp;
                resp.plugin = req.plugin;

                // Finish can be called (to unload the plugin) even if
                // the plugin has never been configured or used. But
                // we want to make sure we call getRemainingFeatures
                // only if we have actually configured the plugin.
                if (m_mapper.isConfigured(h)) {
                    resp.features = req.plugin->getRemainingFeatures();
                }

                rj = VampJson::fromRpcResponse_Finish
                    (resp, m_mapper, serialisation, id);

                m_pluginStaticData.erase(h);
                m_mapper.removePlugin(h);
                delete req.plugin;
            }
        }
        break;
    }

    case RRType::NotValid:
        rj = VampJson::fromError("invalid request", type, id);
        break;
    }

    return rj.dump();
}

}

