#ifndef PARSE_H
#define PARSE_H

//Pobiera z std wejscia opis pociagow i polecen
//zwraca krotke:
//	1 el - wskaznik do std::set zawierajacego opis pociagu
//	2 el - wskaznik do std::vector zawierajacego opis polecen
//Opis pociagu - krotka:
//	1 el - faktyczny czas przyjazdu (oczekiwany + opoznienie)
//	2 el - opoznienie
//Opis polecenia - krotka:
//	1 el - znak opisujacy polecenie
//	2 el - czas w minutach
//	3 el - czas w minutach

std::tuple<std::set<std::tuple<int, int>>*, std::vector<std::tuple<char,int,int>>*> parse();





#endif 
