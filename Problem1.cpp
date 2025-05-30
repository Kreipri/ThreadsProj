#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <cstdlib>
#include <ctime>
using namespace std;

#define maxEvents 200
#define maxUsers 200
#define maxTickets 200

//struct
struct Event
{
    string eventID;   
    string eventName;
    string eventDate;
    bool isActive = false;
};

struct User 
{
    string userID;  
    string userName;
    string userPass;
    bool isLoggedin = false;
};

struct Ticket 
{
    string ticketID;    
    string eventID, eventName, eventDate;
    string userID, userName;
    bool isCanceled = false;
};

//classes
class eventManagement
{
    private:
        Event events[maxEvents];
        int eventCount = 0;
        mutable std::shared_mutex eventMtx;
    public:
        bool addEvent (const Event& newEvent)   //function to add an event
        {
            std::unique_lock<std::shared_mutex> lock(eventMtx);
            //prevention of duplicate event IDs
            for (int i = 0; i < eventCount; ++i)
            {
                if (events[i].eventID == newEvent.eventID)
                {
                    cout << "Error. Event ID is taken.\n";
                    return false;
                }
            }
            if (eventCount >= maxEvents)
            {
                return false;
            }
            events[eventCount++] = newEvent;
            cout << "Event has been added!\n";
            return true;
        }
        bool updateEvent (const Event& update)  //function to update event details
        {
            std::unique_lock<std::shared_mutex> lock(eventMtx);
            for (int i = 0; i < eventCount; ++i)
            {
                if (events[i].eventID == update.eventID)
                {
                    events[i] = update;
                    cout << "Event has been updated.\n";
                    return true;
                }
            }
            cout << "Update failed. The event does not exist.\n";
            return false;
        }
        
        void viewEvents() const       //function to display all events and their details
        {
            std::shared_lock<std::shared_mutex> lock(eventMtx);
            if (eventCount == 0)
            {
                cout << "No listed events." << endl;
                return;
            }
            cout << "\n--------Event Details--------\n";
            for (int i = 0; i < eventCount; ++i)
            {
                cout << "Event Name: " << events[i].eventName << "\n";
                cout << "Event ID: " << events[i].eventID << "\n";
                cout << "Date of Event: " << events[i].eventDate << "\n";
                cout << "Status: " << (events[i].isActive ? "Active" : "Inactive") << endl;
                cout << "\n";
            }
        }
        
        bool removeEvent (const string& eventId)    //function to remove an event from the list
        {
            std::unique_lock<std::shared_mutex> lock(eventMtx);
            for (int i = 0; i < eventCount; ++i)
            {
                if (events[i].eventID == eventId)
                {
                    for (int j = i; j < eventCount - 1; ++j)
                    {
                        events[j] = events [j + 1];
                    }
                    eventCount--;
                    cout << "Event has been removed.\n";
                    return true;
                }
            }
            cout << "Couldn't find event.\n";
            return false;
        }
        bool getEventbyID(const string& id, Event& result) const
        {
            std::shared_lock<std::shared_mutex> lock(eventMtx);
            for (int i = 0; i < eventCount; ++i)
            {
                if (events[i].eventID == id && events[i].isActive)
                {
                    result = events[i];
                    return true;
                }
            }
            return false;
        }
        bool isEventLocked() const
        {
            return !eventMtx.try_lock_shared();     //checks if the readable lock is not available
        }
        int getEventcount()
        {
            return eventCount;
        }
        
        Event getEventat(int index)
        {
            if(index >= 0 && index < eventCount)
                return events[index];
            else
                return {};
        }
};

class userManagement
{
    private:
        User users[maxUsers];
        int userCount = 0;
        mutable std::shared_mutex userMtx;
    public:
        bool registerUser(const User& newUser)
        {
            std::unique_lock<std::shared_mutex> lock(userMtx);
            for (int i = 0; i < userCount; ++i)
            {
                if(users[i].userID == newUser.userID)
                {
                    cout << "Error. User already exists.\n";
                    return false;
                }
            }
            if (userCount >= maxUsers)
            {
                return false;
            }
            users[userCount++] = newUser;
            cout << "User has been added.\n";
            return true;
        }
        
