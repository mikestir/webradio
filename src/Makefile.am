AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

AM_CXXFLAGS = -Wall -Werror \
	-I. -Idsp -Iio -Iweb -Idecoders \
	$(libmicrohttpd_CFLAGS) \
	$(libfftw3_CFLAGS) \
	$(librtlsdr_CFLAGS) \
	$(libpulsesimple_CFLAGS)

bin_PROGRAMS = webradio
webradio_SOURCES = main.cxx \
	web/apihandler.cxx \
	web/audiostream.cxx \
	web/filehandler.cxx \
	web/httpserver.cxx \
	web/mp3encoder.cxx \
	io/randsource.cxx \
	io/pulseaudio.cxx \
	io/rtlsdrtuner.cxx \
	io/spectrumsink.cxx \
	dsp/dspblock.cxx \
	dsp/lowpass.cxx \
	dsp/downconverter.cxx \
	dsp/receiver.cxx \
	dsp/amdemod.cxx

webradio_LDADD = -lm -lmp3lame \
	$(libjsoncpp_LIBS) \
	$(libmicrohttpd_LIBS) \
	$(libfftw3_LIBS) \
	$(librtlsdr_LIBS) \
	$(libpulsesimple_LIBS)

	
