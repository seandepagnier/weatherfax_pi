/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  weather fax Plugin
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2015 by Sean D'Epagnier                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#include "FaxDecoder.h"

#if defined(__linux__) && !defined(__ANDROID__)
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#endif

#include <math.h>
#include <complex>

bool FaxDecoder::Configure(int imagewidth, int BitsPerPixel, int carrier,
                           int deviation, enum firfilter::Bandwidth bandwidth,
                           double minus_saturation_threshold,
                           bool bSkipHeaderDetection, bool bIncludeHeadersInImages,
                           bool reset)
{
    /* pause decoder */
    m_DecoderStopMutex.Lock();

    m_BitsPerPixel = BitsPerPixel;
    m_carrier = carrier;
    m_deviation = deviation;
    m_bSkipHeaderDetection = bSkipHeaderDetection;
    m_bIncludeHeadersInImages = bIncludeHeadersInImages;
    
    m_imagecolors = 3; /* hardcoded as wximage wants rgb */

    /* TODO: add options? */
    m_faxcolors = 1;
    m_lpm = 120;
    m_bFM = true;
    m_StartFrequency = 300;
    m_StopFrequency = 450;
    m_StartLength = 5;
    m_StopLength = 5;
    m_phasingLines = 40;
    size = 0;

    firfilters[0] = firfilter(bandwidth);
    firfilters[1] = firfilter(bandwidth);

    m_minus_saturation_threshold = minus_saturation_threshold;

    bool ret = true;
    if(reset) {
        CleanUpBuffers();

        m_DecoderReloadMutex.Lock();
        CloseInput();

        switch(m_CaptureSettings.type) {
        case FaxDecoderCaptureSettings::FILE:
            if(DecodeFaxFromFilename())
                break;
        case FaxDecoderCaptureSettings::RTLSDR:
            if(DecodeFaxFromRTLSDR())
                break;
        case FaxDecoderCaptureSettings::AUDIO:
            if(DecodeFaxFromPortAudio() || DecodeFaxFromDSP())
                break;
        default:
            ret = false;
        }

        SetupBuffers();
        m_DecoderReloadMutex.Unlock();
    }

    /* must reset if image width changes */
    if(m_imagewidth != imagewidth || reset) {
        m_imagewidth = imagewidth;
        InitializeImage();
    }

    m_DecoderStopMutex.Unlock();

    m_bEndDecoding = !ret;
    return ret;
}

int FaxDecoder::AudioDeviceCount()
{
#ifdef OCPN_USE_PORTAUDIO
     return Pa_GetDeviceCount();
#endif
     return 1;
}

/* Note: the decoding algorithms are adapted from yahfax (on sourceforge)
   which was an improved adaptation of hamfax. */
bool FaxDecoder::Error(wxString error)
{
    wxMessageDialog w( &m_parent, _("Failure Decoding Fax: ") + error, _("Failed"),
                       wxOK | wxICON_ERROR );
    w.ShowModal();
    return false;
}

static double apply_firfilter(FaxDecoder::firfilter *filter, double sample)
{
// Narrow, middle and wide fir low pass filter from ACfax
     const int buffer_count = 17;
     const double lpfcoeff[3][17]={
          { -7,-18,-15, 11, 56,116,177,223,240,223,177,116, 56, 11,-15,-18, -7},
          {  0,-18,-38,-39,  0, 83,191,284,320,284,191, 83,  0,-39,-38,-18,  0},
          {  6, 20,  7,-42,-74,-12,159,353,440,353,159,-12,-74,-42,  7, 20,  6}};

     const double *c = lpfcoeff[filter->bandwidth];
     const double *c_end=lpfcoeff[filter->bandwidth] + ((sizeof *lpfcoeff) / (sizeof **lpfcoeff));
     double* const b_begin=filter->buffer;
     double* const b_end=filter->buffer + buffer_count;
     double sum=0;

     double *current = filter->buffer + filter->current;
     // replace oldest value with current
     *current=sample;

     // convolution
     while(current!=b_end)
          sum+=(*current++)*(*c++);
     current=b_begin;
     while(c!=c_end)
          sum+=(*current++)*(*c++);

     // point again to oldest value
     if(--current<b_begin)
          current=b_end-1;

     filter->current = current - filter->buffer;
     return sum;
}