        bool loginUser(const string& userID, const string& pass)
        {
            std::unique_lock<std::shared_mutex> lock(userMtx);
            for (int i = 0; i < userCount; ++i)
            {
                if (users[i].userID == userID && users[i].userPass == pass)
                {
                    if (users[i].isLoggedin)
                    {
                        cout << "User is already logged in.\n";
                        return false;
                    }
                    users[i].isLoggedin = true;
                    cout << "Successfully logged in.\n";
                    return true;
                }
            }
            cout << "Credentials are invalid.\n";
            return false;
        }
        
        bool logoutUser(const string& userID)
        {
            std::unique_lock<std::shared_mutex> lock(userMtx);
            for (int i = 0; i <userCount; ++i)
            {
                if (users[i].userID == userID)
                {
                    if (!users[i].isLoggedin)
                    {
                        cout << "User is currently not logged in.\n";
                        return false;
                    }
                    users[i].isLoggedin = false;
                    cout << "Successfully logged out.\n";
                    return true;
                }
            }
            cout << "Unable to find user.\n";
            return false;
        }
        bool isUserLocked() const 
        {
            return !userMtx.try_lock_shared();
        }
        int getUsercount()
        {
            return userCount;
        }
        
        User getUserat(int index)
        {
            if(index >= 0 && index < userCount)
                return users[index];
            else
                return{};
        }
};

class ticketManagement
{
    private:
        Ticket tickets[100];
        int ticketCount = 0;
        mutable std::shared_mutex ticketMtx;
  
    public:
        bool purchaseTicket (const string& userID, const string& userName, const Event& event)
        {
            std::unique_lock<std::shared_mutex> lock(ticketMtx);
    
            if (ticketCount >= maxTickets)
            {
                cout << "Ticket limit reached.\n";
                return false;
            }
            string ticketID = "T" + to_string(ticketCount + 1);
    
            Ticket newTicket;
            newTicket.ticketID = ticketID;
            newTicket.userID = userID;
            newTicket.userName = userName;
            newTicket.eventID = event.eventID;
            newTicket.eventName = event.eventName;
            newTicket.eventDate = event.eventDate;
            newTicket.isCanceled = false;
    
            tickets[ticketCount++] = newTicket;
            cout << "Thank you for your purchase. Your Ticket ID is: " << ticketID << endl;
            return true;
        }
        bool cancelTicket (const string& ticketID)
        {
            std::unique_lock<std::shared_mutex> lock(ticketMtx);
            for (int i = 0; i < ticketCount; ++i)
            {
                if (tickets[i].ticketID == ticketID && !tickets[i].isCanceled)
                {
                    tickets[i].isCanceled = true;
                    cout << "Ticket " << ticketID << " has been successfully canceled.\n";
                    return true;
                }
            }
            cout << "Error. Cannot find ticket.\n";
            return false;
        }
        void viewEventtickets (const string& eventID) const
        {
            std::shared_lock<std::shared_mutex> lock(ticketMtx);
            bool found = false;
            
            cout << "\n--------Tickets for Event ID: " << eventID << "--------\n";
            for (int i = 0; i < ticketCount; ++i)
            {
                const Ticket& tix = tickets[i];
                if (tix.eventID == eventID && !tix.isCanceled)
                {
                    cout << "Ticket ID: " << tix.ticketID << "\n";
                    cout << "User: " << tix.userName << " (ID: " <<tix.userID << ")\n\n";
                    found = true;
                }
            }
            if (!found)
            {
                cout << "There are no active tickets for this event.\n";
            }
        }
        void viewActiveTickets() const
        {
            std::shared_lock<std::shared_mutex> lock(ticketMtx);
            cout << "\n--------Active Tickets--------\n";
            bool found = false;
            for (int i = 0; i < ticketCount; ++i)
            {
                const Ticket& tix = tickets[i];
                if (!tix.isCanceled)
                {
                    cout << "Ticket ID: " << tix.ticketID << "\n";
                    cout << "Event Name: " << tix.eventName << "(ID: " << tix.eventID << ")\n";
                    cout << "User: " << tix.userName << "(ID: " << tix.userID << ")\n\n";
                    found = true;
                }
            }
            if (!found)
            {
                cout << "There are no active tickets available.\n";
            }
        }
        bool isTicketLocked() const
        {
            return !ticketMtx.try_lock_shared();
        }
        string findTicketID(string userID, string eventID)
        {
            for (int i = 0; i < ticketCount; i++)
            {
                if (tickets[i].userID == userID && tickets[i].eventID == eventID)
                {
                    return tickets[i].ticketID;
                }
            }
            return "";
        }

};

