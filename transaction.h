#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <ctime>

using namespace std;

enum TransactionType
{
    CREDIT,
    DEBIT
};
enum OrderStatus
{
    PENDING,
    PAID,
    COMPLETED,
    CANCELLED
};

// Bank Transaction
class Transaction
{
private:
    int id;
    int customerId;
    TransactionType type;
    double amount;
    time_t timestamp;
    string description;

public:
    Transaction(int id, int customerId, TransactionType type, double amount, const string &desc)
        : id(id), customerId(customerId), type(type), amount(amount), description(desc)
    {
        timestamp = time(nullptr);
    }

    int getId() const { return id; }
    int getCustomerId() const { return customerId; }
    TransactionType getType() const { return type; }
    double getAmount() const { return amount; }
    time_t getTimestamp() const { return timestamp; }
    string getDescription() const { return description; }

    void setTimestamp(time_t ts) { timestamp = ts; }
};

// Store Order/Transaction
class Order
{
private:
    int orderId;
    int buyerId;
    int sellerId;
    int itemId;
    string itemName;
    int quantity;
    double totalPrice;
    OrderStatus status;
    time_t timestamp;

public:
    Order(int orderId, int buyerId, int sellerId, int itemId, const string &itemName,
          int quantity, double totalPrice)
        : orderId(orderId), buyerId(buyerId), sellerId(sellerId), itemId(itemId),
          itemName(itemName), quantity(quantity), totalPrice(totalPrice)
    {
        status = PENDING;
        timestamp = time(nullptr);
    }

    int getOrderId() const { return orderId; }
    int getBuyerId() const { return buyerId; }
    int getSellerId() const { return sellerId; }
    int getItemId() const { return itemId; }
    string getItemName() const { return itemName; }
    int getQuantity() const { return quantity; }
    double getTotalPrice() const { return totalPrice; }
    OrderStatus getStatus() const { return status; }
    time_t getTimestamp() const { return timestamp; }

    void setStatus(OrderStatus newStatus) { status = newStatus; }
    void setTimestamp(time_t ts) { timestamp = ts; }
};

#endif // TRANSACTION_H