void FaxDecoder::DemodulateData(int n)
{
     double f=0;
     double ifirold = 0, qfirold = 0;
     int i;

     for(i=0; i<n; i++) {
          f += (double)m_carrier/m_SampleRate;

          wxInt16 samplei = m_SampleSize == 2 ? sample[i] : ((wxInt8*)sample)[i];

          double ifirout=apply_firfilter(firfilters+0, samplei*cos(2*M_PI*f));
          double qfirout=apply_firfilter(firfilters+1, samplei*sin(2*M_PI*f));

          if(m_bFM) {
               double mag=sqrt(qfirout*qfirout+ifirout*ifirout);
               ifirout/=mag;
               qfirout/=mag;
               if(mag>10000) {
                    double y=qfirold*ifirout-ifirold*qfirout;
                    double x=m_SampleRate/m_deviation*asin(y)/2.0/M_PI;
                    datadouble[i] = x; /* for demod display */
                    if(x<m_minus_saturation_threshold)
                        x = 1;
                    if(x<-1.0)
                         x=-1.0;
                    else if(x>1.0)
                         x=1.0;

                    data[i]=(int)((x/2.0+0.5)*255.0);
               } else
                    data[i]=255;

               ifirold=ifirout;
               qfirold=qfirout;
          } else {
               ifirout/=96000;
               qfirout/=96000;
               data[i]=(int)sqrt(ifirout*ifirout+qfirout*qfirout);
          }
     }
}

/* perform fourier transform at a specific frequency to look for start/stop */
double FaxDecoder::FourierTransformSub(wxUint8* buffer, int buffer_len, int freq)
{
    double k = -2 * M_PI * freq * 60.0 / m_lpm / buffer_len;
    double retr = 0, reti = 0;
    for(int n=0; n<buffer_len; n++) {
        retr += buffer[n]*cos(k*n);
        reti += buffer[n]*sin(k*n);
    }
    return sqrt(retr*retr + reti*reti);
}

/* see if the fourier transform at the start and stop frequencies reveils header */
FaxDecoder::Header FaxDecoder::DetectLineType(wxUint8* buffer, int buffer_len)
{
     const int threshold = 5; /* 5 is pretty arbitrary but works in practice even with lots of noise */
     if(FourierTransformSub(buffer, buffer_len, m_StartFrequency) / buffer_len > threshold)
         return START;
     if(FourierTransformSub(buffer, buffer_len, m_StopFrequency) / buffer_len > threshold)
         return STOP;
     return IMAGE;
}

static int minus_int(const void *a, const void *b)
{
    return *(const int*)a - *(const int*)b;
}

static int median(int *x, int n)
{
     qsort(x, n, sizeof *x, minus_int);
     return x[n/2];
}

/* detect start position from phasing line. 
   using 7% of the image (image should have 5% black 95% white)
   wide of a ^ shaped wedge, find positon it fits to the minimum.

   This isn't very fast, but only is used for phasing lines */
int FaxDecoder::FaxPhasingLinePosition(wxUint8 *image, int imagewidth)
{
    int n = imagewidth * .07;
    int i;
    int mintotal = -1, min = 0;
    for(i = 0; i<imagewidth; i++) {
        int total = 0, j;
        for(j = 0; j<n; j++)
            total += (n/2-abs(j-n/2))*(255-image[(i+j)%imagewidth]);
        if(total < mintotal || mintotal == -1) {
            mintotal = total;
            min = i;
        }
    }

    return (min+n/2) % imagewidth;
}