//global instances
eventManagement event;
userManagement user;
ticketManagement ticket;


//prototype
 void displayMenu();
 void manageEvents();
 void manageUsers();
 void manageTickets();
 void concurrencyControl();
 void liveness();
 void simulateOperations();

int main(){
	displayMenu();
	
}

//function
void displayMenu()
{
	int choice;
	
	while (true)
	{
		cout << "==============================\n";
		cout << "    EVENT TICKETING SYSTEM    \n";
		cout << "==============================\n";
		cout << "1. Event Management" << endl;
		cout << "2. User Management" << endl;
		cout << "3. Ticket Management" << endl;
		cout << "4. Concurrency Control" << endl;
		cout << "5. Simulate Multiple Threads" << endl;
		cout << "6. Exit" <<endl;
		cout << "==============================\n";
		cout << "Enter your choice: ";
		cin >> choice;
		
		switch (choice)
		{
			case 1:             //event management
				manageEvents();
				break;
			case 2:            //user management
				manageUsers();
				break;
			case 3:            //ticket management
				manageTickets();
				break;
			case 4:
				concurrencyControl();
				break;
			case 5:
			    simulateOperations();
			    break;
			case 6:            //exits the program
				cout << "Exiting Program..." << endl;
				return;
			default:           //will be displayed when the user enters a number greater than 6.
				cout << "Invalid input. Please choose from 1-6 only.\n";
				continue;
		}
	}
}

void manageEvents()
{
	int choice;
	
	while (true)
	{
		cout << "==============================\n";
		cout << "        Event Management      \n";
		cout << "==============================\n";
		cout << "1. Add new event" << endl;
		cout << "2. Update event details" << endl;
		cout << "3. Remove an event" << endl;
		cout << "4. View all events" << endl;
		cout << "5. Return to Main Menu" << endl;
		cout << "==============================\n";
		cout << "Enter your choice: ";
		cin >> choice;
		
		switch (choice)
		{
			case 1:                     //add new event
			{	
				Event newEvent;
				cout << "Event ID: ";
				cin >> newEvent.eventID;
				cout << "Event Name: ";
				cin.ignore();
				getline(cin, newEvent.eventName);
				cout << "Date of Event: ";
				getline(cin, newEvent.eventDate);
				cout << "Event status (1 = Yes, 0 = No): ";
				cin >> newEvent.isActive;
				
				event.addEvent(newEvent);
				break;
			}
			case 2:                     //update event details
			{	
				Event update;
				cout << "Update to Event ID: ";
				cin >> update.eventID;
				cout << "Update to Event Name: ";
				cin.ignore();
				getline(cin, update.eventName);
				cout << "Update to the Date of Event: ";
				getline(cin, update.eventDate);
				cout << "Is the event still active? (1 = Yes, 0 = No): ";
				cin >> update.isActive;
				
				event.updateEvent(update);
				break;
			}
			case 3:                    //remove an event from the existing events
			{
			    event.viewEvents();
			    
			    string eventToremove;
			    cout << "What event would you like to remove? Enter the Event ID: ";
			    cin >> eventToremove;
			    
			    event.removeEvent(eventToremove);
				break;
			}
			case 4:                    //displays all existing events and their corresponding details
				event.viewEvents();
				break;
			case 5:                    //returns to main menu (Event Ticketing System)
				return;
			default:                   //will be displayed when the user enters a number greater than 5.
				cout << "Invalid input. Please choose fom 1-5 only. \n";
				continue;
		}
	}
}

