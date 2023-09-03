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
map<string, bool> is_stopword;

// GA hyperparameters
const int POPULATION_SIZE = 100;
const int G_BEST = 10;
const int G_SUM_PROFIT = 10;

struct solution{
    int profit;
    int cost;
    vector<int> sentences;
    vector<int> unused_sentences;
};

vector<solution> population, new_population;
solution best_solution;
int sum_profit_of_current_population;
int best_solution_not_changed_cnt, sum_profit_not_increase_cnt;

string normalize(string doc){
    string res = "";
    for(int i = 0; i < (int)doc.size(); i++){
        if(doc[i] >= 'A' && doc[i] <= 'Z') res += (char)(doc[i] - 'A' + 'a');
        else if(doc[i] >= 'a' && doc[i] <= 'z') res += doc[i];
        else if(doc[i] >= '0' && doc[i] <= '9') res += doc[i];
    }
    return res;
}

void build_stopword_list(){
    ifstream stopwordFile("stopwords.txt");
    string stopword_list;
    getline(stopwordFile, stopword_list);
    stopwordFile.close();
    string current_word = "";
    for(int i = 0; i < (int)stopword_list.size(); i++){
        if(stopword_list[i] == ','){
            current_word = normalize(current_word);
            if(current_word != "") is_stopword[current_word] = true;
            current_word = "";
        }
        else current_word += stopword_list[i];
    }
    if(current_word != ""){
        current_word = normalize(current_word);
        if(current_word != "") is_stopword[current_word] = true;
    }
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
            if(is_stopword.find(one_word) != is_stopword.end()) continue;
            if(checked.find(one_word) == checked.end()) profit += (string_cnt[one_word] - 1);
            checked[one_word] = true;
            if(j == (int)chapter_sentences_words[id].size() - 1) break;
            if(is_stopword.find(chapter_sentences_words[id][j + 1]) != is_stopword.end()) continue;
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

solution crossover(solution sol_A, solution sol_B){
    solution res;
    initialize_solution(&res);

    vector<bool> available_sentences(num_sentences, false);
    for(int i = 0; i < (int)sol_A.sentences.size(); i++) available_sentences[sol_A.sentences[i]] = true;
    for(int i = 0; i < (int)sol_B.sentences.size(); i++) available_sentences[sol_B.sentences[i]] = true;
    int available_sentences_cnt = 0;
    for(int i = 0; i < num_sentences; i++) if(available_sentences[i]) available_sentences_cnt++;
    //int len = rand() % summary_length_limit + 1;
    int len = summary_length_limit;
    while(res.cost < len && available_sentences_cnt > 0){
        int pos = rand() % available_sentences_cnt + 1;
        for(int i = 0; i < num_sentences; i++){
            if(available_sentences[i]) pos--;
            if(pos == 0){
                available_sentences[i] = false;
                available_sentences_cnt--;
                for(int j = 0; j < (int)res.unused_sentences.size(); j++){
                    if(res.unused_sentences[j] == i){
                        res.unused_sentences.erase(res.unused_sentences.begin() + j);
                        break;
                    }
                }
                res.sentences.push_back(i);
                res.cost += sentences_length[i];
                break;
            }
        }
    }
    res.profit = calculate_profit(res.sentences);

    return res;
}

int main(){
    srand(time(NULL));
    ios_base::sync_with_stdio(false);
    ifstream inputFile("MCS-GA-One-and-Two-Words-noStopword-input.txt");
    ofstream outputFile("MCS-GA-One-and-Two-Words-noStopword-output.txt");
    
    build_stopword_list();
    inputFile >> num_sentences >> summary_length_limit;
    for(int i = 0; i < num_sentences; i++) inputFile >> sentences_length[i];
    string temp_string; getline(inputFile, temp_string);
    for(int i = 0; i < num_sentences; i++){
        getline(inputFile, temp_string);
        chapter_sentences_words[i] = split_string(temp_string);
        for(int j = 0; j < (int)chapter_sentences_words[i].size(); j++){
            string one_word = chapter_sentences_words[i][j];
            if(is_stopword.find(one_word) != is_stopword.end()) continue;
            if(string_cnt.find(one_word) == string_cnt.end()) string_cnt[one_word] = 0;
            string_cnt[one_word]++;
            if(j == (int)chapter_sentences_words[i].size() - 1) break;
            if(is_stopword.find(chapter_sentences_words[i][j + 1]) != is_stopword.end()) continue;
            string two_words = chapter_sentences_words[i][j] + " " + chapter_sentences_words[i][j + 1];
            if(string_cnt.find(two_words) == string_cnt.end()) string_cnt[two_words] = 0;
            string_cnt[two_words]++;
        }
    }

    // Genetic algorithm
    // Initialize population
    for(int i = 1; i <= POPULATION_SIZE; i++){
        solution temp_sol = build_random_solution();
        population.push_back(temp_sol);
        update_best_solution(temp_sol);
        sum_profit_of_current_population += temp_sol.profit;
    }
    sort(population.begin(), population.end(), compare_solutions);
    // Genetic process
    int generation_cnt = 0;
    while(best_solution_not_changed_cnt < G_BEST || sum_profit_not_increase_cnt < G_SUM_PROFIT){
        solution old_best_solution = best_solution;

        new_population = population;
        vector<int> sum_len(POPULATION_SIZE, 0);
        sum_len[0] = population[0].profit;
        for(int i = 1; i < POPULATION_SIZE; i++) sum_len[i] = sum_len[i - 1] + population[i].profit;

        solution new_sol;
        for(int i = 0; i < POPULATION_SIZE; i++){
            // Crossover
            int x = (int)((double)(rand() % 10001) / 10000.0 * (double)sum_len[POPULATION_SIZE - 1]);
            int l = 0, r = POPULATION_SIZE - 1;
            while(l < r){
                int mid = (l + r) / 2;
                if(sum_len[mid] >= x) r = mid;
                else l = mid + 1;
            }
            new_sol = crossover(population[i], population[l]);
            new_population.push_back(new_sol);
            update_best_solution(new_sol);
            // Mutation
            // Remove a sentence
            if(!population[i].sentences.empty()){
                new_sol = population[i];
                int pos = rand() % (int)population[i].sentences.size();
                int id = population[i].sentences[pos];
                new_sol.unused_sentences.push_back(id);
                new_sol.cost -= sentences_length[id];
                new_sol.sentences.erase(new_sol.sentences.begin() + pos);
                new_sol.profit = calculate_profit(new_sol.sentences);
                new_population.push_back(new_sol);
                update_best_solution(new_sol);
            }
            // Add a sentence
            if(!population[i].unused_sentences.empty() && population[i].cost < summary_length_limit){
                new_sol = population[i];
                int pos = rand() % (int)population[i].unused_sentences.size();
                int id = population[i].unused_sentences[pos];
                new_sol.sentences.push_back(id);
                new_sol.cost += sentences_length[id];
                new_sol.unused_sentences.erase(new_sol.unused_sentences.begin() + pos);
                new_sol.profit = calculate_profit(new_sol.sentences);
                new_population.push_back(new_sol);
                update_best_solution(new_sol);
            }
            // Swap a sentence with another unused sentence
            if(!population[i].sentences.empty() && !population[i].unused_sentences.empty()){
                int pos_1 = rand() % (int)population[i].sentences.size();
                int pos_2 = rand() % (int)population[i].unused_sentences.size();
                int id_1 = population[i].sentences[pos_1];
                int id_2 = population[i].unused_sentences[pos_2];
                if(population[i].cost - sentences_length[id_1] >= summary_length_limit) continue;
                new_sol = population[i];
                new_sol.cost = new_sol.cost - sentences_length[id_1] + sentences_length[id_2];
                new_sol.sentences.erase(new_sol.sentences.begin() + pos_1);
                new_sol.sentences.push_back(id_2);
                new_sol.unused_sentences.erase(new_sol.unused_sentences.begin() + pos_2);
                new_sol.unused_sentences.push_back(id_1);
                new_sol.profit = calculate_profit(new_sol.sentences);
                new_population.push_back(new_sol);
                update_best_solution(new_sol);
            }
        }
        // Selection
        sort(new_population.begin(), new_population.end(), compare_solutions);
        while((int)new_population.size() > POPULATION_SIZE) new_population.pop_back();
        if(!compare_solutions(best_solution, old_best_solution)) best_solution_not_changed_cnt++;
        else best_solution_not_changed_cnt = 0;
        int new_sum_profit_of_population = 0;
        for(int i = 0; i < POPULATION_SIZE; i++) new_sum_profit_of_population += new_population[i].profit;
        if(new_sum_profit_of_population <= sum_profit_of_current_population) sum_profit_not_increase_cnt++;
        else{
            sum_profit_not_increase_cnt = 0;
            sum_profit_of_current_population = new_sum_profit_of_population;
        }
        population = new_population;
        //generation_cnt++; cout << generation_cnt << ": " << best_solution_not_changed_cnt << " " << sum_profit_not_increase_cnt << " | " << best_solution.profit << " " << sum_profit_of_current_population/POPULATION_SIZE << endl;
    }

    // Print result to file
    outputFile << (int)best_solution.sentences.size() << '\n';
    for(int i = 0; i < (int)best_solution.sentences.size(); i++) outputFile << best_solution.sentences[i] << '\n';
    outputFile << best_solution.profit << '\n';
    outputFile << best_solution.cost << '\n';

    inputFile.close();
    outputFile.close();

    return 0;
}