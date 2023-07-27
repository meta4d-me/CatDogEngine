#include <iostream>
#include <json/json.hpp>

using json = nlohmann::json;

int main() {
    std::string jsonString = R"({"name": "John", "age": 30, "city": "New York"})";

    try {
        // Parse the JSON string
        json data = json::parse(jsonString);

        // Access JSON values
        std::string name = data["name"];
        int age = data["age"];
        std::string city = data["city"];

        // Print the values
        std::cout << "Name: " << name << ", Age: " << age << ", City: " << city << std::endl;
    }
    catch (json::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }

    return 0;
}
