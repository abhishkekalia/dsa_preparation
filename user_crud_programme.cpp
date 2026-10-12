#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <limits>

using namespace std;

// User structure with fields: name, email, mobile
struct User {
    int id;
    string name;
    string email;
    string mobile;
};

// Helper function to clear leftover newline from cin buffer
void clearBuffer() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// 1. CREATE: Function to add a new user
void createUser(vector<User>& users, int& nextId) {
    User newUser;
    newUser.id = nextId++;

    cout << "\n--- Add New User ---" << endl;
    cout << "Enter Name: ";
    getline(cin, newUser.name);

    cout << "Enter Email: ";
    getline(cin, newUser.email);

    cout << "Enter Mobile Number: ";
    getline(cin, newUser.mobile);

    users.push_back(newUser);
    cout << "User created successfully! (User ID: " << newUser.id << ")" << endl;
}

// 2. READ: Function to view all users
void displayUsers(const vector<User>& users) {
    cout << "\n--- User List ---" << endl;
    if (users.empty()) {
        cout << "No users found." << endl;
        return;
    }

    cout << left << setw(8)  << "ID"
         << setw(25) << "Name"
         << setw(30) << "Email"
         << setw(18) << "Mobile No." << endl;
    cout << string(81, '-') << endl;

    for (const auto& user : users) {
        cout << left << setw(8)  << user.id
             << setw(25) << user.name
             << setw(30) << user.email
             << setw(18) << user.mobile << endl;
    }
}

// 3. UPDATE: Function to modify user details
void updateUser(vector<User>& users) {
    if (users.empty()) {
        cout << "\nNo users available to update." << endl;
        return;
    }

    int id;
    cout << "\nEnter User ID to update: ";
    if (!(cin >> id)) {
        cout << "Invalid ID input!" << endl;
        cin.clear();
        clearBuffer();
        return;
    }
    clearBuffer(); // Clear newline after reading integer

    for (auto& user : users) {
        if (user.id == id) {
            cout << "\n--- Updating User ID: " << id << " ---" << endl;
            cout << "Current Name: " << user.name << endl;
            cout << "Enter New Name (press Enter to keep unchanged): ";
            string nameInput;
            getline(cin, nameInput);
            if (!nameInput.empty()) {
                user.name = nameInput;
            }

            cout << "Current Email: " << user.email << endl;
            cout << "Enter New Email (press Enter to keep unchanged): ";
            string emailInput;
            getline(cin, emailInput);
            if (!emailInput.empty()) {
                user.email = emailInput;
            }

            cout << "Current Mobile: " << user.mobile << endl;
            cout << "Enter New Mobile Number (press Enter to keep unchanged): ";
            string mobileInput;
            getline(cin, mobileInput);
            if (!mobileInput.empty()) {
                user.mobile = mobileInput;
            }

            cout << "User details updated successfully!" << endl;
            return;
        }
    }

    cout << "User with ID " << id << " not found!" << endl;
}

// 4. DELETE: Function to remove a user
void deleteUser(vector<User>& users) {
    if (users.empty()) {
        cout << "\nNo users available to delete." << endl;
        return;
    }

    int id;
    cout << "\nEnter User ID to delete: ";
    if (!(cin >> id)) {
        cout << "Invalid ID input!" << endl;
        cin.clear();
        clearBuffer();
        return;
    }
    clearBuffer(); // Clear newline after reading integer

    for (auto it = users.begin(); it != users.end(); ++it) {
        if (it->id == id) {
            users.erase(it);
            cout << "User with ID " << id << " deleted successfully!" << endl;
            return;
        }
    }

    cout << "User with ID " << id << " not found!" << endl;
}

int main() {
    vector<User> users;
    int nextId = 1;
    int choice = 0;

    do {
        cout << "\n=================================" << endl;
        cout << "      USER MANAGEMENT SYSTEM     " << endl;
        cout << "=================================" << endl;
        cout << "1. Add User (Create)" << endl;
        cout << "2. View All Users (Read)" << endl;
        cout << "3. Update User (Update)" << endl;
        cout << "4. Delete User (Delete)" << endl;
        cout << "5. Exit" << endl;
        cout << "Enter your choice (1-5): ";

        if (!(cin >> choice)) {
            cout << "Invalid option! Please enter a number." << endl;
            cin.clear();
            clearBuffer();
            continue;
        }
        clearBuffer(); // Clear leftover newline from cin >> choice

        switch (choice) {
            case 1:
                createUser(users, nextId);
                break;
            case 2:
                displayUsers(users);
                break;
            case 3:
                updateUser(users);
                break;
            case 4:
                deleteUser(users);
                break;
            case 5:
                cout << "Exiting system. Goodbye!" << endl;
                break;
            default:
                cout << "Invalid choice! Please enter a number between 1 and 5." << endl;
                break;
        }
    } while (choice != 5);

    return 0;
}
