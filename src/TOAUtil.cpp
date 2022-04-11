#include <iostream>
#include <sys/stat.h>
#include <fstream>
#include <map>
#include <cassert>
#include <unistd.h>
#include <string>
#include <math.h>
#include "gdal_priv.h"
#include "cpl_conv.h"
#include "pugixml.hpp"
#include "Misc.h"
#include "TOAUtil.h"
#include <algorithm>
using namespace std;
using namespace pugi;
typedef std::string String;

/* *****************************
 * function SetCalibration( 
 *   const char* imd_filename,
 *   const char* xml_filename, 
 *   const char* ntf_filename )
 */
std::map<String,String> SetCalibrationAndBandWidth( const char* imd_filename, 
  const char* xml_filename, 
  const char* ntf_filename )
{
  // first determine the number of bands in the image using GDAL
  GDALAllRegister();
  String ErrorMsg = "";
  GDALDataset *imageDataset;
  imageDataset    = (GDALDataset*) GDALOpen( ntf_filename,GA_ReadOnly );
  GDALClose(imageDataset);
  GDALDestroyDriverManager();

  // initialize map to hold band metadata (calibration and bandwidth factors)
  std::map<String,String> CalibrationAndBandwidths;

  // now based on number of bands, open XML and parse metadata
  // first try to open and create file object. if failure, then
  // send an exit message to the terminal.
  pugi::xml_document xmldoc;
  pugi::xml_parse_result xml_parse_result = xmldoc.load_file(xml_filename);

  // exit if failure to read the XML file
  if(!xml_parse_result)
  {
    ErrorMsg = "  ERROR (fatal): unable to read XML file: "+(String)xml_filename;
    print_error_msg_and_exit( ErrorMsg.c_str() );
  }

  // get the satellite ID
  const char* SatelliteID = xmldoc.child("isd").child("IMD").child("IMAGE").child("SATID").child_value();
  CalibrationAndBandwidths[(String)"SatelliteID"] = (String)SatelliteID;

  /* Do something like the following:
   *   xmldoc.child("isd").child("IMD").child("BAND_C").child("ULLON").child_value() << endl;
   * to parse out values from the bands.
   */
  xml_node IMD = xmldoc.child("isd").child("IMD");
  int BandNumber = 0;

  for( pugi::xml_node xml_child: IMD.children()) {
    
    const char* xml_child_name = xml_child.name();
    bool isBand = true;
    isBand = ((String)xml_child_name).find("BAND")!=std::string::npos;
    if(!isBand){continue;}

    for( pugi::xml_node xml_subchild: xml_child.children() ) {
 
      // set variables for keys and values to store into a hash or map
      const char* xml_subchild_name  = xml_subchild.name();
      const char* xml_subchild_value = IMD.child( 
        xml_child_name ).child( xml_subchild_name ).child_value(); 
      String Key = std::to_string(BandNumber)+"__"+(String)xml_child_name+"__"+(String)xml_subchild_name;
      String Val = (String)xml_subchild_value;
      CalibrationAndBandwidths[Key]=Val;
    }
    BandNumber++;
  }
  return CalibrationAndBandwidths;
}

/* ***************************************************************
 * function SolarZenithAngle( const char* MetadataLine)
 * This function takes in a string (pointer to string that is)
 * corresponding to the line in the .IMD metadata file containing
 * the solar zenith information. This function parses out the 
 * data from the line and calculates the solar zenith angle.
 *
 * Returns the angle in degrees. Not radians.
 */
double SolarZenithAngle( const char* MetadataLine ){
  double solarZenithAngle = NODATA;
  String MeanSunElevStr = String(MetadataLine);
  MeanSunElevStr = trim(MeanSunElevStr.substr( 
    MeanSunElevStr.find("=")+1,MeanSunElevStr.length()-1 ));
  double solarElevationAngle = stod(MeanSunElevStr);
  solarZenithAngle = 90.0 - solarElevationAngle;  
  return solarZenithAngle; 
}

