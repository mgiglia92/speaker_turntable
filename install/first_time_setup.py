import sys
import os
import subprocess
from pathlib  import *
import traceback

p = Path(__file__).resolve().parent
root_path = (p / '..').resolve()

# Check python version, make sure it is 3.11
print(sys.version_info)
if sys.version_info[0] <3 or sys.version_info[1] != 11:
    raise Exception("Python must be version 3.11")

# Build venv
try:
    # Chg to project root directory
    prev_dir = Path.cwd()
    os.chdir(root_path)
    # Check if .venv exists
    venv_dir = Path(root_path) / '.venv'

    if((venv_dir).is_dir()):
        os.remove(venv_dir)
    venv_cmd = 'python3 -m venv .venv'.split(" ")
    print(subprocess.run(venv_cmd))
    venv_pip3 = Path.cwd() / '.venv' / 'Scripts' / 'pip3.exe' 
    req_txt = root_path / 'python' / 'req.txt'
    install_cmd = f'{str(venv_pip3)} install -r {str(req_txt)}'
    subprocess.run(install_cmd)

    # add paths.pth file to .venv/Lib/site-packages
    site_packages_dir = venv_dir / 'Lib' / 'site-packages'
    paths = [root_path / 'serial-packets\lib\python', 
                root_path,
                root_path / 'python\gui',
                root_path / 'python']
    
    with open(site_packages_dir / 'paths.pth', "w") as f:
        try:
            for p in paths:
                f.write(str(p) + '\n')
        except:
            traceback.print_exc()
        finally:
            f.close()
    pass

except:
    traceback.print_exc()

finally:
    os.chdir(prev_dir)


# install necessary python libs in virtual environment

# Ask if user wants to install the arduino firmware stuff

# 