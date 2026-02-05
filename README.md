Sym link libb64 (arduino branch) to arduino/libraries

Create virtual env (tested on python 3.11.9)

pip install in venv pyqt6-tools, numpy TODO: Add other reqs for install

In venv/Lib/site-packages add a paths.pth file with
the absoulte directory location of:
...\speaker_turntable\serial-packets\lib\python
...\speaker_turntable

so that vscode intellisense with auto complete, and python will find
our modules

Make sure to symbolic link libb64 to Arduino/libraries so arduino complier can find it

Make sure you submodule init and submodule update