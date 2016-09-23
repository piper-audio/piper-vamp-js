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

#include "VamPipePluginLibrary.h"
#include "VamPipeAdapter.h"
#include "json/VampJson.h"

using namespace std;
using namespace json11;

namespace vampipe {

//!!! too many explicit namespaces here

//!!! dup with vampipe-convert
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
    } else if (!j["type"].is_string()) {
	err = "string expected for type field";
    } else if (!j["content"].is_null() && !j["content"].is_object()) {
	err = "object expected for content field";
    }
    return j;
}

VamPipePluginLibrary::VamPipePluginLibrary(vector<VamPipeAdapterInterface *> pp) :
    m_useBase64(false)
{
    for (VamPipeAdapterInterface *p: pp) {
	string key = p->getStaticData().pluginKey;
	m_adapters[key] = p;
    }
}

Vamp::HostExt::ListResponse
VamPipePluginLibrary::listPluginData() const
{
    Vamp::HostExt::ListResponse resp;
    for (auto a: m_adapters) {
	resp.pluginData.push_back(a.second->getStaticData());
    }
    return resp;
}

Vamp::HostExt::LoadResponse
VamPipePluginLibrary::loadPlugin(Vamp::HostExt::LoadRequest req, string &err) const
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

Vamp::HostExt::ConfigurationResponse
VamPipePluginLibrary::configurePlugin(Vamp::HostExt::ConfigurationRequest req,
                                      string &err) const
{
    for (Vamp::HostExt::PluginConfiguration::ParameterMap::const_iterator i =
             req.configuration.parameterValues.begin();
         i != req.configuration.parameterValues.end(); ++i) {
        req.plugin->setParameter(i->first, i->second);
    }

    if (req.configuration.currentProgram != "") {
        req.plugin->selectProgram(req.configuration.currentProgram);
    }

    Vamp::HostExt::ConfigurationResponse response;

    response.plugin = req.plugin;

    if (req.plugin->initialise(req.configuration.channelCount,
                               req.configuration.stepSize,
                               req.configuration.blockSize)) {
        response.outputs = req.plugin->getOutputDescriptors();
    } else {
        err = "configuration failed (wrong channel count, step size, block size?)";
    }

    return response;
}

string
VamPipePluginLibrary::processRawImpl(int pluginHandle,
                                     const float *const *inputBuffers,
                                     int sec,
                                     int nsec)
{
    Vamp::Plugin *plugin = m_mapper.handleToPlugin(pluginHandle);
    if (!plugin) {
        return VampJson::fromError("unknown plugin handle", RRType::Process)
            .dump();
    }
    
    if (!m_mapper.isConfigured(pluginHandle)) {
        return VampJson::fromError("plugin has not been configured", RRType::Process)
            .dump();
    }

    Vamp::RealTime timestamp(sec, nsec);

    Vamp::HostExt::ProcessResponse resp;
    resp.plugin = plugin;
    resp.features = plugin->process(inputBuffers, timestamp);
    
    m_useBase64 = true;
    
    return VampJson::fromVampResponse_Process
        (resp, m_mapper,
         VampJson::BufferSerialisation::Base64)
        .dump();
}

string
VamPipePluginLibrary::requestJsonImpl(string req)
{
    string err;
    
    Json j = convertRequestJson(req, err);
    if (err != "") {
	return VampJson::fromError(err, RRType::NotValid).dump();
    }
    
    RRType type = VampJson::getRequestResponseType(j, err);
    if (err != "") {
	return VampJson::fromError(err, RRType::NotValid).dump();
    }

    VampJson::BufferSerialisation serialisation =
        (m_useBase64 ?
         VampJson::BufferSerialisation::Base64 :
         VampJson::BufferSerialisation::Text);

    Json rj;
    
    switch (type) {

    case RRType::List:
        rj = VampJson::fromVampResponse_List(listPluginData());
        break;

    case RRType::Load:
    {
        auto req = VampJson::toVampRequest_Load(j, err);
        if (err != "") {
            rj = VampJson::fromError(err, type);
        } else {
            auto resp = loadPlugin(req, err);
            if (err != "") {
                rj = VampJson::fromError(err, type);
            } else {
                m_mapper.addPlugin(resp.plugin);
                rj = VampJson::fromVampResponse_Load(resp, m_mapper);
            }
        }
        break;
    }

    case RRType::Configure:
    {
        auto req = VampJson::toVampRequest_Configure(j, m_mapper, err);
        if (err != "") {
            rj = VampJson::fromError(err, type);
        } else {
            auto h = m_mapper.pluginToHandle(req.plugin);
            if (h == m_mapper.INVALID_HANDLE) {
                rj = VampJson::fromError("unknown or invalid plugin handle", type);
            } else if (m_mapper.isConfigured(h)) {
                rj = VampJson::fromError("plugin has already been configured", type);
            } else {
                auto resp = configurePlugin(req, err);
                if (err != "") {
                    rj = VampJson::fromError(err, type);
                } else {
                    m_mapper.markConfigured(h,
                                            req.configuration.channelCount,
                                            req.configuration.blockSize);
                    rj = VampJson::fromVampResponse_Configure(resp, m_mapper);
                }
            }
        }
        break;
    }

    case RRType::Process:
    {
        VampJson::BufferSerialisation serialisation;
            
        auto req = VampJson::toVampRequest_Process(j, m_mapper,
                                                   serialisation, err);
        if (err != "") {
            rj = VampJson::fromError(err, type);
        } else {
            auto h = m_mapper.pluginToHandle(req.plugin);
            int channels = int(req.inputBuffers.size());
            if (h == m_mapper.INVALID_HANDLE) {
                rj = VampJson::fromError("unknown or invalid plugin handle", type);
            } else if (!m_mapper.isConfigured(h)) {
                rj = VampJson::fromError("plugin has not been configured", type);
            } else if (channels != m_mapper.getChannelCount(h)) {
                rj = VampJson::fromError("wrong number of channels supplied", type);
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
                        rj = VampJson::fromError("wrong block size supplied", type);
                        break;
                    }
                    fbuffers[i] = req.inputBuffers[i].data();
                }

                if (fbuffers) {
                    Vamp::HostExt::ProcessResponse resp;
                    resp.plugin = req.plugin;
                    resp.features = req.plugin->process(fbuffers, req.timestamp);
                    delete[] fbuffers;
                    rj = VampJson::fromVampResponse_Process
                        (resp, m_mapper, serialisation);
                }
            }
        }
        break;
    }

    case RRType::Finish:
    {
        auto req = VampJson::toVampRequest_Finish(j, m_mapper, err);
        if (err != "") {
            rj = VampJson::fromError(err, type);
        } else {
            auto h = m_mapper.pluginToHandle(req.plugin);
            if (h == m_mapper.INVALID_HANDLE) {
                rj = VampJson::fromError("unknown or invalid plugin handle", type);
            } else if (!m_mapper.isConfigured(h)) {
                rj = VampJson::fromError("plugin has not been configured", type);
            } else {

                Vamp::HostExt::ProcessResponse resp;
                resp.plugin = req.plugin;
                resp.features = req.plugin->getRemainingFeatures();

                rj = VampJson::fromVampResponse_Finish
                    (resp, m_mapper, serialisation);
	
                m_mapper.removePlugin(h);
                delete req.plugin;
            }
        }
        break;
    }

    case RRType::NotValid:
        rj = VampJson::fromError("invalid request", type);
        break;
    }

    return rj.dump();
}

}

