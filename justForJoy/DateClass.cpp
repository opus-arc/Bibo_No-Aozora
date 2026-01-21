#include<iostream>
using namespace std;

class Date{
private:
    int year = 0;
    int month = 0;
    int day = 0;
public:
    Date(const int y, const int m, const int d) : year(y), month(m), day(d){}

    void printfDate() const{
        cout<<"year: "<<year<<", month: "<<month<<", day: "<<day<<endl;
    }

    bool isLeap() const{
        return !(year % 4) && !(year % 400) && (year % 100);
    }
    bool isLeap(int y){
        return !(y % 4) && !(y % 400) && (y % 100);
    }

    int yearDay(){
        return isLeap(year) ? 366 : 365;
    }
    int yearDay(int y){
        return isLeap(y) ? 366 : 365;
    }

    int monthDay(){
        return 
        month == 2 ? (isLeap(year) ? 29 : 28) : 
        (month == 4 || month == 6 || month == 9 || month == 11 ) ? 30 : 31;
    }
    int monthDay(int y, int m){
        return 
        m == 2 ? (isLeap(y) ? 29 : 28) : 
        (m == 4 || m == 6 || m == 9 || m == 11 ) ? 30 : 31;
    }

    void addADay(){
        day++;
        if(day > monthDay(year, month)){
            month++;
            day = 1;
            if(month > 12){
                year++;
                month = 1;
            }
        }
    }
    void addDays(int days){
        for(int i = 0; i < days; i++){
            addADay();
        }
    }

    void reSetData(int y, int m, int d){
        year = y;
        month = m;
        day = d;
    }

};

int main(){
    cout<<"Date Program Start."<<endl;
    Date d(25, 11, 12);
    d.printfDate();
    d.addDays(4);
    d.printfDate();
    d.addDays(23);
    d.printfDate();
    d.addDays(7482);
    d.printfDate();
    return 0;
}
