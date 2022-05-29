WeatherFax Update Notes:

Operationsal Notes
=======================
To start out with a clean slate, no previous weatherfax xml files.
1. Remove the %localappdata%/opencpn/plugins/weatherfax/data files   Use Options > Plugins > Uninstall the plugin.
2. Remove the local user directory data, in particular coordinateSets.xml   Manually remove this directory  "C:\ProgramData\opencpn\plugins\weatherfax" or emtpy it.
3. Not necessary, but you can manually remove all weatherfax_pi tarballs from "C:\ProgramData\opencpn\plugins\cache\tarballs"
For more information about the location of installation files see the Development Manual - Installation Paths https://opencpn-manuals.github.io/main/plugin-installer/Installation-paths.html
4. Use search "CoordinateSets.xml" in your files to determine where the Opencpn local user (writable) files are stored if you don't know already.

When weatherfax_pi tarballs are installed, updated  %localappdata%/opencpn/plugins/weatherfax/data files are installed.
After internet weatherfax files are retrieved and when weatherfax or opencpn is closed, the coordinateSets.xml in memory is saved to the User writable directory "C:\ProgramData\opencpn\plugins\weatherfax" with any Image Wizard coordinate changes made by the User. This coordinateSets.xml file is used instead of the installation coordinateSets.xml file at %localappdata%/opencpn/plugins/weatherfax/data.

Maintenance of XML Files
=======================

The weatherfaxinternetretrieval.xml and weatherfaxschedules.xml files must be maintained and updated by all the USERS!
Contribute your special url's and schedules for the benefit of all.

weatherfaxinternetretrieval.xml is dependent on many MET website services worldwide, and users who contribute xml files.
weatherfaxschedules.xml is created from NOAA's current Worldwide RF Radio Schedule 
https://www.weather.gov/nwr&in_desc=NOAA+Weather+Radio/
https://www.nhc.noaa.gov/radiofax.shtml

Users are asked to submit error reports and updated xml files through github.com/rgleason/weatherfax_pi   using Issues and attaching files or make a pull request.

WeatherFaxInternetRetrieval Files (12 Regional Files)
=======================
WeatherFaxInternetRetrieval_PWx_Pacific.xml    Passageweather Pacific
WeatherFaxInternetRetrieval_PWx_India.xml      Passageweather India
WeatherFaxInternetRetrieval_PWx_Euro_Atl.xml   Passageweather European
WeatherFaxInternetRetrieval_PWx_Amer_Atl.xml   Passageweather American Atlantic
WeatherFaxInternetRetrieval_NOAA_OPC.xml       NOAA Ocean Prediction Center
WeatherFaxInternetRetrieval_NOAA.xml           NOAA
WeatherFaxInternetRetrieval_Navy.xml           Navy Gulf Stream
WeatherFaxInternetRetrieval_Misc.xml           Bermuda, French Polynesia, Chile, South Africa
WeatherFaxInternetRetrieval_LaMMA.xml          Ligurian, Mediterranean, Adriatic, Ionian, Sardinia, Ionian, etc.
WeatherFaxInternetRetrieval_Europe.xml         German Weather DWD, ECMWF, GFS, UK MET, Arpege, Arome, all of Europe
WeatherFaxInternetRetrieval_Aviation.xml       Sat Images, Infra-red
WeatherFaxInternetRetrieval_Australia.xml      Australia BOM


Use of STRDATE 
====================
Date and Time Manipulation using DATE and STRDATE Functions

https://www.ibm.com/support/pages/date-and-time-manipulation-using-date-and-strdate-functions
https://www.ibm.com/support/pages/date-and-time-manipulation-using-date-and-strdate-functions

Download the IBM White Paper  June 26 2009

https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/strdate-wstrdate?view=msvc-170

Format Specifier 	Description 
%8 	ISO-8601 date format. Valid format is YYYYMMDD'T'HHMMSS'.'sssZ 
Note This date format cannot be combined with any other format specifier. 
%a 	Abbreviated weekday name. 
%A 	Full weekday name. 
%b 	Abbreviated month name. 
%B 	Full month name. 
%d 	Day of the month as a decimal number (01 - 31). 
%H 	Hour in 24-hour format (00 - 23). 
%I 	Hour in 12-hour format (01 - 12). 
%j 	Day of the year as a decimal number (001 - 366). 
%m 	Month as a decimal number (01 - 12). 
%M 	Minute as a decimal number (00 - 59). 
%S 	Second as a decimal number (00 - 59) 
%U 	Week of the year as a decimal number, with Sunday as the first day of the week (00 - 51). 
%w 	Weekday as a decimal number (0 - 6, with Sunday as "0"). 
%W 	Week of the year as a decimal number, with Monday as the first day of the week (00 - 51). 
%y 	Year without the century as a decimal number (00 - 99). 
%Y 	Year with the century as a decimal number. 
%% 	Percent sign. 


Update Notes
=====================
May 5/17/2022
Basically everything in this file have been reviewed and corrected or discarded.:

    Mauricius: Discarded
    Singapore: Discarded but awaiting for a reply from them hopefully
    Japan: Discarded
    South Africa: Updated
    French Polynesia: Updated
    Chile: Remapped (one Mercator changed to Fixed-Flat and one Conic to Polar)
    PWx, GFS, ECMWF and UKMO for a better coverage
    Europe: Updated


1. Mauritius – Files exist confirmed via browser on the web, accessible vie the homepage within browser.  Directly URL access not possilbe….Protection of some sort?

2. Singapore – This needs work. Several charts, wave height and swell, no longer work. Format must has changed. The surface wind charts work.

3. Japan - Unable to calibrate the charts, format is not polar or anything similar. Also VERY small, numbers hard to read,  left at the best compromise

4. South Africa - Internals of the website must have changed, not able to get any access to the charts.

5. French Polynesia – same thing as in SA.

6. Chile – Polar file is hard to calibrate almost impossible to get a perfect mapping.

7. We need better coverage for Asia, Malysia and Japan.

8. We have lost some sources for Europe and could use more coverage.

======================
Some potential URL's

India  India Meteorological Deptartment  
   Graphical Coastal Guidance  https://mausam.imd.gov.in/imd_latest/contents/coastal_forecast.php
   Tropical Cyclones  https://mausam.imd.gov.in/imd_latest/contents/cyclone.php
   Interactive Track of Cyclones https://ddgmui.imd.gov.in/dwr_img/GIS/cyclone.html
   Advances of Moonsoon  https://www.facebook.com/photo/?fbid=312951504343836&set=pb.100068869665517.-2207520000..
   Sat  https://rsmcnewdelhi.imd.gov.in/
   https://www.imdpune.gov.in/
   
   Singapore
http://www.weather.gov.sg/weather-marine-wind-waves
http://www.weather.gov.sg/learn_weather_systems/#forecast8
http://satellite.imd.gov.in/insat.htm

======================