/* decode a single line of fax data from buffer placing it in image pointer
   buffer should contain m_SampleRate*60.0/m_lpm*colors bytes
   image will contain imagewidth*colors bytes
*/
void FaxDecoder::DecodeImageLine(wxUint8* buffer, int buffer_len, wxUint8 *image)
{
     int n = m_SampleRate*60.0/m_lpm;

     if(buffer_len != n*m_faxcolors)
         wxLogMessage(_("DecodeImageLine requires specific buffer length"));

     int i, c;
     for(i = 0; i<m_imagewidth; i++)
          for(c = 0; c<m_faxcolors; c++) {
               int firstsample = n*c+n*i/m_imagewidth;
               int lastsample = n*c+n*(i+1)/m_imagewidth - 1;

               int pixel = 0, pixelSamples = 0, sample = firstsample;
               do {
                    pixel+=buffer[sample];
                    pixelSamples++;
               } while(sample++ < lastsample);

               int shift = 8 - m_BitsPerPixel, maxcolor = (1<<m_BitsPerPixel) - 1;
               for(int j = c; j<m_imagecolors; j++)
                   image[i*m_imagecolors + j] = 255*((pixel/pixelSamples) >> shift) / maxcolor;
          }
}

void FaxDecoder::InitializeImage()
{
    height = size / 2 / m_SampleRate / 60.0 * m_lpm / m_faxcolors;
    imgpos = 0;

    if(height == 0) /* for unknown size, start out at 256 */
        height = 256;

    FreeImage();
    m_imgdata = (wxUint8*)malloc(m_imagewidth*height*3);

    m_imageline = 0;
    lasttype = IMAGE;
    typecount = 0;

    gotstart = false;
}

void FaxDecoder::FreeImage()
{
     free(m_imgdata);
     m_imageline = 0;
}

void FaxDecoder::CloseInput()
{
     switch(m_CaptureSettings.type) {
     case FaxDecoderCaptureSettings::AUDIO:
#ifdef OCPN_USE_PORTAUDIO
         Pa_CloseStream( pa_stream );
#endif
         if(dsp)
             close(dsp);
         break;
     case FaxDecoderCaptureSettings::FILE:
#ifdef USE_AUDIOFILE
         afCloseFile(aFile);
#endif
         break;
     case FaxDecoderCaptureSettings::RTLSDR:
#ifdef BUILTIN_RTLAIS
         
#endif         
         break;
     default:;
     }
//     m_inputtype = NONE;
}

void FaxDecoder::SetupBuffers()
{
    m_blocksize = m_SampleRate*60.0/m_lpm*m_faxcolors;
    
    sample = new wxInt16[m_blocksize];
    data = new wxUint8[m_blocksize];
    datadouble = new double[m_blocksize];

    phasingPos = new int[m_phasingLines];
    phasingLinesLeft = phasingSkipData = phasingSkippedData = 0;

    // we reset at start and stop, so if already offset phasing will follow
    if(m_CaptureSettings.offset)
        phasingLinesLeft = m_phasingLines;
}

void FaxDecoder::CleanUpBuffers()
{
     delete [] sample;
     delete [] data;
     delete [] datadouble;
     delete [] phasingPos;
}

