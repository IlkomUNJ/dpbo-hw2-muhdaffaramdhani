#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "bank.h"
#include "store.h"
#include "buyer.h"
#include "seller.h"
#include <fstream>
#include <sstream>

using namespace std;

class Serialization
{
public:
   
    static void saveBank(const Bank &bank, const string &filename)
    {
        ofstream file(filename);
        if (!file.is_open())
            return;

        file << bank.getName() << endl;
        file << bank.getCustomerCount() << endl;

        auto customers = bank.getAllCustomers();
        for (const auto &customer : customers)
        {
            file << customer->getId() << ","
                 << customer->getName() << ","
                 << customer->getBalance() << endl;
        }

        file.close();
    }

 
    static void loadBank(Bank &bank, const string &filename)
    {
        ifstream file(filename);
        if (!file.is_open())
            return;

        string name;
        int count;
        getline(file, name);
        file >> count;
        file.ignore();

        string line;
        while (getline(file, line))
        {
            if (line.empty())
                continue;

            stringstream ss(line);
            string id, customerName, balance;

            getline(ss, id, ',');
            getline(ss, customerName, ',');
            getline(ss, balance, ',');

           
            double bal = stod(balance);
            bank.createAccount(customerName, bal);
        }

        file.close();
    }

    static void saveStore(const Store &store, const string &filename)
    {
        ofstream file(filename);
        if (!file.is_open())
            return;

        file << store.getStoreName() << endl;

        auto orders = store.getAllOrders();
        for (const auto &order : orders)
        {
            file << order.getOrderId() << ","
                 << order.getBuyerId() << ","
                 << order.getSellerId() << ","
                 << order.getItemId() << ","
                 << order.getItemName() << ","
                 << order.getQuantity() << ","
                 << order.getTotalPrice() << ","
                 << order.getStatus() << ","
                 << order.getTimestamp() << endl;
        }

        file.close();
    }

    static void loadStore(Store &store, const string &filename)
    {
        ifstream file(filename);
        if (!file.is_open())
            return;

        string storeName;
        getline(file, storeName);

        string line;
        while (getline(file, line))
        {
            if (line.empty())
                continue;

            stringstream ss(line);
            string orderId, buyerId, sellerId, itemId, itemName, quantity, price, status, timestamp;

            getline(ss, orderId, ',');
            getline(ss, buyerId, ',');
            getline(ss, sellerId, ',');
            getline(ss, itemId, ',');
            getline(ss, itemName, ',');
            getline(ss, quantity, ',');
            getline(ss, price, ',');
            getline(ss, status, ',');
            getline(ss, timestamp, ',');

           
            int oid = stoi(orderId);
            int bid = stoi(buyerId);
            int sid = stoi(sellerId);
            int iid = stoi(itemId);
            int qty = stoi(quantity);
            double prc = stod(price);

            store.createOrder(bid, sid, iid, itemName, qty, prc);
            
        }

        file.close();
    }
};

#endif 
