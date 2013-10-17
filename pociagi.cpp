/*
* JNP1 zad 1
* Andrzej Sułecki (as320426), Michał Woś (mw336071)
* grupa nr 7
* g++ -std=c++11 -lboost_date_time -lboost_regex
*/
#include <iostream>
#include <string>
#include <sstream>
#include <ctype.h>
#include <tuple>
#include <map>
#include <vector>
#include <stdlib.h>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

using namespace std;
typedef pair<int,int> traintype;
typedef tuple<char,int,int> cmdtype;
const bool DEBUG_MODE = false;
const unsigned int LINES_MAX_NO = 3e7;
const int DOBA = 60*24;
const traintype BADTRAIN = traintype(-1,-1);
const cmdtype BADCMD = cmdtype('E',-1,-1);

//const int MAX_DELEY =
//TODO: podac ograniczenie na maksymalne opoznienie (sizeof int - DOBA w min)

/*
 *	Kody bledow:
 *
 *	1 - EOF
 *	2 - NaN
 *	30 - Zly czas
 *	40 - Zla data
 *	41 - ujemny delay
 *	50 - zle polecenie
 *	51 - zle polecenie - czas startu po czasie konca
 *	52 - brak czasu startu/konca
 *	100 - TODO ?
 *
 */

int strToInt(string str){
	try {
		return boost::lexical_cast<int>(str);
	}
	catch(...) {
		return -1;
	}
}

bool checkDate(string str) {
	vector<string> parts;
	boost::split( parts, str, boost::is_any_of(".") );
	if ( parts.size() != 3 ) {
		return false;
	}
	if (parts[0].length() > 2 || parts[1].length() > 2)
		return false;
	string formated = parts[2] + "/" + parts[1] + "/" + parts[0];

	try {
		//boost::gregorian::date d( boost::gregorian::from_string( formated ) );
		boost::gregorian::from_string( formated );
	} catch(...) {
		return false;
	}
	return true;
}

// returns time from 00:00 to 'time' in minutes
int getTime(string time){
	static const boost::regex time_ex("^([0-9]|0[0-9]|1[0-9]|2[0-3])\\.[0-5][0-9]$");
	if( !boost::regex_match(time, time_ex) ) {
		return -1;
	}
	int h,m;
	sscanf (time.c_str(),"%d\.%d",&h,&m);
	return h * 60 + m;
}

// parsowanie wiersza z potencjalnym opisem pociagu
traintype parse_line(string line){
	stringstream ss;
	ss << line;

	string trainnumber, traindate, traintime, traindelay = "0";

	// numer pociagu
	if (ss.eof()) { return BADTRAIN; }
	ss >> trainnumber;
	// jesli numer pociagu nie jest 'kilkucyfrowy'
	if ( 3 > trainnumber.size() || trainnumber.size() > 9 ) {
		return BADTRAIN;
	}

	// jesli problem ze sparsowaniem (-1) lub numer pociagu ujemnyy
	if ( strToInt(trainnumber) < 0 ) { return BADTRAIN; }
	// data
	if (ss.eof()) { return BADTRAIN; }
	ss >> traindate;
	if ( !checkDate(traindate) ) { return BADTRAIN; }

	// planowany czas przejazdu
	if (ss.eof()) { return BADTRAIN; }
	ss >> traintime;
	int time = getTime(traintime);
	if (time == -1) { return BADTRAIN; }
	// opoznienie - opcjonalne
	int delay = 0;
	if (!ss.eof()) {
		if (ss >> traindelay) {
			delay = strToInt( traindelay );
			if (delay < 0) { return BADTRAIN; }
		}
	}
	if (!ss.eof()) {
		if (ss >> traindelay ) //zwraca true jesli napotkal gdzies nie biale znaki
			return BADTRAIN;
	}

	return traintype( (time + delay) % DOBA, delay);
}

