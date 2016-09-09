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

VamPipePluginLibrary::VamPipePluginLibrary(vector<VamPipeAdapterBase *> pp) :
    m_useBase64(false)
{
    for (VamPipeAdapterBase *p: pp) {
	string key = p->getStaticData().pluginKey;
	m_adapters[key] = p;
    }
}

RequestOrResponse
VamPipePluginLibrary::readRequest(string req)
{
    RequestOrResponse rr;
    rr.direction = RequestOrResponse::Request;

    Json j = convertRequestJson(req);

    //!!! reduce, reduce
    rr.type = VampJson::getRequestResponseType(j);
    VampJson::BufferSerialisation serialisation = VampJson::BufferSerialisation::Text;

    switch (rr.type) {

    case RRType::List:
	VampJson::toVampRequest_List(j); // type check only
	break;
    case RRType::Load:
	rr.loadRequest = VampJson::toVampRequest_Load(j);
	break;
    case RRType::Configure:
	rr.configurationRequest = VampJson::toVampRequest_Configure(j, m_mapper);
	break;
    case RRType::Process:
	rr.processRequest = VampJson::toVampRequest_Process(j, m_mapper, serialisation);
	break;
    case RRType::Finish:
	rr.finishPlugin = VampJson::toVampRequest_Finish(j, m_mapper);
	break;
    case RRType::NotValid:
	break;
    }

    if (serialisation == VampJson::BufferSerialisation::Base64) {
        m_useBase64 = true;
    }
    
    return rr;
}

string
VamPipePluginLibrary::writeResponse(const RequestOrResponse &rr) const
{
    Json j;

    VampJson::BufferSerialisation serialisation =
        (m_useBase64 ?
         VampJson::BufferSerialisation::Base64 :
         VampJson::BufferSerialisation::Text);

    switch (rr.type) {

    case RRType::List:
	j = VampJson::fromVampResponse_List("", rr.listResponse);
	break;
    case RRType::Load:
	j = VampJson::fromVampResponse_Load(rr.loadResponse, m_mapper);
	break;
    case RRType::Configure:
	j = VampJson::fromVampResponse_Configure(rr.configurationResponse);
	break;
    case RRType::Process:
	j = VampJson::fromVampResponse_Process(rr.processResponse, serialisation);
	break;
    case RRType::Finish:
	j = VampJson::fromVampResponse_Finish(rr.finishResponse, serialisation);
	break;
    case RRType::NotValid:
	break;
    }

    return j.dump();
}

vector<Vamp::HostExt::PluginStaticData>
VamPipePluginLibrary::listPluginData() const
{
    vector<Vamp::HostExt::PluginStaticData> data;
    for (auto a: m_adapters) {
	data.push_back(a.second->getStaticData());
    }
    return data;
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

    if (req.plugin->initialise(req.configuration.channelCount,
                               req.configuration.stepSize,
                               req.configuration.blockSize)) {
        response.outputs = req.plugin->getOutputDescriptors();
    }

    return response;
}

string
VamPipePluginLibrary::processImpl(int pluginHandle,
                                  const float *const *inputBuffers,
                                  int sec,
                                  int nsec)
{
    RequestOrResponse response;
    response.direction = RequestOrResponse::Response;
    response.type = RRType::Process;

    try {
        if (!m_mapper.isConfigured(pluginHandle)) {
            throw runtime_error("plugin has not been configured");
        }

        Vamp::Plugin *plugin = m_mapper.handleToPlugin(pluginHandle);
        Vamp::RealTime timestamp(sec, nsec);
        
        response.processResponse.features = plugin->process(inputBuffers, timestamp);
        response.success = true;
        
	return writeResponse(response);

    } catch (const std::exception &e) {
	return VampJson::fromException(e, RRType::Process).dump();
    }

    m_useBase64 = true; //!!! todo: return something raw as well!
}

string
VamPipePluginLibrary::requestJsonImpl(string req)
{
    RequestOrResponse request;

    try {
	request = readRequest(req);
    } catch (const std::exception &e) {
	return VampJson::fromException(e, RRType::NotValid).dump();
    }

    RequestOrResponse response;
    response.direction = RequestOrResponse::Response;
    response.type = request.type;

    try {
	switch (request.type) {

	case RRType::List:
	    response.listResponse = listPluginData();
	    response.success = true;
	    break;

	case RRType::Load:
	    response.loadResponse = loadPlugin(request.loadRequest);
	    if (response.loadResponse.plugin) {
		m_mapper.addPlugin(response.loadResponse.plugin);
		response.success = true;
	    }
	    break;
	
	case RRType::Configure:
	{
	    auto &creq = request.configurationRequest;
	    auto h = m_mapper.pluginToHandle(creq.plugin);
	    if (m_mapper.isConfigured(h)) {
		throw runtime_error("plugin has already been configured");
	    }

	    response.configurationResponse = configurePlugin(creq);
	
	    if (!response.configurationResponse.outputs.empty()) {
		m_mapper.markConfigured
		    (h, creq.configuration.channelCount, creq.configuration.blockSize);
		response.success = true;
	    }
	    break;
	}

	case RRType::Process:
	{
	    auto &preq = request.processRequest;
	    auto h = m_mapper.pluginToHandle(preq.plugin);
	    if (!m_mapper.isConfigured(h)) {
		throw runtime_error("plugin has not been configured");
	    }

	    int channels = int(preq.inputBuffers.size());
	    if (channels != m_mapper.getChannelCount(h)) {
		throw runtime_error("wrong number of channels supplied to process");
	    }

	    const float **fbuffers = new const float *[channels];
	    for (int i = 0; i < channels; ++i) {
		if (int(preq.inputBuffers[i].size()) != m_mapper.getBlockSize(h)) {
		    delete[] fbuffers;
		    throw runtime_error("wrong block size supplied to process");
		}
		fbuffers[i] = preq.inputBuffers[i].data();
	    }

	    response.processResponse.features =
                preq.plugin->process(fbuffers, preq.timestamp);
	    response.success = true;

	    delete[] fbuffers;
	    break;
	}

	case RRType::Finish:
	{
	    auto h = m_mapper.pluginToHandle(request.finishPlugin);

	    response.finishResponse.features =
		request.finishPlugin->getRemainingFeatures();
	    
	    m_mapper.removePlugin(h);
	    delete request.finishPlugin;
	    response.success = true;
	    break;
	}

	case RRType::NotValid:
	    break;
	}
    
	return writeResponse(response);

    } catch (const std::exception &e) {
	return VampJson::fromException(e, request.type).dump();
    }
}

}

