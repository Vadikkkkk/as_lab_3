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
void write_on_disk(const int& _address,string& _data);
void read_from_disk(const int& address);

void menu(){
    cout << "Option Parameters: " << endl << "1. Message length = " << MESSAGE_SIZE << endl <<
    "2. Disk capacity = " << DISK_CAPACITY << endl << "3. Number of disks = " << NUM_DISKS << endl << endl;

    cout << "Commands available :" << endl << "1. <address> write <data>" << endl << "2. <address> read" << endl <<
    "3. exit " << endl 
     << "Data is a " << MESSAGE_SIZE << " digit hexadecimal number. Eхаmple of commands: "<< endl <<
     "12 write 124ab23df400" << endl << "12 read" << endl << endl;
}

void input(){
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
                cout << "Adress out of range! Valid value - 0 to " << DISK_CAPACITY - 1 << endl;
                continue;  // Пропускаем текущую итерацию цикла
            }
            write_on_disk(address, data);
        }
        else if (regex_match(input, match, readRegex)){ //проверка, если чтение
            int address = stoi(match[1]);
            if (address < 0 || address > DISK_CAPACITY - 1) {
                cout << "Adress out of range! Valid value - 0 to " << DISK_CAPACITY - 1 << endl;
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

void create_files(){
    for (int i = 0; i < NUM_DISKS; i++){
        ofstream File(DISK_PREFIX + to_string(i));
        for (int i = 0; i < DISK_CAPACITY; i++){
            File << "\n";
        }
        File.close();
    }
    
}

void write_full_data(int address, const string& _data){
    vector<string> blocks;
    for (int i = 0; i < NUM_DISKS; i++){
        string filename = DISK_PREFIX + to_string(i);
        blocks.push_back(_data.substr(i * BLOCK_SIZE, BLOCK_SIZE));
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
        lines[address] = blocks[i];
        ofstream outFile(filename);
        if (!outFile)
        {
            cerr << "Cant open file " << filename << " to write." << endl;
            return;
        }
        
        for (int i = 0; i < lines.size(); i++) {
            outFile << lines[i] << "\n";
        }
        outFile.close();
    }
    
}

void poly(int missing_disk, string& data){
    int rs = 0;
    if(missing_disk == NUM_DISKS - 1){//если последнего нет
        for (int i = 0; i < NUM_DISKS - 1; i++){
            rs += hexStringToInt(data.substr(i * BLOCK_SIZE, BLOCK_SIZE));
        }
        string redundancy = intToHexString(rs);
        if (redundancy.length() < BLOCK_SIZE){
            redundancy.insert(0, BLOCK_SIZE - redundancy.length(), '0');
        }
        else if(redundancy.length() > BLOCK_SIZE){
            redundancy.substr(redundancy.length() - 2);
        }
        data += redundancy;
    }
    else{//если нет одного
        vector<string> blocks;
        for (int i = 0; i < data.length(); i += BLOCK_SIZE){
            blocks.push_back(data.substr(i, BLOCK_SIZE));
        }
        for(int i = 0; i < blocks.size() - 1; i++){
            rs -= hexStringToInt(blocks[i]);
        }
        rs += hexStringToInt(blocks.back());

        string new_data = intToHexString(rs);
        if (new_data.length() < BLOCK_SIZE){
            new_data.insert(0, BLOCK_SIZE - new_data.length(), '0');
        }
        else if (new_data.length() > BLOCK_SIZE){
            new_data.substr(new_data.length() - BLOCK_SIZE);
        }
        data.insert(BLOCK_SIZE * missing_disk, new_data);
    }
    return;
}

void write_on_disk(const int& _address,string& _data){
    poly(NUM_DISKS - 1, _data);
    write_full_data(_address, _data);
    cout << "Message was successfully recorded!" << endl << endl; 
}

void read_from_disk(const int& address){
    string result = "";
    vector<int> missing_disks;
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
            if (count < address) missing_disks.push_back(i);
            file.close();
        }
        else{
            ofstream File(filename);
            for (int i = 0; i < DISK_CAPACITY; i++){
            File << "\n";
            }
            File.close();
            missing_disks.push_back(i);
        }
    }
    if (missing_disks.size() > 1 || result.length() < 10){
        cout << "Data cannot be restored! There are too many damaged disks!" << endl << endl;
    }
    else if(missing_disks.size() == 0){//все на месте
        result.erase(result.length() - BLOCK_SIZE, BLOCK_SIZE);
        cout << "Message - " << result << endl << endl;
    }
    else{
        poly(missing_disks[0], result);
        write_full_data(address, result);
        result.erase(result.length() - BLOCK_SIZE, BLOCK_SIZE);
        cout << "Disk" << missing_disks[0] << " was damaged!" << endl << "Message - " << result << endl << endl;
    }
}

int main (){
    create_files();
    menu();
    input();
    return 0;
}
