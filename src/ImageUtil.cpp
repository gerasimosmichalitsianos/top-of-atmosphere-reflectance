#include "ImageUtil.h"

// overloaded constructors
ImageUtil::ImageUtil() {};

ImageUtil::ImageUtil( const char* ImageFileName ) {
  /* *******************************************************************
   * this is a constructor method for this class. To use this, pass in
   * the filename (e.g. NTF or TIF) represented by a pointer to a 
   * character array (e.g. const char*). This function allocates a
   * block of memory to copy this filename string into a new
   * character array. It also sets the dimensions of the image.
   * It also opens the dataset as a GDAL dataset; that is, a pointer
   * to a GDAL dataset object.
   */

  /* check to make sure the file is not corrupt and
   * we are able to open it.
   */	
  int filename_len;
  ifstream ImageFile;
  ImageFile.open( ImageFileName );
  if(!ImageFile) {
    String ErrorMessage = (String)"ERROR (fatal): Could not open: "+
      (String)ImageFileName+". Exiting ...\n";
    throw std::runtime_error(ErrorMessage); 
  }

  /* use C++ dynamic memory allocation to to make copy of
   * filename.
   */
  filename_len = strlen( ImageFileName );
  filename     = new char[filename_len+1]; 
  if(!filename){
    String ErrorMessage = (String)"ERROR (fatal): could not allocate memory for filename.\n";
    throw std::runtime_error(ErrorMessage); 
  }

  // copy filename's contents into new char* array
  strcpy((char*)filename,ImageFileName);
  
  // call GDAL file registers
  GDALAllRegister();

  // Open the image file as a GDAL dataset.
  ImageDataset = (GDALDataset*) GDALOpen(filename,GA_ReadOnly );

  // set private variables N_rows,N_cols,N_bands
  N_rows  = GDALGetRasterYSize( ImageDataset ); 
  N_cols  = GDALGetRasterXSize( ImageDataset );
  N_bands = GDALGetRasterCount( ImageDataset );
}

ImageUtil::~ImageUtil() {
  /* *************************************************
   * This is a destructor method for this class.
   * It closes the image dataset stored in the GDAL
   * dataset, releases the memory for the filename
   * character array created in the constructor,
   * and releases the GDAL drivers. 
   */
  GDALClose(ImageDataset);
  GDALDestroyDriverManager();
  delete [] filename;
}

const char* ImageUtil::GetProjection() {
  /* **********************************************************
   * function const char* GetProjection():
   * This function returns a pointer to the first character
   * in an array of characters that containst the projection
   * string (WKT) for the Image file. Note the "->" notation
   * is used. This is because the 'ImageDataset' class
   * variable is a pointer to an object or structure, and not
   * the object itself, so we don't use the "dot" '.' notation.
   */
  return ImageDataset->GetProjectionRef();
}

double* ImageUtil::GetGeoTransform() {
  /* *****************************************************
   * function double *GetGeoTransform() 
   *  This function returns a pointer to a double that 
   *  is the first element of a 6 element array that holds
   *  the geotransform of the image file.  
   */
  ImageDataset->GetGeoTransform(geotransform);
  return geotransform; 
} 
   
long ImageUtil::GetNoDataValue() { 
  /* ****************************************************************
   * function GetNoDataValue(): 
   *  This function returns the NoDataValue for the Geotiff dataset. 
   *  Returns the NoData as a double. 
   */
  return (long)ImageDataset->GetRasterBand(1)->GetNoDataValue();  
}

int *ImageUtil::GetDimensions() {
  /* ****************************************************************
   * int *GetDimensions(): 
   * 
   *  This function returns a pointer to an array of 3 integers 
   *  holding the dimensions of the Geotiff. The array holds the 
   *  dimensions in the following order:
   *   (1) number of columns (x size)
   *   (2) number of rows (y size)
   *   (3) number of bands (number of bands, z dimension)
   */
  dimensions[0] = N_rows; 
  dimensions[1] = N_cols;
  dimensions[2] = N_bands; 
  return dimensions;  
}

