#ifndef STORE_H
#define STORE_H

#include "transaction.h"
#include "item.h"
#include "buyer.h"
#include "seller.h"
#include <vector>
#include <string>
#include <map>
#include <ctime>
#include <algorithm>

using namespace std;

class Store
{
private:
    string storeName;
    vector<Order> orders;
    int nextOrderId;

public:
    Store(const string &name) : storeName(name), nextOrderId(1) {}

    string getStoreName() const { return storeName; }

    // Create new order
    int createOrder(int buyerId, int sellerId, int itemId, const string &itemName,
                    int quantity, double totalPrice)
    {
        Order newOrder(nextOrderId, buyerId, sellerId, itemId, itemName, quantity, totalPrice);
        orders.push_back(newOrder);
        return nextOrderId++;
    }

    // Update order status
    bool updateOrderStatus(int orderId, OrderStatus status)
    {
        for (auto &order : orders)
        {
            if (order.getOrderId() == orderId)
            {
                order.setStatus(status);
                return true;
            }
        }
        return false;
    }

    // Get all orders for a buyer
    vector<Order> getBuyerOrders(int buyerId) const
    {
        vector<Order> buyerOrders;
        for (const auto &order : orders)
        {
            if (order.getBuyerId() == buyerId)
            {
                buyerOrders.push_back(order);
            }
        }
        return buyerOrders;
    }

    // Get orders by status
    vector<Order> getOrdersByStatus(int userId, OrderStatus status, bool isBuyer = true) const
    {
        vector<Order> filteredOrders;
        for (const auto &order : orders)
        {
            if (isBuyer && order.getBuyerId() == userId && order.getStatus() == status)
            {
                filteredOrders.push_back(order);
            }
            else if (!isBuyer && order.getSellerId() == userId && order.getStatus() == status)
            {
                filteredOrders.push_back(order);
            }
        }
        return filteredOrders;
    }

    // Get all orders for a seller
    vector<Order> getSellerOrders(int sellerId) const
    {
        vector<Order> sellerOrders;
        for (const auto &order : orders)
        {
            if (order.getSellerId() == sellerId)
            {
                sellerOrders.push_back(order);
            }
        }
        return sellerOrders;
    }

    // Get all paid but not completed orders
    vector<Order> getPaidNotCompletedOrders() const
    {
        vector<Order> paidOrders;
        for (const auto &order : orders)
        {
            if (order.getStatus() == PAID)
            {
                paidOrders.push_back(order);
            }
        }
        return paidOrders;
    }

    // Get transactions for last K days
    vector<Order> getTransactionsLastKDays(int k) const
    {
        vector<Order> recentOrders;
        time_t now = time(nullptr);
        time_t kDaysAgo = now - (k * 24 * 60 * 60);

        for (const auto &order : orders)
        {
            if (order.getTimestamp() >= kDaysAgo)
            {
                recentOrders.push_back(order);
            }
        }
        return recentOrders;
    }

    // Get top M most sold items
    vector<pair<string, int>> getTopMSoldItems(int m) const
    {
        map<string, int> itemSales;

        for (const auto &order : orders)
        {
            if (order.getStatus() == COMPLETED || order.getStatus() == PAID)
            {
                itemSales[order.getItemName()] += order.getQuantity();
            }
        }

        vector<pair<string, int>> salesVec(itemSales.begin(), itemSales.end());
        sort(salesVec.begin(), salesVec.end(),
             [](const pair<string, int> &a, const pair<string, int> &b)
             {
                 return a.second > b.second;
             });

        if (salesVec.size() > (size_t)m)
        {
            salesVec.resize(m);
        }

        return salesVec;
    }

    // Get most active buyers by transaction count today
    vector<pair<int, int>> getMostActiveBuyersToday() const
    {
        map<int, int> buyerTransactions;
        time_t now = time(nullptr);
        time_t startOfDay = now - (now % 86400);

        for (const auto &order : orders)
        {
            if (order.getTimestamp() >= startOfDay)
            {
                buyerTransactions[order.getBuyerId()]++;
            }
        }

        vector<pair<int, int>> buyersVec(buyerTransactions.begin(), buyerTransactions.end());
        sort(buyersVec.begin(), buyersVec.end(),
             [](const pair<int, int> &a, const pair<int, int> &b)
             {
                 return a.second > b.second;
             });

        return buyersVec;
    }

    // Get most active sellers by transaction count today
    vector<pair<int, int>> getMostActiveSellersToday() const
    {
        map<int, int> sellerTransactions;
        time_t now = time(nullptr);
        time_t startOfDay = now - (now % 86400);

        for (const auto &order : orders)
        {
            if (order.getTimestamp() >= startOfDay)
            {
                sellerTransactions[order.getSellerId()]++;
            }
        }

        vector<pair<int, int>> sellersVec(sellerTransactions.begin(), sellerTransactions.end());
        sort(sellersVec.begin(), sellersVec.end(),
             [](const pair<int, int> &a, const pair<int, int> &b)
             {
                 return a.second > b.second;
             });

        return sellersVec;
    }

    // Get buyer spending in last K days
    double getBuyerSpendingLastKDays(int buyerId, int k) const
    {
        double totalSpent = 0.0;
        time_t now = time(nullptr);
        time_t kDaysAgo = now - (k * 24 * 60 * 60);

        for (const auto &order : orders)
        {
            if (order.getBuyerId() == buyerId &&
                order.getTimestamp() >= kDaysAgo &&
                (order.getStatus() == PAID || order.getStatus() == COMPLETED))
            {
                totalSpent += order.getTotalPrice();
            }
        }
        return totalSpent;
    }

    // Get order by ID
    Order *getOrderById(int orderId)
    {
        for (auto &order : orders)
        {
            if (order.getOrderId() == orderId)
            {
                return &order;
            }
        }
        return nullptr;
    }

    // Get all orders
    vector<Order> getAllOrders() const
    {
        return orders;
    }

    // Get loyal customers for a seller (repeat buyers this month)
    map<int, int> getLoyalCustomers(int sellerId) const
    {
        map<int, int> buyerPurchases;
        time_t now = time(nullptr);
        struct tm *timeInfo = localtime(&now);
        timeInfo->tm_mday = 1;
        timeInfo->tm_hour = 0;
        timeInfo->tm_min = 0;
        timeInfo->tm_sec = 0;
        time_t startOfMonth = mktime(timeInfo);

        for (const auto &order : orders)
        {
            if (order.getSellerId() == sellerId &&
                order.getTimestamp() >= startOfMonth &&
                (order.getStatus() == PAID || order.getStatus() == COMPLETED))
            {
                buyerPurchases[order.getBuyerId()]++;
            }
        }

        // Filter only buyers with more than 1 purchase
        map<int, int> loyalCustomers;
        for (const auto &pair : buyerPurchases)
        {
            if (pair.second > 1)
            {
                loyalCustomers[pair.first] = pair.second;
            }
        }

        return loyalCustomers;
    }
};

#endif // STORE_H
