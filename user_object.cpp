#include <iostream>
#include <string>
#include <iomanip>
#include <limits>

using namespace std;

// Class representing a User Object
class User {
private:
    int id;
    string name;
    string email;
    int age;
    string mobile;

public:
    // 1. Default Constructor
    User() : id(0), name(""), email(""), age(0), mobile("") {}

    // 2. Parameterized Constructor
    User(int uId, string uName, string uEmail, int uAge, string uMobile)
        : id(uId), name(uName), email(uEmail), age(uAge), mobile(uMobile) {}

    // Method to enter/input user information
    void enterInfo() {
        cout << "\n=================================" << endl;
        cout << "      ENTER USER INFORMATION     " << endl;
        cout << "=================================" << endl;

        cout << "Enter User ID: ";
        while (!(cin >> id)) {
            cout << "Invalid input! Please enter a numeric User ID: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear newline buffer

        cout << "Enter Full Name: ";
        getline(cin, name);

        cout << "Enter Email Address: ";
        getline(cin, email);

        cout << "Enter Age: ";
        while (!(cin >> age) || age <= 0) {
            cout << "Invalid input! Please enter a valid positive Age: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear newline buffer

        cout << "Enter Mobile Number: ";
        getline(cin, mobile);

        cout << "\nUser information captured successfully!" << endl;
    }

    // Method to render/display user information
    void render() const {
        cout << "\n=================================" << endl;
        cout << "         USER DETAILS CARD       " << endl;
        cout << "=================================" << endl;
        cout << left << setw(15) << "User ID:" << id << endl;
        cout << left << setw(15) << "Name:" << name << endl;
        cout << left << setw(15) << "Email:" << email << endl;
        cout << left << setw(15) << "Age:" << age << endl;
        cout << left << setw(15) << "Mobile No.:" << mobile << endl;
        cout << "=================================" << endl;
    }

    // Getters
    int getId() const { return id; }
    string getName() const { return name; }
    string getEmail() const { return email; }
    int getAge() const { return age; }
    string getMobile() const { return mobile; }
};

int main() {
    // Construct User Object 1 using default constructor & enter details interactively
    cout << "--- Creating User Object 1 ---" << endl;
    User user1;
    user1.enterInfo();
    
    // Render User Object 1
    user1.render();

    // Construct User Object 2 using parameterized constructor & render directly
    cout << "\n--- Creating User Object 2 (Parameterized Constructor) ---" << endl;
    User user2(102, "Alice Smith", "alice@example.com", 25, "+1-9876543210");
    user2.render();

    return 0;
}
