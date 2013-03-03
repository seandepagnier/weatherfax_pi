/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  weather fax Plugin
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2013 by Sean D'Epagnier                                 *
 *   sean at depagnier dot com                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

#ifdef __linux__
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#endif

#include <wx/progdlg.h>

#include <math.h>
#include <complex>

/* Note: the decoding algorithms are adapted from yahfax (on sourceforge)
   which was an improved adaptation of hamfax.
*/

//WX_DEFINE_LIST(FaxImageList);

bool FaxDecoder::Error(wxString error)
{
    wxMessageDialog w( &m_parent, _("Failure Decoding Fax: ") + error, _("Failed"),
                       wxOK | wxICON_ERROR );
    w.ShowModal();
    return false;
}

double apply_firfilter(struct firfilter *filter, double sample)
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

void FaxDecoder::DemodulateData(wxUint8 *data, wxInt16 *sample, int n)
{
     double f=0;
     double ifirold = 0, qfirold = 0;
     int i;

     for(i=0; i<n; i++) {
          f += (double)m_carrier/sampleRate;

          double ifirout=apply_firfilter(firfilters+0, sample[i]*cos(2*M_PI*f));
          double qfirout=apply_firfilter(firfilters+1, sample[i]*sin(2*M_PI*f));

          if(m_bFM) {
               double mag=sqrt(qfirout*qfirout+ifirout*ifirout);
               ifirout/=mag;
               qfirout/=mag;
               if(mag>10000) {
                    double y=qfirold*ifirout-ifirold*qfirout;
                    double x=sampleRate/m_deviation*asin(y)/2.0/M_PI;
                    datadouble[i] = x; /* for demod display */
                    if(x<minus_saturation_threshold)
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

/* perform fourier transform at a specific frequency */
double FaxDecoder::FourierTransformSub(wxUint8* buffer, int buffer_len, int freq)
{
    std::complex<double> k = freq * 60.0 / m_lpm, buffer_lenc = buffer_len;
    std::complex<double> ret = 0;
    std::complex<double> im(0, 1);
    int n;
    for(n=0; n<buffer_len; n++) {
        std::complex<double> nc = n;
        ret += (std::complex<double>)buffer[n] * exp(-(2.0*M_PI*im*k*nc) / buffer_lenc);
    }
    return abs(ret);
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
   buffer should contain sampleRate*60.0/m_lpm*colors bytes
   image will contain imagewidth*colors bytes
*/
void FaxDecoder::DecodeImageLine(wxUint8* buffer, int buffer_len, wxUint8 *image)
{
     int n = sampleRate*60.0/m_lpm;

     if(buffer_len != n*m_faxcolors)
         Error(_("fax_decode_image_line requires specific buffer length"));

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

               for(int j = c; j<m_imagecolors; j++)
                   image[i*m_imagecolors + j] = (pixel/pixelSamples)>>(8-m_BitsPerPixel)<<(8-m_BitsPerPixel);
          }
}

void FaxDecoder::SetupToDecode()
{
    blocksize = sampleRate*60.0/m_lpm*m_faxcolors;
    
    sample = new wxInt16[blocksize];
    data = new wxUint8[blocksize];
    datadouble = new double[blocksize];
    
    height = size / 2 / sampleRate / 60.0 * m_lpm / m_faxcolors;
    imgpos = 0;

    if(height == 0) /* for unknown size, start out at 256 */
        height = 256;

    imgdata = (wxUint8*)malloc(m_imagewidth*height*3);

    line = 0;
    imageline = 0;
    lasttype = IMAGE;
    typecount = 0;

    gotstart = false;

    phasingPos = new int[m_phasingLines];
    phasingLinesLeft = phasingSkipData = phasingSkippedData = 0;
}

void FaxDecoder::CleanUp()
{
     if(m_DecoderMutex.Lock() == wxMUTEX_NO_ERROR) {
         free(imgdata);
         imageline = 0;
         m_DecoderMutex.Unlock();
     }
     line = 0;

     delete [] sample;
     delete [] data;
     delete [] datadouble;

     switch(inputtype) {
     case DSP:
         close(dsp);
         break;
     case FILENAME:
         afCloseFile(aFile);
         break;
     }
}

bool FaxDecoder::DecodeFax()
{
    for(;;) {
        int len;
        switch(inputtype) {
        case DSP:
        {
            int count;
            for(len = 0; len < 2*blocksize; len += count)
                if((count = read(dsp, (char*)sample + len, 2*blocksize - len)) <= 0)
                    break;
            len /= 2;
        } break;
        case FILENAME:
        {
            if((len = afReadFrames(aFile, AF_DEFAULT_TRACK, sample, blocksize)) != blocksize)
                /* end of file, end decoding */
                goto done;
        } break;
        }

        DemodulateData(data, sample, len);

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

            if(type == START && typecount == m_StartLength*m_lpm/60.0 - leewaylines) {
                /* image start detected, prepare for phasing */
                phasingLinesLeft = m_phasingLines;
                gotstart = true;
            } else
                if(type == STOP && typecount == m_StopLength*m_lpm/60.0 - leewaylines) {
#if 0
                    int is = m_imagewidth*imageline*3;
                    unsigned char *id = (unsigned char*)malloc(is); /* wximage needs malloc */
                    memcpy(id, imgdata, is);
                    images.Append(new wxImage(m_imagewidth, imageline, id));
                    imageline = 0;
                    imgpos = 0;
#endif
                    typecount = 0;
                    gotstart = false;
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
                if(imageline >= height) {
                    height *= 2;
                    imgdata = (wxUint8*)realloc(imgdata, m_imagewidth*height*3);
                }

                DecodeImageLine(data, len, imgdata+imgpos);
                
                int skiplen = ((phasingSkipData-phasingSkippedData)%len)*m_imagewidth/len;
                phasingSkippedData = phasingSkipData; /* reset skipped position */

                imgpos += (m_imagewidth-skiplen)*m_imagecolors;
                imageline++;
                m_DecoderMutex.Unlock();
            }
        }

        if(m_bEndDecoding)
            break;

        line++;
     }
done:

     delete [] phasingPos;

     /* put left overdata into an image */
     if((m_bIncludeHeadersInImages || gotstart) &&
        imageline > 10 /* throw away really short images */) {
         int is = m_imagewidth*imageline*3;
         unsigned char *id = (unsigned char *)malloc(is); /* wximage needs malloc */
         memcpy(id, imgdata, is);

         /* fill rest of the line with zeros */
         memset(id+imgpos, 0, m_imagewidth*imageline*m_imagecolors - imgpos);

//         images.Append(new wxImage(m_imagewidth, imageline, id));
     }


     CleanUp();

     return true;
}


bool FaxDecoder::DecodeFaxFromFilename(wxString fileName)
{
    size = 0;
    AFfilesetup fs=0;
    if((aFile=afOpenFile(fileName.ToUTF8(),"r",fs))==AF_NULL_FILEHANDLE)
        return Error(_("could not open input file: ") + fileName);
    
    if(afGetFrameSize(aFile,AF_DEFAULT_TRACK,0)!=2)
        return Error(_("samples size not 16 bit"));
    
    sampleRate = afGetRate(aFile,AF_DEFAULT_TRACK);
    
    size = afGetTrackBytes (aFile, AF_DEFAULT_TRACK);
    if(size < 0)
        // file is still being written to..
        return Error(_("cannot deal with incomplete input file"));

    inputtype = FILENAME;
    SetupToDecode();
    return true;
}

bool FaxDecoder::DecodeFaxFromDSP()
{
#ifdef __linux__
     if((dsp=open("/dev/dsp",O_RDONLY))==-1)
         return false;
		
     int format=AFMT_S16_LE;
     if(ioctl(dsp,SNDCTL_DSP_SETFMT,&format)==-1 || format!=AFMT_S16_LE)
         return false;

     int channels=1;
     if(ioctl(dsp,SNDCTL_DSP_CHANNELS,&channels)==-1 || channels!=1)
         return false;

     int speed=sampleRate;
     if(ioctl(dsp,SNDCTL_DSP_SPEED,&sampleRate)==-1
        || speed<sampleRate*0.99 || speed>sampleRate*1.01)
         return false;

    inputtype = DSP;
    size = 0;
    SetupToDecode();
    return true;
#else
    return false;
#endif
}
