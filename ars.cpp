#include <iostream>
#include <fstream>
#include <cstring>
#include <queue>
#include <stack>
#include <vector>
#include <iomanip>
#include <string>
#include <regex>
#include <random>
#include <cctype>

using namespace std;

// ==========================================
// PASSENGER CLASS
// ==========================================
class Passenger
{
private:
    string firstName, lastName, phone, email, passportID;
    string meal, travelClass, pnr, status;
    int age, row, col;
    double price;

    void generatePNR()
    {
        static const string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        static random_device rd;
        static mt19937 gen(rd());
        uniform_int_distribution<size_t> distChar(0, chars.size() - 1);
        uniform_int_distribution<int> distDigit(0, 9);

        pnr = "";
        for (int i = 0; i < 2; i++) pnr += chars[distChar(gen)];
        for (int i = 0; i < 2; i++) pnr += to_string(distDigit(gen));
        pnr += chars[distChar(gen)];
        for (int i = 0; i < 3; i++) pnr += to_string(distDigit(gen));
    }

public:
    Passenger(string fn = "", string ln = "", int a = 0, string ph = "", 
              string em = "", string pass = "", string cls = "", string m = "")
        : firstName(fn), lastName(ln), phone(ph), email(em), passportID(pass),
          meal(m), travelClass(cls), status("Active"), age(a), row(-1), col(-1), price(0.0)
    {
        generatePNR();
    }

    void assignSeat(int r, int c) { row = r; col = c; }
    void setPrice(double p)       { price = p; }
    void setStatus(string s)      { status = s; }

    double getPrice() const       { return price; }
    string getFullName() const    { return firstName + " " + lastName; }
    int getRow() const            { return row; }
    int getCol() const            { return col; }
    string getPNR() const         { return pnr; }
    string getClass() const       { return travelClass; }
    string getMeal() const        { return meal; }
    string getStatus() const      { return status; }
};

// ==========================================
// LINKED LIST NODE & FLIGHT CLASS
// ==========================================
struct PassengerNode
{
    Passenger data;
    PassengerNode *next;
    PassengerNode(const Passenger &p) : data(p), next(nullptr) {}
};

class Flight
{
private:
    string flightID, from, to, depTime, arrTime;
    static constexpr int ROWS = 27;
    static constexpr int COLS = 10;
    int seats[ROWS][COLS];
    PassengerNode *passengerHead;
    stack<pair<int, int>> undoStack;
    double revenue;
    int totalPassengers;
    string status;

    void clearPassengerList()
    {
        PassengerNode *curr = passengerHead;
        while (curr)
        {
            PassengerNode *temp = curr;
            curr = curr->next;
            delete temp;
        }
        passengerHead = nullptr;
    }

public:
    Flight(string fid = "", string f = "", string t = "", string d = "", string a = "")
        : flightID(fid), from(f), to(t), depTime(d), arrTime(a),
          passengerHead(nullptr), revenue(0.0), totalPassengers(0), status("On-Time")
    {
        memset(seats, 0, sizeof(seats));
    }

    // Rule of Three: destructor, copy constructor, and copy assignment
    ~Flight()
    {
        clearPassengerList();
    }

    Flight(const Flight &other)
        : flightID(other.flightID), from(other.from), to(other.to),
          depTime(other.depTime), arrTime(other.arrTime),
          undoStack(other.undoStack), revenue(other.revenue),
          totalPassengers(other.totalPassengers), status(other.status),
          passengerHead(nullptr)
    {
        memcpy(seats, other.seats, sizeof(seats));
        PassengerNode **tail = &passengerHead;
        for (PassengerNode *curr = other.passengerHead; curr; curr = curr->next)
        {
            *tail = new PassengerNode(curr->data);
            tail = &((*tail)->next);
        }
    }

    Flight &operator=(const Flight &other)
    {
        if (this != &other)
        {
            clearPassengerList();
            flightID = other.flightID;
            from = other.from;
            to = other.to;
            depTime = other.depTime;
            arrTime = other.arrTime;
            undoStack = other.undoStack;
            revenue = other.revenue;
            totalPassengers = other.totalPassengers;
            status = other.status;
            memcpy(seats, other.seats, sizeof(seats));

            PassengerNode **tail = &passengerHead;
            for (PassengerNode *curr = other.passengerHead; curr; curr = curr->next)
            {
                *tail = new PassengerNode(curr->data);
                tail = &((*tail)->next);
            }
        }
        return *this;
    }