double *ImageUtil::GetCalibrationAndBandwidthForBand( 
  int BandNumber,SolarMetadata* Metadata,
  std::map<String,String> CalibrationAndBandWidths,String& BandName 
                                                   // got lazy with reference parameter.
){
  /* This function returns a 2-element array (a POINTER to the first
   * element in a float array, that is) that contains the calibration and
   * bandwidth for some band in the image file.
   */
  double *CalibrationAndBandWidth = new double[2];
  for( auto const& [key,val]:CalibrationAndBandWidths ){

    // move onto next iteration if key does not start with digit
    if(!isdigit(key[0])){ 
      continue;}

    // get the band number from the key, move on if not a match
    int BandNumberFromKey = stoi(key.substr(0,key.find("_")));
    if(!( BandNumberFromKey == BandNumber )){
      continue;}

    // get the band name
    String keystr = key;
    String Delimiter = "__";
    size_t Position  = 0;
    String Token;
    int Counter      = 0;

    while((Position = keystr.find(Delimiter))!=String::npos){
      Token = keystr.substr(0,Position);
      keystr.erase(0,Position+Delimiter.length());
      if( Counter == 1 ) BandName = Token;
      Counter++;
    }
    BandName = trim(BandName);
    // get absolute calibration factor and effective bandwidth 
    // 1_BAND_C_ABSCALFACTOR -> 1.397474000000000e-02
    // 1_BAND_C_EFFECTIVEBANDWIDTH -> 4.050000000000000e-02
    if( key.find("ABSCALFACTOR")!=String::npos ){
      CalibrationAndBandWidth[0] = stod(val);
    }

    if( key.find("EFFECTIVEBANDWIDTH")!=String::npos ){
      CalibrationAndBandWidth[1] = stod(val);
    }
  }
  return CalibrationAndBandWidth;
}

void ImageUtil::WriteRadianceAndReflectanceGeotiffs( 
  SolarMetadata* Metadata, std::map<String,String> CalibrationAndBandWidths ){
  /* *************************************************************************
   * This function writes out Geotiffs containing:
   * (1) the top of atmosphere radiances
   * (2) the top of atmosphere reflectances
   */
  
  // first calculate+set the radiances and reflectances
  // note the keyword "this" is redundant and not needed. we put it here for emphasis
  // that is meant to be called using an object.
  this->CalculateSpectralRadiancesAndReflectances<float>( Metadata,CalibrationAndBandWidths );
}