void manageUsers()
{
	int choice;
	string id;
	string name;
	string pass;
	
	while (true)
	{
		cout << "==============================\n";
		cout << "        User Management       \n";
		cout << "==============================\n";
		cout << "1. Register new user" << endl;
		cout << "2. User Login" << endl;
		cout << "3. User Logout" << endl;
		cout << "4. Return to Main Menu" << endl;
		cout << "==============================\n";
		cout << "Enter your choice: ";
		cin >> choice;
		
		switch (choice)
		{
			case 1:
			{
                User newUser;
                cout << "Enter User ID: ";
                cin >> newUser.userID;
                cout << "Enter Username: ";
                cin.ignore();
                getline(cin, newUser.userName);
                cout << "Enter Your Password: ";
                cin >> newUser.userPass;
                
                user.registerUser(newUser);
				break;
			}
			case 2:
			{
				cout << "User ID: ";
				cin >> id;
				cout << "Password: ";
				cin >> pass;
				
				user.loginUser(id, pass);
				break;
			}
			case 3:
			{
				cout << "Enter User ID to Logout: ";
				cin >> id;
				
				user.logoutUser(id);
				break;
			}
			case 4:
				return;
			default:
				cout << "Invalid input. Please choose from 1-4 only.\n";
				continue;
		}
	}
}

void manageTickets()
{
	int choice;
	string ticketID;
	
	while (true)
	{
		cout << "=============================================\n";
		cout << "              Ticket Management              \n";
		cout << "=============================================\n";
		cout << "1. Purchase a ticket" << endl;
		cout << "2. View tickets by event" << endl;
		cout << "3. Cancel purchased tickets" << endl;
		cout << "4. Return to Main Menu " << endl;
		cout << "=============================================\n";
		cout << "Enter your choice: ";
		cin >> choice;
		
		switch (choice)
		{
			case 1:     //purchase a ticket
            {  
                string userID;
                string userName;
                string eventID;
                
                cout << "User ID: ";
                cin >> userID;
                cout << "Username: ";
                cin.ignore();
                getline(cin, userName);
                cout << "Purchase ticket for Event ID: ";
                cin >> eventID;
                
                Event chosenEvent;
                if (!event.getEventbyID(eventID, chosenEvent))
                {
                    cout << "Cannot find event.\n";
                    break;
                }
                
                bool success = ticket.purchaseTicket(userID, userName, chosenEvent);
                if (!success)
                {
                    cout << "Purchase failed.\n";
                }
				break;
            }
			case 2:     //view tickets by event
            {
                string eventID;
                cout << "Enter Event ID to view tickets: ";
                cin >> eventID;
                
                ticket.viewEventtickets(eventID);
				break;
            }
			case 3:     //cancel purchased tickets
            {
                ticket.viewActiveTickets();
                cout << "Enter the Ticket ID you want to cancel: ";
                cin >> ticketID;
                
                ticket.cancelTicket(ticketID);
				break;
            }
			case 4:    //returns to main menu (EVENT TICKETING SYSTEM)
				return;
			default:    //will be displayed if the user enters a number greater than 4.
				cout << "Invalid input. Please choose from 1-4 only.\n";
				continue;	
		}
	}
}

void concurrencyControl()
{
    cout << "\n--------Lock Status--------\n";
    cout << "Event Lock: " << (event.isEventLocked() ? "Locked" : "Unlocked") << "\n";
    cout << "User Lock: " << (user.isUserLocked() ? "Locked" : "Unlocked") << "\n";
    cout << "Ticket Lock: " << (ticket.isTicketLocked() ? "Locked" : "Unlocked") << "\n\n";
}

void simulateDeadlockavoidance()
{
    auto task1 = []()
    {
        //simulation of the logging and ticket purchasing of a user (1)
        user.loginUser("U01", "pass");
        Event evt;
        if (event.getEventbyID("E01", evt))
        {
            ticket.purchaseTicket("U01", "User 1", evt);
        }
        user.logoutUser("U01");
    };
    
    auto task2 = []()
    {
        //simulation of the logging in and ticket purchasing of another user (2)
        user.loginUser("U02", "pass");
        Event evt;
        if (event.getEventbyID("E01", evt))
        {
            ticket.purchaseTicket("U02", "User 2", evt);
        }
        user.logoutUser("U02");
    };
    
    thread t1(task1);
    thread t2(task2);
    
    t1.join();
    t2.join();
    
    cout << "No deadlocks encountered.\n";
}

