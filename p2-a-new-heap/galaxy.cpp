// Identifier: AD48FB4835AF347EB0CA8009E24C3B13F8519882
#include <getopt.h>
#include <string>
#include <cstring>
#include "galaxy.h"
#include "P2random.h" 
using namespace std;

int CURRENT_TIMESTAMP = 0;

//record output modes
bool median_on = false;
bool gen_eval_on = false;
string input_mode;

void galaxy::get_mode(int argc, char** argv){
    int option_index = 0, option = 0;
    
    // Don't display getopt error messages about options
    opterr = false;

    struct option longOpts[] = {{ "verbose", no_argument, nullptr, 'v' },
                                { "median", no_argument, nullptr, 'm' },
                                { "general-eval", no_argument, nullptr, 'g' },
                                { "watcher", no_argument, nullptr, 'w' },
                                { nullptr, 0, nullptr, '\0' }};
    

    while ((option = getopt_long(argc, argv, "vmgw", longOpts, &option_index)) != -1){
        switch (option){
            case 'v':
                verbose_on = true;
                break;

            case 'm':
                median_on = true;
                break;

            case 'g':
                gen_eval_on = true;
                break;

            case 'w':
                watcher_on = true;
                break;

            default:
                cerr << "Unknown command line option\n";
                exit(1);
        }
    }
}

bool galaxy::targeted(int planet){
    if(nebula[planet].rebellion.empty() || nebula[planet].empire.empty()) return false;
    //cout << nebula[planet].rebellion.size() << endl;
    const battalion & jedi = nebula[planet].rebellion.top();
    const battalion & sith = nebula[planet].empire.top();
    if(sith.force >= jedi.force)    return true;
    else                            return false;
}


void galaxy::engage(){

    for(int i = 0; i < (int)nebula.size(); ++i){
        while(targeted(i)){
            int loss;
            //cout << nebula[i].rebellion.size() << endl;
            battalion jedi = nebula[i].rebellion.top();
            battalion sith = nebula[i].empire.top();
            nebula[i].rebellion.pop();
            nebula[i].empire.pop();

            if(jedi.troops > sith.troops){
                loss = sith.troops;
                jedi.troops -= loss;
                if(jedi.troops != 0){
                    nebula[i].rebellion.push(jedi);
                }
                //cout << nebula[i].rebellion.top().force << " "  << nebula[i].rebellion.top().troops << endl;
            }
            else{
                loss = jedi.troops;
                sith.troops -= loss;
                if(sith.troops != 0){
                    nebula[i].empire.push(sith);
                }
                //cout << nebula[i].empire.top().force << " "  << nebula[i].empire.top().troops << endl;
            }

            deployment = battle{sith.general, jedi.general, i, 2 * loss};
            nebula[i].push(2 * loss);
            ++battles;
            if(verbose_on)  verbose();
            if(gen_eval_on){
                commanders[jedi.general].died += loss;
                commanders[sith.general].died += loss;
            }
            
        }
    }
}

void galaxy::planet::push(int loss){

    if (lower_half.empty() || loss < lower_half.top()) {
        lower_half.push(loss);
    } 
    else {
        upper_half.push(loss);
    }
    int max_size = (int)lower_half.size();
    int min_size = (int)upper_half.size();
    if (max_size - min_size > 1) {
        int top = lower_half.top();
        lower_half.pop();
        upper_half.push(top);
    } 
    else if (min_size - max_size > 1) {
        int top = upper_half.top();
        upper_half.pop();
        lower_half.push(top);
    }

}

int galaxy::planet::get_med() const{

    int max_size = (int)lower_half.size();
    int min_size = (int)upper_half.size();
    if (max_size == min_size) {
        return (lower_half.top() + upper_half.top()) / 2;
    } 
    else if (max_size > min_size) {
        return lower_half.top();
    } 
    else {
        return upper_half.top();
    }
}

