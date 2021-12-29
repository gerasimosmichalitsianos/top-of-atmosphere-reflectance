# top-of-atmosphere-reflectance
C++ code to convert satellite imagery from digital number (DN) to top-of-atmosphere radiance and reflectane.

###### DESCRIPTION

    This C++ code takes converts a satellite imagery from a satellite (e.g. WorldView 1, WorldView2, 
    WorldView3, GeoEye 1, Quickbird 2), and converts its pixel values from Digital Numbers (DN) to 
    both top-of-atmosphere (TOA) radiance as well as top-of-atmosphere (TOA) reflectance. To this end,
    this program will expect command-line inputs for (1) the name of the image file (.NTF,.NITF), as well
    as (2) the corresponding XML (.XML,.xml) and (3) the .IMD metadata file. Hence, these are the
    3 command-line arguments.
    
    It is expected that this code will be used and run in a UNIX-like environment (e.g. UNIX, Linux,
    Mac OSX.). It is also expected that the Docker command-line software is installed on the 
    environment or server in which the code is used. Often this tool is installed as /bin/docker
    or /usr/bin/docker, with a typical standard Linux installation. It is also expected tha the
    command-line Git software will be installed.
    
    Upon completion of running this software, two Geotiffs will be created in the
    same directory as that of the NITF image file that was passed in: (1) on geotiff
    holding the top-of-atmosphere radiances, the other holding the top-of-atmosphere
    reflectances.
    
###### CURRENTLY SUPPORTED SATELLITES FOR THIS C++ CODE

    Quickbird 2 (QB02), WorldView 2/3 (WV02,WV03), Geoeye 1 (OV05).
    
###### GENERAL USAGE
 
    $ make
    $ ./bin/toa -f {filename ntf|tif} -i {filename IMD) -x {filename XML}
    
###### COMMAND LINE USAGE

    Adjust the paths for your GDAL installation in the makefile.
    You may also need to adjust the paths to your g++ compiler, among
    other paths that may need updating for the required header and 
    include files. Upon making the appropriate updates to the makefile,
    you may do the following:
    
    $ make
    $ ./bin/toa 
      -f $DIR/XXXXXXXXXXXXX-M1BS-XXXXXXXXXXXX_01_P013.NTF 
      -i $DIR/XXXXXXXXXXXXX-M1BS-XXXXXXXXXXXX_01_P013.IMD 
      -x $DIR/XXXXXXXXXXXXX-M1BS-XXXXXXXXXXXX_01_P013.XML
    
###### USAGE WITH DOCKER
    
    Command-line usage:
    
    First check out the code:
    
    $ git clone https://github.com/gerasimosmichalitsianos/top-of-atmosphere-reflectance
    $ cd top-of-atmosphere-reflectance
    $ ls
    bin  libs  src  Dockerfile  LICENSE  README.md  makefile
    
    Then build the docker container e.g.:
    
    $ which docker
    /usr/bin/docker
    $ docker build -t toa .
    
    Then use your 3 input data files (.NTF,.IMD,.XML) to run the Docker image
    "toa" as a Docker container:
    
    $ ls -l /home/username/data
    XXXXXXXXXXXXX-M1BS-XXXXXXXXXXXX_01_P013.IMD  
    XXXXXXXXXXXXX-M1BS-XXXXXXXXXXXX_01_P013.NTF 
    XXXXXXXXXXXXX-M1BS-XXXXXXXXXXXX_01_P013.XML
    $ DIR=/home/username/data
    $ docker run -v $DIR:$DIR toa 
      -f $DIR/XXXXXXXXXXXXX-M1BS-XXXXXXXXXXXX_01_P013.NTF 
      -i $DIR/XXXXXXXXXXXXX-M1BS-XXXXXXXXXXXX_01_P013.IMD 
      -x $DIR/XXXXXXXXXXXXX-M1BS-XXXXXXXXXXXX_01_P013.XML
    
    The output Geotiffs (2 of them) shall appear in the directory defined
    by $DIR. For some bands, if the solar irradiance is not available
    (see ImageUtil.cpp), then these bands will be filled with a NoData
    value (-9999.0). Both Geotiffs have a GDAL float data-type
    (GDAL GDT_Float32).
    
###### @author:
    
    Gerasimos Michalitsianos
    28 December 2021
    gerasimosmichalitsianos@gmail.com
