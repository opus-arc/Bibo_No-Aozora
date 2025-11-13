#include<iostream>
#include<string>
#include "Src/Pitch.h"
#define A4_STANDARD 440.0
using namespace std;


int main(){
    Pitch p1("A#4");
    p1.print_frequency();
    return 0;
}