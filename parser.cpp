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

using namespace std;
typedef pair<int,int> traintype;
typedef tuple<char,int,int> cmdtype;

/*
 *	Kody bledow:
 *
 *	1 - EOF
 *	2 - NaN
 *	30 - Zly format
 *	31 - Zla godzina
 *	32 - Zla minuta
 *	41 - zly dzien
 *	42 - zly miesiac
 *	43 - zly rok
 *	50 - zle polecenie
 *
 */

bool isNumber(string str){
	try {
		boost::lexical_cast<int>(str);
	}
	catch(...) { return false; }
	return true;
}

void checkDate(string str) throw(int){
	stringstream ss;
	ss << str;
	int day;
	int month;
	int year;
	{
		string sday;
		getline(ss,sday,'.');
		if (isNumber(sday))
			day = atoi(sday.c_str());
		else
			throw 2;
	}

	{
		string smonth;
		getline(ss,smonth,'.');
		if (isNumber(smonth))
			month = atoi(smonth.c_str());
		else
			throw 2;
	}

	{
		string syear;
		getline(ss,syear,'.');
		if (isNumber(syear))
			year = atoi(syear.c_str());
		else
			throw 2;
	}
	try{
		boost::gregorian::date d(year,month,day);
	}
	catch (out_of_range e){
		throw 43;
	}
}

// returns time from 00:00 to 'time' in minutes
int getTime(string time) throw(int){
	int h;
	int m;

	stringstream ss;
	string tmp;
	ss << time;
	getline(ss, tmp,'.');
	if (!isNumber(tmp) || (tmp.length() > 2))
		throw 30;
	h = atoi(tmp.c_str());
	if (h >= 24 || h < 0)
		throw 31;

	getline(ss, tmp,'.');
	if (!isNumber(tmp) || (tmp.length() != 2))
		throw 30;
	m = atoi(tmp.c_str());
	if (m >= 60 || m < 0)
		throw 32;
	return h*60 + m;
}


traintype parse_line(string line) throw(int){
	stringstream ss;
	ss << line;

	//TODO: sprawdzanie czy ten sam pociag nie powtarza sie na wejsciu w tym samym czasie?
	string trainnumber;
	string traindate;
	string traintime;
	string traindelay = "0";

	if (ss.eof())
		throw 1;
	ss >> trainnumber;
	if (!isNumber(trainnumber) || trainnumber.length()>9)
		throw 2;

	if (ss.eof())
		throw 1;
	ss >> traindate;
	checkDate(traindate);

	if (ss.eof())
		throw 1;
	if (!ss.eof())
		ss >> traintime;
	int time = getTime(traintime);
	if (!ss.eof())
		ss >> traindelay;
	if (!isNumber(traindelay))
		throw 1;
	int delay = atoi(traindelay.c_str());

	return traintype(time+delay, delay);
}

cmdtype parse_command_line(string line) throw(int){
	stringstream ss;
	char cmd;
	int timestart;
	int timeend;

	ss << line;
	ss >> cmd;

	if (!(((cmd == 'L') || (cmd == 'M')) || (cmd == 'S')))
		throw 50;

	string str;
	ss >> str;

	timestart = getTime(str);
	ss >> str;
	timeend = getTime(str);

	auto command = make_tuple(cmd, timestart, timeend);
	return command;
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

					cerr << e << " Error " << current_line << ": " << str << endl;
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
				cerr << suberror << " Error " << current_line << ": " << str << endl;
			}
		}
	}

	return make_tuple(train_map, command_vector);
}

void test() {
	auto line = parse_command_line("M 00.00 12.00");
	cout << get<0>(line) << " " << get<1>(line) << " " << get<2>(line) << endl;
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

	//zakladam poprawnosc polecen
	for ( cmdtype cmd : cmds ) {
		char code = get<0>(cmd);
		int start = get<1>(cmd);
		int end = get<2>(cmd);
		int res = 0;

		switch(code) {
			case 'L' :
				train_it = trains_per_time.lower_bound( start );
				if ( train_it == trains_per_time.end() ) {
					break;
				}

				end_it = trains_per_time.upper_bound( end );
				if ( end_it == trains_per_time.end() ) {
					end_it --;
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
