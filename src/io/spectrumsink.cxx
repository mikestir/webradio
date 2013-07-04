#include <fftw3.h>
#include <string.h>
#include <math.h>

#include "debug.h"
#include "spectrumsink.h"

/* NOTE: Accepts either real or interleaved IQ samples depending on the
 * number of channels specified */
SpectrumSink::SpectrumSink() :
			SampleSink(),
			inbuf(NULL), outbuf(NULL), inoffset(0),
			_fftSize(DEFAULT_FFT_SIZE)
{
	pthread_mutex_init(&mutex, NULL);
}

SpectrumSink::~SpectrumSink()
{
	pthread_mutex_destroy(&mutex);
}

bool SpectrumSink::start()
{
	if (inbuf != NULL)
		stop();
	
	/* FIXME: Handle real->complex for spectrum of real signals */
	inbuf = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * _fftSize);
	outbuf = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * _fftSize);
	inoffset = 0;
	p = fftwf_plan_dft_1d(_fftSize, inbuf, outbuf, FFTW_FORWARD, FFTW_ESTIMATE);

	return true;
}

void SpectrumSink::stop()
{
	if (inbuf == NULL)
		return;

	fftwf_destroy_plan(p);
	fftwf_cleanup();
	fftwf_free(inbuf);
	fftwf_free(outbuf);
	inbuf = outbuf = NULL;
}

void SpectrumSink::setSamplerate(unsigned int samplerate)
{
	if (inbuf != NULL)
		return;
	
	_samplerate = samplerate;
}

void SpectrumSink::setChannels(unsigned int channels)
{
	if (inbuf != NULL)
		return;
	
	if (!channels || channels > 2) {
		LOG_ERROR("channels must be 1 (real) or 2 (IQ)\n");
		return;
	}
	
	_channels = channels;
}

void SpectrumSink::setFftSize(unsigned int size)
{
	if (inbuf != NULL)
		return;
	
	if (size & (size - 1)) {
		LOG_ERROR("size must be a power of 2\n");
		return;
	}
	_fftSize = size;
}

void SpectrumSink::push(float *samples, unsigned int nframes)
{
	if (inbuf == NULL)
		return;
	
	/* NOTE: Must use float version of FFTW because we expect
	 * fftw_complex to be float[2] not double[2] to match our
	 * sample type.  This code should work for both real and
	 * complex DFTs (_channels = 1 or 2) as long as the plan is
	 * configured correctly first */
	while (nframes) {
		unsigned int blocksize = _fftSize - inoffset;
		if (blocksize > nframes)
			blocksize = nframes;
		memcpy(&inbuf[inoffset], samples, blocksize * sizeof(float) * _channels);
		inoffset += blocksize;
		if (inoffset == _fftSize) {
			pthread_mutex_lock(&mutex);
			fftwf_execute(p);
			pthread_mutex_unlock(&mutex);
			inoffset = 0;
		}
		nframes -= blocksize;
		samples += blocksize * _channels;
	}
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
