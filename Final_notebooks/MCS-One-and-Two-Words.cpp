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
vector<int> best_solution;
map<string, int> string_cnt;
map<string, bool> checked_word;
bool used_sentence[7000];

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

int main(){
    srand(time(NULL));
    ios_base::sync_with_stdio(false);
    ifstream inputFile("MCS-One-and-Two-Words-input.txt");
    ofstream outputFile("MCS-One-and-Two-Words-output.txt");
    
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

    int generated_summary_length = 0, profit = 0;
    while(generated_summary_length < summary_length_limit){
        int best_sentence_score = -1;
        int best_sentence_id = -1;
        for(int i = 0; i < num_sentences; i++){
            if(used_sentence[i]) continue;
            map<string, bool> visited_word;
            int score = 0;
            for(int j = 0; j < (int)chapter_sentences_words[i].size(); j++){
                string one_word = chapter_sentences_words[i][j];
                if(checked_word.find(one_word) == checked_word.end() && visited_word.find(one_word) == visited_word.end()) score += (string_cnt[one_word] - 1);
                visited_word[one_word] = true;
                if(j == (int)chapter_sentences_words[i].size() - 1) break;
                string two_words = chapter_sentences_words[i][j] + " " + chapter_sentences_words[i][j + 1];
                if(checked_word.find(two_words) == checked_word.end() && visited_word.find(two_words) == visited_word.end()) score += (string_cnt[two_words] - 1) * 2;
                visited_word[two_words] = true;
            }
            if(score > best_sentence_score){
                best_sentence_score = score;
                best_sentence_id = i;
            }
        }
        for(int j = 0; j < (int)chapter_sentences_words[best_sentence_id].size(); j++){
            string one_word = chapter_sentences_words[best_sentence_id][j];
            checked_word[one_word] = true;
            if(j == (int)chapter_sentences_words[best_sentence_id].size() - 1) break;
            string two_words = chapter_sentences_words[best_sentence_id][j] + " " + chapter_sentences_words[best_sentence_id][j + 1];
            checked_word[two_words] = true;
        }
        used_sentence[best_sentence_id] = true;
        profit += best_sentence_score;
        best_solution.push_back(best_sentence_id);
        generated_summary_length += sentences_length[best_sentence_id];
    }

    // Print result to file
    outputFile << (int)best_solution.size() << '\n';
    for(int i = 0; i < (int)best_solution.size(); i++) outputFile << best_solution[i] << '\n';
    outputFile << profit << '\n';

    inputFile.close();
    outputFile.close();
    return 0;
}