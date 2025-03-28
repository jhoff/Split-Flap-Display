# SCRIPT TO GZIP CRITICAL FILES FOR ACCELERATED WEBSERVING
# https://community.platformio.org/t/script-for-littlefs-upload/16085/3

Import('env', 'projenv')
import os
import gzip
import shutil
import glob

# Complementary function to use gzip
def gzip_file(src_path, dst_path):
    with open(src_path, 'rb') as src, gzip.open(dst_path, 'wb') as dst:
        for chunk in iter(lambda: src.read(4096), b""):
            dst.write(chunk)

# Compress the files defined in 'build/web/' into '.pio/build/littlefs/'
def gzip_webfiles(source, target, env):
    # File types that need to be compressed
    filetypes_to_only_gzip = ['css', 'html', 'js']
    filetypes_to_gzip_and_copy = ['json']
    print('\nGZIP: Starting the gzipping process for the LITTLEFS image...\n')

    web_dir_path = os.path.join(env.get('PROJECT_DIR'), 'build/web')
    data_dir_path = os.path.join(env.get('PROJECT_DIR'), '.pio/build/littlefs')

    # Check if '.pio/build/littlefs' and 'build/web' exist. If the first exists and the second does not, rename it
    if os.path.exists(data_dir_path) and not os.path.exists(web_dir_path):
        print('GZIP: The directory "' + data_dir_path + '" exists, "' + web_dir_path + '" is not found.')
        print('GZIP: Renaming "' + data_dir_path + '" to "' + web_dir_path + '"')
        os.rename(data_dir_path, web_dir_path)

    # Delete the 'data' directory
    if os.path.exists(data_dir_path):
        print('GZIP: Deleting the "data" directory ' + data_dir_path)
        shutil.rmtree(data_dir_path)

    # Recreate the empty 'data' directory
    print('GZIP: Re-creating an empty data directory ' + data_dir_path)
    os.mkdir(data_dir_path)

    # Determine the files to compress
    files_to_only_gzip = []
    for extension in filetypes_to_only_gzip:
        files_to_only_gzip.extend(glob.glob(os.path.join(web_dir_path, '*.' + extension)))
    files_to_gzip_and_copy = []
    for extension in filetypes_to_gzip_and_copy:
        files_to_gzip_and_copy.extend(glob.glob(os.path.join(web_dir_path, '*.' + extension)))

    all_files = glob.glob(os.path.join(web_dir_path, '*.*'))
    files_to_copy = list(set(all_files) - set(files_to_only_gzip))

    for file in files_to_copy:
        print('GZIP: Copying file: ' + file + ' to the data directory')
        shutil.copy(file, data_dir_path)

    # Compress and move the files
    was_error = False
    try:
        for source_file_path in list(set(files_to_only_gzip) | set(files_to_gzip_and_copy)):
            base_file_path = source_file_path
            target_file_path = os.path.join(data_dir_path, os.path.basename(base_file_path) + '.gz')
            print('GZIP: Compressed ' + target_file_path)
            gzip_file(source_file_path, target_file_path)
    except IOError as e:
        was_error = True
        print('GZIP: Failed to compress the file: ' + source_file_path)

    if was_error:
        print('GZIP: Failed/Incomplete.\n')
    else:
        print('GZIP: Successfully compressed.\n')

# IMPORTANT, this needs to be added to call the routine
env.AddPreAction('$BUILD_DIR/littlefs.bin', gzip_webfiles)
