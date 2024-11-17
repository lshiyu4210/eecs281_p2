// Identifier: AD48FB4835AF347EB0CA8009E24C3B13F8519882
#ifndef GALAXY_H
#define GALAXY_H
#include <iostream>
#include <vector>
#include <queue>
#include <cassert>
using namespace std;

class galaxy{
public:

    struct battalion{
        int id;
        int general;
        int force;
        int troops;
    };

    struct general{
        int jedi = 0;
        int sith = 0;
        int died = 0;
    };

    struct battle{
        int sith_gen;
        int jedi_gen;
        int planet;
        int loss = 0;
    };

    enum class State{
        Initial,
        SeenOne,
        SeenBoth,
        MaybeBetter,
    };

    struct Watcher{
        int best_jedi;
        int best_sith;
        int jedi_ts;
        int sith_ts;
        int maybe_force;
        int maybe_ts;
        State s = State::Initial;
    };


    //greater comparator for Jedi battalions
    class Jedi_comp{
    public:
        bool operator()(const battalion & j1, const battalion & j2) const{
            if(j1.force == j2.force)      return j1.id > j2.id;
            else                          return j1.force > j2.force;
        }
    };

    //less comparator for Sith battalions
    class Sith_comp{
    public:
        bool operator()(const battalion & s1, const battalion & s2) const{
            if(s1.force == s2.force)      return s1.id > s2.id;
            return                        s1.force < s2.force;
        }
    };

    class planet{
        public:
            priority_queue<battalion, vector<battalion>, Jedi_comp> rebellion;
            priority_queue<battalion, vector<battalion>, Sith_comp> empire;
            //max PQ for lower half
            priority_queue<int, vector<int>, less<int>> lower_half;
            //min PQ for upper half
            priority_queue<int, vector<int>, greater<int>> upper_half;  //min
            //movie watchers for this specific planet
            Watcher attack;         
            Watcher ambush;

        void push(int loss);

        int get_med() const;
    };

    //constructor
    galaxy(): verbose_on(false), battles(0), timestamp(0){}

    //check if the conditions are met for a battle to begin: highest force sith >= lowest force jedi
    bool targeted(int planet);

    //let the highest force sith fight with the lowest force jedi
    void engage();

    //Read command line options 
    void get_mode(int argc, char** argv);

    //two input modes
    void DL_input(istream &is, int id, int time, int num_gen);

    void update_ambush(int planet, char side, int timestamp, int force);

    void update_attack(int planet, char side, int timestamp, int force);

    pair<int, int> read_input();

    //four output modes
    void verbose();

    void gen_eval();

    void med(int time);

    void watcher();

    void output();

private:

    //output modes
    bool verbose_on;
    bool watcher_on;

    //battle info
    int battles;
    int timestamp;

    //containers
    vector<planet> nebula;
    vector<general> commanders;
    battle deployment;

};
#endif