#ifndef SELLER_H
#define SELLER_H

#include "buyer.h"
#include "item.h"
#include <string>
#include <vector>

using namespace std;

class Seller : public Buyer
{
private:
    vector<Item> items;

public:
    Seller(int id, const string &name, BankCustomer &account)
        : Buyer(id, name, account) {}

    virtual ~Seller() = default;

    // Add new item
    void addNewItem(int newId, const string &newName, int newQuantity, double newPrice)
    {
        Item newItem(newId, newName, newQuantity, newPrice);
        items.push_back(newItem);
    }

    // Update item
    void updateItem(int itemId, const string &newName, int newQuantity, double newPrice)
    {
        for (auto &item : items)
        {
            if (item.getId() == itemId)
            {
                item.alterItemById(itemId, newName, newQuantity, newPrice);
                break;
            }
        }
    }

    // Make item visible to customers
    void makeItemVisibleToCustomer(int itemId)
    {
        for (auto &item : items)
        {
            if (item.getId() == itemId)
            {
                item.setDisplay(true);
                break;
            }
        }
    }

    // Get all items
    vector<Item> &getItems()
    {
        return items;
    }

    // Get item by ID
    Item *getItemById(int itemId)
    {
        for (auto &item : items)
        {
            if (item.getId() == itemId)
            {
                return &item;
            }
        }
        return nullptr;
    }

    // Replenish item stock
    bool replenishItem(int itemId, int quantity)
    {
        Item *item = getItemById(itemId);
        if (item)
        {
            item->setQuantity(item->getQuantity() + quantity);
            return true;
        }
        return false;
    }

    // Discard item stock
    bool discardItem(int itemId, int quantity)
    {
        Item *item = getItemById(itemId);
        if (item && item->getQuantity() >= quantity)
        {
            item->setQuantity(item->getQuantity() - quantity);
            return true;
        }
        return false;
    }

    // Update price
    bool updatePrice(int itemId, double newPrice)
    {
        Item *item = getItemById(itemId);
        if (item)
        {
            item->setPrice(newPrice);
            return true;
        }
        return false;
    }

    // Get visible items only
    vector<Item> getVisibleItems() const
    {
        vector<Item> visibleItems;
        for (const auto &item : items)
        {
            if (item.getQuantity() > 0)
            {
                visibleItems.push_back(item);
            }
        }
        return visibleItems;
    }
};

#endif // SELLER_H