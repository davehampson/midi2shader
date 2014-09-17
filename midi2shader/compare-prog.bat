call build-it.bat
Csvmidi test3.csv > test3.mid
Midicsv.exe test3.mid > ref.txt
MyApplication.exe test3.mid > mine.txt
fc ref.txt mine.txt
del ref.txt mine.txt
