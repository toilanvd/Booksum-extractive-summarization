#include <bits/stdc++.h>
#define fi first
#define se second

using namespace std;

typedef pair<int, int> ii;
typedef pair<ii, int> iii;

const double EPS = 0.000001;
const int INF = 1e9;

int num_sentences, summary_length_limit;
vector<string> chapter_sentences_words[7000];
int sentences_length[7000];
map<string, int> string_cnt;

// GA hyperparameters
const double TEMPERATURE_START = 100;
const double TEMPERATURE_END = 0;
const double TEMPERATURE_DEC = 1;
const int LOOP_TIME = 100;
const int INDIVIDUAL_SWAP_CANDIDATE = 100;
const int POTENTIAL_CANDIDATE_MULTIPLIER = 3;

struct solution{
    int profit;
    int cost;
    vector<int> sentences;
    vector<int> unused_sentences;
};

solution best_solution, current_solution;
vector<solution> candidates;

string normalize(string doc){
    string res = "";
    for(int i = 0; i < (int)doc.size(); i++){
        if(doc[i] >= 'A' && doc[i] <= 'Z') res += (char)(doc[i] - 'A' + 'a');
        else if(doc[i] >= 'a' && doc[i] <= 'z') res += doc[i];
        else if(doc[i] >= '0' && doc[i] <= '9') res += doc[i];
    }
    return res;
}

vector<string> split_string(string doc){
    vector<string> res;
    string current_string = "";
    for(int i = 0; i < (int)doc.size(); i++){
        if(doc[i] == ' '){
            current_string = normalize(current_string);
            if(current_string != "") res.push_back(current_string);
            current_string = "";
        }
        else current_string += doc[i];
    }
    if(current_string != ""){
        current_string = normalize(current_string);
        if(current_string != "") res.push_back(current_string);
    }
    return res;
}

bool compare_solutions(solution sol_A, solution sol_B){ // Return true if solution A is better than solution B
    if(sol_A.profit > sol_B.profit) return true; // profit of A > profit of B
    else if(sol_A.profit == sol_B.profit && sol_A.cost < sol_B.cost) return true;
    else return false;
}

void initialize_solution(solution *res){
    (*res).cost = 0;
    (*res).profit = 0;
    vector<int> temp_vec;
    (*res).sentences = temp_vec;
    (*res).unused_sentences = temp_vec;
    for(int i = 0; i < num_sentences; i++) (*res).unused_sentences.push_back(i);
}

int calculate_profit(vector<int> sentences_id){
    map<string, bool> checked;
    int profit = 0;
    for(int i = 0; i < (int)sentences_id.size(); i++){
        int id = sentences_id[i];
        for(int j = 0; j < (int)chapter_sentences_words[id].size(); j++){
            string one_word = chapter_sentences_words[id][j];
            if(checked.find(one_word) == checked.end()) profit += (string_cnt[one_word] - 1);
            checked[one_word] = true;
            if(j == (int)chapter_sentences_words[id].size() - 1) break;
            string two_words = chapter_sentences_words[id][j] + " " + chapter_sentences_words[id][j + 1];
            if(checked.find(two_words) == checked.end()) profit += (string_cnt[two_words] - 1) * 2;
            checked[two_words] = true;
        }
    }
    return profit;
}

solution build_random_solution(){
    solution res;
    initialize_solution(&res);

    int len = rand() % summary_length_limit + 1;
    while(res.cost < len){
        int i = rand() % (int)res.unused_sentences.size();
        int id = res.unused_sentences[i];
        res.unused_sentences.erase(res.unused_sentences.begin() + i);
        res.sentences.push_back(id);
        res.cost += sentences_length[id];
    }
    res.profit = calculate_profit(res.sentences);
    
    return res;
}

void update_best_solution(solution sol){
    if(compare_solutions(sol, best_solution)) best_solution = sol;
}

