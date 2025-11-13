//
// Created by opus arc on 2025/11/13.
//

#include "Pitch.h"

#include <iostream>
#include<string>
using namespace std;


    bool Pitch::noteName_formatCheck(const string &input_noteName){
        // C4 G#5 A6 C7 Bb4 C1
        return
            2 <= input_noteName.size() &&
            input_noteName.size() <= 3 &&

            'A' <= input_noteName[0] &&
            input_noteName[0] <= 'G' &&

            (
                input_noteName.size() <= 2
                ?
                    '1' <= input_noteName[1] &&
                    input_noteName[1] <= '7'
                :
                    (
                        input_noteName[1] == '#' ||
                        input_noteName[1] == 'b'
                    )
                    &&
                    '1' <= input_noteName[2] &&
                    input_noteName[2] <= '7'
            );
    }
    void Pitch::noteToFrequency(){
        char letterName;
        int octaveNumber;
        int accidental;

        if(noteName.size() == 3){
            letterName = noteName[0];
            octaveNumber = noteName[2] - '0';
            accidental = noteName[1] == '#' ? 1 : -1;
        }else{
            letterName = noteName[0];
            octaveNumber = noteName[1] - '0';
            accidental = 0;
        }

        int _12_TET;

        switch (letterName)
        {
        case 'C':
            _12_TET = 0 + accidental;
            break;

        case 'D':
            _12_TET = 2 + accidental;
            break;

        case 'E':
            _12_TET = 4 + accidental;
            break;

        case 'F':
            _12_TET = 5 + accidental;
            break;

        case 'G':
            _12_TET = 7 + accidental;
            break;

        case 'A':
            _12_TET = 9 + accidental;
            break;

        case 'B':
            _12_TET = 11 + accidental;
            break;

        default:
            _12_TET = 0;
            break;
        }
        midi_n = _12_TET + octaveNumber * 12 + 12;
        double semitoneFromA4 = midi_n - 69;

        frequency = A4_STANDARD * pow(2.0, (double)(semitoneFromA4 / 12.0));
    }

    Pitch::Pitch(const string& input_noteName) :
    noteName
    (
        noteName_formatCheck(input_noteName) ?
        input_noteName : "A4"
    ){
        noteToFrequency();
    }
    void Pitch::print_noteName() const {
        cout<<noteName<<endl;
    }
    void Pitch::print_frequency() const{
        cout<<frequency<<endl;
    }
    void Pitch::midi_n_frequency() const{
        cout<<midi_n<<endl;
    }
    string Pitch::get_noteName(){
        return noteName;
    }
    [[nodiscard]] double Pitch::get_frequency() const{
        return frequency;
    }
    [[nodiscard]] int Pitch::get_midi_n() const{
        return midi_n;
    }
