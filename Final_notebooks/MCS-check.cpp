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
int cover_cnt[7000];
int chosen_sentences_cnt, estimate_profit, estimate_cost, real_profit, real_cost;

int main(){
    ios_base::sync_with_stdio(false);
    ifstream inputFile("MCS-GA-BERT-input.txt");
    ifstream outputFile("MCS-GA-BERT-output.txt");
    
    inputFile >> num_sentences >> summary_length_limit >> alpha;
    for(int i = 0; i < num_sentences; i++) inputFile >> sentences_length[i];
    for(int i = 0; i < num_sentences; i++){
        for(int j = 0; j < num_sentences; j++){
            double x; inputFile >> x;
            if(x + EPS >= alpha) cover_mask[i][j] = true;
        }
    }

    outputFile >> chosen_sentences_cnt;
    while(chosen_sentences_cnt){
        int id; outputFile >> id;
        real_cost += sentences_length[id];
        for(int i = 0; i < num_sentences; i++) if(cover_mask[id][i]) cover_cnt[i]++;
        chosen_sentences_cnt--;
    }
    outputFile >> estimate_profit >> estimate_cost;
    for(int i = 0; i < num_sentences; i++) if(cover_cnt[i]) real_profit += sentences_length[i];

    cout << estimate_profit << " " << real_profit << endl;
    cout << estimate_cost << " " << real_cost << endl;

    inputFile.close();
    outputFile.close();

    return 0;
}