void liveness()
{
    cout << "\n--------Liveness Check--------\n";
    
    //adding an event and two users
    Event e1 = {"E01", "The Ultimate Concierto", "07-13-2025", true};
    User u1 = {"U01", "User 1", "pass"};
    User u2 = {"U02", "User 2", "pass"};
    
    event.addEvent(e1);
    user.registerUser(u1);
    user.registerUser(u2);
    
    simulateDeadlockavoidance();
}

void simulateOperations()
{
    cout << "\n--------Simulating Multiple Threads--------\n";
    
    const int maxUserThreads = 4;
    User registeredUsers[maxUserThreads];
    int userCount = user.getUsercount();
    
    if (userCount == 0)
    {
        cout << "No users registered. Please register users first.\n";
        return;
    }
    
    if (event.getEventcount() == 0)
    {
        cout << "No events available. Please add events first.\n";
        return;
    }
    
    if (userCount < maxUserThreads)
    {
        cout << "Not enough users to simulate. Register at least " << maxUserThreads << " users.\n";
        return;
    }
    
    //copies the 4 users
    for (int i = 0; i < maxUserThreads; ++i)
    {
        registeredUsers[i] = user.getUserat(i);
    }
    
    Event targetEvent;
    if (!event.getEventbyID("E01", targetEvent))
    {
        targetEvent = event.getEventat(0);
    }
    
    srand(time(0));
    
    //thread tasks
    auto userTask = [&](User u)
    {
        cout << "\n[" << u.userName << "] Logging in...\n";
        user.loginUser(u.userID, u.userPass);
        this_thread::sleep_for(chrono::milliseconds(100));
        
        cout << "[" << u.userName << "] Viewing events...\n";
        event.viewEvents();
        this_thread::sleep_for(chrono::milliseconds(100));
        
        cout << "[" <<u.userName << "] Purchasing a ticket for event: " << targetEvent.eventName << "\n";
        ticket.purchaseTicket(u.userID, u.userName, targetEvent);
        this_thread::sleep_for(chrono::milliseconds(100));
        
        int action = rand() % 3;
        
        switch (action)
        {
            case 0:
                cout << "[" <<u.userName << "] Buying another ticket for " <<targetEvent.eventName << "\n";
                ticket.purchaseTicket(u.userID, u.userName, targetEvent);
                break;
            case 1:
            {
                cout << "[" << u.userName << "] Cancelling a ticket for " <<targetEvent.eventName << "\n";
                
                int eventIndex = rand() % event.getEventcount();
                Event targetEvent = event.getEventat(eventIndex);
                
                string ticketID = ticket.findTicketID(u.userID, targetEvent.eventID);
                if (!ticketID.empty())
                {
                    if (ticket.cancelTicket(ticketID))
                    {
                        cout << "[" << u.userName << "] Successfully canceled the ticket for event: " <<targetEvent.eventName << endl;
                    }
                    else
                    {
                        cout << "[" << u.userName << "has no ticket for event: " << targetEvent.eventName << endl;
                    }
                    break;
                }
            }
            case 2:
            {
                cout << "[" <<u.userName << "] Viewing events again...\n";
                event.viewEvents();
                break;
            }
        }
        this_thread::sleep_for(chrono::milliseconds(100));
        
        cout << "[" << u.userName << "] Viewing all active tickets...\n";
        ticket.viewActiveTickets();
        this_thread::sleep_for(chrono::milliseconds(100));
        
        cout << "[" << u.userName << "] Logging out...\n";
        user.logoutUser(u.userID);
    };
    
    thread t1(userTask, registeredUsers[0]);
    thread t2(userTask, registeredUsers[1]);
    thread t3(userTask, registeredUsers[2]);
    thread t4(userTask, registeredUsers[3]);
    
    t1.join();
    t2.join();
    t3.join();
    t4.join();
}
