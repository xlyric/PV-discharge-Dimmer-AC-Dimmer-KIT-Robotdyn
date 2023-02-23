# SCRIPT TO GZIP CRITICAL FILES FOR ACCELERATED WEBSERVING
# see also https://community.platformio.org/t/question-esp32-compress-files-in-data-to-gzip-before-upload-possible-to-spiffs/6274/10
#

Import( 'env', 'projenv' )


env.Replace(
    PROJECT_DATA_DIR='data',
    PROJECTDATA_DIR="$PROJECT_DATA_DIR",  # legacy for dev/platform
)

import os
import gzip
import shutil
import glob

# HELPER TO GZIP A FILE
def gzip_file( src_path, dst_path ):
    with open( src_path, 'rb' ) as src, gzip.open( dst_path, 'wb' ) as dst:
        for chunk in iter( lambda: src.read(4096), b"" ):
            dst.write( chunk )

# GZIP DEFINED FILES FROM 'data' DIR to 'data/gzip/' DIR
def gzip_webfiles( source, target, env ):
    
    # FILETYPES / SUFFIXES WHICh NEED TO BE GZIPPED
    source_file_prefix = '_'
    filetypes_to_gzip = [ 'css', 'html' ]

    print( '\nGZIP: INITIATED GZIP FOR SPIFFS...\n' )
    GZIP_DIR_NAME = 'gzip'


    data_dir_path = 'data' 
    #gzip_dir_path = os.path.join( data_dir_path, GZIP_DIR_NAME )

    # CHECK DATA DIR
    if not os.path.exists( data_dir_path ):
        print( 'GZIP: DATA DIRECTORY MISSING AT PATH: ' + data_dir_path )
        print( 'GZIP: PLEASE CREATE THE DIRECTORY FIRST (ABORTING)' )
        print( 'GZIP: FAILURE / ABORTED' )
        return
    
    # CHECK GZIP DIR
    # if not os.path.exists( gzip_dir_path ):
    #     print( 'GZIP: GZIP DIRECTORY MISSING AT PATH: ' + gzip_dir_path )
    #     print( 'GZIP: TRYING TO CREATE IT...' )
    #     try:
    #         os.mkdir( gzip_dir_path )
    #     except Exception as e:
    #         print( 'GZIP: FAILED TO CREATE DIRECTORY: ' + gzip_dir_path )
    #         # print( 'GZIP: EXCEPTION... ' + str( e ) )
    #         print( 'GZIP: PLEASE CREATE THE DIRECTORY FIRST (ABORTING)' )
    #         print( 'GZIP: FAILURE / ABORTED' )
    #         return

    # DETERMINE FILES TO COMPRESS
    files_to_gzip = []
    for extension in filetypes_to_gzip:
        match_str = source_file_prefix + '*.'
        files_to_gzip.extend( glob.glob( os.path.join( data_dir_path, match_str + extension ) ) )
    
    # print( 'GZIP: GZIPPING FILES... {}\n'.format( files_to_gzip ) )

    # COMPRESS AND MOVE FILES
    was_error = False
    try:
        for source_file_path in files_to_gzip:
            print( 'GZIP: ZIPPING... ' + source_file_path )
            base_file_path = source_file_path.replace( source_file_prefix, '' )
            target_file_path = os.path.join( data_dir_path, os.path.basename( base_file_path ) + '.gz' )
            # CHECK IF FILE ALREADY EXISTS
            if os.path.exists( target_file_path ):
                print( 'GZIP: REMOVING... ' + target_file_path )
                os.remove( target_file_path )

            # print( 'GZIP: GZIPPING FILE...\n' + source_file_path + ' TO...\n' + target_file_path + "\n\n" )
            print( 'GZIP: GZIPPED... ' + target_file_path + "\n" )
            gzip_file( source_file_path, target_file_path )
    except IOError as e:
        was_error = True
        print( 'GZIP: FAILED TO COMPRESS FILE: ' + source_file_path )
        # print( 'GZIP: EXCEPTION... {}'.format( e ) )
    if was_error:
        print( 'GZIP: FAILURE/INCOMPLETE.\n' )
    else:
        print( 'GZIP: SUCCESS/COMPRESSED.\n' )

# IMPORTANT, this needs to be added to call the routine
env.AddPreAction( '$BUILD_DIR/spiffs.bin', gzip_webfiles )