    string getID() const            { return flightID; }
    string getRoute() const         { return from + " -> " + to; }
    string getTime() const          { return depTime + " - " + arrTime; }
    string getStatus() const        { return status; }
    double getRevenue() const       { return revenue; }
    int getTotalPassengers() const  { return totalPassengers; }
    void setStatus(const string &s) { status = s; }

    void displaySeatMap() const
    {
        cout << "\nSeat Map: [O]=Available [X]=Booked | First: 1-9, Business: 10-18, Economy: 19-27\n";
        cout << "    A  B  C     D  E  F  G     H  I  J\n";
        for (int r = 0; r < ROWS; r++)
        {
            if (r == 9 || r == 18)
                cout << "\n";
            cout << setw(2) << r + 1 << " ";
            for (int c = 0; c < COLS; c++)
            {
                cout << "[" << (seats[r][c] ? "X" : "O") << "]";
                if (c == 2 || c == 6)
                    cout << "   ";
            }
            cout << "\n";
        }
    }

    bool bookSeat(Passenger &p, int r, int c)
    {
        if (r < 0 || r >= ROWS || c < 0 || c >= COLS || seats[r][c] == 1)
            return false;

        seats[r][c] = 1;
        p.assignSeat(r, c);
        undoStack.push({r, c});

        double basePrice = 5000.0;
        if (r <= 8)
            p.setPrice(basePrice * 1.75);
        else if (r <= 17)
            p.setPrice(basePrice * 1.40);
        else
            p.setPrice(basePrice);

        revenue += p.getPrice();
        totalPassengers++;

        PassengerNode *node = new PassengerNode(p);
        node->next = passengerHead;
        passengerHead = node;
        return true;
    }

    Passenger *findPassengerByPNR(const string &pnr)
    {
        PassengerNode *curr = passengerHead;
        while (curr)
        {
            if (curr->data.getPNR() == pnr)
                return &curr->data;
            curr = curr->next;
        }
        return nullptr;
    }

    double cancelBookingByPNR(const string &pnr)
    {
        PassengerNode *curr = passengerHead;
        while (curr)
        {
            if (curr->data.getPNR() == pnr && curr->data.getStatus() != "Cancelled")
            {
                curr->data.setStatus("Cancelled");

                int r = curr->data.getRow();
                int c = curr->data.getCol();
                if (r >= 0 && r < ROWS && c >= 0 && c < COLS)
                    seats[r][c] = 0;

                curr->data.assignSeat(-1, -1);

                double refund = 0.8 * curr->data.getPrice();
                revenue -= refund;
                totalPassengers--;

                return refund;
            }
            curr = curr->next;
        }
        return -1.0;
    }

    void displayPassengerList() const
    {
        if (!passengerHead)
        {
            cout << "No passengers booked yet on this flight.\n";
            return;
        }

        cout << left
             << setw(6)  << "ID"     << "|"
             << setw(25) << "Name"   << "|"
             << setw(8)  << "Seat"   << "|"
             << setw(10) << "Class"  << "|"
             << setw(12) << "Meal"   << "|"
             << setw(12) << "PNR"    << "|"
             << setw(12) << "Status" << "\n";
        cout << string(85, '-') << "\n";

        PassengerNode *curr = passengerHead;
        int id = 1;
        while (curr)
        {
            string seatStr = (curr->data.getStatus() == "Cancelled") 
                ? "--" 
                : string(1, char('A' + curr->data.getCol())) + to_string(curr->data.getRow() + 1);

            cout << left
                 << setw(6)  << id                          << "|"
                 << setw(25) << curr->data.getFullName()    << "|"
                 << setw(8)  << seatStr                     << "|"
                 << setw(10) << curr->data.getClass()      << "|"
                 << setw(12) << curr->data.getMeal()       << "|"
                 << setw(12) << curr->data.getPNR()        << "|"
                 << setw(12) << curr->data.getStatus()     << "\n";
            curr = curr->next;
            id++;
        }
    }