cmdtype parse_command_line(string line){
	stringstream ss;
	char cmd;
	int timestart;
	int timeend;

	ss << line;
	ss >> cmd;

	if (!(((cmd == 'L') || (cmd == 'M')) || (cmd == 'S'))) { return BADCMD; }

	string str;
	if (ss.eof()) { return BADCMD; }
	ss >> str;
	timestart = getTime(str);
	if (timestart == -1) { return BADCMD; }
	if (ss.eof()) { return BADCMD; }
	string str2;
	ss >> str2;
	timeend = getTime(str2);
	if (timeend == -1) { return BADCMD; }
	if (timeend < timestart) { return BADCMD; }
	if (!ss.eof()) {
		if (ss >> str2) //zwraca true jesli napotkal gdzies nie biale znaki
			return BADCMD;
	}
	return make_tuple(cmd, timestart, timeend);
}

void execute( multimap<int,int> * trains, multimap<int,int> * trains_per_time,
			cmdtype cmd ){

	char code = get<0>( cmd );
	int start = get<1>( cmd );
	int end = get<2>( cmd );
	unsigned long long res = 0;
	multimap<int,int>::iterator train_it,end_it;

	switch(code) {
		case 'L' :
			train_it = trains_per_time->lower_bound( start );
			if ( train_it == trains_per_time->end() ) {
				break;
			}
			end_it = trains_per_time->upper_bound( end );
			if ( end_it == trains_per_time->end() ) {
				end_it--;
				res = 1ULL;
			}
			res += (*end_it).second - (*train_it).second;
			break;
		case 'M' :
			train_it = trains->lower_bound( start );
			end_it = trains->upper_bound( end );
			for ( ; train_it != end_it; train_it++ ) {
				res = max( res, (unsigned long long)(*train_it).second );
			}
			break;
		case 'S' :
			train_it = trains->lower_bound( start );
			end_it = trains->upper_bound( end );
			for ( ; train_it != end_it; train_it++ ) {
				res += (*train_it).second;
			}
			break;
	}
	printf("%llu\n", res);
}

int main() {

	string str;
	// pociagi - (key: czas oczekiwany + opoznienie) # (value: opoznienie)
	multimap <int,int> train_map;
	multimap <int,int> trains_per_time;
	int current_line = 0;
	bool notEOF;
	cmdtype tmpcmd;
	traintype tmptrain;

	// wczytujemy pociagi
	while ( ( notEOF = getline(cin,str) ) ) {
		current_line ++;

		// przekroczylismy limit spamietanych pociagow
		if ( train_map.size() > LINES_MAX_NO ) {
			cerr << "Error " << current_line << ": " << str << "\n";
			continue;
		}

		tmptrain = parse_line( str );

		// wczytana linia jest poprawnym pociagiem
		if ( tmptrain != BADTRAIN ) {
			train_map.insert(tmptrain);
			continue;
		}

		tmpcmd = parse_command_line( str );

		// wczytana linia jest poprawnym poleceniem
		if ( tmpcmd != BADCMD ) {
			// inicjacja elementow potrzebnych do wykonywania polecen
			int count = 1;

			for ( traintype train : train_map ) {
				trains_per_time.insert( traintype( train.first, count++ ) );
			}

			// wywolujemy dla pierwszego polecenia
			execute( &train_map, &trains_per_time, tmpcmd );

			// przerywamy wczytywanie pociagow
			break;
		}

		// wczytywana linia nie jest ani poprawnym pociagiem ani poleceniem
		cerr << "Error " << current_line << ": " << str << "\n";
	}

	// ostatnio wczytano koniec pliku
	if ( !notEOF ) {
		return 0;
	}

	// wczytujemy pozostale polecenia
	while ( getline(cin,str) ) {

		current_line++;
		tmpcmd = parse_command_line(str);

		// polecenie jest poprawne
		if ( tmpcmd != BADCMD ){
			execute(&train_map, &trains_per_time, tmpcmd);
		}
		else {
			cerr << "Error " << current_line << ": " << str << "\n";
		}
	}

	return 0;
}
