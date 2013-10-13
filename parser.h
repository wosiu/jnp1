#ifndef PARSE_H
#define PARSE_H

//Pobiera z std wejscia opis pociagow i polecen
//zwraca krotke:
//	1 el - wskaznik do std::multimap zawierajacego opis pociagu
//	2 el - wskaznik do std::vector zawierajacego opis polecen
//Opis pociagu - para:
//	1 el - faktyczny czas przyjazdu (oczekiwany + opoznienie)
//	2 el - opoznienie
//Opis polecenia - krotka:
//	1 el - znak opisujacy polecenie
//	2 el - czas w minutach od
//	3 el - czas w minutach do

std::tuple<std::set<std::tuple<int, int>>*, std::vector<std::tuple<char,int,int>>*> parse();





#endif