    void savePassengerList() const
    {
        ofstream fout(flightID + "_passenger_list.txt");
        if (!fout.is_open()) return;

        fout << left
             << setw(6)  << "ID"     << "|"
             << setw(25) << "Name"   << "|"
             << setw(8)  << "Seat"   << "|"
             << setw(10) << "Class"  << "|"
             << setw(12) << "Meal"   << "|"
             << setw(12) << "PNR"    << "|"
             << setw(12) << "Status" << "\n";
        fout << string(85, '-') << "\n";

        PassengerNode *curr = passengerHead;
        int id = 1;
        while (curr)
        {
            string seatStr = (curr->data.getStatus() == "Cancelled")
                ? "--"
                : string(1, char('A' + curr->data.getCol())) + to_string(curr->data.getRow() + 1);

            fout << left
                 << setw(6)  << id                          << "|"
                 << setw(25) << curr->data.getFullName()    << "|"
                 << setw(8)  << seatStr                     << "|"
                 << setw(10) << curr->data.getClass()      << "|"
                 << setw(12) << curr->data.getMeal()       << "|"
                 << setw(12) << curr->data.getPNR()        << "|"
                 << setw(12) << curr->data.getStatus()     << "\n";
            curr = curr->next;
            id++;
        }
    }
};

// ==========================================
// GLOBALS & VALIDATORS
// ==========================================
vector<string> cities = {"Ahmedabad", "Mumbai", "Delhi", "Chennai"};
vector<Flight> flights;

bool validatePhone(const string &phone)
{
    return regex_match(phone, regex("^[0-9]{10}$"));
}

bool validateEmail(const string &email)
{
    return regex_match(email, regex("^[\\w.-]+@[\\w.-]+\\.[A-Za-z]{2,}$"));
}

bool validatePassport(const string &passport)
{
    return regex_match(passport, regex("^[A-Z][0-9]{7}$"));
}

bool parseSeatInput(const string &input, int &row, int &col)
{
    if (input.size() < 2 || input.size() > 3) return false;

    char colChar = toupper(input.back());
    if (colChar < 'A' || colChar > 'J') return false;

    string rowStr = input.substr(0, input.size() - 1);
    for (char ch : rowStr)
    {
        if (!isdigit(ch)) return false;
    }

    int parsedRow = stoi(rowStr);
    if (parsedRow < 1 || parsedRow > 27) return false;

    row = parsedRow - 1;
    col = colChar - 'A';
    return true;
}

// ==========================================
// CONTROLLER FUNCTIONS
// ==========================================
Flight *selectFlight()
{
    cout << "\nSelect Departure City:\n";
    for (size_t i = 0; i < cities.size(); i++)
        cout << i + 1 << ". " << cities[i] << "\n";
    int dep;
    cout << "Enter choice: ";
    if (!(cin >> dep) || dep < 1 || dep > (int)cities.size())
    {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid selection.\n";
        return nullptr;
    }
    dep--;

    cout << "\nSelect Destination City:\n";
    vector<int> validDestIndices;
    for (size_t i = 0; i < cities.size(); i++)
    {
        if ((int)i == dep) continue;
        validDestIndices.push_back(i);
        cout << validDestIndices.size() << ". " << cities[i] << "\n";
    }

    int destChoice;
    cout << "Enter choice: ";
    if (!(cin >> destChoice) || destChoice < 1 || destChoice > (int)validDestIndices.size())
    {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid selection.\n";
        return nullptr;
    }
    int dest = validDestIndices[destChoice - 1];

    string targetRoute = cities[dep] + " -> " + cities[dest];
    vector<Flight *> options;
    for (auto &f : flights)
    {
        if (f.getRoute() == targetRoute)
            options.push_back(&f);
    }

    if (options.empty())
    {
        cout << "No flights available between " << cities[dep] << " and " << cities[dest] << ".\n";
        return nullptr;
    }

    cout << "\nAvailable Flights:\n";
    for (size_t i = 0; i < options.size(); i++)
        cout << i + 1 << ". " << options[i]->getID() << " | " << options[i]->getTime() << " | " << options[i]->getStatus() << "\n";

    int choice;
    cout << "Select flight: ";
    if (!(cin >> choice) || choice < 1 || choice > (int)options.size())
    {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid flight choice.\n";
        return nullptr;
    }
    return options[choice - 1];
}

