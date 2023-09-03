#include <bits/stdc++.h>
#define fi first
#define se second

using namespace std;

typedef pair<int, int> ii;
typedef pair<ii, int> iii;

const double EPS = 0.000001;

int num_sentences, summary_length_limit;
double alpha;
int sentences_length[7000];
bool cover_mask[7000][7000];

// SA hyperparameters
const double TEMPERATURE_START = 100;
const double TEMPERATURE_END = 0;
const double TEMPERATURE_DEC = 1;
const int LOOP_TIME = 100;
const int INDIVIDUAL_SWAP_CANDIDATE = 100;
const int POTENTIAL_CANDIDATE_MULTIPLIER = 3;

struct candidate_modification{
    int type; // 0: Remove, 1: Add, 2: Swap
    int id_1, id_2; // id_2 is only used if type = 2
};

vector<int> current_solution, unused_sentences;
int current_cost, current_profit;
vector<int> best_solution;
int best_cost, best_profit;
int cover_cnt[7000];
vector<vector<int> > candidates, candidates_unused_sentences;
vector<candidate_modification> candidate_mods;
vector<iii> candidates_eval;

bool compare_solutions(ii solution_A_profit_cost, ii solution_B_profit_cost){ // Return true if solution A is not worse than solution B
    if(solution_A_profit_cost.fi > solution_B_profit_cost.fi) return true; // profit of A > profit of B
    else if(solution_A_profit_cost.fi == solution_B_profit_cost.fi && solution_A_profit_cost.se >= solution_B_profit_cost.se) return true;
    else return false;
}

void build_greedy_initial_solution(){
    while(current_cost < summary_length_limit){
        int chosen_i = -1;
        int chosen_profit = -1;
        for(int i = 0; i < (int)unused_sentences.size(); i++){
            int temp_profit = 0;
            for(int j = 0; j < num_sentences; j++){
                if(cover_cnt[j] == 0 && cover_mask[unused_sentences[i]][j] == true){
                    temp_profit += sentences_length[j];
                }
            }
            if(temp_profit > chosen_profit){
                chosen_profit = temp_profit;
                chosen_i = i;
            }
        }
        int id = unused_sentences[chosen_i];
        unused_sentences.erase(unused_sentences.begin() + chosen_i);
        current_solution.push_back(id);
        current_cost += sentences_length[id];
        current_profit += chosen_profit;
        for(int i = 0; i < num_sentences; i++) if(cover_mask[id][i]) cover_cnt[i]++;
    }
}