void galaxy::update_ambush(int planet, char side, int timestamp, int force){

    Watcher &curr = nebula[planet].ambush;
    if(curr.s == State::Initial){
        if(side == 's'){
            curr.s = State::SeenOne;        //SEEN SITH
            curr.best_sith = force;
            curr.sith_ts = timestamp;
        }
    }
    else if(curr.s == State::SeenOne){
        if(side == 's'){
            if(force > curr.best_sith){     //update best sith
                curr.best_sith = force;     
                curr.sith_ts = timestamp;
            }  
        }
        else{
            if(force <= curr.best_sith){     //SEEN JEDI
                curr.s = State::SeenBoth;
                curr.best_jedi = force;
                curr.jedi_ts = timestamp;
            }
        }
    }
    else if(curr.s == State::SeenBoth){
        if(side == 's'){
            if(force > curr.best_sith){     //record maybe and transit to MAYBEBETTER
                curr.s = State::MaybeBetter;    
                curr.maybe_force = force;   //maybe is a sith
                curr.maybe_ts = timestamp;
            }
        }
        else{
            if(force < curr.best_jedi){     //update best jedi
                curr.best_jedi = force;
                curr.jedi_ts = timestamp;
            }
        }
    }
    else{                                   //MAYBEBETTER
        if(side == 'j'){
            int diff = curr.maybe_force - force;
            if(diff > (curr.best_sith - curr.best_jedi)){   //if a better pair appears
                curr.s = State::SeenBoth;                   //back to SEENBOTH
                if(curr.best_sith != curr.maybe_force){
                    curr.best_sith = curr.maybe_force;          //update both best jedi and best sith
                    curr.sith_ts = curr.maybe_ts;
                }
                curr.best_jedi = force;
                curr.jedi_ts = timestamp;
            }
        }
        else{
            if(force > curr.maybe_force){     //update best maybe
                curr.maybe_force = force;     
                curr.maybe_ts = timestamp;
            }
        }
    }
}

void galaxy::update_attack(int planet, char side, int timestamp, int force){

    Watcher &curr = nebula[planet].attack;
    if(curr.s == State::Initial){
        if(side == 'j'){
            curr.s = State::SeenOne;        //SEEN JEDI
            curr.best_jedi = force;
            curr.jedi_ts = timestamp;
        }
    }
    else if(curr.s == State::SeenOne){
        if(side == 'j'){
            if(force < curr.best_jedi){     //update best jedi
                curr.best_jedi = force;     
                curr.jedi_ts = timestamp;
            }  
        }
        else{
            if(force >= curr.best_jedi){     //SEEN SITH
                curr.s = State::SeenBoth;
                curr.best_sith = force;
                curr.sith_ts = timestamp;
            }
        }
    }
    else if(curr.s == State::SeenBoth){
        if(side == 'j'){
            if(force < curr.best_jedi){     //record maybe and transit to MAYBEBETTER
                curr.s = State::MaybeBetter;    
                curr.maybe_force = force;   //maybe is a jedi
                curr.maybe_ts = timestamp;
            }
        }
        else{
            if(force > curr.best_sith){     //update best sith
                curr.best_sith = force;
                curr.sith_ts = timestamp;
            }
        }
    }
    else{                                   //MAYBEBETTER
        if(side == 's'){
            int diff = force - curr.maybe_force;
            if(diff > (curr.best_sith - curr.best_jedi)){   //if a better pair appears
                curr.s = State::SeenBoth;                   //back to SEENBOTH
                if(curr.best_jedi != curr.maybe_force){
                    curr.best_jedi = curr.maybe_force;          //update both best jedi and best sith
                    curr.jedi_ts = curr.maybe_ts;
                }
                curr.best_sith = force;
                curr.sith_ts = timestamp;
            }
        }
        else{
            if(force < curr.maybe_force){     //update best maybe
                curr.maybe_force = force;     
                curr.maybe_ts = timestamp;
            }
        }
    }
}

pair<int, int> galaxy::read_input(){

    string temp;
    int num_gen;
    int num_plan;

    getline(cin, temp);    //skipping comment
    cin >> temp;           //mode:
    cin >> temp;           //DL or PR
    if(temp == "DL")    input_mode = "DL";
    else                input_mode = "PR";

    cin >> temp;    //NUM_GENERALS:
    cin >> num_gen;
    if(gen_eval_on) commanders.resize(num_gen);

    cin >> temp;    //NUM_PLANETS:
    cin >> num_plan;
    nebula.resize(num_plan);

    return pair<int, int> {num_gen, num_plan};

}

void galaxy::DL_input(istream &is, int id, int time, int num_gen){

    char next;
    string temp;
    int gen_id;
    int planet;
    int force;
    int troop;

    is >> temp;        //JEDI/SITH
    //cout << temp << endl;
    
    is >> next;        //G
    is >> gen_id;       
    if(gen_id < 0 || gen_id > num_gen - 1){
        cerr << "Invalid general ID\n";
        exit(1);
    }

    is >> next;        //P
    is >> planet;      
    if(planet < 0 || planet > (int)nebula.size() - 1){
        cerr << "Invalid planet ID\n";
        exit(1);
    }  

    is >> next;        //F
    is >> force;
    if(force <= 0){
        cerr << "Invalid force sensntivity level\n";
        exit(1);
    }  

    is >> next;        //#
    is >> troop;
    if(troop <= 0){
        cerr << "Invalid number of troops\n";
        exit(1);
    }

    //cout << gen_id << " " << planet << " " << force << " " << troop << endl;

    if(temp == "JEDI"){
        nebula[planet].rebellion.push(battalion{id, gen_id, force, troop});
        if(gen_eval_on)
            commanders[gen_id].jedi += troop;
        if(watcher_on){
            update_ambush(planet, 'j', time, force);
            update_attack(planet, 'j', time, force);
        }
    }
    else{
        nebula[planet].empire.push(battalion{id, gen_id, force, troop});
        if(gen_eval_on)
            commanders[gen_id].sith += troop;
        if(watcher_on){
            update_ambush(planet, 's', time, force);
            update_attack(planet, 's', time, force);
        }
    }
}

