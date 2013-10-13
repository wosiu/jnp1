#include <iostream>
#include <string>
#include <sstream>
#include <ctype.h>
#include <tuple>
#include <set>
#include <vector>
#include <stdlib.h>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "parser.h"
//#define traintype std::tuple<int,int>
//#define cmdtype std::tuple<char, int,int>
//typedef std::tuple<int. int> traintype ;
//typedef std::tuple<char, int, int> cmdtype ;



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

bool isNumber(std::string str){
	bool res = true;
	for (std::string::const_iterator k = str.begin(); k != str.end(); ++k)
		res = res && isdigit(*k);
	return res;
}



void checkDate(std::string str) throw(int){
	std::stringstream ss;
	ss << str;
	int day;
	int month;
	int year;
	{
		std::string sday;
		std::getline(ss,sday,'.'); 
		if (isNumber(sday)) 
			day = atoi(sday.c_str());
		else
			throw 2;
	}
	
	{
		std::string smonth;
		std::getline(ss,smonth,'.'); 
		if (isNumber(smonth)) 
			month = atoi(smonth.c_str());
		else
			throw 2;
	}
	
	{
		std::string syear;
		std::getline(ss,syear,'.');
		if (isNumber(syear)) 
			year = atoi(syear.c_str());
		else
			throw 2;
	}
	try{
		boost::gregorian::date d(year,month,day);
	}
	catch (std::out_of_range e){
		throw 43;	
	}
	/*int maxday = 0;
	if (month <= 7) {
		if (month % 2)
			maxday = 30;
		else
			maxday = 31;
	}
	else {
		if (month % 2)
			maxday = 31;
		else
			maxday = 30;
	}
	if (month == 2){	//Uwzgledniamy przestepne
		if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
			maxday = 29;
		else
			maxday = 28; 
	}

	if ((month > 12) || (month < 1))
		throw 42;
	if (day > maxday)
		throw 41;*/
}

// returns time from 00:00 to 'time' in minutes
int getTime(std::string time) throw(int){
	int h;
	int m;
	
	std::stringstream ss;
	std::string tmp;
	ss << time;
	std::getline(ss, tmp,'.');
	if (!isNumber(tmp) || (tmp.length() > 2))
		throw 30;
	h = atoi(tmp.c_str());
	if (h >= 24 || h < 0)
		throw 31;
	
	std::getline(ss, tmp,'.');
	if (!isNumber(tmp) || (tmp.length() != 2))
		throw 30;
	m = atoi(tmp.c_str());
	if (m >= 60 || m < 0)
		throw 32;
	return h*60 + m;
}

traintype parse_line(std::string line) throw(int){
	std::stringstream ss;
	ss << line;
	
	std::string trainnumber; 
	std::string traindate;
	std::string traintime;
	std::string traindelay = "0";

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
	
	auto train = std::make_tuple(time+delay, delay);
}

cmdtype parse_command_line(std::string line) throw(int){
	std::stringstream ss;
	char cmd;
	int timestart;
	int timeend;
	
	ss << line;

	ss >> cmd;

	if (!(((cmd == 'L') || (cmd == 'M')) || (cmd == 'S')))
		throw 50;
	
	std::string str;
	ss >> str;
	timestart = getTime(str);
	ss >> str;
	timeend = getTime(str);

	auto command = std::make_tuple(cmd, timestart, timeend);
}

std::tuple<std::set<traintype> , std::vector<cmdtype>> parse(){
	std::string str;
	std::set < traintype > train_set;
	std::vector < cmdtype > command_vector;
	int current_line = 1;
	try {
		while (!std::cin.eof()){
			try {
				std::getline(std::cin,str);
				train_set.insert(parse_line(str));
			}
			catch (int e){
				try { //jesli blad to sprawdzamy czy nie zaczely sie polecenia
					command_vector.push_back(parse_command_line(str));
					throw 100;
				}
				catch (int error){
					if (error == 100) // jesli to bylo polecenie
						throw 100;
					std::cerr << e << " Error " << current_line << ": " << str << std::endl;
				}
			}
			current_line ++;
		}
	}
	catch (int error){
		while (!std::cin.eof()){
			current_line ++;
			try {
				std::getline(std::cin, str);
				command_vector.push_back(parse_command_line(str));			
			}
			catch (int error){
				std::cerr << error << " Error " << current_line << ": " << str << std::endl;
			}
		}
	}

	return std::make_tuple(train_set, command_vector);
}

int main(){
	parse();
}
