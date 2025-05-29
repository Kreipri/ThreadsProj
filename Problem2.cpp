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
        string type = "Device";
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
        virtual void toggleOn(){
            this->isOn ? isOn = false : isOn = true;
        }
        virtual void waitUntilOn() {
            shared_lock lock(rwLock);
            devStatChanged.wait(lock, [this]() {
                return isOn;
            });
        }
        virtual string getType(){
            return type;
        }
        virtual void showStatus() = 0;
        virtual ~Device(){}
};

class Fridge : public Device {
    protected:
        int temperature;
        string type = "Fridge";
    public:
        Fridge(string id) : Device(id) {temperature = 5;} //get id used in making this obj to base constructor

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
        string brightness; //Low, Mid, High, Mid is default
        string type = "Light";
    public:
        Light(string id) : Device(id) {brightness = "Mid";} //get id used in making this obj to base constructor
        
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

class AirCon : public Device {
    protected:
        int temperature;
        string type = "Air Conditioner";
    public:
        AirCon(string id) : Device(id) {temperature = 20;} //get id used in making this obj to base constructor

        void showStatus() override {
            shared_lock lock(rwLock);
            cout<<id<<"(Air Conditioner) is "<< (isOn ? "ON" : "OFF")<<" set at "<<temperature<<"\u00B0C"<<endl;
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

struct User {
    string user;
    bool isLoggedIn = 0;
};

class TrackedMutex{
    private:
        atomic<bool> isLocked = false;
        string name;
        string owner;

    public:
        TrackedMutex(string name) : name(name), owner("None"){}
        
        void flagLock(string str){
            isLocked = true;
            owner = str;
        }
        void flagUnlock(){
            isLocked = false;
            owner = "None";
        }

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

};

//Global Variables
vector<Device*> devices;
vector<User> users;

mutex devMtx;
mutex userMtx;
recursive_mutex printMtx;

TrackedMutex devMutex("Device Mutex");
TrackedMutex userMutex("User Mutex");
TrackedMutex printMutex("Print Mutex");

//for generating unique randoms for each thread
random_device rd;
mt19937 gen(rd());

//Prototypes
void mainMenu();
void startThreads();
void simulateUsage(int threadId);
void deviceManagement();
void addDev();
void removeDev();
void listDev();
void displayDev();
void userManagement();
void deviceControl();
void concurrencyControl();
void livenessCheck();
void listUser();
void removeUser();

string getColor(int threadId);
void addUser();
void makeExample();

int getCh(int max);
void showLocks();

void flagLock(TrackedMutex& mtx, string name);
void flagUnlock(TrackedMutex& mtx);

//Main
int main(){
    SetConsoleOutputCP(CP_UTF8); //for degrees


    // makeExample();
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
        cout<<"4 - Concurrency Control"<<endl;
        cout<<"0 - Exit"<<endl;
        cout<<"==============================="<<endl;
        cout<<"Enter Choice: ";
        ch = getCh(4);
        if(ch == -1){
            continue;
        }
        cout<<"\n"<<endl;

        switch(ch){
            case 1: startThreads(); break;
            case 2: deviceManagement(); break;
            case 3: userManagement(); break;
            case 4: showLocks(); break;
            case 0:
                cout<<"Exiting... \n"<<endl;
                cout<<"Group 2"<<endl;
                cout<<"Bea Ganotisi"<<endl;
                cout<<"Denise Ballano"<<endl;
                cout<<"Charisse See"<<endl; 
                cout<<"\n"<<endl;
                return;
        }
    }
}

void startThreads(){
    if (devices.empty() || users.size() < 3){
        cout<<"Please ensure at least 3 users and 1 or more devices are added."<<endl;
        return;
    }

    cout<<"\033[1;32mStarting multiple threads...\033[0m"<<endl;
    vector<thread> threads;
    for (int i = 1; i <= 3; i++){
        threads.emplace_back(simulateUsage, i);
    }

    for (auto& t : threads) t.join();
    cout<<"Simulation done!\n\n"<<endl;
    return;
}

void simulateUsage(int threadId){
    uniform_int_distribution<> secDist(1,2); //for random (sec)
    string color = getColor(threadId);
    int sec = secDist(gen); //diff delays for each thread
    // int sec = 0; //set delays

    uniform_int_distribution<> userDist(0, ((users.size())-1)); //for random (user)
    int userId;
    while(true){
        lock_guard<mutex> lock(userMtx);
        userId = userDist(gen);
        if (userId < 0 || userId >= users.size()){ //check if userIndex is out of bounds
            lock_guard<recursive_mutex> lock(printMtx);
            flagLock(printMutex, "Thread "+ to_string(threadId));
            cout<<color<<"[Thread " << threadId << "]"<<"\033[1;31mInvalid user index.\033[0m"<<endl;
            flagUnlock(printMutex);
            continue;
        } 
        User& user = users[userId];
        string name = user.user;
        if (user.isLoggedIn){
            lock_guard<recursive_mutex> lock(printMtx);
            flagLock(printMutex, "Thread "+ to_string(threadId));
            cout<<color<<"[Thread " << threadId << "]"<<"\033[1;31m "<<name<<" is already logged in.\033[0m"<<endl;
            flagUnlock(printMutex);
            continue;
        }
        user.isLoggedIn = true; //log in user
        {
            flagLock(printMutex, "Thread "+ to_string(threadId));
            lock_guard<recursive_mutex> lock(printMtx);
            cout<<color<<"[Thread " << threadId << "] User "<<user.user<<" logged in.\033[0m"<<endl;
            flagUnlock(printMutex);
        }
        
        flagUnlock(userMutex);
        break;
    }
    
    //LOG IN USER
    {
        scoped_lock<recursive_mutex, mutex> lock(printMtx, userMtx); //printMtx makes sure no interleaved output, userMtx makes sure no race conditions
        
    }
    User& user = users[userId];
    string name = user.user;
    this_thread::sleep_for(chrono::seconds(sec));
    
    //USE DEVICES
    for(int i = 0; i < 3; i++){//simulate 3 actions
        Device* dev = nullptr;

        if(devices.empty()){//if no devices
            cout<<color<<"[Thread "<< threadId << "]\033[0mNo devices available."<<endl;
            return;
        }
        uniform_int_distribution<> devDist(0, ((devices.size())-1)); //for random (user)
        int deviceIndex = devDist(gen); //get a random device index 
        dev = devices[deviceIndex];

        {//Print using device
            lock_guard<recursive_mutex> lock(printMtx); //makes sure no interleaved output
            flagLock(printMutex, name);
                cout<<color<<"[Thread " << threadId << "]\033[0m "<<users[userId].user<<" is using device "<<dev->getId()<<"."<<endl; //User is using device
            flagUnlock(printMutex);
        }
        this_thread::sleep_for(chrono::seconds(sec));
        {//Turn on device
            scoped_lock<mutex, recursive_mutex> lock(devMtx, printMtx); //makes sure no interleaved output
            flagLock(devMutex, name);
            flagLock(printMutex, name);
                uniform_int_distribution<> dist(0,1); //for random (on or off)

                cout<<color<<"[Thread " << threadId << "]\033[0m ";
                dev->turnOn(dist(gen)); //user turns on device
            flagUnlock(printMutex);
            flagUnlock(devMutex);
        }
        this_thread::sleep_for(chrono::seconds(sec));
        //Change device settings
        if(Fridge* fridge = dynamic_cast<Fridge*>(dev)){
            scoped_lock<mutex, recursive_mutex> lock(devMtx, printMtx); //makes sure no interleaved output
            flagLock(devMutex, name);
            flagLock(printMutex, name);
                uniform_int_distribution<> dist(-5,5); //for random (temp)

                cout<<color<<"[Thread " << threadId << "]\033[0m ";
                fridge->putTemp(dist(gen)); //Varying temps
            flagUnlock(printMutex);
            flagUnlock(devMutex);
        } 
        else if (Light* light = dynamic_cast<Light*>(dev)){
            scoped_lock<mutex, recursive_mutex> lock(devMtx, printMtx); //makes sure no interleaved output
            flagLock(devMutex, name);
            flagLock(printMutex, name);
                uniform_int_distribution<> dist(0,3); //for random (temp)

                cout<<color<<"[Thread " << threadId << "]\033[0m ";
                light->putBrightness(dist(gen));
            flagUnlock(printMutex);
            flagUnlock(devMutex);
        }
        this_thread::sleep_for(chrono::seconds(sec));

        {//Print device status
            lock_guard<recursive_mutex> lock(printMtx); //makes sure no interleaved output
            flagLock(printMutex, name);
                cout<<color<<"[Thread " << threadId << "]\033[0m ";
                dev->showStatus(); //show dev status
            flagUnlock(printMutex);
        }
        this_thread::sleep_for(chrono::seconds(sec));
    }
    
    //LOG OUT USER
    {
        scoped_lock<mutex, recursive_mutex> lock(userMtx, printMtx); //lockguard user mutex
        flagLock(userMutex, name);
        flagLock(printMutex, name);
            if (userId < 0 || userId >= static_cast<int>(users.size())){ //check if userIndex is out of bounds
                cout<<"\033[1;31m[Thread " << threadId << "] Invalid user index.\033[0m"<<endl;
                return;
            }
            
            user.isLoggedIn = false; //logs out user
            cout<<"\033[1;31m[Thread " << threadId << "] User "<<user.user<<" logged out.\033[0m"<<endl;
        flagUnlock(printMutex);
        flagUnlock(userMutex);
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
        ch = getCh(3);
        if(ch == -1){
            continue;
        }
        cout<<"\n"<<endl;

        switch(ch){
            case 1: addDev(); break;
            case 2: removeDev(); break;
            case 3: deviceControl(); break;
            case 0: return;
        }
    }
}
void addDev(){
    int type;
    string id;
    cout<<"========= Add Device =========="<<endl;
    cout<<"Enter Device ID (F1,L1,etc.): ";
    cin>>id;
    cout<<"Choose Device Type: "<<endl;
    cout<<"\t1 - Fridge"<<endl;
    cout<<"\t2 - Light"<<endl;
    cout<<"\t3 - Air Conditioner"<<endl;
    cout<<"Choice: ";
    cin>>type;
    cout<<"==============================="<<endl;
    switch(type){
        case 1:
            devices.push_back(new Fridge(id)); 
            cout<<"Fridge "<<id<<" added successfully!"<<endl;
            break;
        case 2: 
            devices.push_back(new Light(id)); 
            cout<<"Light "<<id<<" added successfully!"<<endl;
            break;
        case 3:
            devices.push_back(new AirCon(id)); 
            cout<<"Air Conditioner "<<id<<" added successfully!"<<endl;
            break;
        default:
            cout<<"Error adding device."<<endl;
    }
    cout<<"\n"<<endl;
    return;
}

void removeDev(){
    int indx;
    while(true){
        listDev();
        cout<<"0 - Back"<<endl;
        cout<<"==============================="<<endl;
        cout<<"Choose device to remove: ";
        
        indx = getCh(devices.size());
        if(indx == 0){
            cout<<"\n"<<endl;
            return;
        }
        else if(indx == -1)continue;
        else if(indx <= devices.size() && indx > 0){
            devices.erase(devices.begin() + (indx-1));
            cout<<"Sucessfully deleted."<<endl;
        }
        else{}
        cout<<"\n"<<endl;
        return;
        
    }
}

void listDev(){
    if(devices.empty()){//if no devices
        cout<<"\033[1;33mNo devices available.\033[0m"<<endl;
        return;
    }
    int i = 1;
    for (auto& d : devices) {
        cout<<i<<" - ";
        d->showStatus();
        i++;
    }
}

void userManagement(){
    while(true){
        cout<<"======= User Management ======="<<endl;
        cout<<"1 - Register Users"<<endl;
        cout<<"2 - User List"<<endl;
        cout<<"3 - Remove Users"<<endl;
        cout<<"0 - Back"<<endl;
        cout<<"==============================="<<endl;
        cout<<"Choice: ";
        int ch = getCh(3);
        if(ch == -1){
            cout<<"\n"<<endl;
            continue;
        }
        switch(ch){
            case 1: addUser(); break;
            case 2: 
                cout<<"\n========= User List ==========="<<endl;
                listUser();
                cout<<"===============================\n\n"<<endl; 
                break;
            case 3: removeUser(); break;
            case 0: 
                cout<<"\n"<<endl;
                return;
        }
    }

}

void deviceControl(){
    int indx;
    while(true){
        cout<<"======= Control Device ========"<<endl;
        listDev();
        cout<<"0 - Back"<<endl;
        cout<<"==============================="<<endl;
        cout<<"Choose device to manage: ";
        cin>>indx;
        indx--;
        if(indx == -1){
            cout<<"\n"<<endl; 
            return;
        }
        else if(indx >= 0 && indx < devices.size()){
            cout<<"\n";
            break;
        }
        else{
            cout<<"Enter a valid number."<<endl;
            cout<<"\n";
            continue;
        }
    }
    Device* dev = devices[indx];
    cout<<"======= Control Device ========"<<endl;
    if(Fridge* fridge = dynamic_cast<Fridge*>(dev)){
        fridge->showStatus();
        cout<<"1 - Turn On/Off"<<endl;
        cout<<"2 - Set Temp"<<endl;
        cout<<"0 - Back"<<endl;
        cout<<"==============================="<<endl;
        cout<<"Choice: ";
        int ch = getCh(2);
        cout<<"\n";
        cout<<"==============================="<<endl;
        switch(ch){
            case 1: fridge->toggleOn(); fridge->showStatus(); break;
            case 2: 
                int temp;
                cout<<"Set Temp to: ";
                cin>>temp;
                fridge->putTemp(temp);
                break;
            case 0:
                break;
        }
        cout<<"==============================="<<endl;
    } 
    else if (Light* light = dynamic_cast<Light*>(dev)){
        light->showStatus();
        cout<<"1 - Turn On/Off"<<endl;
        cout<<"2 - Set Brightness"<<endl;
        cout<<"0 - Back"<<endl;
        cout<<"==============================="<<endl;
        cout<<"Choice: ";
        int ch = getCh(2);
        cout<<"\n";
        cout<<"==============================="<<endl;
        switch(ch){
            case 1: light->toggleOn(); light->showStatus(); break;
            case 2: 
                int lvl;
                cout<<"(1 - Low, 2 - Mid, 3 - High)"<<endl;
                cout<<"Set Brightness to : ";
                cin>>lvl;
                cout<<"\n";
                light->putBrightness(lvl);
                break;
            case 0:
                break;
        }
        cout<<"==============================="<<endl;
    }
    else if(AirCon* aircon = dynamic_cast<AirCon*>(dev)){
        aircon->showStatus();
        cout<<"1 - Turn On/Off"<<endl;
        cout<<"2 - Set Temp"<<endl;
        cout<<"0 - Back"<<endl;
        cout<<"==============================="<<endl;
        cout<<"Choice: ";
        int ch = getCh(2);
        cout<<"\n";
        cout<<"==============================="<<endl;
        switch(ch){
            case 1: aircon->toggleOn(); aircon->showStatus(); break;
            case 2: 
                int temp;
                cout<<"Set Temp to: ";
                cin>>temp;
                aircon->putTemp(temp);
                break;
            case 0:
                break;
        }
        cout<<"==============================="<<endl;
    } 
    cout<<"\n"<<endl;
}

void showLocks(){
    cout<<"========= Lock Status ========="<<endl;
    cout<<devMutex.getName()<<": "<<(devMutex.checkLock() ? "Locked by "+ devMutex.getOwner() : "Unlocked")<<endl;
    cout<<userMutex.getName()<<": "<<(userMutex.checkLock() ? "Locked by "+ userMutex.getOwner() : "Unlocked")<<endl;
    cout<<printMutex.getName()<<": "<<(printMutex.checkLock() ? "Locked by "+ printMutex.getOwner() : "Unlocked")<<endl;
    cout<<"==============================="<<endl;
    cout<<"\n"<<endl;
}

void addUser(){
    string user;
    cout<<"\n======== Register User ========"<<endl;
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
    cout<<"===============================\n\n"<<endl;
}

void listUser(){
    int i = 1;
    for (auto& u : users){
        cout<<i<<" - "<<u.user<<endl;
        i++;
    }
}

void removeUser(){
    int indx;
    while(true){
        cout<<"\n======== Remove User =========="<<endl;
        listUser();
        cout<<"0 - Back"<<endl;
        cout<<"==============================="<<endl;
        cout<<"Choose user to remove: ";
        cin>>indx;
        
        if(indx == 0){
            cout<<"\n"<<endl; 
            return;
        }else if(indx <= users.size() && indx > 0){
            users.erase(users.begin() + (indx-1));
            cout<<"Sucessfully deleted."<<endl;
            break;
        }
        else{
            cout<<"Enter a valid number.\n"<<endl;
            continue;
        }
    }
    cout<<"\n"<<endl;
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
    User u1{"Bea",false};
    User u2{"Denise", false};
    User u3{"Charisse", false};

    users.push_back(u1);
    users.push_back(u2);
    users.push_back(u3);

    cout<<"Examples added."<<endl;
}

string getColor(int num){
    switch(num){
        case 1: return "\033[1;34m"; // Blue
        case 2: return "\033[1;35m"; // Magenta
        case 3: return "\033[1;36m"; // Cyan
        default: return "\033[1;37m"; // White (default)
    }
}

int getCh(int max){
    int ch;
    cin>>ch;
    if(cin.fail() || (ch > max || ch < 0)){
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout<<"Invalid input. Please choose between the numbers listed."<<endl;
        return -1;
    }
    return ch;
}

void flagLock(TrackedMutex& mtx, string name){
    lock_guard<recursive_mutex> lock(printMtx);
    mtx.flagLock(name);
    cout<<"\033[30m"<<mtx.getName()<<" locked by "<< mtx.getOwner()<<"!\033[0m"<<endl;
    return;
}

void flagUnlock(TrackedMutex& mtx){
    lock_guard<recursive_mutex> lock(printMtx);
    mtx.flagUnlock();
    cout<<"\033[90m"<<mtx.getName()<<" unlocked!\033[0m"<<endl;
    return;
}