void handleAdminPanel()
{
    int attempts = 3;
    const string adminPassword = "Admin123";

    while (attempts > 0)
    {
        cout << "\n--- Admin Access ---\n";
        cout << "Enter Password (" << attempts << " attempts remaining): ";
        string enteredPassword;
        cin >> enteredPassword;

        if (enteredPassword == adminPassword)
        {
            cout << "\nAccess granted.\nEnter Flight ID to manage: ";
            string fid;
            cin >> fid;

            Flight *targetFlight = nullptr;
            for (auto &f : flights)
            {
                if (f.getID() == fid)
                {
                    targetFlight = &f;
                    break;
                }
            }

            if (!targetFlight)
            {
                cout << "Flight not found.\n";
                return;
            }

            int achoice = 0;
            while (achoice != 6)
            {
                cout << "\n--- Admin Menu (" << targetFlight->getID() << ") ---\n"
                     << "1. View Seat Map\n"
                     << "2. View Passenger List\n"
                     << "3. Add New Flight\n"
                     << "4. Booking Statistics\n"
                     << "5. Update Flight Status\n"
                     << "6. Exit Admin Menu\n"
                     << "Enter choice: ";
                if (!(cin >> achoice))
                {
                    cin.clear();
                    cin.ignore(1000, '\n');
                    continue;
                }

                switch (achoice)
                {
                case 1:
                    targetFlight->displaySeatMap();
                    break;
                case 2:
                    targetFlight->displayPassengerList();
                    break;
                case 3:
                {
                    string id, from, to, dep, arr;
                    cout << "Flight ID: "; cin >> id;
                    cout << "From: ";      cin >> from;
                    cout << "To: ";        cin >> to;
                    cout << "Departure: "; cin >> dep;
                    cout << "Arrival: ";   cin >> arr;
                    flights.push_back(Flight(id, from, to, dep, arr));
                    cout << "Flight " << id << " added successfully.\n";
                    break;
                }
                case 4:
                    cout << "Total Passengers: " << targetFlight->getTotalPassengers() << "\n";
                    cout << "Current Revenue: INR " << fixed << setprecision(2) << targetFlight->getRevenue() << "\n";
                    break;
                case 5:
                {
                    string status;
                    cout << "Enter new status (On-Time / Delayed / Cancelled): ";
                    cin >> status;
                    targetFlight->setStatus(status);
                    cout << "Status updated to " << status << ".\n";
                    break;
                }
                case 6:
                    cout << "Exiting admin menu...\n";
                    break;
                default:
                    cout << "Invalid choice.\n";
                }
            }
            return;
        }
        else
        {
            attempts--;
            cout << "Incorrect password.\n";
        }
    }
    cout << "Too many failed attempts. Returning to main menu.\n";
}

