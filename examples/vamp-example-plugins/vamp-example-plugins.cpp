/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Piper

    Centre for Digital Music, Queen Mary, University of London.
    Copyright 2015-2017 QMUL.
  
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

#include "PiperExport.h"

#include "ZeroCrossing.h"
#include "SpectralCentroid.h"
#include "PercussionOnsetDetector.h"
#include "FixedTempoEstimator.h"
#include "AmplitudeFollower.h"
#include "PowerSpectrum.h"

using piper_vamp_js::PiperAdapter;
using piper_vamp_js::PiperPluginLibrary;

static std::string libname("vamp-example-plugins");

static PiperAdapter<ZeroCrossing>
zeroCrossingAdapter(
    libname,
    { "Low Level Features" },
    {
        { "counts",
            { "http://purl.org/ontology/af/ZeroCrossingCount" }
        },
        { "zerocrossings",
            { "http://purl.org/ontology/af/ZeroCrossing" }
        }
    }
    );

static PiperAdapter<SpectralCentroid>
spectralCentroidAdapter(
    libname,
    { "Low Level Features" },
    {
        { "logcentroid",
            { "http://purl.org/ontology/af/LogFrequencyCentroid" }
        },
        { "linearcentroid",
            { "http://purl.org/ontology/af/LinearFrequencyCentroid" }
        }
    }
    );

static PiperAdapter<PercussionOnsetDetector>
percussionOnsetsAdapter(
    libname,
    { "Time", "Onsets" },
    {
        { "onsets",
            { "http://purl.org/ontology/af/Onset" }
        },
        { "detectionfunction",
            { "http://purl.org/ontology/af/OnsetDetectionFunction" }
        }
    }
    );

static PiperAdapter<AmplitudeFollower>
amplitudeFollowerAdapter(
    libname,
    { "Low Level Features" },
    {
        { "amplitude",
            { "http://purl.org/ontology/af/Signal" }
        }
    }
    );

static PiperAdapter<FixedTempoEstimator>
fixedTempoAdapter(
    libname,
    { "Time", "Tempo" },
    {
        { "tempo",
            { "http://purl.org/ontology/af/Tempo" }
        },
        { "candidates",
            { "http://purl.org/ontology/af/Tempo" }
        },
        { "detectionfunction",
            { "http://purl.org/ontology/af/OnsetDetectionFunction" }
        },
        { "acf",
            { "http://purl.org/ontology/af/Signal" }
        },
        { "filtered_acf",
            { "http://purl.org/ontology/af/Signal" }
        }
    }
    );

static PiperAdapter<PowerSpectrum>
powerSpectrumAdapter(
    libname,
    { "Visualisation" },
    {
        { "powerspectrum",
            { "http://purl.org/ontology/af/Signal" }
        }
    }
    );

static PiperPluginLibrary library({
    &zeroCrossingAdapter,
    &spectralCentroidAdapter,
    &percussionOnsetsAdapter,
    &amplitudeFollowerAdapter,
    &fixedTempoAdapter,
    &powerSpectrumAdapter
});

PIPER_EXPORT_LIBRARY(library);

