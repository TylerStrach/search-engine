//
//  Description: This program creates a simplified search engine using a reverse index system, where the keys are the
//  cleaned words in a website, and the values are all the websites where that specific word exists. All the words
//  are "cleaned" meaning all special characters preceding and following are removed and all letters are converted to
//  lowercase.
//
//  Unique Component ( getStopWords() + overall stop word implementation): Offers the user an option to sort out
//  stopwords, which are given in the form of a .txt file. If the file is given, all the tokens are first compared to the
//  words in the stopwords file, and if there is a match, they are filtered out and not included in the reverse index.
//

#pragma once

#include <iostream>
#include <set>
#include <map>
#include <sstream>
#include <fstream>
using namespace std;

/* takes in a string and converts it all to lowercase, and removes any special characters using ispunct
 *  and checks if the string has any characters, will return a cleaned version of the string */
string cleanToken(string s) {
    bool hasLetters = false; // bool value to check if a character is present
    while(ispunct(s[0])){ // erases all the special characters in the beginning of the string
        s.erase(0, 1);
    }

    while(ispunct(s[s.size() - 1])){ // erases all the special character in the end of the string
        s.pop_back();
    }

    for(char &c : s){  // checks for letters, and then converts all letters to lowercase
        if(isalpha(c)){
            hasLetters = true;
        }
        if(!ispunct(c))
            c = tolower(c);
    }

    if(hasLetters) // if letters are present, returns the cleaned string, else returns empty string
        return s;
    else
        return "";
}

/* takes in name of file with stopwords and cleans all the words and adds to a set, which is returned */
set<string> getStopWords(string &file){
    set<string> stopwords; // set that will hold cleaned stopwords
    string curword;

    // opens the given stopwords file
    ifstream inStream;
    inStream.open(file);
    if(!inStream.is_open()){
        return stopwords;
    }

    getline(inStream, curword);
    while(!inStream.eof()){ // gets words from file until file is empty, cleans and adds to set
        curword = cleanToken(curword);
        stopwords.insert(curword);
        getline(inStream, curword);
    }
    return stopwords;
}

/* takes in a string which is the filename and a set with all the stopwords if used
 * this function goes through the string of words, and cleans each string seperated by spaces
 * then it adds the cleaned tokens to the set and returns the set filled with clean tokens */
set<string> gatherTokens(string &text, set<string> &stopwords) {
    set<string> tokens;
    stringstream inText(text); // creates stringstream to go through all words in the given string
    string curStr;

    while(inText >> curStr){ // goes through the entire string until no more words can be taken from string
        curStr = cleanToken(curStr); // cleans the token
        if(!stopwords.empty()){ // if stopwords were loaded, it checks if the token is a stop word
            for(const string &stopword : stopwords){
                if(curStr == stopword){
                    curStr = "";
                }
            }
        }
        if(!curStr.empty()) // if the token is not an empty string, adds to the set of token values
            tokens.insert(curStr);
    }
    return tokens;  // returns the set
}

/* takes in the name of the data file, and an empty map which will be the reverse index filled
 * this function goes through the given file, and builds the reverse index
 * returns the number of urls that were analyzed and added to the reverse index (map)  */
int buildIndex(string filename, map<string, set<string>>& index) {

    set<string> stopwords;
    string stopWordFile;
    cout << "Do you want to use a stopwords file? (Y or N): ";
    string choice;
    getline(cin, choice);
    if(choice == "Y"){ // if the user has a stopwords file to use, it cleans and loads the stopwords file
        cout << "Name of stopwords file: ";
        getline(cin, stopWordFile);
        stopwords = getStopWords(stopWordFile);
    }

    int totalUrls = 0; // counts the urls to be returned later
    ifstream inStream; // opens the given file
    inStream.open(filename);
    if(!inStream.is_open()){
        return 0;
    }

    string curRow, curUrl, curText;
    int curLine = 1;
    getline(inStream, curRow);

    while(!inStream.eof()){ // while there are still rows of data in the file
        if(curLine % 2 == 1){ // if the current line is odd, stores the url
            totalUrls++; // increments when a new url is found
            curUrl = curRow;
        }
        else{ // if the current line is even, uses the url stored above and pairs it with the current row of text
            curText = curRow;
            set<string> tokens = gatherTokens(curText, stopwords);  // cleans all the tokens in this row
            set<string> urls;
            for (auto &token: tokens) { // for all the tokens in this url
                if(index.count(token) == 1){ // if this token exists in the map
                    index.at(token).insert(curUrl); // adds the current url to the already existing set of tokens
                }
                else{ // if the token doesn't exist, adds the url to a new set and emplaces that set matched with the token
                    urls.insert(curUrl);
                    index.emplace(token, urls);
                }
            }
        }
        curLine++; // increments the current line
        getline(inStream, curRow);
    }
    return totalUrls; // returns the number of URLs
}

