#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <random>
#include <stdlib.h>
#include <windows.h>
#include <atomic>
#include <shared_mutex>
#include <condition_variable>

using namespace std;

//Class/Strucs
class Device {//Parent Class of fridge and light
    protected:
        mutable shared_mutex rwLock; //Read-write lock
        condition_variable_any devStatChanged;
        string id;
        bool isOn = false;
    public:
        Device(string id) : id(id), isOn(false){}

        virtual string getId(){
            return this->id;
        }
        virtual void turnOn(bool b){
            unique_lock lock(rwLock);
            if(isOn == b){
                cout<<"\033[38;5;208m"<<id<<" is already "<<(isOn ? "ON" : "OFF")<<"\033[0m"<<endl;
                return;
            }
            isOn = b;
            cout<<"\033[1;33m"<<id<<" is turned "<<(isOn ? "ON" : "OFF")<<"\033[0m"<<endl;
            devStatChanged.notify_all();
        }
        virtual string getOn(){
            return this->isOn ? "ON" : "OFF";
        }
        virtual void waitUntilOn() {
            shared_lock lock(rwLock);
            devStatChanged.wait(lock, [this]() {
                return isOn;
            });
        }
        virtual void showStatus() = 0;
        virtual ~Device(){}
};

class Fridge : public Device {
    protected:
        int temperature;
    public:
        Fridge(string id) : Device(id) {} //get id used in making this obj to base constructor

        void showStatus() override {
            shared_lock lock(rwLock);
            cout<<id<<"(Fridge) is "<< (isOn ? "ON" : "OFF")<<" set at "<<temperature<<"\u00B0C"<<endl;
        }

        int getTemp(){
            shared_lock lock(rwLock);
            return this->temperature;
        }

        void setTemp(int temp){
            unique_lock lock(rwLock);
            this->temperature = temp;
        }

        void putTemp(int temp){
            setTemp(temp);
            cout<<"Turned "<<this->id<<" temperature to "<<this->temperature<<"\u00B0C."<<endl;
        }
};

class Light : public Device {
    protected:
        string brightness = "Mid"; //Low, Mid, High, Mid is default
    public:
        Light(string id) : Device(id) {} //get id used in making this obj to base constructor
        
        void showStatus() override {
            shared_lock lock(rwLock);
            cout<<id<<"(Light) is "<< (isOn ? "ON" : "OFF")<<" set at "<<brightness<<" brightness"<<endl;
        }
        void setBrightness(int lvl){
            unique_lock lock(rwLock);
            switch(lvl){
                case 1: this->brightness = "Low"; break;
                case 2: this->brightness = "Mid"; break;
                case 3: this->brightness = "High"; break;
            }
            
        }
        void putBrightness(int lvl){
            setBrightness(lvl);
            cout<<"Turned "<<this->id<<" brightness to "<<this->brightness<<"."<<endl;
        }
};

struct User {
    string user;
    bool isLoggedIn = 0;
};

class TrackedMutex{
    private:
        mutex mtx;
        atomic<bool> isLocked = false;
        string name;
        string owner;

    public:
        TrackedMutex(string name) : name(name), owner("None"){}
        
        void flagLock(string str){
            isLocked = true;
            owner = str;
            cout<<name<<" locked by "<< owner<<"!"<<endl;
        }
        void flagUnlock(){
            isLocked = false;
            cout<<name<<" unlocked by "<< owner<<"!"<<endl;
            owner = "None";
        }
        // bool try_lock(){
        //     if(mtx.try_lock()){
        //         isLocked = true;
        //         owner = this_thread::get_id();
        //         return true;
        //     }
        //     return false;
        // }
        
        //read onlys
        bool checkLock() const{ 
            return isLocked;
        }
        string getOwner() const{
            return owner;
        }
        string getName() const{
            return name;
        }
        mutex& getMutex(){
            return mtx;
        }
};

//Global Variables
vector<Device*> devices;
vector<User> users;

TrackedMutex devMtx("Device Mutex");
TrackedMutex userMtx("User Mutex");
TrackedMutex printMtx("Print Mutex");

//for generating unique randoms for each thread
random_device rd;
mt19937 gen(rd());

//Prototypes
void mainMenu();
void startThreads();
void simulateUsage(int threadId, int userIndex);
void deviceManagement();
void userManagement();
void deviceControl();
void concurrencyControl();
void livenessCheck();

string getColor(int threadId);
void addUser();
void makeExample();

int getCh();