bool FaxDecoder::DecodeFax()
{
    while(!m_bEndDecoding) {
        m_DecoderReloadMutex.Lock();
        int len;
        switch(m_CaptureSettings.type) {
        case FaxDecoderCaptureSettings::AUDIO:
            if(dsp) {
                int count;
                for(len = 0; len < m_SampleSize*m_blocksize; len += count)
                    if((count = read(dsp, (char*)sample + len, m_SampleSize*m_blocksize - len)) <= 0)
                        break;
                len /= 2;
            } else {
#ifdef OCPN_USE_PORTAUDIO
                PaError err = Pa_ReadStream( pa_stream, sample, m_blocksize);
                if(err == paInputOverflow)
                    wxLogMessage(_("Port audio overflow on input, some data lost!"));
            
#endif
            }
            len = m_blocksize;
            break;
        case FaxDecoderCaptureSettings::FILE:
#ifdef USE_AUDIOFILE
            if((len = afReadFrames(aFile, AF_DEFAULT_TRACK, sample, m_blocksize)) == m_blocksize)
                break;
#endif
#ifdef BUILTIN_RTLAIS
        case FaxDecoderCaptureSettings::RTLSDR:
#endif            
        default:
            m_DecoderReloadMutex.Unlock();
            goto done;
        }
        m_DecoderReloadMutex.Unlock();

        m_DecoderStopMutex.Lock();

        DemodulateData(len);
        
        enum Header type;
        if(m_bSkipHeaderDetection)
            type = IMAGE;
        else
            type = DetectLineType(data, len);

        /* accumulate how many start or stop lines we are getting */
        if(type == lasttype && type != IMAGE)
            typecount++;
        else {
            typecount--; /* can deal with noisy input if we had a miss rather than reset here */
            if(typecount < 0)
                typecount = 0;
        }
        lasttype = type;

        if(type != IMAGE) { /* if type is start or stop */
            /* require 4 less lines than there really are to handle
               noise and also misalignment on first and last lines */
            const int leewaylines = 4;

            if(typecount == m_StartLength*m_lpm/60.0 - leewaylines) {
                if(type == START/* && m_imageline < 100*/) {
                    /* prepare for phasing */
                    /* image start detected, reset image at 0 lines  */
                    if(!m_bIncludeHeadersInImages) {
                        m_imageline = 0;
                        imgpos = 0;
                    }

                    phasingLinesLeft = m_phasingLines;
                    gotstart = true;
                } else if(gotstart) // detect stop only if detected start
                {
                    // exit at stop
#ifdef USE_AUDIOFILE
                    if(m_CaptureSettings.type == FaxDecoderCaptureSettings::FILE)
                        m_stop_audio_offset = afTellFrame (aFile, AF_DEFAULT_TRACK);
                    else
#endif
                        m_stop_audio_offset = 1; // used as flag to indicate stop was reached
                    m_DecoderStopMutex.Unlock();
                    goto done;
                }
            }
        }

        /* throw away first 2 lines of phasing because we are not sure
           if they are misaligned start lines */
        const int phasingSkipLines = 2;
        if(phasingLinesLeft > 0 && phasingLinesLeft <= m_phasingLines - phasingSkipLines)
            phasingPos[phasingLinesLeft-1] = FaxPhasingLinePosition(data, len);

        if(type == IMAGE && phasingLinesLeft >= -phasingSkipLines)
            if(!--phasingLinesLeft) /* decrement each phasing line */
                phasingSkipData = median(phasingPos, m_phasingLines - phasingSkipLines);

        /* go past the phasing lines we skipping to make sure we are in the image */
        if(m_bIncludeHeadersInImages || (type == IMAGE && phasingLinesLeft < -phasingSkipLines)) {
            if(m_DecoderMutex.Lock() == wxMUTEX_NO_ERROR) {
                if(m_imageline >= height) {
                    height *= 2;
                    m_imgdata = (wxUint8*)realloc(m_imgdata, m_imagewidth*height*3);
                }
               
                DecodeImageLine(data, m_blocksize, m_imgdata+imgpos);
                
                int skiplen = ((phasingSkipData-phasingSkippedData)%len)*m_imagewidth/len;
                phasingSkippedData = phasingSkipData; /* reset skipped position */
                    
                imgpos += (m_imagewidth-skiplen)*m_imagecolors;
                m_imageline++;

                m_DecoderMutex.Unlock();
            }
        }

        m_DecoderStopMutex.Unlock();
    }
done:

#if 0
    // XXX FIXME Work in progress? leak
    m_DecoderStopMutex.Lock();
     /* put left overdata into an image */
     if((m_bIncludeHeadersInImages || gotstart) &&
        m_imageline > 10) { /* throw away really short images */
         int is = m_imagewidth*m_imageline*3;
         unsigned char *id = (unsigned char *)malloc(is); /* wximage needs malloc */
         memcpy(id, m_imgdata, is);

         /* fill rest of the line with zeros */
         memset(id+imgpos, 0, m_imagewidth*m_imageline*m_imagecolors - imgpos);
     }
     m_DecoderStopMutex.Unlock();
#endif
     CloseInput();

     return true;
}