void galaxy::verbose(){

    if(deployment.loss == 0){
        return;
    }
    else{
        cout << "General " << deployment.sith_gen << "'s battalion attacked General "
                << deployment.jedi_gen << "'s battalion on planet " << deployment.planet << ". "
                << deployment.loss << " troops were lost.\n";
    }
}

void galaxy::gen_eval(){

    cout << "---General Evaluation---\n"; 
    for(int i = 0; i < (int)commanders.size(); ++i){
        general &temp = commanders[i];
        int sum = temp.jedi + temp.sith;
        cout << "General " << i << " deployed " << temp.jedi << " Jedi troops and "
             << temp.sith << " Sith troops, and " << (sum - temp.died) << "/"
             << sum << " troops survived.\n";
    }
}

void galaxy::med(int time){

    for(int i = 0; i < (int)nebula.size(); ++i){
        if(nebula[i].lower_half.empty() && nebula[i].upper_half.empty()){
            continue;
        }
        cout << "Median troops lost on planet " << i << " at time " 
             << time << " is " << nebula[i].get_med() << ".\n";
    }
}

void galaxy::watcher(){
    
    cout << "---Movie Watcher---\n";
    for(int i = 0; i < (int)nebula.size(); ++i){
        if(nebula[i].ambush.s == State::Initial || nebula[i].ambush.s == State::SeenOne){
            cout << "A movie watcher would not see an interesting ambush on planet " << i << ".\n";
        }
        else{
            cout << "A movie watcher would enjoy an ambush on planet " << i 
                 << " with Sith at time " << nebula[i].ambush.sith_ts
                 << " and Jedi at time " << nebula[i].ambush.jedi_ts
                 << " with a force difference of " 
                 <<  nebula[i].ambush.best_sith -  nebula[i].ambush.best_jedi << ".\n";
        }

        if(nebula[i].attack.s == State::Initial || nebula[i].attack.s == State::SeenOne){
            cout << "A movie watcher would not see an interesting attack on planet " << i << ".\n";
        }
        else{
            cout << "A movie watcher would enjoy an attack on planet " << i 
                 << " with Jedi at time " << nebula[i].attack.jedi_ts
                 << " and Sith at time " << nebula[i].attack.sith_ts
                 << " with a force difference of " 
                 <<  nebula[i].attack.best_sith -  nebula[i].attack.best_jedi << ".\n";
        }
        
    }
}

void galaxy::output(){

    //Summary 
    cout << "---End of Day---\n";
    cout << "Battles: " << battles << "\n";
    //watcher
    if(gen_eval_on) gen_eval();
    if(watcher_on)  watcher();
    
}


int main(int argc, char** argv){

    ios_base::sync_with_stdio(false); //turn off sync
    galaxy g;
    int time;
    int id = 0;
    stringstream ss;
    g.get_mode(argc, argv);

    //Startup
    cout << "Deploying troops...\n";
    pair<int, int> info = g.read_input();

    if(input_mode == "PR"){
        string temp;
        int seed;
        int num_deploys;
        int rate;

        cin >> temp >> seed >> temp >> num_deploys >> temp >> rate;
        P2random::PR_init(ss, seed, info.first, info.second, num_deploys, rate);
    }

    istream &inputStream = input_mode == "PR" ? ss : cin;
    
    while(inputStream >> time){         //read in timestamps
        //check invalid timestamp
        if(time < CURRENT_TIMESTAMP){
            cerr << "Invalid decreasing timestamp\n";
            exit(1);
        }

        //takes in one line of deployment
        g.DL_input(inputStream, id, time, info.first);
        ++id;

        //update timestamp if necessary
        if(CURRENT_TIMESTAMP < time){
            //print median
            if(median_on)   g.med(CURRENT_TIMESTAMP);

            //update timestamp
            CURRENT_TIMESTAMP = time;
            //cout << CURRENT_TIMESTAMP << endl;
        }
        //start battles
        g.engage();

    }

    if(median_on)   g.med(CURRENT_TIMESTAMP);
    g.output();

}