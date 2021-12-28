#ifndef IMAGEUTIL_H_
#define IMAGEUTIL_H_
#include "gdal_priv.h"
#include "cpl_conv.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include "TOAUtil.h"
#include "Misc.h"
#define NODATA -9999
typedef std::string String;
using namespace std;

class ImageUtil {
  private:
    const char* filename      = nullptr;
    GDALDataset *ImageDataset = nullptr;
    double geotransform[6];
    int dimensions[3];
    int N_rows,N_cols,N_bands;

    // solar irradiances structure
    struct SolarIrradiances {
      double BAND_P;
      double BAND_C;
      double BAND_B;
      double BAND_G;
      double BAND_Y;
      double BAND_R;
      double BAND_N;
      double BAND_N2;
      double BAND_RE;
    };

  public:
    // constructors and destructors
    ImageUtil();
    ImageUtil( const char* );
    ~ImageUtil();

    // methods to obtain file metadata and 
    // geotransform and/or projection information.
    const char *GetProjection();
    double *GetGeoTransform();
    long GetNoDataValue();
    int *GetDimensions();

    // function to set TOA radiances and reflectances
    void SetSolarIrradiances( SolarIrradiances&, String );
    double *GetCalibrationAndBandwidthForBand( int,SolarMetadata*,std::map<String,String>,String& );
    void WriteRadianceAndReflectanceGeotiffs( SolarMetadata*, std::map<String,String> );
    template<typename T>
    void CalculateSpectralRadiancesAndReflectances( SolarMetadata*,std::map<String,String> );
};
#endif