//Main
int main(){
    SetConsoleOutputCP(CP_UTF8); //for degrees


    makeExample();
    //Main Menu
    mainMenu();

    return 0;
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
        if(ch == -1){
            continue;
        }
        
        switch(ch){
            case 1: startThreads(); break;
            case 2: deviceManagement(); break;
            case 3: 
                break;
            case 4: 
                break;
            case 0:
                return;
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
    cout<<"Simulation done!\n\n"<<endl;
    return;
}

void simulateUsage(int threadId, int userId){
    uniform_int_distribution<> secDist(1,3); //for random (sec)
    string color = getColor(threadId);
    int sec = secDist(gen); //diff delays for each thread
    //int sec = 3; //set delays
    User& user = users[userId];
    
    //LOG IN USER
    {
        scoped_lock lock(printMtx, userMtx); //printMtx makes sure no interleaved output, userMtx makes sure no race conditions
        userMtx.flagLock("Thread "+ threadId);
        printMtx.flagLock("Thread "+ threadId);
            if (userId < 0 || userId >= static_cast<int>(users.size())){ //check if userIndex is out of bounds
                cout<<color<<"[Thread " << threadId << "]"<<"\033[1;31mInvalid user index.\033[0m"<<endl;
                return;
            }
            
            user.isLoggedIn = true; //log in user
        
            cout<<color<<"[Thread " << threadId << "] User "<<user.user<<" logged in.\033[0m"<<endl;
        userMtx.flagUnlock();
        printMtx.flagUnlock();
    }
    this_thread::sleep_for(chrono::seconds(sec));
    
    //USE DEVICES
    for(int i = 0; i < 5; i++){//simulate 5 actions
        Device* dev = nullptr;

        if(devices.empty()){//if no devices
            cout<<color<<"[Thread "<< threadId << "]\033[0mNo devices available."<<endl;
            return;
        }
        {
            mutex& mtx = devMtx.getMutex();
            if(mtx.try_lock()){
                lock_guard<mutex> lock(printMtx.getMutex());
                printMtx.flagLock(user.user);
                    cout<<color<<"[Thread "<< threadId << "]\033[0mFailed to acquire device lock. Retrying later..." << endl;
                printMtx.flagUnlock();
                this_thread::sleep_for(chrono::seconds(1));
                continue;
            }
            devMtx.flagLock(user.user);
            int deviceIndex = (userId+i) % devices.size(); //get a random device index 
            dev = devices[deviceIndex];
        }
        {//Print using device
            lock_guard<mutex> lock(printMtx.getMutex()); //makes sure no interleaved output
            printMtx.flagLock(user.user);
                cout<<color<<"[Thread " << threadId << "]\033[0m "<<users[userId].user<<" is using device "<<dev->getId()<<"."<<endl; //User is using device
            printMtx.flagUnlock();
        }
        this_thread::sleep_for(chrono::seconds(sec));
        {//Turn on device
            lock_guard<mutex> lock(printMtx.getMutex()); //makes sure no interleaved output
            printMtx.flagLock(user.user);
                uniform_int_distribution<> dist(0,1); //for random (on or off)

                cout<<color<<"[Thread " << threadId << "]\033[0m ";
                dev->turnOn(dist(gen)); //user turns on device
            printMtx.flagUnlock();
        }
        this_thread::sleep_for(chrono::seconds(sec));
        //Change device settings
        if(Fridge* fridge = dynamic_cast<Fridge*>(dev)){
            lock_guard<mutex> lock(printMtx.getMutex()); //makes sure no interleaved output
            printMtx.flagLock(user.user);
                uniform_int_distribution<> dist(-5,5); //for random (temp)

                cout<<color<<"[Thread " << threadId << "]\033[0m ";
                fridge->putTemp(dist(gen)); //Varying temps
            printMtx.flagUnlock();
        } 
        else if (Light* light = dynamic_cast<Light*>(dev)){
            lock_guard<mutex> lock(printMtx.getMutex()); //makes sure no interleaved output
            printMtx.flagLock(user.user);
                uniform_int_distribution<> dist(0,3); //for random (temp)

                cout<<color<<"[Thread " << threadId << "]\033[0m ";
                light->putBrightness(dist(gen));
            printMtx.flagUnlock();
        }
        this_thread::sleep_for(chrono::seconds(sec));

        {//Print device status
            lock_guard<mutex> lock(printMtx.getMutex()); //makes sure no interleaved output
            printMtx.flagLock(user.user);
                cout<<color<<"[Thread " << threadId << "]\033[0m ";
                dev->showStatus(); //show dev status
            printMtx.flagUnlock();
        }
        this_thread::sleep_for(chrono::seconds(sec));
    }
    
    //LOG OUT USER
    {
        lock_guard<mutex> lock(userMtx.getMutex()); //lockguard user mutex
        userMtx.flagLock(user.user);
            if (userId < 0 || userId >= static_cast<int>(users.size())){ //check if userIndex is out of bounds
                cout<<"\033[1;31m[Thread " << threadId << "] Invalid user index.\033[0m"<<endl;
                return;
            }
            
            user.isLoggedIn = false; //logs out user
            cout<<"\033[1;31m[Thread " << threadId << "] User "<<user.user<<" logged out.\033[0m"<<endl;
        userMtx.flagUnlock();
    }

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
        if(ch == -1){
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


// void concurrencyControl(){
//     cout<<"Displaying lock status"
// }
// void livenessCheck();

void showLocks(){
    cout<<"========= Lock Status ========="<<endl;
    cout<<devMtx.getName()<<": "<<(devMtx.checkLock() ? "Locked by "+ devMtx.getOwner() : "Unlocked")<<endl;
    cout<<userMtx.getName()<<": "<<(userMtx.checkLock() ? "Locked by "+ userMtx.getOwner() : "Unlocked")<<endl;
    cout<<printMtx.getName()<<": "<<(printMtx.checkLock() ? "Locked by "+ printMtx.getOwner() : "Unlocked")<<endl;
    cout<<"==============================="<<endl;
}

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
    l1->setBrightness(3);
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

string getColor(int threadId){
    switch(threadId){
        case 1: return "\033[1;34m"; // Blue
        case 2: return "\033[1;35m"; // Magenta
        case 3: return "\033[1;36m"; // Cyan
        default: return "\033[1;37m"; // White (default)
    }
}

int getCh(){
    int ch;
    cin>>ch;
    if(cin.fail() || (ch > 5 && ch < 0)){
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout<<"Invalid input. Please choose between the numbers listed."<<endl;
        return -1;
    }
    return ch;
}