#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <algorithm>

using namespace std;


const int NUM_DISKS = 7;
const int MESSAGE_SIZE = 12; // Размер сообщения в байтах
const int BLOCK_SIZE = MESSAGE_SIZE / (NUM_DISKS - 1); // Размер блока данных в байтах
const string DISK_PREFIX = "disk";

void create_disks(){
    for (int i = 0; i < NUM_DISKS; i++){
        string filename = DISK_PREFIX + to_string(i);
        ofstream file(filename, ofstream::trunc);
            if (file.is_open()) {
            cout << "Файл \"" << filename << "\" найден и очищен." << std::endl;
            file.close();
            } else {
                cerr << "Ошибка открытия файла для очистки." << std::endl;
            }
    }
}
int main (){
    create_disks();
}