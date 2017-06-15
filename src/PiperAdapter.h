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

#ifndef PIPER_ADAPTER_H
#define PIPER_ADAPTER_H

#include "vamp-support/PluginStaticData.h"
#include "vamp-support/PluginConfiguration.h"
#include "vamp-support/RequestResponse.h"
#include "vamp-support/StaticOutputDescriptor.h"

#include <vamp-hostsdk/PluginInputDomainAdapter.h>
#include <vamp-hostsdk/PluginBufferingAdapter.h>
#include <vamp-hostsdk/PluginChannelAdapter.h>

namespace piper_vamp_js { //!!! not a good name for this namespace

class PiperAdapterInterface
{
public:
    virtual std::string getLibraryName() const = 0;
    virtual piper_vamp::PluginStaticData getStaticData() const = 0;
    virtual piper_vamp::LoadResponse loadPlugin(piper_vamp::LoadRequest r) const = 0;
    virtual Vamp::Plugin *createPlugin(float inputSampleRate) const = 0;
};

template <typename P>
class PiperAdapterBase : public PiperAdapterInterface
{
    const int adaptInputDomain = 0x01;
    const int adaptChannelCount = 0x02;
    const int adaptBufferSize = 0x04;

protected:
    PiperAdapterBase(std::string libname,
                     std::vector<std::string> category = {},
                     piper_vamp::StaticOutputInfo staticOutputInfo = {}) :
        m_soname(libname),
        m_category(category),
        m_staticOutputInfo(staticOutputInfo) { }
    
public:
    virtual std::string getLibraryName() const override {
        return m_soname;
    }
    
    virtual piper_vamp::PluginStaticData getStaticData() const override {
        Vamp::Plugin *p = createPlugin(44100.f);
	auto data = piper_vamp::PluginStaticData::fromPlugin
	    (m_soname + ":" + p->getIdentifier(),
             m_category,
	     p);
        data.staticOutputInfo = m_staticOutputInfo;
        delete p;
        return data;
    }

    virtual piper_vamp::LoadResponse loadPlugin(piper_vamp::LoadRequest r)
        const override {
	
	// We assume the caller has guaranteed that the request is for
	// the correct plugin (so we don't have to check the plugin
	// key field here)

	Vamp::Plugin *p = createPlugin(r.inputSampleRate);
	
	if (r.adapterFlags & adaptInputDomain) {
	    if (p->getInputDomain() == Vamp::Plugin::FrequencyDomain) {
		p = new Vamp::HostExt::PluginInputDomainAdapter(p);
	    }
	}

	if (r.adapterFlags & adaptBufferSize) {
	    p = new Vamp::HostExt::PluginBufferingAdapter(p);
	}

	if (r.adapterFlags & adaptChannelCount) {
	    p = new Vamp::HostExt::PluginChannelAdapter(p);
	}

	piper_vamp::LoadResponse response;
	response.plugin = p;

	response.staticData = piper_vamp::PluginStaticData::fromPlugin
	    (m_soname + ":" + p->getIdentifier(),
             m_category,
	     p);
        response.staticData.staticOutputInfo = m_staticOutputInfo;

	int defaultChannels = 0;
	if (p->getMinChannelCount() == p->getMaxChannelCount()) {
	    defaultChannels = p->getMinChannelCount();
	}

        int defaultBlockSize = p->getPreferredBlockSize();
        int defaultStepSize = p->getPreferredStepSize();

        if (defaultBlockSize == 0) {
            defaultBlockSize = 1024;
        }
        if (defaultStepSize == 0) {
            if (p->getInputDomain() == Vamp::Plugin::FrequencyDomain) {
                defaultStepSize = defaultBlockSize / 2;
            } else {
                defaultStepSize = defaultBlockSize;
            }
        }
        
	response.defaultConfiguration =
            piper_vamp::PluginConfiguration::fromPlugin
	    (p,
	     defaultChannels,
	     defaultStepSize,
             defaultBlockSize);
    
	return response;
    }
    
private:
    std::string m_soname;
    std::vector<std::string> m_category;
    piper_vamp::StaticOutputInfo m_staticOutputInfo;
};

template <typename P>
class PiperAdapter : public PiperAdapterBase<P>
{
public:
    PiperAdapter(std::string libname,
                 std::vector<std::string> category = {},
                 piper_vamp::StaticOutputInfo staticOutputInfo = {}) :
        PiperAdapterBase<P>(libname, category, staticOutputInfo) { }
    
    virtual Vamp::Plugin *createPlugin(float inputSampleRate) const override {
        return new P(inputSampleRate);
    }
};

}

#endif