bool FaxDecoder::DecodeFaxFromFilename()
{
#ifdef USE_AUDIOFILE
    size = 0;
    AFfilesetup fs=0;
    if((aFile=afOpenFile(m_CaptureSettings.filename.ToUTF8(),"r",fs))==AF_NULL_FILEHANDLE)
        return Error(_("could not open input file: ") + m_CaptureSettings.filename);
    
    m_SampleSize = afGetFrameSize(aFile,AF_DEFAULT_TRACK,0);
    if(m_SampleSize!=1 && m_SampleSize!=2)
        return Error(_("sample size not 8 or 16 bit: ") + wxString::Format(_T("%d"), m_SampleSize));
    
    m_SampleRate = afGetRate(aFile, AF_DEFAULT_TRACK);

    afSeekFrame (aFile, AF_DEFAULT_TRACK, m_CaptureSettings.offset);
//    size = afGetTrackBytes (aFile, AF_DEFAULT_TRACK);
//    if(size < 0 || size == 0x80000000)
        size = 0; // file is still being written to..

    return true;
#else
    return Error(_("no audiofile support compiled"));
#endif
}

bool FaxDecoder::DecodeFaxFromDSP()
{
#if defined(__linux__) && !defined(__ANDROID__)
     if((dsp=open("/dev/dsp",O_RDONLY))==-1)
         return false;

     m_SampleSize = 2;
     int format=AFMT_S16_LE;
     if(ioctl(dsp,SNDCTL_DSP_SETFMT,&format)==-1 || format!=AFMT_S16_LE)
         return false;

     int channels=1;
     if(ioctl(dsp,SNDCTL_DSP_CHANNELS,&channels)==-1 || channels!=1)
         return false;

     int speed=m_CaptureSettings.audio_samplerate;
     if(ioctl(dsp,SNDCTL_DSP_SPEED,&speed)==-1
        || speed<m_SampleRate*0.99 || speed>m_SampleRate*1.01)
         return false;

    size = 0;

    return true;
#else
    return false;
#endif
}

bool FaxDecoder::DecodeFaxFromPortAudio()
{
#ifdef OCPN_USE_PORTAUDIO
    PaError err = Pa_Initialize();
    if( err != paNoError ) {
        printf( "PortAudio Initialize() error: %s\n", Pa_GetErrorText( err ) );
        return false;
    }

    m_SampleRate = m_CaptureSettings.audio_samplerate;
    
    PaSampleFormat sampleformat;
    m_SampleSize = 2; /* for now hardcode */
    switch(m_SampleSize) {
    case 1: sampleformat = paInt8; break;
    case 2: sampleformat = paInt16; break;
    default:
        printf( "invalid sample size for capture: %d\n", m_SampleSize);
        return false;
    }

    if(m_CaptureSettings.audio_deviceindex == -1)
        m_CaptureSettings.audio_deviceindex = Pa_GetDefaultInputDevice();

    while(m_CaptureSettings.audio_deviceindex < Pa_GetDeviceCount()) {
        PaStreamParameters inputParameters;
        inputParameters.device = m_CaptureSettings.audio_deviceindex;
        inputParameters.channelCount = 1; /* mono input */
        inputParameters.sampleFormat = sampleformat;
        inputParameters.suggestedLatency = 0;
        inputParameters.hostApiSpecificStreamInfo = NULL;

        int blocksize = m_SampleRate*60.0/m_lpm*m_faxcolors;
        err = Pa_OpenStream( &pa_stream,
                             &inputParameters,
                             NULL, /* no output channels */
                             m_SampleRate,
                             blocksize,
                             paNoFlag,
                             NULL, NULL);

        if( err == paNoError ) {
            err = Pa_StartStream( pa_stream );
            if( err != paNoError ) {
                Pa_CloseStream( pa_stream );
                printf( "PortAudio StartStream() error: %s\n", Pa_GetErrorText( err ) );
                return false;
            }

            size = 0;

            return true;
        }

        m_CaptureSettings.audio_deviceindex++; // try next device
    }

    printf( "PortAudio OpenStream() error: %s\n", Pa_GetErrorText( err ) );
#endif
    return false;
}


bool FaxDecoder::DecodeFaxFromRTLSDR()
{
#ifdef BUILTIN_RTLAIS
    m_SampleRate = 8000;
#endif
    return false;
}
