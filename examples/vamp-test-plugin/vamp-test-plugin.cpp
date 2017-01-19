/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */
/*
    Vamp Test Plugin
    Copyright (c) 2013-2016 Queen Mary, University of London

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
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    Except as contained in this notice, the names of the Centre for
    Digital Music and Queen Mary, University of London shall not be
    used in advertising or otherwise to promote the sale, use or other
    dealings in this Software without prior written authorization.
*/

#include "PiperExport.h"

#include "VampTestPlugin.h"

using piper_vamp_js::PiperAdapter;
using piper_vamp_js::PiperAdapterBase;
using piper_vamp_js::PiperPluginLibrary;

static std::string soname("vamp-test-plugin");

/*
   This is an example of a library that exports more than one "plugin"
   from a single C++ class. The VampTestPlugin class is constructed
   with an argument that determines whether it is a time- or
   frequency-domain plugin, and the library offers both.

   Where normally a library that offered two plugins would have two
   static PiperAdapters specialised with the two plugin classes, here
   we want two static PiperAdapters specialised with the same class
   but different constructor arguments. This is one way to do that,
   taking advantage of the fact that PiperAdapterBase exposes
   createPlugin as a virtual method.

   Note that a very similar mechanism is used in the standard Vamp
   version of this plugin library.
*/

class Adapter : public PiperAdapterBase<VampTestPlugin>
{
public:
    Adapter(bool freq) :
        PiperAdapterBase<VampTestPlugin>(soname),
        m_freq(freq) { }

protected:
    bool m_freq;

    Vamp::Plugin *createPlugin(float inputSampleRate) const {
        return new VampTestPlugin(inputSampleRate, m_freq);
    }
};

static Adapter timeAdapter(false);
static Adapter freqAdapter(true);

static PiperPluginLibrary library({
    &timeAdapter,
    &freqAdapter
});

PIPER_EXPORT_LIBRARY(library);

