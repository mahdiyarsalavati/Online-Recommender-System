#include <iostream>
#include <fstream>

using namespace std;

int main() {
    string inputSize = "large";
    ifstream file1("output.txt");
    ifstream file2("dataset/" + inputSize + "/" + inputSize + ".refout");


    string line1, line2;
    int lineNum = 1;

    while (getline(file1, line1) && getline(file2, line2)) {
        if (line1 != line2) {
            cout << "Files are different at line " << lineNum << endl;
            return 0;
        }
        lineNum++;
    }

    if (getline(file1, line1) || getline(file2, line2)) {
        cout << "Files have different number of lines" << endl;
        return 0;
    }

    cout << "Files are identical" << endl;
    return 0;
}