int main(){
    srand(time(NULL));
    ios_base::sync_with_stdio(false);
    ifstream inputFile("MCS-SA-BERT-input.txt");
    ofstream outputFile("MCS-SA-BERT-output.txt");
    
    inputFile >> num_sentences >> summary_length_limit >> alpha;
    for(int i = 0; i < num_sentences; i++) inputFile >> sentences_length[i];
    for(int i = 0; i < num_sentences; i++){
        for(int j = 0; j < num_sentences; j++){
            double x; inputFile >> x;
            if(x + EPS >= alpha) cover_mask[i][j] = true;
        }
    }

    // Simulated annealing algorithm
    for(int i = 0; i < num_sentences; i++) unused_sentences.push_back(i);
    build_greedy_initial_solution();
    /*
    for(int i = 0; i < num_sentences && current_cost < summary_length_limit; i++){
        current_solution.push_back(i);
        unused_sentences.erase(unused_sentences.begin());
        current_cost += sentences_length[i];
        for(int j = 0; j < num_sentences; j++) if(cover_mask[i][j]) cover_cnt[j]++;
    }
    for(int i = 0; i < num_sentences; i++) if(cover_cnt[i] > 0) current_profit += sentences_length[i];
    */
    best_solution = current_solution; best_cost = current_cost; best_profit = current_profit;
    //outputFile << current_profit << " " << current_cost << endl;

    double T = TEMPERATURE_START;
    while(T > TEMPERATURE_END + EPS){
        for(int timer = 1; timer <= LOOP_TIME; timer++){
            //outputFile << "T = " << T << ", timer = " << timer << ", current profit = " << current_profit << ", current cost = " << current_cost << endl;
            candidates.clear();
            candidates_unused_sentences.clear();
            candidates_eval.clear();
            candidate_mods.clear();
            // Remove a sentence
            if(!current_solution.empty()){
                for(int i = 0; i < (int)current_solution.size(); i++){
                    vector<int> neighbor = current_solution;
                    vector<int> neighbor_unused_sentences = unused_sentences;
                    int id = neighbor[i];
                    int neighbor_profit = current_profit;
                    int neighbor_cost = current_cost - sentences_length[id];
                    for(int j = 0; j < num_sentences; j++){
                        if(cover_mask[id][j] == true && cover_cnt[j] == 1){
                            neighbor_profit -= sentences_length[j];
                        }
                    }
                    neighbor.erase(neighbor.begin() + i);
                    candidates.push_back(neighbor);
                    neighbor_unused_sentences.push_back(id);
                    candidates_unused_sentences.push_back(neighbor_unused_sentences);
                    candidates_eval.push_back(iii(ii(neighbor_profit, neighbor_cost), (int)candidates_eval.size()));
                    candidate_modification modification;
                    modification.type = 0;
                    modification.id_1 = id;
                    candidate_mods.push_back(modification);
                    //outputFile << "Remove case: " << neighbor_profit << " " << neighbor_cost << endl;
                }
            }
            // Add a sentence
            if(current_cost < summary_length_limit){
                for(int i = 0; i < (int)unused_sentences.size(); i++){
                    vector<int> neighbor = current_solution;
                    vector<int> neighbor_unused_sentences = unused_sentences;
                    int id = unused_sentences[i];
                    int neighbor_profit = current_profit;
                    int neighbor_cost = current_cost + sentences_length[id];
                    for(int j = 0; j < num_sentences; j++){
                        if(cover_mask[id][j] == true && cover_cnt[j] == 0){
                            neighbor_profit += sentences_length[j];
                        }
                    }
                    neighbor.push_back(id);
                    candidates.push_back(neighbor);
                    neighbor_unused_sentences.erase(neighbor_unused_sentences.begin() + i);
                    candidates_unused_sentences.push_back(neighbor_unused_sentences);
                    candidates_eval.push_back(iii(ii(neighbor_profit, neighbor_cost), (int)candidates_eval.size()));
                    candidate_modification modification;
                    modification.type = 1;
                    modification.id_1 = id;
                    candidate_mods.push_back(modification);
                    //outputFile << "Add case: " << neighbor_profit << " " << neighbor_cost << endl;
                }
            }
            // Swap a sentence to another sentence
            if(!unused_sentences.empty()){
                for(int i = 0; i < (int)current_solution.size(); i++){
                    int id_1 = current_solution[i];
                    if(current_cost - sentences_length[id_1] >= summary_length_limit) continue;
                    for(int swap_candidate_timer = 1; swap_candidate_timer <= INDIVIDUAL_SWAP_CANDIDATE; swap_candidate_timer++){
                        int j = rand() % (int)unused_sentences.size();
                        int id_2 = unused_sentences[j];
                        int neighbor_profit = current_profit;
                        int neighbor_cost = current_cost - sentences_length[id_1] + sentences_length[id_2];
                        for(int j = 0; j < num_sentences; j++){
                            if(cover_mask[id_1][j] == true && cover_mask[id_2][j] == false && cover_cnt[j] == 1){
                                neighbor_profit -= sentences_length[j];
                            }
                            if(cover_mask[id_1][j] == false && cover_mask[id_2][j] == true && cover_cnt[j] == 0){
                                neighbor_profit += sentences_length[j];
                            }
                        }
                        vector<int> neighbor = current_solution;
                        vector<int> neighbor_unused_sentences = unused_sentences;
                        neighbor.erase(neighbor.begin() + i);
                        neighbor.push_back(id_2);
                        candidates.push_back(neighbor);
                        neighbor_unused_sentences.erase(neighbor_unused_sentences.begin() + j);
                        neighbor_unused_sentences.push_back(id_1);
                        candidates_unused_sentences.push_back(neighbor_unused_sentences);
                        candidates_eval.push_back(iii(ii(neighbor_profit, neighbor_cost), (int)candidates_eval.size()));
                        candidate_modification modification;
                        modification.type = 2;
                        modification.id_1 = id_1;
                        modification.id_2 = id_2;
                        candidate_mods.push_back(modification);
                        //outputFile << "Swap case: " << neighbor_profit << " " << neighbor_cost << endl;
                    }
                }
            }
            // Sort candidates and choose one
            sort(candidates_eval.rbegin(), candidates_eval.rend());
            int chosen_neighbor_id = rand() % min((int)candidates_eval.size(), POTENTIAL_CANDIDATE_MULTIPLIER * num_sentences);
            // Update current solution
            if(compare_solutions(candidates_eval[chosen_neighbor_id].fi, ii(current_profit, current_cost))){
                current_solution = candidates[candidates_eval[chosen_neighbor_id].se];
                current_profit = candidates_eval[chosen_neighbor_id].fi.fi;
                current_cost = candidates_eval[chosen_neighbor_id].fi.se;
                unused_sentences = candidates_unused_sentences[candidates_eval[chosen_neighbor_id].se];
                for(int i = 0; i < num_sentences; i++){
                    if(candidate_mods[candidates_eval[chosen_neighbor_id].se].type == 0){
                        int id = candidate_mods[candidates_eval[chosen_neighbor_id].se].id_1;
                        if(cover_mask[id][i]) cover_cnt[i]--;
                    }
                    else if(candidate_mods[candidates_eval[chosen_neighbor_id].se].type == 1){
                        int id = candidate_mods[candidates_eval[chosen_neighbor_id].se].id_1;
                        if(cover_mask[id][i]) cover_cnt[i]++;
                    }
                    else{
                        int id_1 = candidate_mods[candidates_eval[chosen_neighbor_id].se].id_1;
                        int id_2 = candidate_mods[candidates_eval[chosen_neighbor_id].se].id_2;
                        if(cover_mask[id_1][i]) cover_cnt[i]--;
                        if(cover_mask[id_2][i]) cover_cnt[i]++;
                    }
                }
            }
            else if((double)(rand() % (int)TEMPERATURE_START + 1) <= T + EPS){
                current_solution = candidates[candidates_eval[chosen_neighbor_id].se];
                current_profit = candidates_eval[chosen_neighbor_id].fi.fi;
                current_cost = candidates_eval[chosen_neighbor_id].fi.se;
                unused_sentences = candidates_unused_sentences[candidates_eval[chosen_neighbor_id].se];
                for(int i = 0; i < num_sentences; i++){
                    if(candidate_mods[candidates_eval[chosen_neighbor_id].se].type == 0){
                        int id = candidate_mods[candidates_eval[chosen_neighbor_id].se].id_1;
                        if(cover_mask[id][i]) cover_cnt[i]--;
                    }
                    else if(candidate_mods[candidates_eval[chosen_neighbor_id].se].type == 1){
                        int id = candidate_mods[candidates_eval[chosen_neighbor_id].se].id_1;
                        if(cover_mask[id][i]) cover_cnt[i]++;
                    }
                    else{
                        int id_1 = candidate_mods[candidates_eval[chosen_neighbor_id].se].id_1;
                        int id_2 = candidate_mods[candidates_eval[chosen_neighbor_id].se].id_2;
                        if(cover_mask[id_1][i]) cover_cnt[i]--;
                        if(cover_mask[id_2][i]) cover_cnt[i]++;
                    }
                }
            }
            // Update best solution
            if(compare_solutions(ii(current_profit, current_cost), ii(best_profit, best_cost))){
                best_solution = current_solution;
                best_profit = current_profit;
                best_cost = current_cost;
            }
        }
        T -= TEMPERATURE_DEC;
    }

    // Print result to file
    outputFile << (int)best_solution.size() << '\n';
    for(int i = 0; i < (int)best_solution.size(); i++) outputFile << best_solution[i] << '\n';
    outputFile << best_profit << '\n';
    outputFile << best_cost << '\n';

    inputFile.close();
    outputFile.close();

    return 0;
}