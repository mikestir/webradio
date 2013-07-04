AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

AM_CXXFLAGS = -Wall -Werror \
	-I. -Idsp -Iio -Iweb \
	$(libmicrohttpd_CFLAGS) \
	$(libfftw3_CFLAGS) \
	$(librtlsdr_CFLAGS) \
	$(libpulsesimple_CFLAGS)

bin_PROGRAMS = webradio
webradio_SOURCES = main.cxx \
	web/apihandler.cxx \
	web/audioencoder.cxx \
	web/audiostream.cxx \
	web/filehandler.cxx \
	web/httpserver.cxx \
	web/mp3encoder.cxx \
	io/pulseaudio.cxx \
	io/rtlsdrtuner.cxx \
	io/samplesink.cxx \
	io/samplesource.cxx \
	io/spectrumsink.cxx \
	dsp/amdemod.cxx \
	dsp/filter.cxx

webradio_LDADD = -lm -lmp3lame \
	$(libmicrohttpd_LIBS) \
	$(libfftw3_LIBS) \
	$(librtlsdr_LIBS) \
	$(libpulsesimple_LIBS)

	
