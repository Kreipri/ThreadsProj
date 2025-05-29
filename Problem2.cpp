#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <random>

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
        virtual void turnOn(bool b){
            isOn = b;
        }
        virtual void showStatus() = 0;
        virtual ~Device(){}
};

class Fridge : public Device {
    public:
        Fridge(string id) : Device(id) {} //get id used in making this obj to base constructor
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
        Light(string id) : Device(id) {} //get id used in making this obj to base constructor
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
    bool isLoggedIn = 0;
};

//Global Variables
vector<Device*> devices;
vector<User> users;
mutex devMtx;
mutex userMtx;

//Prototypes
void mainMenu();
void startThreads();
void simulateUsage(int threadId, int userIndex);
void deviceManagement();
void userManagement();
void deviceControl();
void concurrencyControl();
void livenessCheck();

void addUser();
void makeExample();

int getCh();

//Main
int main(){
    makeExample();
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
        ch = getCh();
        if(ch == NULL){
            continue;
        }
        
        switch(ch){
            case 1: startThreads(); break;
            case 2: deviceManagement(); break;
            case 3: 
                break;
            case 4: 
                break;
        }
    }
}

void startThreads(){
    if (devices.empty() || users.size() < 3){
        cout<<"Please ensure at least 3 users and 1 or more devices are added."<<endl;
        return;
    }

    cout<<"Starting multiple threads..."<<endl;
    vector<thread> threads;
    for (int i = 0; i < 3; i++){
        threads.emplace_back(simulateUsage, i+1, i);
    }

    for (auto& t : threads) t.join();
    cout<<"Simulation done!";
    return;
}

void simulateUsage(int threadId, int userId){
    {//Log in
        lock_guard<mutex> lock(userMtx); //lockguard user mutex
        if(userId >= users.size()){ //check if userIndex is out of bounds
            cout<<"[Thread "<<threadId<<"] Invalid user index."<<endl;
            return;
        }
        
        users[userId].isLoggedIn = true; //log in user
        cout<<"[Thread "<<threadId<<"] User "<<users[userId].user<<" logged in."<<endl;
    }
    //Use devices
    for(int i = 0; i < 5; i++){//simulate 5 actions
        int deviceIndex;
        if(devices.empty()){
            cout<<"[Thread "<<threadId<<"] No devices available."<<endl;
            return;
        }
        {
            lock_guard<mutex> lock(devMtx); //lockguard device mutex in this block
            deviceIndex = (userId+i) % devices.size(); //get a random device index 
        }
        Device* dev = devices[deviceIndex];
        {
            lock_guard<mutex> lock(devMtx); //lockguard device mutex in this block
            cout<<"[Thread "<<threadId<<"] "<<users[userId].user<<" is using device "<<deviceIndex<<"."<<endl; //User is using device
            dev->turnOn(true); //user turns on device
            dev->showStatus(); //show dev status
        }   

        this_thread::sleep_for(chrono::milliseconds(500));
    }
    
    //

}


void deviceManagement(){
    int ch;
    while(true){
        cout<<"====== Device Management ======"<<endl;
        cout<<"1 - Add Devices"<<endl;
        cout<<"2 - Remove Devices"<<endl;
        cout<<"3 - Manage Devices"<<endl;
        cout<<"0 - Back"<<endl;
        cout<<"==============================="<<endl;
        cout<<"Enter Choice: ";
        ch = getCh();
        if(ch == NULL){
            continue;
        }

        switch(ch){ //Add choices here
            case 1: break;
                //Output:
                //Enter Device ID: (make sure its unique, or just automatically set an id)
                //Enter Device Type: (Thermostat, Fridge or Light)
            case 2: break;
                //Show device list, make user choose which to remove
            case 3: break;
                //Show device list, make user choose which to modify settings, call deviceControl() menu
            case 0: return; break;
            default:
                break;
        }
    }

}
// void userManagement();
    // ==== User Management ====
    // Add Users (add username)
    // User List (show user list)
    // Remove Users (cant remove user if user is logged in)
// void deviceControl();
    // === Device Control ==== 
    // Turn on
    // Turn off
    // Adjust device settings (temp, brightness, etc)
    // Check device status (call showStatus() of device)


// void concurrencyControl();
// void livenessCheck();

void addUser(){
    string user;
    cout<<"Enter username: ";
    cin>>user;

    for (const auto& u : users){
        if(u.user == user){
            cout<<"Username already exists!"<<endl;
            return;
        }
    } 

    User newUser;
    newUser.user = user;
    newUser.isLoggedIn = false;

    //add to users vector
    users.push_back(newUser);
    cout<<"User "<<user<<" registered successfully!"<<endl;
}

void makeExample(){
    //devices sample
    Fridge* f1 = new Fridge("F1");
    f1->setTemp(5);
    devices.push_back(f1);

    Light* l1 = new Light("L1");
    l1->setBrightness("high");
    devices.push_back(l1);

    //users sample
    User u1{"bea",false};
    User u2{"jane", false};
    User u3{"kate", false};

    users.push_back(u1);
    users.push_back(u2);
    users.push_back(u3);

    cout<<"Examples added."<<endl;
}

int getCh(){
    int ch;
    cin>>ch;
    if(cin.fail() || (ch < 5 && ch > 0)){
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout<<"Invalid input. Please choose between the numbers listed."<<endl;
        return NULL;
    }
    return ch;
}