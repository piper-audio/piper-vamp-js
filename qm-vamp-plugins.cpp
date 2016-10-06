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

#include "plugins/BeatTrack.h"
#include "plugins/OnsetDetect.h"
#include "plugins/ChromagramPlugin.h"
#include "plugins/ConstantQSpectrogram.h"
#include "plugins/TonalChangeDetect.h"
#include "plugins/KeyDetect.h"
#include "plugins/MFCCPlugin.h"
#include "plugins/SegmenterPlugin.h"
#include "plugins/SimilarityPlugin.h"
#include "plugins/BarBeatTrack.h"
//!!!#include "plugins/AdaptiveSpectrogram.h"
#include "plugins/DWT.h"
#include "plugins/Transcription.h"

using vampipe::VamPipeAdapter;
using vampipe::VamPipePluginLibrary;

static std::string soname("qm-vamp-plugins");

static VamPipeAdapter<BeatTracker> beatTrackerAdapter(soname);
static VamPipeAdapter<OnsetDetector> onsetDetectorAdapter(soname);
static VamPipeAdapter<ChromagramPlugin> chromagramPluginAdapter(soname);
static VamPipeAdapter<ConstantQSpectrogram> constantQAdapter(soname);
static VamPipeAdapter<TonalChangeDetect> tonalChangeDetectorAdapter(soname);
static VamPipeAdapter<KeyDetector> keyDetectorAdapter(soname);
static VamPipeAdapter<MFCCPlugin> mfccPluginAdapter(soname);
static VamPipeAdapter<SegmenterPlugin> segmenterPluginAdapter(soname);
static VamPipeAdapter<SimilarityPlugin> similarityPluginAdapter(soname);
static VamPipeAdapter<BarBeatTracker> barBeatTrackPluginAdapter(soname);
//!!!static VamPipeAdapter<AdaptiveSpectrogram> adaptiveSpectrogramAdapter(soname);
static VamPipeAdapter<DWT> dwtAdapter(soname);
static VamPipeAdapter<Transcription> transcriptionAdapter(soname);

static VamPipePluginLibrary library({
	&beatTrackerAdapter,
	    &onsetDetectorAdapter,
	    &chromagramPluginAdapter,
	    &constantQAdapter,
	    &tonalChangeDetectorAdapter,
	    &keyDetectorAdapter,
	    &mfccPluginAdapter,
	    &segmenterPluginAdapter,
	    &similarityPluginAdapter,
	    &barBeatTrackPluginAdapter,
//!!!	    &adaptiveSpectrogramAdapter,
	    &dwtAdapter,
	    &transcriptionAdapter
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