void ImageUtil::SetSolarIrradiances( SolarIrradiances& Irradiances, String SatelliteID ){
  /* ***********************************************************************
   * Set the radiances in the solar irradiances structure. See ImageUtil.h
   * for this definition. This function will take a pointer to this
   * structure as well as a String for the satellite ID (e.g. "WV02").
   *
   * This function uses the pointer to the structure containing
   * the solar irradiances and sets those values on the satellite.
   */
  if( SatelliteID == "WV03" ) {
    // https://dg-cms-uploads-production.s3.amazonaws.com/uploads/document/file/207/Radiometric_Use_of_WorldView-3_v2.pdf
    Irradiances.BAND_P = 1583.58;        // PAN 
    Irradiances.BAND_C = 1743.81;        // COASTAL
    Irradiances.BAND_B = 1971.48;        // BLUE
    Irradiances.BAND_G = 1856.26;        // GREEN
    Irradiances.BAND_Y = 1749.4 ;        // YELLOW
    Irradiances.BAND_R = 1555.11;        // RED
    Irradiances.BAND_N = 1071.98;        // NIR
    Irradiances.BAND_N2 = 863.296;       // NIR2
    Irradiances.BAND_RE = 1343.95;       // REDEDGE
  } else if ( SatelliteID == "WV02" ) {
    // https://www.yumpu.com/en/document/read/43552535/radiometric-use-of-worldview-2-imagery-technical-note-pancroma
    Irradiances.BAND_P = 1580.8140;      // PAN 
    Irradiances.BAND_C = 1758.2229;      // COASTAL
    Irradiances.BAND_B = 1974.2416;      // BLUE
    Irradiances.BAND_G = 1856.4104;      // GREEN
    Irradiances.BAND_Y = 1738.4791;      // YELLOW
    Irradiances.BAND_R = 1559.4555;      // RED
    Irradiances.BAND_N = 1069.7302;      // NIR
    Irradiances.BAND_N2 = 861.2866;      // NIR2
    Irradiances.BAND_RE = 1342.0695;     // REDEDGE
  } else if( SatelliteID == "QB02"  ) {
    // https://grasswiki.osgeo.org/wiki/QuickBird
    Irradiances.BAND_P = 1381.79;        // PAN 
    Irradiances.BAND_C = -9999.0;        // COASTAL
    Irradiances.BAND_B = 1924.59;        // BLUE
    Irradiances.BAND_G = 1843.08;        // GREEN
    Irradiances.BAND_Y = -9999.0;        // YELLOW
    Irradiances.BAND_R = 1574.77;        // RED
    Irradiances.BAND_N = 1113.71;        // NIR
    Irradiances.BAND_N2 = -9999.0;       // NIR2
    Irradiances.BAND_RE = -9999.0;       // REDEDGE
  } else if( SatelliteID == "GE01" ) {
    // https://apollomapping.com/wp-content/user_uploads/2011/09/GeoEye1_Radiance_at_Aperture.pdf 
    Irradiances.BAND_P = 161.7 * (1.0/1000.0) * (1.0/0.0001);         // PAN 
    Irradiances.BAND_C = -9999.0;                                     // COASTAL
    Irradiances.BAND_B = 196.0 * (1.0/1000.0) * (1.0/0.0001);         // BLUE
    Irradiances.BAND_G = 185.3 * (1.0/1000.0) * (1.0/0.0001);         // GREEN
    Irradiances.BAND_Y = -9999.0;                                     // YELLOW
    Irradiances.BAND_R = 150.5 * (1.0/1000.0) * (1.0/0.0001);         // RED
    Irradiances.BAND_N = 103.9 * (1.0/1000.0) * (1.0/0.0001);         // NIR
    Irradiances.BAND_N2 = -9999.0;                                    // NIR2
    Irradiances.BAND_RE = -9999.0;                                    // REDEDGE
  } else {
    String ErrorMessage = (String)"ERROR (fatal): satellite key from XML not recognized: "+SatelliteID+". Exiting ...\n";
    throw std::runtime_error(ErrorMessage); 
  }
}

