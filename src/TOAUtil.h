#ifndef TOAUTIL_H_
#define TOAUTIL_H_
#define NODATA -9999
typedef std::string String;

// define strucutre in header file ... appropriate
// to define it here instead of the implementation file
// because multiple implementation files may use this.
struct SolarMetadata {
  double earthSunDistance;
  double solarZenithAngle;
};

// method to return std::map containing effective calibration and bandwidth for each band
std::map<String,String> SetCalibrationAndBandWidth( const char*, const char*, const char* );

// function to get the solar zenith angle for the dataset
double SolarZenithAngle( const char* );

// function to set Earth-sun distance (in AU)
void EarthSunDistance( const char*, SolarMetadata* );
#endif
