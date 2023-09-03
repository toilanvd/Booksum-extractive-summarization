#include <bits/stdc++.h>
#define fi first
#define se second

using namespace std;

int num_of_vocab_doc;

int main(){
    srand(time(NULL));
    ios_base::sync_with_stdio(false);
    ifstream vocabulary_file("vocabulary.txt");
    vocabulary_file >> num_of_vocab_doc;
    int freq, cnt = 0;
    string word;
    while(vocabulary_file >> freq){
        getline(vocabulary_file, word);
        if(word[0] == ' ') word.erase(word.begin());
        cout << freq << ":" << word << endl;
        cnt++; if(cnt == 100) break;
    }
    vocabulary_file.close();

    return 0;
}