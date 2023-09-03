#include <bits/stdc++.h>
#define fi first
#define se second

using namespace std;

typedef pair<int, int> ii;
typedef pair<ii, int> iii;

const double EPS = 0.000000001;

int num_sentences, summary_length_limit;
double temp_alpha;
int dp[7000][18000];
int sentences_length[7000], sentences_length_sum[7000];
double cosine_scores[7000][7000];
vector<iii> interval_list;

int main(){
    ios_base::sync_with_stdio(false);
    ifstream inputFile("SOBP-MA-BERT-dp-input.txt");
    ofstream outputFile("SOBP-MA-BERT-dp-output.txt");
    
    inputFile >> num_sentences >> summary_length_limit >> temp_alpha;
    for(int i = 0; i < num_sentences; i++) inputFile >> sentences_length[i];
    for(int i = 0; i < num_sentences; i++) sentences_length_sum[i + 1] = sentences_length_sum[i] + sentences_length[i];
    for(int i = 0; i < num_sentences; i++){
        for(int j = 0; j < num_sentences; j++){
            inputFile >> cosine_scores[i][j];
        }
    }
    //interval_list[0][0] = 0; interval_list[0][1] = 0; interval_list[0][2] = -1;
    //for(int i = 1; i <= num_sentences; i++) inputFile >> interval_list[i][0] >> interval_list[i][1] >> interval_list[i][2];

    interval_list.push_back(iii(ii(0, 0), -1));
    for(int j = 0; j < num_sentences; j++){
        double temp_sum = 1;
        int le = j;
        for(int k = j - 1; k >= 0; k--){
            temp_sum += cosine_scores[j][k];
            if(temp_sum / (double)(j - k + 1) + EPS < temp_alpha) break;
            le = k;
        }
        temp_sum = 1;
        int ri = j;
        for(int k = j + 1; k < num_sentences; k++){
            temp_sum += cosine_scores[j][k];
            if(temp_sum / (double)(k - j + 1) + EPS < temp_alpha) break;
            ri = k;
        }
        interval_list.push_back(iii(ii(le + 1, ri + 1), j));
    }
    sort(interval_list.begin(), interval_list.end());

    // DP
    for(int j = num_sentences; j >= 0; j--){
        for(int s = 0; s < summary_length_limit; s++){
            dp[j][s] = 0;
            for(int k = j + 1; k <= num_sentences; k++){
                int ri_1 = interval_list[j].fi.se;
                int le_2 = interval_list[k].fi.fi;
                int ri_2 = interval_list[k].fi.se;
                int id_2 = interval_list[k].se;
                int cost = sentences_length[id_2];
                int profit = 0;
                if(ri_2 > ri_1){
                    int le_3 = max(ri_1 + 1, le_2);
                    int ri_3 = ri_2;
                    profit = sentences_length_sum[ri_3] - sentences_length_sum[le_3 - 1];
                }
                if(s + cost < summary_length_limit){
                    dp[j][s] = max(dp[j][s], dp[k][s + cost] + profit);
                }
                else{
                    dp[j][s] = max(dp[j][s], profit);
                }
            }
        }
    }

    // Trace
    int temp_j = 0;
    int temp_s = 0;
    int new_state = false;
    vector<int> res;
    while(temp_s < summary_length_limit){
        for(int temp_k = temp_j + 1; temp_k <= num_sentences; temp_k++){
            int ri_1 = interval_list[temp_j].fi.se;
            int le_2 = interval_list[temp_k].fi.fi;
            int ri_2 = interval_list[temp_k].fi.se;
            int id_2 = interval_list[temp_k].se;
            int cost = sentences_length[id_2];
            int profit = 0;
            if(ri_2 > ri_1){
                int le_3 = max(ri_1 + 1, le_2);
                int ri_3 = ri_2;
                profit = sentences_length_sum[ri_3] - sentences_length_sum[le_3 - 1];
            }
            if(temp_s + cost < summary_length_limit && dp[temp_j][temp_s] == dp[temp_k][temp_s + cost] + profit){
                res.push_back(id_2);
                temp_j = temp_k;
                temp_s += cost;
                new_state = true;
                break;
            }
            else if(temp_s + cost >= summary_length_limit && dp[temp_j][temp_s] == profit){
                res.push_back(id_2);
                temp_j = temp_k;
                temp_s += cost;
                new_state = true;
                break;
            }
        }
        if(new_state == false) break;
        new_state = false;
    }

    // Print result to file
    outputFile << (int)res.size() << '\n';
    for(int i = 0; i < (int)res.size(); i++){
        outputFile << res[i] << '\n';
    }

    inputFile.close();
    outputFile.close();

    return 0;
}