/* ****************************************************************
 * function EarthSunDistance( const char* ):
 * Function to compute the Earth Sun Distance in Astronomical
 * Unit (AU). To this end, this function uses the metadata 
 * prseent in the IMD file (.IMD) that came with the NITF file.
 * This function will parse that file, and find the year, month,
 * day, hour, and minutes/seconds of the scene acquisition. This
 * function determines the solar zenith angle based on the
 * metadata in the .IMD file.
 * 
 * ****************************************************************
 */
void EarthSunDistance( const char* imd_filename, SolarMetadata* Metadata ) {
  
  // first make sure that input IMD file pointed by imd_filename exists.
  String ErrorMsg = "";
  if(!file_exists(imd_filename)) {
    ErrorMsg = "  ERROR (fatal): file does not exist: "+(String)imd_filename;
    print_error_msg_and_exit( ErrorMsg.c_str() );
  }

  // open up the file for parsing.
  // iterate through the lines in the file
  std::ifstream imd_stream( imd_filename );
  String line;
  String firstTimeLine   = "";
  String solarZenithLine = "";
  String searchStr("firstLineTime");
  String searchStrZenithAngle("meanSunEl");

  while( std::getline(imd_stream,line)) {
    if( line.find(searchStr) != String::npos ){
      firstTimeLine   = line;
    }
    if( line.find(searchStrZenithAngle) != String::npos ) {
      solarZenithLine = line; 
    }
  }

  // close out the file-stream
  // *************************
  imd_stream.close();

  // check to make sure IMD file had firstLineTime entry,
  // if not, error and exit.
  if( firstTimeLine.length()<1 )
  {
    ErrorMsg = "  ERROR (fatal): IMD file does not have first line time: "+(String)imd_filename;
    print_error_msg_and_exit( ErrorMsg.c_str() );
  }

  // check to make sure solar zenith line was found...
  if( solarZenithLine.length()<1 )
  {
    ErrorMsg = "  ERROR (fatal): IMD file does not have solar zenith line: "+(String)imd_filename;
    print_error_msg_and_exit( ErrorMsg.c_str() );
  }

  // create some C++ datetime object to extract the
  // the year,month,day,hour,minutes, and seconds
  //   e.g. use this string:
  //     2021-10-27 11:41:57.133850
  // to parse out the dates.
  String DateTimeStr = trim(firstTimeLine.substr( 
    firstTimeLine.find("=")+1, firstTimeLine.length()-1 ));
  std::replace( DateTimeStr.begin(),DateTimeStr.end(),'T',' ' ); 
  std::replace( DateTimeStr.begin(),DateTimeStr.end(),'Z',' ' ); 
  std::replace( DateTimeStr.begin(),DateTimeStr.end(),';',' ' ); 

  size_t year  = std::stoi(DateTimeStr.substr(0,4));
  size_t month = std::stoi(DateTimeStr.substr(5,6)); 
  size_t day   = std::stoi(DateTimeStr.substr(8,9));
  size_t hour  = std::stoi(DateTimeStr.substr(11,12));
  double mmss  = stod(DateTimeStr.substr(17,DateTimeStr.length()-1 ));  

  // ---- if month is Jan/Feb ... make adjustment per documentation
  // from digital globe:
  if( (month == 1 )||(month == 2 )) {
    month = month+12;
    year  = year-1;
  }
  
  /* compute Earth-sun distance
   */
  double UTC = (double)hour+((double)(month/60.0))+(mmss/3600.0);
  size_t A = (int)(year/100.0);
  size_t B = 2 - A + ((int)(A/4.0));
  int JulianDay = (int)365.25*(year+4716) + 
    (int)(30.6001*(month+1))+day+(UTC/24.0)+(double)B-1524.5;
  double D = JulianDay - 2451545.0;
  double g = 357.529   + 0.98560028*D;
  double earthSunDist = 1.00014 - 0.01671*cos(g) - 0.00014*cos(2*g);

  // define structure whose pointer to return...
  Metadata->earthSunDistance = earthSunDist;
  Metadata->solarZenithAngle = (double)0.0;

  // now parse out and compute the solar zenith angle
  double solarZenithAngle = SolarZenithAngle( solarZenithLine.c_str() ); 
  Metadata->solarZenithAngle = solarZenithAngle;
}
