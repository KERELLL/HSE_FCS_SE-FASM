#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include "set"
#include "queue"

static const int speed = 1;
static const int sizeOfShoppingListMin = 5;
static const int sizeOfShoppingListMax = 10;

static int countProducts;

class Customer {
public:
    int id;
    std::condition_variable cv;
    bool isSleep = true;
    std::thread *th;
    Customer(int _id);
    std::vector<int> list;
    int sizeOfShoppingList;
    std::mutex m;
};

class Shop {
public:
    std::mutex ma, mb;
    std::set<int> items;
    std::queue<Customer *> queue;
    std::thread *thread;
    std::mutex cv_m;
    std::string shopName;
    std::condition_variable cv;
    bool isSleep = true;
    Shop(std::vector<int> &data, std::string _name);
    bool checkInItems(int val);
    void addToQueue(Customer *customer);
};

void client(Customer *customer, int id);
void seller(Shop *s, std::string name);

std::mutex msgMtx;
Shop *a, *b;

Customer::Customer(int _id) {
    id = _id;
    sizeOfShoppingList = rand() % (sizeOfShoppingListMax - sizeOfShoppingListMin) + sizeOfShoppingListMin;

    for (int i = 0; i < sizeOfShoppingList; i++)
        list.push_back(rand() % countProducts + 1);

    msgMtx.lock();
    std::cout << "[Customer " << id << "] a list of items: ";
    for (int i = 0; i < sizeOfShoppingList; i++)
        std::cout << list[i] << " ";
    std::cout << std::endl;
    msgMtx.unlock();
    th = new std::thread(client, this, id);
}

Shop::Shop(std::vector<int> &data, std::string _name) {
    shopName = std::move(_name);
    msgMtx.lock();
    std::cout << "Shop " << shopName << " has the following assortment of items: ";
    for (int i : data) {
        std::cout << " " << i;
    }
    std::cout << std::endl;
    msgMtx.unlock();
    for (int &i : data) {
        ma.lock();
        items.insert(i);
        ma.unlock();
    }
    thread = new std::thread(seller, this, shopName);
}

bool Shop::checkInItems(int val) {
    ma.lock();
    bool r = items.find(val) != items.end();
    ma.unlock();
    return r;
}

void Shop::addToQueue(Customer *customer) {
    mb.lock();
    queue.push(customer);
    mb.unlock();
}

void getInLine(Shop* department, Customer* customer, int index, std::unique_lock<std::mutex>& lock) {
    department->addToQueue(customer);
    msgMtx.lock();
    std::cout << "[Customer " << customer->id << "] queued in shop " << department->shopName << " for item "
         << customer->list[index] << std::endl;
    msgMtx.unlock();
    department->isSleep = false;
    department->cv.notify_one();
    customer->isSleep = true;
    while (customer->isSleep)
        customer->cv.wait(lock);

    msgMtx.lock();
    std::cout << "[Customer " << customer->id << "] get an item " << customer->list[index] << std::endl;
    msgMtx.unlock();
}

void client(Customer *customer, int id) {
    std::this_thread::sleep_for(std::chrono::milliseconds((rand() % 2000 + 500) * speed));
    std::unique_lock<std::mutex> lock(customer->m);
    int i = 0;
    while (i < customer->list.size()) {
        std::this_thread::sleep_for(std::chrono::milliseconds((rand() % 2000 + 500) * speed));
        if (i < customer->list.size() && a->checkInItems(customer->list[i]))
            getInLine(a, customer, i, lock);
        else if (i < customer->list.size() && b->checkInItems(customer->list[i]))
            getInLine(b, customer, i, lock);
        i++;
    }
    msgMtx.lock();
    std::cout << "[Customer " << customer->id << "] bought all needed items." << std::endl;
    msgMtx.unlock();
}

void seller(Shop *s, std::string name) {
    std::unique_lock<std::mutex> lock(s->cv_m);

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 * speed));
        int sz;

        s->mb.lock();
        sz = s->queue.size();
        s->mb.unlock();
        if (sz) {
            while (s->isSleep)
                s->cv.wait(lock);
            s->isSleep = true;
        }
        s->mb.lock();
        if (!s->queue.empty()) {
            msgMtx.lock();
            std::cout << "[Shop " << name << "] give the item to the customer" << s->queue.front()->id << std::endl;
            msgMtx.unlock();
            s->queue.front()->isSleep = false;
            s->queue.front()->cv.notify_one();
            s->queue.pop();

            s->isSleep = false;
            s->cv.notify_one();
        }
        s->mb.unlock();
    }
}

std::vector<int> createNumsVec(int count) {
    std::vector<int> numbers;
    for (int i = 1; i <= count; ++i) {
        numbers.push_back(i);
    }
    return numbers;
}

void readNumber(int &num, int minValue, int maxValue = INT_MAX) {
    std::cin >> num;
    while (num < minValue || num > maxValue) {
        std::cout << "Incorrect input!" << std::endl;
        std::cout << "Enter number again:";
        std::cin >> num;
    }
}

void createShops(int countProducts) {
    std::vector<int> vec;
    std::vector<int> numbers = createNumsVec(countProducts);

    for (int i = 0; i < countProducts / 2; i++) {
        int rndPos = rand() % numbers.size();
        int z = numbers.at(rndPos);
        numbers.erase(numbers.begin() + rndPos);
        vec.push_back(z);    // Добавляем их.
    }
    
    a = new Shop(vec, "Food");
    vec.clear();

    for (int number : numbers)
        vec.push_back(number);

    b = new Shop(vec, "Building Materials");
}

int main() {
    std::cout << "Enter count of kind of items:";
    readNumber(countProducts, 10, 500);
    createShops(countProducts);
    int n;
    std::cout << "Enter count of clients:";
    readNumber(n, 1, 100);
    std::vector<Customer *> customers;
    for (int i = 0; i < n; i++) {
        std::cout << "[Client " << i + 1 << "] go into the shop." << std::endl;
        customers.push_back(new Customer(i + 1));
    }
    for (int i = 0; i < n; i++) {
        customers[i]->th->join();
    }

    return 0;
}
