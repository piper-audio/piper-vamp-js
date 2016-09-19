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
static Json
convertRequestJson(string input)
{
    string err;
    Json j = Json::parse(input, err);
    if (err != "") {
	throw VampJson::Failure("invalid json: " + err);
    }
    if (!j.is_object()) {
	throw VampJson::Failure("object expected at top level");
    }
    if (!j["type"].is_string()) {
	throw VampJson::Failure("string expected for type field");
    }
    if (!j["content"].is_null() && !j["content"].is_object()) {
	throw VampJson::Failure("object expected for content field");
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
VamPipePluginLibrary::loadPlugin(Vamp::HostExt::LoadRequest req) const
{
    string key = req.pluginKey;
    if (m_adapters.find(key) != m_adapters.end()) {
	return m_adapters.at(key)->loadPlugin(req);
    } else {
	throw runtime_error("no adapter for plugin key " + key);
    }
}

Vamp::HostExt::ConfigurationResponse
VamPipePluginLibrary::configurePlugin(Vamp::HostExt::ConfigurationRequest req) const
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
    }

    return response;
}

string
VamPipePluginLibrary::processRawImpl(int pluginHandle,
                                     const float *const *inputBuffers,
                                     int sec,
                                     int nsec)
{
    try {
        if (!m_mapper.isConfigured(pluginHandle)) {
            throw runtime_error("plugin has not been configured");
        }

        Vamp::Plugin *plugin = m_mapper.handleToPlugin(pluginHandle);
        Vamp::RealTime timestamp(sec, nsec);

        Vamp::HostExt::ProcessResponse resp;
        resp.plugin = plugin;
        resp.features = plugin->process(inputBuffers, timestamp);
        
        m_useBase64 = true;

        return VampJson::fromVampResponse_Process
            (resp, m_mapper,
             VampJson::BufferSerialisation::Base64)
            .dump();

    } catch (const std::exception &e) {
	return VampJson::fromException(e, RRType::Process)
            .dump();
    }
}

string
VamPipePluginLibrary::requestJsonImpl(string req)
{
    Json j = convertRequestJson(req);

    RRType type;
    try {
        type = VampJson::getRequestResponseType(j);
    } catch (const std::exception &e) {
	return VampJson::fromException(e, RRType::NotValid)
            .dump();
    }

    VampJson::BufferSerialisation serialisation =
        (m_useBase64 ?
         VampJson::BufferSerialisation::Base64 :
         VampJson::BufferSerialisation::Text);

    Json rj;
    
    try {
        switch (type) {

        case RRType::List:
            rj = VampJson::fromVampResponse_List(listPluginData());
            break;

        case RRType::Load:
        {
            auto req = VampJson::toVampRequest_Load(j);
            auto resp = loadPlugin(req);
            if (resp.plugin) {
                m_mapper.addPlugin(resp.plugin);
            }
            rj = VampJson::fromVampResponse_Load(resp, m_mapper);
            break;
        }

        case RRType::Configure:
        {
            auto req = VampJson::toVampRequest_Configure(j, m_mapper);
            auto h = m_mapper.pluginToHandle(req.plugin);
            if (m_mapper.isConfigured(h)) {
                throw runtime_error("plugin has already been configured");
            }

            auto resp = configurePlugin(req);
            if (!resp.outputs.empty()) {
                m_mapper.markConfigured(h,
                                        req.configuration.channelCount,
                                        req.configuration.blockSize);
            }

            rj = VampJson::fromVampResponse_Configure(resp, m_mapper);
            break;
        }

        case RRType::Process:
        {
            VampJson::BufferSerialisation serialisation;
            
            auto req = VampJson::toVampRequest_Process(j, m_mapper,
                                                       serialisation);

            auto h = m_mapper.pluginToHandle(req.plugin);
            if (!m_mapper.isConfigured(h)) {
                throw runtime_error("plugin has not been configured");
            }

            int channels = int(req.inputBuffers.size());
            if (channels != m_mapper.getChannelCount(h)) {
                throw runtime_error("wrong number of channels supplied to process");
            }

            if (serialisation == VampJson::BufferSerialisation::Base64) {
                m_useBase64 = true;
            }
            
            const float **fbuffers = new const float *[channels];
            for (int i = 0; i < channels; ++i) {
                if (int(req.inputBuffers[i].size()) != m_mapper.getBlockSize(h)) {
                    delete[] fbuffers;
                    throw runtime_error("wrong block size supplied to process");
                }
                fbuffers[i] = req.inputBuffers[i].data();
            }

            Vamp::HostExt::ProcessResponse resp;
            resp.plugin = req.plugin;
            resp.features = req.plugin->process(fbuffers, req.timestamp);
            delete[] fbuffers;

            rj = VampJson::fromVampResponse_Process(resp, m_mapper, serialisation);
            break;
        }

        case RRType::Finish:
        {
            auto req = VampJson::toVampRequest_Finish(j, m_mapper);
            auto h = m_mapper.pluginToHandle(req.plugin);
            if (!m_mapper.isConfigured(h)) {
                throw runtime_error("plugin has not been configured");
            }

            Vamp::HostExt::ProcessResponse resp;
            resp.plugin = req.plugin;
            resp.features = req.plugin->getRemainingFeatures();

            rj = VampJson::fromVampResponse_Finish(resp, m_mapper, serialisation);
	
            m_mapper.removePlugin(h);
            delete req.plugin;
            break;
        }

        case RRType::NotValid:
            rj = VampJson::fromError("invalid request", type);
            break;
        }
        
    } catch (const std::exception &e) {
        rj = VampJson::fromException(e, type);
    }

    return rj.dump();
}

}

