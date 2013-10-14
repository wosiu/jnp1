/*
* JNP1 zad 1
* Andrzej Sułecki (TODO), Michał Woś (mw336071)
* grupa nr (TODO)
* g++ -std=cpp11 -lboost_date_time -lboost_regex-mt
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
/*
 *	Kody bledow:
 *
 *	1 - EOF
 *	2 - NaN
 *	30 - Zly czas
 *	40 - Zla data
 *	50 - zle polecenie
 *	100 - TODO ?
 *
 */

int strToInt(string str) throw(int){
	try {
		return boost::lexical_cast<int>(str);
	}
	catch(...) {
		throw 2;
	}
}

bool checkDate(string str) {
	vector<string> parts;
	boost::split( parts, str, boost::is_any_of(".") );
	if ( parts.size() != 3 ) {
		return false;
	}
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
int getTime(string time) throw(int){
	static const boost::regex time_ex("^([0-9]|0[0-9]|1[0-9]|2[0-3])\\.[0-5][0-9]$");
	if( !boost::regex_match(time, time_ex) ) {
		throw 30;
	}

	int h,m;
	sscanf (time.c_str(),"%d\.%d",&h,&m);
	return h * 60 + m;
}


traintype parse_line(string line) throw(int){
	stringstream ss;
	ss << line;

	string trainnumber, traindate, traintime, traindelay = "0";

	// numer pociagu
	if (ss.eof()) { throw 1; }
	ss >> trainnumber;
	strToInt(trainnumber); //throws 2

	// data
	if (ss.eof()) { throw 1; }
	ss >> traindate;
	if ( !checkDate(traindate) ) { throw 40; }

	// planowany czas przejazdu
	if (ss.eof()) { throw 1; }
	ss >> traintime;
	int time = getTime(traintime);

	// opoznienie - opcjonalne
	int delay = 0;
	if (!ss.eof()) {
		ss >> traindelay;
		delay = strToInt( traindelay ); // throws 2
	}

	return traintype(time + delay, delay);
}

cmdtype parse_command_line(string line) throw(int){
	stringstream ss;
	char cmd;
	int timestart;
	int timeend;

	ss << line;
	ss >> cmd;

	if (!(((cmd == 'L') || (cmd == 'M')) || (cmd == 'S'))) { throw 50; }

	string str;
	ss >> str;

	timestart = getTime(str); //throws 30
	ss >> str;
	timeend = getTime(str); //throws 30

	return make_tuple(cmd, timestart, timeend);
}


tuple< multimap<int,int>, vector<cmdtype> > parse(){
	string str;
	//pociagi - (key: czas oczekiwany + opoznienie) # (value: opoznienie)
	multimap <int,int> train_map;
	//polecenia - (znak polecenia, czas [min] od, czas [min] do)
	vector <cmdtype> command_vector;

	int current_line = 1;
	try {
		while ( getline(cin,str) ){
			try {
				train_map.insert(parse_line(str));
			}
			catch (int e){
				try { //jesli blad to sprawdzamy czy nie zaczely sie polecenia
					command_vector.push_back(parse_command_line(str));
					throw 100;
				}
				catch (int suberror){
					if (suberror == 100) // jesli to bylo polecenie
						throw 100;

					if( DEBUG_MODE ) cerr << e << " ";
					cerr << "Error " << current_line << ": " << str << "\n";
				}
			}
			current_line ++;
		}
	}
	catch (int error){
		while ( getline(cin,str) ){
			current_line ++;
			try {
				//getline(cin, str);
				command_vector.push_back(parse_command_line(str));
			}
			catch (int suberror){
				if( DEBUG_MODE ) cerr << suberror << " ";
				cerr << "Error " << current_line << ": " << str << "\n";
			}
		}
	}

	return make_tuple(train_map, command_vector);
}

int main(){
	// wczytanie, sprawdzenie danych
	tuple <multimap<int,int>, vector<cmdtype>> input = parse();
	multimap<int,int> trains = get<0>(input);
	// (key: czas) # (value: ilosc pociagow do czasu wlacznie)
	multimap<int,int> trains_per_time;
	multimap<int,int>::iterator train_it,end_it;
	vector<cmdtype> cmds = get<1>(input);

	// zliczenie ilosci pociagow do danego czasu (potrzebne do polecenia 'L')
	int count = 1;
	for ( traintype train : trains ) {
		trains_per_time.insert( traintype( train.first, count++ ) );
	}

	// zakladam poprawnosc polecen
	for ( cmdtype cmd : cmds ) {
		char code = get<0>(cmd);
		int start = get<1>(cmd);
		int end = get<2>(cmd);
		int res = 0;

		if ( DEBUG_MODE ) cout << code << " " << start << " " << end << endl;

		switch(code) {
			case 'L' :
				train_it = trains_per_time.lower_bound( start );
				if ( train_it == trains_per_time.end() ) {
					break;
				}

				end_it = trains_per_time.upper_bound( end );
				if ( end_it == trains_per_time.end() ) {
					end_it--;
					res = 1;
				}
				res += (*end_it).second - (*train_it).second;
			break;

			case 'M' :
				train_it = trains.lower_bound( start );
				end_it = trains.upper_bound( end );
				for ( ; train_it != end_it; train_it++ ) {
					res = max( res, (*train_it).second );
				}
			break;

			case 'S' :
				train_it = trains.lower_bound( start );
				end_it = trains.upper_bound( end );
				for ( ; train_it != end_it; train_it++ ) {
					res += (*train_it).second;
				}
			break;
		}

		printf("%d\n", res);
	}
}
