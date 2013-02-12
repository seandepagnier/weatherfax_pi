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

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <wx/listimpl.cpp>
WX_DECLARE_LIST(wxImage, FaxImageList);

enum Bandwidth {NARROW, MIDDLE, WIDE};

struct firfilter {
    firfilter() {}
    firfilter(enum Bandwidth b) : bandwidth(b), current(0)
        { for(int i=0; i<17; i++) buffer[i] = 0; }
    enum Bandwidth bandwidth;
    double buffer[17];
    int current;
};

class FaxDecoder
{
public:

    FaxDecoder(wxWindow &parent, int imagewidth, int BitsPerPixel, int carrier,
               int deviation, enum Bandwidth bandwidth,
               bool bSkipHeaderDetection, bool bIncludeHeadersInImages)
        : m_parent(parent), m_imagewidth(imagewidth), m_BitsPerPixel(BitsPerPixel),
        m_carrier(carrier), m_deviation(deviation),
        m_bSkipHeaderDetection(bSkipHeaderDetection),
        m_bIncludeHeadersInImages(bIncludeHeadersInImages),
        m_imagecolors(3), m_faxcolors(1), m_lpm(120), m_bFM(true),
        m_StartFrequency(300), m_StopFrequency(450),
        m_StartLength(5), m_StopLength(5), m_phasingLines(40)
    {
        firfilters[0] = firfilter(bandwidth);
        firfilters[1] = firfilter(bandwidth);
    }

    bool DecodeFaxFromAudio(wxString filename);

    FaxImageList images;

private:
    enum Header {IMAGE, START, STOP};

    bool Error(wxString error);
    double FourierTransformSub(wxUint8* buffer, int buffer_len, int freq);
    Header DetectLineType(wxUint8* buffer, int buffer_len);
    void DemodulateData(wxUint8 *data, wxInt16 *sample, int n);
    void DecodeImageLine(wxUint8* buffer, int buffer_len, wxUint8 *image);
    int FaxPhasingLinePosition(wxUint8 *image, int imagewidth);

    wxWindow &m_parent;

    int sampleRate;
    int m_imagewidth;
    int m_BitsPerPixel;
    double m_carrier, m_deviation;
    struct firfilter firfilters[2];
    bool m_bSkipHeaderDetection;
    bool m_bIncludeHeadersInImages;
    int m_imagecolors, m_faxcolors;
    int m_lpm;
    bool m_bFM;
    int m_StartFrequency, m_StopFrequency;
    int m_StartLength, m_StopLength;
    int m_phasingLines;
};
