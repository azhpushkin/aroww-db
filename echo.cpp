#include <iostream>
#include <algorithm>
#include <vector>
#include <sstream>

using namespace std;


int main() {
    cout << "Arrow DB [v0.0]" << endl;
    cout << "Available commands: " << endl;
    cout << " * set <key> <value> " << endl;
    cout << " * get <key> " << endl;
    cout << " * delete <key> " << endl;
    cout << " * exit " << endl << endl;

    string command;
    vector<string> words;
    while (true) {
        words.clear();
        cout << "< " << flush;
        getline(cin, command);
        transform(command.begin(), command.end(), command.begin(), ::tolower);

        istringstream iss(command);
        for(string word; iss >> word;)
        {
            words.push_back(word);
        }
            
        if (words.empty()) {
            cout << "Ah.." <<endl;
        }
        else if (words[0] == "get") {
            cout << "Get produced!"; 
        }
        else if (words[0] == "set") {
            cout << "Set produced!"; 
        }
        else if (words[0] == "delete") {
            cout << "Delete produced!";
        }
        else if (words[0] == "exit") {
            break;
        }
        else {
            cout << "Unknown operation!.."; 
        }
    }

    return 0;
}