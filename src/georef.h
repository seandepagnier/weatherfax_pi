/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Georef utility
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************

 ***************************************************************************
 *  Parts of this file were adapted from source code found in              *
 *  John F. Waers (jfwaers@csn.net) public domain program MacGPS45         *
 ***************************************************************************

 */

#ifndef PI
      #define PI        3.1415926535897931160E0      /* pi */
#endif

static void WFDistanceBearingMercator(double lat0, double lon0, double lat1, double lon1, double *brg, double *dist)
{
      //    Calculate bearing by conversion to SM (Mercator) coordinates, then simple trigonometry

      double lon0x = lon0;
      double lon1x = lon1;

      //    Make lon points the same phase
      if((lon0x * lon1x) < 0.)
      {
          lon0x < 0.0 ? lon0x += 360.0 : lon1x += 360.0;
            //    Choose the shortest distance
            if(fabs(lon0x - lon1x) > 180.)
            {
                lon0x > lon1x ? lon0x -= 360.0 : lon1x -= 360.0;
            }

            //    Make always positive
            lon1x += 360.;
            lon0x += 360.;
      }

      //    Classic formula, which fails for due east/west courses....
      if(dist)
      {
          //    In the case of exactly east or west courses
          //    we must make an adjustment if we want true Mercator distances

          //    This idea comes from Thomas(Cagney)
          //    We simply require the dlat to be (slightly) non-zero, and carry on.
          //    MAS022210 for HamishB from 1e-4 && .001 to 1e-9 for better precision
          //    on small latitude diffs
          const double mlat0 = fabs(lat1 - lat0) < 1e-9 ? lat0 + 1e-9 : lat0;

          double east, north;
          toSM_ECC(lat1, lon1x, mlat0, lon0x, &east, &north);
          const double C = atan2(east, north);
          if(cos(C))
          {
              const double dlat = (lat1 - mlat0) * 60.;              // in minutes
              *dist = (dlat /cos(C));
          }
          else
          {
              const double mlon0 = fabs(lon1 - lon0) < 1e-9 ? lon0 + 1e-9 : lon0;
              const double dlon = (lon1 - mlon0) * 60.;              // in minutes
              *dist = (dlon /sin(C));
          }
      }

      //    Calculate the bearing using the un-adjusted original latitudes and Mercator Sailing
      if(brg)
      {
          double east, north;
          toSM_ECC(lat1, lon1x, lat0, lon0x, &east, &north);

          const double C = atan2(east, north);
          const double brgt = 180. + (C * 180. / PI);
          if (brgt < 0)
              *brg = brgt + 360.;
          else if (brgt > 360.)
              *brg = brgt - 360.;
          else
              *brg = brgt;
      }
}
