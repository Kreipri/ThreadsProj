#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

//Class/Strucs
class Device {
    protected:
        string id;
        bool isOn = false;
    public:
        Device(string id){
            this->id = id;
            isOn = false;
        }
        virtual void turnOn(){
            isOn = true;
        }
        virtual void showStatus() = 0;
        virtual ~Device(){}
};

class Fridge : public Device {
    public:
        int temperature;
        void showStatus() override {
            cout<<id<<"(Fridge) is "<< (isOn ? "ON" : "OFF")<<" at "<<temperature<<"C"<<endl;
        }
        void setTemp(int temp){
            this->temperature = temp;
        }
};

class Light : public Device {
    public:
        string brightness = "normal"; //low, normal, high, normal is default
        void showStatus() override {
            cout<<id<<"(Light) is "<< (isOn ? "ON" : "OFF")<<" at "<<brightness<<" brightness"<<endl;
        }
        void setBrightness(string lvl){
            this->brightness = lvl;
        }
};

struct User {
    string user;
    string pass;
    bool isLoggedIn = 0;
};

//Global Variables
mutex devMtx;
mutex userMtx;

//Prototypes
void mainMenu();
void simulateThreads();
void deviceManagement();
void userManagement();
void deviceControl();
void concurrencyControl();
void livenessCheck();

//Main
int main(){
    //Main Menu
    mainMenu();
}

//Functions
void mainMenu(){
    int ch;
    while(true){
        cout<<"====== Smart Home System ======"<<endl;
        cout<<"1 - Simulate Multiple Threads"<<endl;
        cout<<"2 - Device Management"<<endl;
        cout<<"3 - User Management"<<endl;
        cout<<"4 - Device Control"<<endl;
        cout<<"5 - Concurrency Control"<<endl;
        cout<<"0 - Exit"<<endl;
        cout<<"==============================="<<endl;
        cout<<"Enter Choice: ";
        cin>>ch;
        if(cin.fail() || (ch < 5 && ch > 0)){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout<<"Invalid input. Please choose between the numbers listed."<<endl;
            continue;
        }
        switch(ch){
            case 1: 
                break;
            case 2: 
                break;
            case 3: 
                break;
            case 4: 
                break;
        }



    }
    void simulateThreads();
    void deviceManagement(){
      //cout<<"====== Smart Home System ======"<<endl;
        cout<<"====== Device Management ======"<<endl;
        cout<<""
        cout<<"==============================="<<endl;
    }
    void userManagement();
    void deviceControl();
    void concurrencyControl();
    void livenessCheck();

}