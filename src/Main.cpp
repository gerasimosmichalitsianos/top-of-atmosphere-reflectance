#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <map>
#include <sstream>
#include <getopt.h>
#include <stdio.h>
#include <ctime>
#include "Misc.h"
#include "TOAUtil.h"
#include "ImageUtil.h"
using namespace std; 

/* ***********************************************
 * function void show_version():
 * This function prints the version information
 * for this C++ program.
 * ***********************************************
 */
void show_version(){
  cout << " **************************************************\n\n";
  cout << "  Convert-TOA                                      \n\n";
  cout << "  A C++ program to convert high-resolution imagery   \n";
  cout << "  to top-of-atmosphere (TOA) reflectance.          \n\n";
  cout << "                                                     \n";
  cout << "    Version 1.0.0, 23 January 2022                   \n";
  cout << "    Gerasimos 'Geri' Michalitsianos                  \n";
  cout << "    Arnold, Maryland                                 \n";
  cout << "    gerasimosmichalitsianos@gmail.com              \n\n";
  cout << " **************************************************  \n";
  cout << endl;
}

/* ***********************************************************
 * function void usage():
 * Function to print the usage message for this command-line
 * program to the terminal.
 * ***********************************************************
 */
void usage() {
  cout << "\n";
  cout << " ***********************************************************************************   \n";
  cout << "                                                                                       \n";
  cout << "   NAME:                                                                               \n";
  cout << "     Top of Atmosphere (TOA) Reflectance Code                                          \n";
  cout << "   DESCRIPTION                                                                         \n";
  cout << "     This program takes in a high resolution image file (NITF/NTF or Geotiff/TIF)      \n";
  cout << "     and converts its values from Digital Numbers (DNs) to Top of Atmosphere (TOA)     \n";
  cout << "     reflectance. This program also writes out a separate Geotiff containing           \n";
  cout << "     containing top-of-atmosphere radiances.                                           \n";
  cout << "   USAGE:                                                                              \n";
  cout << "     This program should be used in a Linux and/or UNIX-like command-line              \n";
  cout << "     environment (Mac OSX).                                                            \n";
  cout << "                                                                                       \n";
  cout << "       $ make                                                                          \n";
  cout << "       $ bin/toa -f {filename ntf|tif}                                                 \n";
  cout << "         -i {filename IMD}                                                             \n";
  cout << "         -x {filename xml}                                                             \n";
  cout << "                                                                                       \n";
  cout << "   EXAMPLE USAGE:                                                                      \n";
  cout << "                                                                                       \n";
  cout << "     $ make                                                                            \n";
  cout << "     $ ./bin/toa -f DATA/21OCT27114157-M1BS-014586809010_01_P013.NTF                   \n";
  cout << "                 -i DATA/21OCT27114157-M1BS-014586809010_01_P013.IMD                   \n";
  cout << "                 -x DATA/21OCT27114157-M1BS-014586809010_01_P013.XML                   \n";
  cout << "                                                                                       \n";
  cout << "   SATELLITES SUPPORTED                                                                \n";
  cout << "                                                                                       \n";
  cout << "     This software supports high-resolution imagery from the  following                \n";
  cout << "     satellites: QuickBird 2, WorldView 3, WorldView 2, and GeoEye 1 (OV05).           \n";
  cout << "                                                                                       \n";
  cout << "   LAST UPDATED:                                                                       \n";
  cout << "     Gerasimos Michalitsianos                                                          \n";
  cout << "     Arnold, Maryland                                                                  \n";
  cout << "     23 January 2022                                                                   \n";
  cout << "     gerasimosmichalitsianos@gmail.com                                                 \n";
  cout << "                                                                                       \n";
  cout << " ***********************************************************************************   \n";
  cout << "\n";
  exit(1);
}

int main( int argc , char* argv[] ) {

  /* initialize counter for getopt for obtaining
   * command-line arguments.
   */
  cout << "beginning program to convert file to top-of-atmosphere reflectance\n";
  print_datetime();
  int opt = 0;
  const char* img_filename = nullptr;
  const char* xml_filename = nullptr;
  const char* imd_filename = nullptr;

  /* check to make sure script has correct number of
   * input arguments */
  if( argc<4 )
  {
    usage();
  }

  /* iterate through command-line args. */
  while((opt=getopt(argc,argv,":f:i:x:h"))!=-1) {
    switch(opt){
      case 'f':
        img_filename = optarg;
	break;
      case 'x':
	xml_filename = optarg;
	break;
      case 'i':
	imd_filename = optarg;
      default:
        ; 
    }
  }

 /* make sure user passed in 3 args:
  *   (1) NITF/NTF filename
  *   (2) XML filename
  *   (3) IMD filename
  * *********************************
  */
  if(!strlen(img_filename))        {
    cout << "    Please pass in name of image file with -f flag (TIF/NITF/NTF).\n";
    usage();  
  } else if(!strlen(xml_filename)) {
    cout << "    Please pass in name of XML with -x flag (.XML or .xml).       \n";
    usage();  
  } else if(!strlen(imd_filename)) {
    cout << "    Please pass in name of IMD file with -i flag (.IMD).          \n";
    usage();  
  } 
  else 
  {
    /* print name of file to the console */
    cout << "  name of input image file (TIF or NTF): " << endl;
    cout << "    " << img_filename << "\n";
    cout << "  name of input IMD file (.IMD): " << endl;
    cout << "    " << imd_filename << "\n";
    cout << "  name of input XML file (.XML): " << endl;
    cout << "    " << xml_filename << "\n";
  };

  /* make sure the input image file does indeed exist
   * on the local file-system. Should be a Geotiff or a
   * NITF/NTF file.
   */ 
  if(!file_exists(img_filename)){
    cout << "  ERROR (fatal): file does not exist:\n";
    cout << "    " << img_filename << "\n";
    cout << "  exiting at ...\n";
    print_datetime();
    exit(1); 
  }

  /* make sure the IMD file exists on the file-system
   */
  if(!file_exists(imd_filename)){
    cout << "  ERROR (fatal): file does not exist:\n";
    cout << "    " << imd_filename << "\n";
    cout << "  exiting at ...\n";
    print_datetime();
    exit(1); 
  }

  /* make sure the XML file exists
   */
  if(!file_exists(xml_filename)){
    cout << "  ERROR (fatal): file does not exist:\n";
    cout << "    " << xml_filename << "\n";
    cout << "  exiting at ...\n";
    print_datetime();
    exit(1); 
  }

  /* initialize C++ structure to hold all the
   * metadata from the XML and IMD files for
   * the NGA scene. 
   */
  SolarMetadata Metadata;
  Metadata.earthSunDistance = (double)0.0;
  Metadata.solarZenithAngle = (double)0.0;
	  
  /* now pass a POINTER to the structure so the 
   * Earth-sun distance (in AU) is parsed and calculated, along with
   * the solar zenith angle.
   */
  EarthSunDistance( imd_filename,&Metadata );
  
  /* read and set effective and absolute calibration factors */
  std::map<String,String> CalibrationAndBandWidths = SetCalibrationAndBandWidth( 
    imd_filename, xml_filename, img_filename );
  
  /* create ImageUtil object for purpose of calculating/writing TOA radiances/reflectances*/
  ImageUtil Image(img_filename);
  Image.WriteRadianceAndReflectanceGeotiffs( &Metadata,CalibrationAndBandWidths );  
  return 0;
}
