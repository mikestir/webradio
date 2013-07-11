#include <fftw3.h>
#include <string.h>
#include <math.h>

#include "debug.h"
#include "spectrumsink.h"

// FIXME: define M_PI

/* NOTE: Accepts either real or interleaved IQ samples depending on the
 * number of channels specified */
SpectrumSink::SpectrumSink(const string &name) :
			SampleSink(name, "SpectrumSink"),
			inbuf(NULL), outbuf(NULL), inoffset(0),
			_fftSize(DEFAULT_FFT_SIZE)
{
	pthread_mutex_init(&mutex, NULL);
}

SpectrumSink::~SpectrumSink()
{
	pthread_mutex_destroy(&mutex);
}

void SpectrumSink::setFftSize(unsigned int size)
{
	if (isRunning())
		return;

	if (size & (size - 1)) {
		LOG_ERROR("size must be a power of 2\n");
		return;
	}
	_fftSize = size;
}

bool SpectrumSink::init()
{
	/* FIXME: Check number of channels, select appropriate plan */

	/* FIXME: Handle real->complex for spectrum of real signals */
	inbuf = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * _fftSize);
	outbuf = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * _fftSize);
	inoffset = 0;
	p = fftwf_plan_dft_1d(_fftSize, inbuf, outbuf, FFTW_FORWARD, FFTW_ESTIMATE);

	/* Calculate Hamming window */
	window.resize(_fftSize);
	for (unsigned int n = 0; n < _fftSize; n++) {
		window[n] = 0.54 - 0.46 * cosf(2 * M_PI * (float)n / (float)(_fftSize - 1));
	}

	return true;
}

void SpectrumSink::deinit()
{
	fftwf_destroy_plan(p);
	fftwf_cleanup();
	fftwf_free(inbuf);
	fftwf_free(outbuf);
	inbuf = outbuf = NULL;
}

bool SpectrumSink::process(const vector<sample_t> &inBuffer, vector<sample_t> &outBuffer)
{
	const float *in = (const float*)inBuffer.data();
	unsigned int nframes = inBuffer.size() / inputChannels();
	
	/* FIXME: This currently assumes 2 channels.  It is also pointless
	 * converting the entire input buffer if there is no output queue */

	/* NOTE: Must use float version of FFTW because we expect
	 * fftw_complex to be float[2] not double[2] to match our
	 * sample type.  This code should work for both real and
	 * complex DFTs (_channels = 1 or 2) as long as the plan is
	 * configured correctly first */
	while (nframes) {
		unsigned int blocksize = _fftSize - inoffset;
		if (blocksize > nframes)
			blocksize = nframes;
		memcpy(&inbuf[inoffset], in, blocksize * inputChannels() * sizeof(float));
		inoffset += blocksize;
		if (inoffset == _fftSize) {
			/* Apply window */
			for (unsigned int n = 0; n < _fftSize; n++) {
				inbuf[n][0] *= window[n];
				inbuf[n][1] *= window[n];
			}

			pthread_mutex_lock(&mutex);
			fftwf_execute(p);
			pthread_mutex_unlock(&mutex);
			inoffset = 0;
		}
		nframes -= blocksize;
		in += blocksize * inputChannels();
	}
	return true;
}

void SpectrumSink::getSpectrum(float *magnitudes)
{
	float scaledb = 20 * log10f((float)_fftSize);

	/* Re-order bins into ascending order of frequency:
	 * IFFT output in standard order:
	 * bin:  0  1 ... N/2  N/2+1 ... N-1
	 * freq: DC 1 ... N/2 -N/2-1 ... -1
	 * For an even-length DFT the N/2 bin is both +ve and -ve
	 * Nyquist (N/2) frequency.
	 */
	pthread_mutex_lock(&mutex);
	for (unsigned int n = 0; n < _fftSize; n++) {
		float db = 10 * log10f(outbuf[n][0] * outbuf[n][0] + outbuf[n][1] * outbuf[n][1]);
		magnitudes[(n < _fftSize / 2) ? (n + _fftSize / 2) : (n - _fftSize / 2)] = db - scaledb;
	}
	pthread_mutex_unlock(&mutex);
}
