#include<iostream>
#include<string>
#include "src/Pitch.h"
#define A4_STANDARD 440.0
using namespace std;


class Note{
private:
    Pitch pitch; // C4 G#5 A6 C7 Bb4 C1
    double amplitude{};   // 0.0 ~ 1.0
    double duration{};   // ms
};

int main(){
    Pitch p1("A#4");
    p1.print_frequency();
    return 0;
}