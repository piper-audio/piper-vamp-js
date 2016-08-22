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

#include "VamPipeAdapter.h"
#include "VamPipePluginLibrary.h"

#include "vamp-plugin-sdk/examples/ZeroCrossing.h"
#include "vamp-plugin-sdk/examples/SpectralCentroid.h"
#include "vamp-plugin-sdk/examples/PercussionOnsetDetector.h"
#include "vamp-plugin-sdk/examples/FixedTempoEstimator.h"
#include "vamp-plugin-sdk/examples/AmplitudeFollower.h"
#include "vamp-plugin-sdk/examples/PowerSpectrum.h"

using vampipe::VamPipeAdapter;
using vampipe::VamPipePluginLibrary;

static std::string soname("vamp-example-plugins");

static VamPipeAdapter<ZeroCrossing> zeroCrossingAdapter(soname);
static VamPipeAdapter<SpectralCentroid> spectralCentroidAdapter(soname);
static VamPipeAdapter<PercussionOnsetDetector> percussionOnsetAdapter(soname);
static VamPipeAdapter<FixedTempoEstimator> fixedTempoAdapter(soname);
static VamPipeAdapter<AmplitudeFollower> amplitudeAdapter(soname);
static VamPipeAdapter<PowerSpectrum> powerSpectrumAdapter(soname);

static VamPipePluginLibrary library({
    &zeroCrossingAdapter,
    &spectralCentroidAdapter,
    &percussionOnsetAdapter,
    &fixedTempoAdapter,
    &amplitudeAdapter,
    &powerSpectrumAdapter
});

extern "C" {

const char *vampipeRequestJson(const char *request) {
    return library.requestJson(request);
}

void vampipeFreeJson(const char *json) {
    return library.freeJson(json);
}

}

