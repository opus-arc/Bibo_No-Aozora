#include<iostream>
#include<string>
#include "Src/Pitch.h"
#include "Src/RecordPlayer.h"
#define A4_STANDARD 440.0
using namespace std;


int main(){
    const Pitch p1("Bb4");
    p1.print_frequency();
    p1.playTheSound();

    const Pitch p2("D5");
    p2.print_frequency();
    p2.playTheSound();
    return 0;
}