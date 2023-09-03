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
vector<ii> sentence_eval;

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
    ifstream inputFile("One-and-Two-Words-input.txt");
    ofstream outputFile("One-and-Two-Words-output.txt");
    
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
    for(int i = 0; i < num_sentences; i++){
        map<string, bool> checked;
        int score = 0;
        for(int j = 0; j < (int)chapter_sentences_words[i].size(); j++){
            string one_word = chapter_sentences_words[i][j];
            if(checked.find(one_word) == checked.end()) score += (string_cnt[one_word] - 1);
            checked[one_word] = true;
            if(j == (int)chapter_sentences_words[i].size() - 1) break;
            string two_words = chapter_sentences_words[i][j] + " " + chapter_sentences_words[i][j + 1];
            if(checked.find(two_words) == checked.end()) score += (string_cnt[two_words] - 1) * 2;
            checked[two_words] = true;
        }
        sentence_eval.push_back(ii(score, i));
    }
    sort(sentence_eval.begin(), sentence_eval.end());
    reverse(sentence_eval.begin(), sentence_eval.end());
    int generated_summary_length = 0, profit = 0;
    for(int i = 0; i < num_sentences && generated_summary_length < summary_length_limit; i++){
        best_solution.push_back(sentence_eval[i].se);
        generated_summary_length += sentences_length[sentence_eval[i].se];
        profit += sentence_eval[i].fi;
        //cout << sentence_eval[i].fi << endl;
    }

    // Print result to file
    outputFile << (int)best_solution.size() << '\n';
    for(int i = 0; i < (int)best_solution.size(); i++) outputFile << best_solution[i] << '\n';
    outputFile << profit << '\n';

    inputFile.close();
    outputFile.close();
    return 0;
}