/* takes in the reverse index, and a string which is input by the user
 * this function goes through the reverse index and searches for urls based on the user query and the modifiers
 * returns a set of urls that matches the users search requirements */
set<string> findQueryMatches(map<string, set<string>>& index, string sentence) {
    set<string> result; // set that will hold the final collection of urls after the set calculations are done
    set<string> tempHolder; // temp set to hold the urls after set calculations are done

    stringstream inText(sentence); // stream to go through each word from the user search
    string term;
    set<string> curUrls; // hold urls from current search term

    while(inText >> term){ // while still words in the query
        // clear all sets from previous calculations
        curUrls.clear();
        tempHolder.clear();

        if(term[0] == '+'){ // if the term precedes with a +, perform set_intersection calculation with the running
                            // set (result) and the set from the current word (curUrls)
            term = cleanToken(term);
            if(index.count(term)) // if the word exists in the index
                curUrls = index.at(term); // retrieves curUrls
            else
                curUrls = {};
            set_intersection(result.begin(), result.end(), curUrls.begin(), curUrls.end(),
                             inserter(tempHolder, tempHolder.begin())); // tempHolder set holds the calculation
            result.clear(); // clears running set before adding new set from calculation
            result = tempHolder;
        }
        else if(term[0] == '-'){ // if the term precedes with a -, perform set_difference calculation with the running
                                 // set (result) and the set from the current word (curUrls)
            term = cleanToken(term);
            if(index.count(term)) // if the word exists in the index
                curUrls = index.at(term); // retrieves curUrls
            else
                curUrls = {};
            set_difference(result.begin(), result.end(), curUrls.begin(), curUrls.end(),
                             inserter(tempHolder, tempHolder.begin())); // tempHolder set holds the calculation
            result.clear(); // clears running set before adding new set from calculation
            result = tempHolder;
        }
        else{ // perform set_union calculation with the running set (result) and set from the current word (curUrls)
            term = cleanToken(term);
            if(index.count(term)) // if the word exists in the index
                curUrls = index.at(term); // retrieves curUrls
            else
                curUrls = {};
            set_union(result.begin(), result.end(), curUrls.begin(), curUrls.end(),
                      inserter(tempHolder, tempHolder.begin()));
            result.clear(); // clears running set before adding new set from calculation
            result = tempHolder;
        }
    }
    return result;  // returns final set after all calculations
}

/* takes in the name of the file with alternating rows of urls and text from that url
 * this function runs the entire program and uses all the functions to build the reverse index,
 * and get the user search and search the index to find the matching urls based on the user search */
void searchEngine(string filename) {
    // builds the index
    cout << "Stand by while building index..." << endl;
    map<string, set<string>> index;
    buildIndex(filename, index);

    // calculates the total number of urls and pages in index
    uint sumTokens = index.size();
    set<string> totalUrls;
    for(auto &pair: index){
        for(const string &url : pair.second)
        totalUrls.insert(url);
    }

    cout << "Indexed " << totalUrls.size() << " pages containing " << sumTokens << " unique terms" << endl << endl;

    // gets the user query
    string query;
    set<string> results;
    cout << "Enter query sentence (press enter to quit): ";
    getline(cin, query);

    while(!query.empty()){ // until the user enters a empty string
        results = findQueryMatches(index, query); // gets the URLs that match the query search
        cout << "Found " << results.size() << " matching pages" << endl;
        for(const string &url : results){ // prints out each url from the set
            cout << url << endl;
        }
        cout << endl;

        cout << "Enter query sentence (press enter to quit): "; // gets next user search
        getline(cin, query);
    }
    cout << "Thank you for searching!" << endl; // ends when user hits enter
}