// ==========================================
// MAIN ENTRY POINT
// ==========================================
int main()
{
    // Seed initial flight schedules
    flights.push_back(Flight("EY0641", "Ahmedabad", "Mumbai", "06:00", "08:00"));
    flights.push_back(Flight("AI2035", "Ahmedabad", "Mumbai", "09:30", "11:30"));
    flights.push_back(Flight("AI3042", "Mumbai", "Ahmedabad", "07:00", "09:00"));
    flights.push_back(Flight("6E2020", "Mumbai", "Ahmedabad", "12:00", "14:00"));

    int menu = 0;

    while (menu != 5)
    {
        cout << "\n==============================================\n";
        cout << "          AIRLINE RESERVATION SYSTEM          \n";
        cout << "==============================================\n";
        cout << "1. Book Flights\n";
        cout << "2. Manage Booking\n";
        cout << "3. View All Flights\n";
        cout << "4. Admin Panel\n";
        cout << "5. Exit\n";
        cout << "Enter your choice: ";

        if (!(cin >> menu))
        {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        switch (menu)
        {
        case 1:
        {
            Flight *flight = selectFlight();
            if (!flight) break;

            int num;
            cout << "Enter number of passengers: ";
            if (!(cin >> num) || num <= 0)
            {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << "Invalid passenger count.\n";
                break;
            }

            queue<Passenger> bookingQueue;
            for (int i = 0; i < num; i++)
            {
                string fn, ln, ph, em, pass, cls, meal;
                int age;
                cout << "\n--- Passenger " << i + 1 << " Details ---\n";
                cout << "First Name: "; cin >> fn;
                cout << "Last Name: ";  cin >> ln;
                cout << "Age: ";        cin >> age;

                do {
                    cout << "Phone (10 digits): ";
                    cin >> ph;
                } while (!validatePhone(ph));

                do {
                    cout << "Email: ";
                    cin >> em;
                } while (!validateEmail(em));

                do {
                    cout << "Passport ID (1 letter + 7 digits, e.g. A1234567): ";
                    cin >> pass;
                } while (!validatePassport(pass));

                cout << "Select Travel Class (Economy/Business/First): ";
                cin >> cls;
                cout << "Select Meal (Veg/Non-Veg/Jain): ";
                cin >> meal;

                bookingQueue.push(Passenger(fn, ln, age, ph, em, pass, cls, meal));
            }

            while (!bookingQueue.empty())
            {
                Passenger p = bookingQueue.front();
                bookingQueue.pop();

                flight->displaySeatMap();
                cout << "Select seat for " << p.getFullName() << " (e.g. 2A, 12D): ";
                string seatInput;
                cin >> seatInput;

                int r = -1, c = -1;
                if (!parseSeatInput(seatInput, r, c))
                {
                    cout << "Invalid seat coordinate format. Try again.\n";
                    bookingQueue.push(p);
                    continue;
                }

                if (flight->bookSeat(p, r, c))
                {
                    cout << "Ticket booked successfully! PNR: " << p.getPNR() << "\n";
                    cout << "Price: INR " << fixed << setprecision(2) << p.getPrice() << "\n";
                }
                else
                {
                    cout << "Seat unavailable or out of bounds. Try again.\n";
                    bookingQueue.push(p);
                }
            }
            flight->savePassengerList();
            break;
        }

        case 2:
        {
            cout << "\nManage Booking:\n1. View My Booking\n2. Cancel & Refund\nEnter choice: ";
            int mchoice;
            if (!(cin >> mchoice))
            {
                cin.clear();
                cin.ignore(1000, '\n');
                break;
            }

            if (mchoice == 1)
            {
                string pnr;
                cout << "Enter your PNR: ";
                cin >> pnr;

                bool found = false;
                for (auto &f : flights)
                {
                    Passenger *p = f.findPassengerByPNR(pnr);
                    if (p)
                    {
                        found = true;
                        cout << "\n--- Booking Details ---\n";
                        cout << "Flight: "  << f.getID() << " (" << f.getRoute() << ")\n";
                        cout << "Name:   "  << p->getFullName() << "\n";
                        cout << "Class:  "  << p->getClass() << "\n";
                        cout << "Meal:   "  << p->getMeal() << "\n";
                        cout << "PNR:    "  << p->getPNR() << "\n";
                        cout << "Status: "  << p->getStatus() << "\n";
                        break;
                    }
                }
                if (!found) cout << "PNR not found.\n";
            }
            else if (mchoice == 2)
            {
                string pnr;
                cout << "Enter PNR to cancel: ";
                cin >> pnr;

                bool found = false;
                for (auto &f : flights)
                {
                    Passenger *p = f.findPassengerByPNR(pnr);
                    if (p)
                    {
                        found = true;
                        if (p->getStatus() == "Cancelled")
                        {
                            cout << "This ticket has already been cancelled.\n";
                            break;
                        }

                        cout << "Refund amount (80%): INR " << fixed << setprecision(2) << 0.8 * p->getPrice() << "\n";
                        cout << "Confirm cancellation? (Y/N): ";
                        char conf;
                        cin >> conf;
                        if (toupper(conf) == 'Y')
                        {
                            double refund = f.cancelBookingByPNR(pnr);
                            cout << "Booking cancelled. Refund issued: INR " << refund << "\n";
                            f.savePassengerList();
                        }
                        else
                        {
                            cout << "Cancellation aborted.\n";
                        }
                        break;
                    }
                }
                if (!found) cout << "PNR not found.\n";
            }
            break;
        }

        case 3:
        {
            cout << "\n--- All Flights ---\n";
            for (const auto &f : flights)
            {
                cout << left << setw(8) << f.getID() 
                     << " | " << setw(25) << f.getRoute() 
                     << " | " << setw(15) << f.getTime() 
                     << " | " << f.getStatus() << "\n";
            }
            break;
        }

        case 4:
            handleAdminPanel();
            break;

        case 5:
            cout << "Thank you for using the Airline Reservation System!\n";
            break;

        default:
            cout << "Invalid choice. Please select from the menu.\n";
            break;
        }
    }

    return 0;
}