int main(){
    srand(time(NULL));
    ios_base::sync_with_stdio(false);
    ifstream inputFile("MCS-SA-One-and-Two-Words-input.txt");
    inputFile >> num_sentences >> summary_length_limit;
    for(int i = 0; i < num_sentences; i++) inputFile >> sentences_length[i];
    string temp_string; getline(inputFile, temp_string);
    for(int i = 0; i < num_sentences; i++){
        getline(inputFile, temp_string);
        chapter_sentences_words[i] = split_string(temp_string);
        for(int j = 0; j < (int)chapter_sentences_words[i].size(); j++){
            string one_word = chapter_sentences_words[i][j];
            if(string_cnt.find(one_word) == string_cnt.end()) string_cnt[one_word] = 0;
            string_cnt[one_word]++;
            if(j == (int)chapter_sentences_words[i].size() - 1) break;
            string two_words = chapter_sentences_words[i][j] + " " + chapter_sentences_words[i][j + 1];
            if(string_cnt.find(two_words) == string_cnt.end()) string_cnt[two_words] = 0;
            string_cnt[two_words]++;
        }
    }
    inputFile.close();

    current_solution = build_random_solution();
    best_solution = current_solution;
    
    double T = TEMPERATURE_START;
    while(T > TEMPERATURE_END + EPS){
        for(int timer = 1; timer <= LOOP_TIME; timer++){
            //cout << T << " " << timer << ": " << best_solution.profit << endl;
            candidates.clear();
            // Remove a sentence
            if(!current_solution.sentences.empty()){
                for(int i = 0; i < (int)current_solution.sentences.size(); i++){
                    solution neighbor = current_solution;
                    int id = neighbor.sentences[i];
                    neighbor.cost -= sentences_length[id];
                    neighbor.sentences.erase(neighbor.sentences.begin() + i);
                    neighbor.unused_sentences.push_back(id);
                    neighbor.profit = calculate_profit(neighbor.sentences);
                    candidates.push_back(neighbor);
                    update_best_solution(neighbor);
                }
            }
            // Add a sentence
            if(current_solution.cost < summary_length_limit){
                for(int i = 0; i < (int)current_solution.unused_sentences.size(); i++){
                    solution neighbor = current_solution;
                    int id = neighbor.unused_sentences[i];
                    neighbor.cost += sentences_length[id];
                    neighbor.sentences.push_back(id);
                    neighbor.unused_sentences.erase(neighbor.unused_sentences.begin() + i);
                    neighbor.profit = calculate_profit(neighbor.sentences);
                    candidates.push_back(neighbor);
                    update_best_solution(neighbor);
                }
            }
            // Swap a sentence to another sentence
            if(!current_solution.unused_sentences.empty()){
                for(int i = 0; i < (int)current_solution.sentences.size(); i++){
                    int id_1 = current_solution.sentences[i];
                    if(current_solution.cost - sentences_length[id_1] >= summary_length_limit) continue;
                    for(int swap_candidate_timer = 1; swap_candidate_timer <= INDIVIDUAL_SWAP_CANDIDATE; swap_candidate_timer++){
                        solution neighbor = current_solution;
                        int j = rand() % (int)current_solution.unused_sentences.size();
                        int id_2 = current_solution.unused_sentences[j];
                        neighbor.cost = current_solution.cost - sentences_length[id_1] + sentences_length[id_2];
                        neighbor.sentences.erase(neighbor.sentences.begin() + i);
                        neighbor.sentences.push_back(id_2);
                        neighbor.unused_sentences.erase(neighbor.unused_sentences.begin() + j);
                        neighbor.unused_sentences.push_back(id_1);
                        neighbor.profit = calculate_profit(neighbor.sentences);
                        update_best_solution(neighbor);
                    }
                }
            }
            // Sort candidates and choose one
            sort(candidates.begin(), candidates.end(), compare_solutions);
            int chosen_neighbor_id = rand() % min((int)candidates.size(), POTENTIAL_CANDIDATE_MULTIPLIER * num_sentences);
            // Update current solution
            if(compare_solutions(candidates[chosen_neighbor_id], current_solution)) current_solution = candidates[chosen_neighbor_id];
            else if((double)(rand() % (int)TEMPERATURE_START + 1) <= T + EPS) current_solution = candidates[chosen_neighbor_id];
        }
        T -= TEMPERATURE_DEC;
    }

    // Print result to file
    ofstream outputFile("MCS-SA-One-and-Two-Words-output.txt");
    outputFile << (int)best_solution.sentences.size() << '\n';
    for(int i = 0; i < (int)best_solution.sentences.size(); i++) outputFile << best_solution.sentences[i] << '\n';
    outputFile << best_solution.profit << '\n';
    outputFile << best_solution.cost << '\n';
    outputFile.close();

    return 0;
}