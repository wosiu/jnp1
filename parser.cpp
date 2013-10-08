#include <iostream>
#include <string>
#include <sstream>
#include <ctype.h>
#include <tuple>
#include <set>
#include <stdlib.h>

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
			throw 1;
	}
	
	{
		std::string smonth;
		std::getline(ss,smonth,'.'); 
		if (isNumber(smonth)) 
			month = atoi(smonth.c_str());
		else
			throw 1;
	}
	
	{
		std::string syear;
		std::getline(ss,syear,'.');
		if (isNumber(syear)) 
			year = atoi(syear.c_str());
		else
			throw 1;
	}
	int maxday = 0;
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
	if (((month > 12) || (month < 1)) || (day > maxday))
		throw 1;
}

// returns time from 00:00 to 'time' in minutes
int getTime(std::string time) throw(int){
	int h;
	int m;
	
	std::stringstream ss;
	std::string tmp;
	ss << time;
	std::getline(ss, tmp,'.');
	if (!isNumber(tmp) || (tmp.length() != 2))
		throw 1;
	h = atoi(tmp.c_str());
	if (h >= 24 || h < 0)
		throw 1;
	
	std::getline(ss, tmp,'.');
	if (!isNumber(tmp) || (tmp.length() != 2))
		throw 1;
	m = atoi(tmp.c_str());
	if (m >= 60 || m < 0)
		throw 1;
	return h*60 + m;
}


std::tuple<int, int> parse_line(std::string line) throw(int){
	std::stringstream ss;
	ss << line;
	
	std::string trainnumber; 
	std::string traindate;
	std::string traintime;
	std::string traindelay = "0";

	if (ss.eof())
		throw 1;
	ss >> trainnumber;
	if (!isNumber(trainnumber))	
		throw 1;

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

std::set<tuple<int,int>> * parse(){
	std::string str;
	std::set<tuple<int,int>> * train_set = new std::set<tuple<int,int>>();
	int current_line = 1;
	while (!std::cin.eof()){
		try {
			std::getline(std::cin,str);
			train_set->insert(parse_line(str));
		}
		catch (int error){
			std::cerr << "Error " << current_line << ": " << str;
		}
		current_line ++;
	}
	return train_set;
}

int main(){
	parse();
}
