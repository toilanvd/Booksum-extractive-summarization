#include <bits/stdc++.h>
#define fi first
#define se second

using namespace std;

typedef pair<int, int> ii;
typedef pair<ii, int> iii;

const double EPS = 0.000001;

int num_sentences, summary_length_limit;
int sentences_length[7000];
double pairwise_sentences_score[7000][7000];
double sentence_cover[7000];
int chosen_sentences_cnt, estimate_cost, real_cost;
double estimate_profit, real_profit;

int main(){
    ios_base::sync_with_stdio(false);
    ifstream inputFile("GA-ROUGE-input.txt");
    ifstream outputFile("GA-ROUGE-output.txt");
    
    inputFile >> num_sentences >> summary_length_limit;
    for(int i = 0; i < num_sentences; i++){
        inputFile >> sentences_length[i];
        sentence_cover[i] = -1;
    }
    for(int i = 0; i < num_sentences; i++){
        for(int j = 0; j < num_sentences; j++){
            inputFile >> pairwise_sentences_score[i][j];
        }
    }

    outputFile >> chosen_sentences_cnt;
    while(chosen_sentences_cnt){
        int id; outputFile >> id;
        real_cost += sentences_length[id];
        for(int i = 0; i < num_sentences; i++) sentence_cover[i] = max(sentence_cover[i], pairwise_sentences_score[id][i]);
        chosen_sentences_cnt--;
    }
    outputFile >> estimate_profit >> estimate_cost;
    for(int i = 0; i < num_sentences; i++) real_profit += sentence_cover[i];

    cout << estimate_profit << " " << real_profit << endl;
    cout << estimate_cost << " " << real_cost << endl;

    inputFile.close();
    outputFile.close();

    return 0;
}