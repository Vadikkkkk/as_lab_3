#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <regex>
#include <algorithm>

using namespace std;


const int NUM_DISKS = 7;
const int MESSAGE_SIZE = 12; // Размер сообщения в байтах
const int BLOCK_SIZE = MESSAGE_SIZE / (NUM_DISKS - 1); // Размер блока данных в байтах
const string DISK_PREFIX = "disk";
const int DISK_CAPACITY = 64;

void menu(){
    cout << "Option Parameters: " << endl << "1. Message length = " << MESSAGE_SIZE << endl <<
    "2. Disk capacity = " << DISK_CAPACITY << endl << "3. Number of disks = " << NUM_DISKS << endl;

    cout << "Commands available :" << endl << "1. <address> write <data>" << endl << "2. <address> read" << endl <<
    "3. exit " << endl << endl;
}

int hexStringToInt(const string& hexStr) {
    int value;
    stringstream ss;
    ss << hex << hexStr;
    ss >> value;
    return value;
}

string intToHexString(int value) {
    stringstream ss;
    ss << hex << value;
    string ts = ss.str();
    if (ts.length() >= 2){
        return ts.substr(ts.length() - 2);
    }
    else{
        return ts;
    }
}

void write_on_disk(const int& _address, const string& _data){
    vector<string> blocks;
    int rs = 0;
    for (int i = 0; i < NUM_DISKS - 1; i++){
        string filename = DISK_PREFIX + to_string(i);
        blocks.push_back(_data.substr(i * 2, 2));

        rs += hexStringToInt(blocks[i]);


        ifstream inFile(filename);
        vector<string> lines;
        string line;
        while (getline(inFile, line)) {
            lines.push_back(line);
        }
        inFile.close();
        while (lines.size() < DISK_CAPACITY) {
            lines.push_back("");
        }
        lines[_address] = blocks[i];
        ofstream outFile(filename);
        if (!outFile)
        {
            cerr << "Cant open file " << filename << " to write." << endl;
            return;
        }
        
        for (const auto& l : lines) {
            outFile << l << "\n";
        }

        outFile.close();
    }
    string redundancy = intToHexString(rs);
    if (redundancy.length() < 2){
        redundancy.insert(0, 2 - redundancy.length(), '0');
    }
    blocks.push_back(redundancy);

    string filename = DISK_PREFIX + to_string(NUM_DISKS - 1);

    ifstream inFile(filename);
    vector<string> lines;
    string line;
    while (getline(inFile, line)) {
        lines.push_back(line);
    }
    inFile.close();
    while (lines.size() < DISK_CAPACITY) {
        lines.push_back("");
    }
    lines[_address] = blocks[NUM_DISKS - 1];
    ofstream outFile(filename);
    if (!outFile)
    {
        cerr << "Cant open file! " << filename << " to write." << endl;
        return;
    }
    
    for (const auto& l : lines) {
        outFile << l << "\n";
    }

    outFile.close();
    cout << "The data has been successfully recorded!" << endl; 
}

