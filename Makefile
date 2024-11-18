SBR-FC.exe : SBR-FC.cpp reglasYhechos.o
	g++ -std=c++11 --static SBR-FC.cpp reglasYhechos.o -o SBR-FC.exe

reglasYhechos.o : reglasYhechos.hpp reglasYhechos.cpp
	g++ -std=c++11 -c reglasYhechos.hpp reglasYhechos.cpp