template<typename T>
void ImageUtil::CalculateSpectralRadiancesAndReflectances(
  /*  this class method function writes out the two Geotiffs: one Geotiff
   *  holding the top-of-atmosphere radiances and the other the top-of-atmosphere
   *  reflectances. Both are written out as GDAL "float" datasets.
   */
		
  SolarMetadata* Metadata,std::map<String,String> CalibrationAndBandWidths ){
  // register GDAL drivers for C/C++	
  GDALAllRegister();
  String ErrorMsg = "";
 
  GDALDataType BandType; 
  int BandIndex=1; // GDAL starts at 1, not 0.
  long NoDataValue = this->GetNoDataValue();
  String SatelliteID = CalibrationAndBandWidths[ "SatelliteID" ];

  // based on the satellite ID, get array of solar irradiances
  /* ****************************************************** */
  SolarIrradiances Irradiances;
  this->SetSolarIrradiances( Irradiances,SatelliteID ); 

  // initial variales (float) for TOA radiance, TOA reflectance
  float radiance_TOA;
  float reflectance_TOA;
  double earthSunDistance = Metadata->earthSunDistance;
  double solarZenithAngle = Metadata->solarZenithAngle * ( M_PI / 180.0 );

  // create output filenames for the two geotiffs, one Geotiff
  // holding the top-of-atmosphere radiances, the other top-of-atmosphere reflectances
  String image_filename;
  image_filename = (String)this->filename;
  image_filename = image_filename.substr( 0,image_filename.length()-4 );
  String radiances_filename    = image_filename+"_TOA_RADIANCES.TIF";
  String reflectances_filename = image_filename+"_TOA_REFLECTANCES.TIF";
 
  // delete either output file if it already exists (radiances geotiff file)
  // ***********************************************************************
  if(file_exists( radiances_filename.c_str() ))
  {
    if( std::remove( radiances_filename.c_str() ) != 0 ){
      String ErrorMessage = (String)"ERROR (fatal): unable to remove file: "+radiances_filename+"\n";
      throw std::runtime_error(ErrorMessage);
    }
  }

  // delete either output file if it already exists (reflectances geotiff file)
  // **************************************************************************
  if(file_exists( reflectances_filename.c_str() ))
  {
    if( std::remove( reflectances_filename.c_str() ) != 0 ){
      String ErrorMessage = (String)"ERROR (fatal): unable to remove file: "+reflectances_filename+"\n";
      throw std::runtime_error(ErrorMessage);
    }
  }

  // open up GDAL Geotiff dataset objects for writing geotiffs for
  //   (1) geotiff holding top-of-atmosphere radiances
  //   (2) geotiff holding top-of-atmosphere reflectances
  GDALDriver *DriverTiff_Radiances, *DriverTiff_Reflectances;
  DriverTiff_Radiances    = GetGDALDriverManager()->GetDriverByName("GTiff");
  DriverTiff_Reflectances = GetGDALDriverManager()->GetDriverByName("GTiff");

  GDALDataset *ReflectancesDataset = DriverTiff_Reflectances->Create( 
    reflectances_filename.c_str(),N_cols,N_rows,N_bands,GDT_Float32,NULL);
  GDALDataset *RadiancesDataset    = DriverTiff_Radiances->Create( 
    radiances_filename.c_str(),N_cols,N_rows,N_bands,GDT_Float32,NULL);

  // set geotransforms and map projections
  // *************************************
  ReflectancesDataset->SetGeoTransform( 
    this->GetGeoTransform());
  ReflectancesDataset->SetProjection( 
    this->GetProjection() );
  RadiancesDataset->SetGeoTransform( 
    this->GetGeoTransform());
  RadiancesDataset->SetProjection( 
    this->GetProjection() );

  // iterate through bands in image file
  while( BandIndex<N_bands+1 ) {

    // create GetRasterBand() object.
    BandType = GDALGetRasterDataType(
      ImageDataset->GetRasterBand(BandIndex));
    unsigned short *rowBuffer = (unsigned short*) CPLMalloc(sizeof(unsigned short)*N_cols);

    // create memory buffers for output radiances/reflectances
    // *******************************************************
    T *radiancesRowBuff    = (T*) CPLMalloc(sizeof(T)*N_cols);
    T *reflectancesRowBuff = (T*) CPLMalloc(sizeof(T)*N_cols);

    // for specific or current band, get calibration and bandwidth
    String BandName = "";
    double *CalibrationAndBandWidth = this->GetCalibrationAndBandwidthForBand( 
      BandIndex,Metadata,CalibrationAndBandWidths,BandName );
    float BandEffectiveCalibration = (float)CalibrationAndBandWidth[0];
    float BandWidth = (float)CalibrationAndBandWidth[1];
    double SolarIrradianceForBand;

    // based on the name of the band ,get the solar irradiance for this band
    // possible bands:
    //   BAND_P, BAND_C, BAND_B, BAND_G, BAND_Y, 
    //   BAND_R, BAND_N, BAND_N2, BAND_RE 
    if( BandName == "BAND_P" ) {
      SolarIrradianceForBand = Irradiances.BAND_P; 
    } else if( BandName == "BAND_C" ) {
      SolarIrradianceForBand = Irradiances.BAND_C; 
    } else if( BandName == "BAND_B" ) {
      SolarIrradianceForBand = Irradiances.BAND_B; 
    } else if( BandName == "BAND_G" ) {
      SolarIrradianceForBand = Irradiances.BAND_G; 
    } else if( BandName == "BAND_Y" ) {
      SolarIrradianceForBand = Irradiances.BAND_Y; 
    } else if( BandName == "BAND_R" ) {
      SolarIrradianceForBand = Irradiances.BAND_R; 
    } else if( BandName == "BAND_N" ) {
      SolarIrradianceForBand = Irradiances.BAND_N; 
    } else if( BandName == "BAND_N2" ) {
      SolarIrradianceForBand = Irradiances.BAND_N2; 
    } else if( BandName == "BAND_RE" ) {
      SolarIrradianceForBand = Irradiances.BAND_RE; 
    } else {
      String ErrorMessage = (String)"ERROR (fatal): unable to get solar irradiance for band: "+BandName+"\n";
      throw std::runtime_error(ErrorMessage); 
    }

    // iterate through rows in image
    for( int row=0; row<N_rows; row++ ) {
      CPLErr e = ImageDataset->GetRasterBand(BandIndex)->RasterIO(
        GF_Read,0,row,N_cols,1,rowBuffer,N_cols,1,BandType,0,0);
      
      // make sure scanline was read correctly.
      if(!(e == 0)){
        ErrorMsg = "  ERROR (fatal): unable to read image file: "+(String)filename;
        print_error_msg_and_exit( ErrorMsg.c_str() );
      }

      // iterate through columns
      for( int col=0; col<N_cols; col++ ) {

	// set the spectral top-of-atmosphere radiance pixel value using DN
	radiance_TOA = (float)((float)rowBuffer[col]*BandEffectiveCalibration )/BandWidth;
 
	// calculate the solar reflectance
	if( SolarIrradianceForBand<1.0 ) {
	  reflectance_TOA = (float)-9999.0;
	} else {
	  reflectance_TOA = (float)(( radiance_TOA * (
	    earthSunDistance*earthSunDistance) * M_PI ) / ( SolarIrradianceForBand * cos(solarZenithAngle)));
	}
      
        // if in the original image, the digital number (DN) value was NoData, then set
	// the radiance and reflectance to -9999.0
	if( rowBuffer[col] == NoDataValue ) {
          radiance_TOA = float(-9999.0);
	  reflectance_TOA = float(-9999.0);
	}

	// fill in the memory buffers with pixel values
	radiancesRowBuff[col]    = radiance_TOA;
	reflectancesRowBuff[col] = (float)reflectance_TOA;
      }

      // write the scanline or row of values to the output geotiff datasets
      CPLErr RadianceWriteStatus = RadiancesDataset->GetRasterBand(BandIndex)->RasterIO(
        GF_Write,0,row,N_cols,1,radiancesRowBuff,N_cols,1,GDT_Float32,0,0);
      CPLErr ReflectanceWriteStatus = ReflectancesDataset->GetRasterBand(BandIndex)->RasterIO(
        GF_Write,0,row,N_cols,1,reflectancesRowBuff,N_cols,1,GDT_Float32,0,0);

      // check write status of radiances scanline
      if(!(RadianceWriteStatus == 0) ) {
        ErrorMsg = "  ERROR (fatal): unable to write scanline into image file: "+radiances_filename+". Exiting ...\n";
        print_error_msg_and_exit( ErrorMsg.c_str() );
      } 

      // check write status of reflectances scanline
      if(!(ReflectanceWriteStatus == 0) ) {
        ErrorMsg = "  ERROR (fatal): unable to write scanline into image file: "+reflectances_filename+". Exiting ...\n";
        print_error_msg_and_exit( ErrorMsg.c_str() );
      } 
    }

    // free up memory for scanline, array that holds calibration/bandwidth
    CPLFree( rowBuffer );
    CPLFree( radiancesRowBuff );
    CPLFree( reflectancesRowBuff );
    delete [] CalibrationAndBandWidth; 
    BandIndex++;
  }
  GDALClose( RadiancesDataset    );
  GDALClose( ReflectancesDataset );
}