void read_from_disk(const int& address){
    string result;
    vector<int> missing_disks;
    int missing_disk;
    for (int i = 0; i < NUM_DISKS; i++){
        string filename = DISK_PREFIX + to_string(i);
        ifstream file(filename);
        int count = 0;
        if (file.is_open()){
            string line;
            
            while(getline(file, line)){
                if (count == address){
                    if (line.length() == BLOCK_SIZE){
                        result += line;
                        break;
                    }
                    else{
                        missing_disks.push_back(i);
                        break;
                    }
                    
                }
                count++;
            }
            file.close();
        }
        else{
            cerr << "Cant open file!" << filename;
        }
    }
    
    if (missing_disks.size() > 1){
        cout << "Data cannot be restored!" << endl;
    }
    else if(missing_disks.size() == 0){//все на месте
        result.erase(result.length() - 2, 2);
        cout << "Message - " << result << endl;
    }
    else if(missing_disks[0] == 6){ //отсутствует диск проверки
        cout << "Message - " << result << endl;
        int rs = 0;
        vector<int> ms;
        for (size_t i = 0; i < result.length(); i += 2) {
            string subStr = result.substr(i, 2);
            int decimalValue = hexStringToInt(subStr); 
            ms.push_back(decimalValue); 
        }
        for(int i = 0; i < ms.size(); i++){
            rs += ms[i];
        };
        string redundancy = intToHexString(rs);
        if (redundancy.length() < 2){
            redundancy.insert(0, 2 - redundancy.length(), '0');
        }

        ifstream inFile(DISK_PREFIX + to_string(NUM_DISKS - 1));
        vector<string> lines;
        string line;
        while (getline(inFile, line)) {
            lines.push_back(line);
        }
        inFile.close();
        while (lines.size() < DISK_CAPACITY) {
            lines.push_back("");
        }
        lines[address] = redundancy;
        ofstream outFile(DISK_PREFIX + to_string(NUM_DISKS - 1));
        if (!outFile)
        {
            cerr << "Cant open file! " << DISK_PREFIX + to_string(NUM_DISKS - 1) << " to write." << endl;
            return;
        }
        
        for (const auto& l : lines) {
            outFile << l << "\n";
        }
        outFile.close();
    }
    else{//отсутствует один
        
        int rs = 0;
        string tmp = result;
        vector<int> blocks;
        for (int i = 0; i < result.length(); i += 2){
            blocks.push_back(hexStringToInt(result.substr(i, 2)));
        }
        for(int i = 0; i < blocks.size() - 1; i++){
            rs -= blocks[i];
        }
        rs += blocks.back();

        string new_data = intToHexString(rs);
        if (new_data.length() < 2){
            new_data.insert(0, 2 - new_data.length(), '0');
        }
        else if (new_data.length() > 2){
            new_data.substr(new_data.length() - 2);
        }

        ifstream inFile(DISK_PREFIX + to_string(missing_disks[0]));
        vector<string> lines;
        string line;
        while (getline(inFile, line)) {
            lines.push_back(line);
        }
        inFile.close();
        while (lines.size() < DISK_CAPACITY) {
            lines.push_back("");
        }
        lines[address] = new_data;
        ofstream outFile(DISK_PREFIX + to_string(missing_disks[0]));
        if (!outFile)
        {
            cerr << "Cant open file! " << DISK_PREFIX + to_string(missing_disks[0]) << " to write." << endl;
            return;
        }
        
        for (const auto& l : lines) {
            outFile << l << "\n";
        }
        outFile.close();
        
        result.erase(result.length() - 2, 2);
        result.insert(2 * missing_disks[0], new_data);
        cout << "Message - " << result << endl;

    }
}

int main (){
    
    menu();
    string input;
    regex writeRegex("^\\s*(\\d{1,2})\\s+(write)\\s+([0-9a-fA-F]{12})\\s*$");
    regex readRegex("^\\s*(\\d{1,2})\\s+read\\s*$");
    regex exitRegex("exit", regex_constants::icase);
    
    while(true){
        cout << "Enter command : ";
        getline(cin, input);

        smatch match;
        if (regex_match(input, match, writeRegex)) { //проверка, если запись 
            // Извлечение числа, команды и данных из ввода
            int address = stoi(match[1]);
            string data = match[3];
            if (address < 0 || address > DISK_CAPACITY - 1) {
                cout << "Adress out of range! Valid value - 0 to " << DISK_CAPACITY << endl;
                continue;  // Пропускаем текущую итерацию цикла
            }
            write_on_disk(address, data);
        }
        else if (regex_match(input, match, readRegex)){ //проверка, если чтение
            int address = stoi(match[1]);
            if (address < 0 || address > DISK_CAPACITY - 1) {
                cout << "Adress out of range! Valid value - 0 to " << DISK_CAPACITY << endl;
                continue;  // Пропускаем текущую итерацию цикла
            }
            read_from_disk(address);

        }
        else if (regex_match(input, match, exitRegex)){
            break;
        }
        else{
            cout << "Unknown command! Try again!" << endl;
        }
    }
    return 0;
}
