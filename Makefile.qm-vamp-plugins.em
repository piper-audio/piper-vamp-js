
MODULE_NAME	:= QMVampPlugins

QMV	:= ../qm-vamp-plugins
DSP	:= $(QMV)/qm-dsp
EXT	:= $(DSP)/ext

INCPATH	:= -I$(QMV) -I$(DSP) -I$(EXT)/kissfft -I$(EXT)/kissfft/tools
DEFINES := -DNO_BLAS_WRAP -DADD_ -Dkiss_fft_scalar=double -I$(EXT)/clapack/include -I$(EXT)/cblas/include

CLAPACK_SOURCES := \
  	$(EXT)/clapack/src/dgetrf.c \
	$(EXT)/clapack/src/dgetri.c \
	$(EXT)/clapack/src/dgetf2.c \
	$(EXT)/clapack/src/xerbla.c \
	$(EXT)/clapack/src/dlaswp.c \
	$(EXT)/clapack/src/dtrtri.c \
	$(EXT)/clapack/src/ilaenv.c \
	$(EXT)/clapack/src/iparmq.c \
	$(EXT)/clapack/src/s_cat.c \
	$(EXT)/clapack/src/s_copy.c \
	$(EXT)/clapack/src/s_cmp.c \
	$(EXT)/clapack/src/pow_di.c \
	$(EXT)/clapack/src/ieeeck.c \
	$(EXT)/clapack/src/i_nint.c \
	$(EXT)/clapack/src/dtrti2.c \
	$(EXT)/clapack/src/f77_aloc.c \
	$(EXT)/clapack/src/exit_.c 

CBLAS_SOURCES := \
	$(EXT)/cblas/src/dgemm.c \
	$(EXT)/cblas/src/ddot.c \
	$(EXT)/cblas/src/dgemv.c \
	$(EXT)/cblas/src/dswap.c \
	$(EXT)/cblas/src/dtrsm.c \
	$(EXT)/cblas/src/dger.c \
	$(EXT)/cblas/src/idamax.c \
	$(EXT)/cblas/src/dscal.c \
	$(EXT)/cblas/src/dtrmm.c \
	$(EXT)/cblas/src/lsame.c \
	$(EXT)/cblas/src/dlamch.c \
	$(EXT)/cblas/src/dtrmv.c \
	$(EXT)/cblas/src/cblas_globals.c \
	$(EXT)/cblas/src/cblas_dgemm.c \
	$(EXT)/cblas/src/cblas_ddot.c \
	$(EXT)/cblas/src/cblas_xerbla.c

DSP_SOURCES := \
	$(DSP)/base/Pitch.cpp \
	$(DSP)/base/KaiserWindow.cpp \
	$(DSP)/base/SincWindow.cpp \
	$(DSP)/dsp/chromagram/Chromagram.cpp \
	$(DSP)/dsp/chromagram/ConstantQ.cpp \
	$(DSP)/dsp/keydetection/GetKeyMode.cpp \
	$(DSP)/dsp/mfcc/MFCC.cpp \
	$(DSP)/dsp/onsets/DetectionFunction.cpp \
	$(DSP)/dsp/onsets/PeakPicking.cpp \
	$(DSP)/dsp/phasevocoder/PhaseVocoder.cpp \
	$(DSP)/dsp/rateconversion/Decimator.cpp \
	$(DSP)/dsp/rateconversion/DecimatorB.cpp \
	$(DSP)/dsp/rateconversion/Resampler.cpp \
	$(DSP)/dsp/rhythm/BeatSpectrum.cpp \
	$(DSP)/dsp/segmentation/ClusterMeltSegmenter.cpp \
	$(DSP)/dsp/segmentation/Segmenter.cpp \
	$(DSP)/dsp/signalconditioning/DFProcess.cpp \
	$(DSP)/dsp/signalconditioning/Filter.cpp \
	$(DSP)/dsp/signalconditioning/FiltFilt.cpp \
	$(DSP)/dsp/signalconditioning/Framer.cpp \
	$(DSP)/dsp/tempotracking/DownBeat.cpp \
	$(DSP)/dsp/tempotracking/TempoTrack.cpp \
	$(DSP)/dsp/tempotracking/TempoTrackV2.cpp \
	$(DSP)/dsp/tonal/ChangeDetectionFunction.cpp \
	$(DSP)/dsp/tonal/TCSgram.cpp \
	$(DSP)/dsp/tonal/TonalEstimator.cpp \
	$(DSP)/dsp/transforms/DCT.cpp \
	$(DSP)/dsp/transforms/FFT.cpp \
	$(DSP)/dsp/wavelet/Wavelet.cpp \
	$(DSP)/maths/Correlation.cpp \
	$(DSP)/maths/CosineDistance.cpp \
	$(DSP)/maths/KLDivergence.cpp \
	$(DSP)/maths/MathUtilities.cpp

#	$(DSP)/thread/Thread.cpp \

#	$(QMV)/plugins/AdaptiveSpectrogram.cpp \

PLUGIN_SOURCES := \
	$(QMV)/plugins/BarBeatTrack.cpp \
	$(QMV)/plugins/BeatTrack.cpp \
	$(QMV)/plugins/DWT.cpp \
	$(QMV)/plugins/OnsetDetect.cpp \
	$(QMV)/plugins/ChromagramPlugin.cpp \
	$(QMV)/plugins/ConstantQSpectrogram.cpp \
	$(QMV)/plugins/KeyDetect.cpp \
	$(QMV)/plugins/MFCCPlugin.cpp \
	$(QMV)/plugins/SegmenterPlugin.cpp \
	$(QMV)/plugins/SimilarityPlugin.cpp \
	$(QMV)/plugins/TonalChangeDetect.cpp \
	$(QMV)/plugins/Transcription.cpp \
	$(DSP_SOURCES)

C_SOURCES	:= \
	$(QMV)/g2cstubs.c \
	$(DSP)/dsp/segmentation/cluster_segmenter.c \
	$(DSP)/dsp/segmentation/cluster_melt.c \
	$(DSP)/hmm/hmm.c \
	$(DSP)/maths/pca/pca.c \
	$(DSP)/ext/kissfft/kiss_fft.c \
	$(DSP)/ext/kissfft/tools/kiss_fftr.c \
	$(CLAPACK_SOURCES) \
	$(CBLAS_SOURCES)

MODULE_SOURCE	:= qm-vamp-plugins.cpp

include Makefile.inc.em
