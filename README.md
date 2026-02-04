Sym link libb64 (arduino branch) to arduino/libraries

Create virtual env (tested on python 3.11.9)

In venv/Lib/site-packages add a paths.pth file with
the absoulte directory location of:
...\speaker_turntable\serial-packets\lib\python
...\speaker_turntable

so that vscode intellisense with auto complete, and python will find
our modules

TODO: Make libb64 submodule of this project, update local symlink to libb64 in arduino/libraries