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
    
    Upon completion
    
###### SUPPORTED SATELLITES

    Quickbird 2 (QB02), WorldView 2/3, Geoeye 1.
    
###### USAGE
    
    Command-line usage:
    
    $ git clone https://github.com/gerasimosmichalitsianos/top-of-atmosphere-reflectance
    $ cd top-of-atmosphere-reflectance
    $ ls
    
