#include <bits/stdc++.h>
#define fi first
#define se second

using namespace std;

const int NUM_OF_DOC = 287113;
map<string, int> vocabulary;
map<string, int> word_in_doc;

string normalize(string doc){
    string res = "";
    string w = "";
    for(int i = 0; i < (int)doc.size(); i++){
        if(doc[i] >= 'A' && doc[i] <= 'Z') w += (char)(doc[i] - 'A' + 'a');
        else if(doc[i] >= 'a' && doc[i] <= 'z') w += doc[i];
        else if(doc[i] >= '0' && doc[i] <= '9') w += doc[i];
        else if(doc[i] == ' ' || doc[i] == '	'){
            if(w != ""){
                if(res != "") res += " ";
                res += w;
            }
            w = "";
        }
    }
    if(w != ""){
        if(res != "") res += " ";
        res += w;
    }
    return res;
}

int main(){
    srand(time(NULL));
    ios_base::sync_with_stdio(false);
    ifstream inputFile("vocabulary_cnn-dailymail.txt");
    ofstream outputFile("vocabulary_cnn-dailymail_processed.txt");
    
    int doc_id;
    string temp_word;
    while(inputFile >> doc_id){
        getline(inputFile, temp_word);
        string word = normalize(temp_word);
        if(word != "" && (word_in_doc.find(word) == word_in_doc.end() || word_in_doc[word] < doc_id)){
            word_in_doc[word] = doc_id;
            if(vocabulary.find(word) == vocabulary.end()) vocabulary[word] = 0;
            vocabulary[word]++;
        }
    }
    outputFile << NUM_OF_DOC << endl;
    for(map<string, int> :: iterator it = vocabulary.begin(); it != vocabulary.end(); it++){
        outputFile << (*it).se << " " << (*it).fi << endl;
    }

    inputFile.close();
    outputFile.close();

    return 0;
}