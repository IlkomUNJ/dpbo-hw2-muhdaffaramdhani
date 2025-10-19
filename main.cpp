#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <ctime>
#include <iomanip>
#include "bank_customer.h"
#include "buyer.h"
#include "seller.h"
#include "bank.h"
#include "store.h"
#include "transaction.h"
#include "serialization.h"

using namespace std;

enum PrimaryPrompt
{
    LOGIN,
    REGISTER,
    EXIT_MAIN,
    ADMIN_LOGIN
};
enum RegisterPrompt
{
    CREATE_BUYER,
    CREATE_SELLER,
    BACK
};
enum BuyerMenu
{
    CHECK_ACCOUNT = 1,
    UPGRADE_TO_SELLER,
    BANKING,
    BROWSE_STORE,
    VIEW_ORDERS,
    PAYMENT,
    LOGOUT,
    DELETE_ACCOUNT
};
enum SellerMenu
{
    CHECK_INVENTORY = 1,
    ADD_ITEM,
    REMOVE_ITEM,
    UPDATE_ITEM,
    VIEW_SELLER_ORDERS,
    ANALYTICS,
    BACK_TO_BUYER
};

// global data structures
Bank globalBank("Central Bank");
Store globalStore("Online Marketplace");
map<int, Buyer *> buyers;
map<int, Seller *> sellers;
map<string, pair<int, string>> userCredentials; // username -> (id, role: "buyer" or "seller")
int nextBuyerId = 1;
int nextSellerId = 1;
int nextItemId = 1;

// shopping cart structure
struct CartItem
{
    int sellerId;
    int itemId;
    string itemName;
    int quantity;
    double pricePerUnit;
};
vector<CartItem> shoppingCart;

//utilities functions
void clearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

string getTimeString(time_t timestamp)
{
    char buffer[80];
    struct tm *timeinfo = localtime(&timestamp);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return string(buffer);
}

void printOrderStatus(OrderStatus status)
{
    switch (status)
    {
    case PENDING:
        cout << "PENDING";
        break;
    case PAID:
        cout << "PAID";
        break;
    case COMPLETED:
        cout << "COMPLETED";
        break;
    case CANCELLED:
        cout << "CANCELLED";
        break;
    }
}

// registrations functions
void registerBuyer()
{
    string username, password, name;
    double initialDeposit;

    cout << "\n=== Register New Buyer ===" << endl;
    cout << "Enter username: ";
    cin >> username;

    if (userCredentials.find(username) != userCredentials.end())
    {
        cout << "Username already exists!" << endl;
        cout << "\nPress Enter to continue...";
        cin.ignore();
        cin.get();
        return;
    }

    cout << "Enter password: ";
    cin >> password;
    cout << "Enter full name: ";
    cin.ignore();
    getline(cin, name);
    cout << "Enter initial deposit: $";
    cin >> initialDeposit;

    if (initialDeposit < 0)
    {
        cout << "Invalid amount!" << endl;
        cout << "\nPress Enter to continue...";
        cin.get();
        return;
    }

    // create bank account
    BankCustomer *account = globalBank.createAccount(name, initialDeposit);

    // create buyer account
    Buyer *newBuyer = new Buyer(nextBuyerId, name, *account);
    buyers[nextBuyerId] = newBuyer;
    userCredentials[username] = {nextBuyerId, "buyer"};

    cout << "\n========================================" << endl;
    cout << "Buyer account created successfully!" << endl;
    cout << "========================================" << endl;
    cout << "Your Buyer ID: " << nextBuyerId << endl;
    cout << "Bank Account ID: " << account->getId() << endl;
    cout << "Username: " << username << endl;
    cout << "========================================" << endl;
    cout << "\nPress Enter to continue...";
    cin.get();

    nextBuyerId++;
}

void registerSeller()
{
    string username, password, name;
    double initialDeposit;

    cout << "\n=== Register New Seller ===" << endl;
    cout << "Enter username: ";
    cin >> username;

    if (userCredentials.find(username) != userCredentials.end())
    {
        cout << "Username already exists!" << endl;
        cout << "\nPress Enter to continue...";
        cin.ignore();
        cin.get();
        return;
    }

    cout << "Enter password: ";
    cin >> password;
    cout << "Enter full name: ";
    cin.ignore();
    getline(cin, name);
    cout << "Enter initial deposit: $";
    cin >> initialDeposit;

    if (initialDeposit < 0)
    {
        cout << "Invalid amount!" << endl;
        cout << "\nPress Enter to continue...";
        cin.get();
        return;
    }

    // create bank account
    BankCustomer *account = globalBank.createAccount(name, initialDeposit);

    // create seller (upgrading buyer acc)
    Seller *newSeller = new Seller(nextSellerId, name, *account);
    sellers[nextSellerId] = newSeller;
    buyers[nextSellerId] = newSeller; 
    userCredentials[username] = {nextSellerId, "seller"};

    cout << "\n========================================" << endl;
    cout << "Seller account created successfully!" << endl;
    cout << "========================================" << endl;
    cout << "Your Seller ID: " << nextSellerId << endl;
    cout << "Bank Account ID: " << account->getId() << endl;
    cout << "Username: " << username << endl;
    cout << "========================================" << endl;
    cout << "\nPress Enter to continue...";
    cin.get();

    nextSellerId++;
}

// bank functions
void bankingMenu(Buyer *buyer)
{
    BankCustomer &account = buyer->getAccount();

    while (true)
    {
        cout << "\n=== Banking Menu ===" << endl;
        cout << "Current Balance: $" << account.getBalance() << endl;
        cout << "1. Top-up" << endl;
        cout << "2. Withdraw" << endl;
        cout << "3. View Transaction History (Last 7 days)" << endl;
        cout << "4. View Cash Flow (Today)" << endl;
        cout << "5. View Cash Flow (This Month)" << endl;
        cout << "6. Back" << endl;
        cout << "Choice: ";

        int choice;
        cin >> choice;

        if (choice == 1)
        {
            double amount;
            cout << "Enter amount to top-up: $";
            cin >> amount;

            if (amount > 0)
            {
                globalBank.topUp(account.getId(), amount);
                cout << "Top-up successful! New balance: $" << account.getBalance() << endl;
            }
            else
            {
                cout << "Invalid amount!" << endl;
            }
        }
        else if (choice == 2)
        {
            double amount;
            cout << "Enter amount to withdraw: $";
            cin >> amount;

            if (globalBank.withdraw(account.getId(), amount))
            {
                cout << "Withdrawal successful! New balance: $" << account.getBalance() << endl;
            }
        }
        else if (choice == 3)
        {
            auto transactions = globalBank.getTransactionsLast7Days(account.getId());
            cout << "\n=== Transaction History (Last 7 Days) ===" << endl;

            if (transactions.empty())
            {
                cout << "No transactions found." << endl;
            }
            else
            {
                for (const auto &trans : transactions)
                {
                    cout << "[" << getTimeString(trans.getTimestamp()) << "] ";
                    cout << (trans.getType() == CREDIT ? "+" : "-") << "$" << trans.getAmount();
                    cout << " - " << trans.getDescription() << endl;
                }
            }
        }
        else if (choice == 4)
        {
            time_t now = time(nullptr);
            time_t startOfDay = now - (now % 86400);
            auto cashFlow = globalBank.getCashFlow(account.getId(), startOfDay);

            cout << "\n=== Cash Flow (Today) ===" << endl;
            cout << "Credit: +$" << cashFlow.first << endl;
            cout << "Debit: -$" << cashFlow.second << endl;
            cout << "Net: $" << (cashFlow.first - cashFlow.second) << endl;
        }
        else if (choice == 5)
        {
            time_t now = time(nullptr);
            struct tm *timeInfo = localtime(&now);
            timeInfo->tm_mday = 1;
            timeInfo->tm_hour = 0;
            timeInfo->tm_min = 0;
            timeInfo->tm_sec = 0;
            time_t startOfMonth = mktime(timeInfo);
            auto cashFlow = globalBank.getCashFlow(account.getId(), startOfMonth);

            cout << "\n=== Cash Flow (This Month) ===" << endl;
            cout << "Credit: +$" << cashFlow.first << endl;
            cout << "Debit: -$" << cashFlow.second << endl;
            cout << "Net: $" << (cashFlow.first - cashFlow.second) << endl;
        }
        else if (choice == 6)
        {
            break;
        }
    }
}

// view store and add to cart
void browseStore(Buyer *buyer)
{
    cout << "\n=== Browse Store ===" << endl;

    if (sellers.empty())
    {
        cout << "No sellers available." << endl;
        return;
    }

    // display all sellers and their items
    for (auto &sellerPair : sellers)
    {
        Seller *seller = sellerPair.second;
        cout << "\n--- Seller: " << seller->getName() << " (ID: " << seller->getId() << ") ---" << endl;

        auto items = seller->getVisibleItems();
        if (items.empty())
        {
            cout << "No items available." << endl;
            continue;
        }

        for (const auto &item : items)
        {
            cout << "Item ID: " << item.getId() << " | " << item.getName()
                 << " | Price: $" << item.getPrice()
                 << " | Stock: " << item.getQuantity() << endl;
        }
    }

    cout << "\nEnter Item ID to add to cart (0 to cancel): ";
    int itemId;
    cin >> itemId;

    if (itemId == 0)
        return;

    // find item
    bool found = false;
    for (auto &sellerPair : sellers)
    {
        Seller *seller = sellerPair.second;
        Item *item = seller->getItemById(itemId);

        if (item && item->getQuantity() > 0)
        {
            cout << "Enter quantity: ";
            int qty;
            cin >> qty;

            if (qty > 0 && qty <= item->getQuantity())
            {
                CartItem cartItem;
                cartItem.sellerId = seller->getId();
                cartItem.itemId = item->getId();
                cartItem.itemName = item->getName();
                cartItem.quantity = qty;
                cartItem.pricePerUnit = item->getPrice();

                shoppingCart.push_back(cartItem);
                cout << "Item added to cart!" << endl;
                found = true;
                break;
            }
            else
            {
                cout << "Invalid quantity or insufficient stock!" << endl;
                return;
            }
        }
    }

    if (!found)
    {
        cout << "Item not found!" << endl;
    }
}

// view and manage orders
void viewOrders(Buyer *buyer)
{
    cout << "\n=== My Orders ===" << endl;
    cout << "1. All Orders" << endl;
    cout << "2. Pending Orders" << endl;
    cout << "3. Paid Orders" << endl;
    cout << "4. Completed Orders" << endl;
    cout << "5. Cancelled Orders" << endl;
    cout << "6. View Shopping Cart" << endl;
    cout << "7. Checkout" << endl;
    cout << "8. Total Spending (Last K Days)" << endl;
    cout << "9. Back" << endl;
    cout << "Choice: ";

    int choice;
    cin >> choice;

    if (choice >= 1 && choice <= 5)
    {
        vector<Order> orders;

        if (choice == 1)
        {
            orders = globalStore.getBuyerOrders(buyer->getId());
        }
        else
        {
            OrderStatus status = static_cast<OrderStatus>(choice - 2);
            orders = globalStore.getOrdersByStatus(buyer->getId(), status, true);
        }

        if (orders.empty())
        {
            cout << "No orders found." << endl;
        }
        else
        {
            for (const auto &order : orders)
            {
                cout << "\nOrder ID: " << order.getOrderId() << endl;
                cout << "Item: " << order.getItemName() << endl;
                cout << "Quantity: " << order.getQuantity() << endl;
                cout << "Total: $" << order.getTotalPrice() << endl;
                cout << "Status: ";
                printOrderStatus(order.getStatus());
                cout << endl;
                cout << "Date: " << getTimeString(order.getTimestamp()) << endl;
            }
        }
    }
    else if (choice == 6)
    {
        cout << "\n=== Shopping Cart ===" << endl;

        if (shoppingCart.empty())
        {
            cout << "Cart is empty." << endl;
        }
        else
        {
            double total = 0;
            for (size_t i = 0; i < shoppingCart.size(); i++)
            {
                const auto &item = shoppingCart[i];
                double itemTotal = item.quantity * item.pricePerUnit;
                cout << i + 1 << ". " << item.itemName
                     << " x" << item.quantity
                     << " @ $" << item.pricePerUnit
                     << " = $" << itemTotal << endl;
                total += itemTotal;
            }
            cout << "\nTotal: $" << total << endl;

            cout << "\nRemove item? (0 = No, Item# = Yes): ";
            int removeIdx;
            cin >> removeIdx;

            if (removeIdx > 0 && removeIdx <= (int)shoppingCart.size())
            {
                shoppingCart.erase(shoppingCart.begin() + removeIdx - 1);
                cout << "Item removed from cart." << endl;
            }
        }
    }
    else if (choice == 7)
    {
        // checkout
        if (shoppingCart.empty())
        {
            cout << "Cart is empty!" << endl;
            return;
        }

        double total = 0;
        for (const auto &item : shoppingCart)
        {
            total += item.quantity * item.pricePerUnit;
        }

        cout << "\n=== Checkout ===" << endl;
        cout << "Total Amount: $" << total << endl;
        cout << "Your Balance: $" << buyer->getAccount().getBalance() << endl;

        if (buyer->getAccount().getBalance() < total)
        {
            cout << "Insufficient balance!" << endl;
            return;
        }

        cout << "Confirm purchase? (1=Yes, 0=No): ";
        int confirm;
        cin >> confirm;

        if (confirm == 1)
        {
            // process each item in cart
            for (const auto &cartItem : shoppingCart)
            {
                // Create order
                int orderId = globalStore.createOrder(
                    buyer->getId(),
                    cartItem.sellerId,
                    cartItem.itemId,
                    cartItem.itemName,
                    cartItem.quantity,
                    cartItem.quantity * cartItem.pricePerUnit);

                // update order status to pending
                globalStore.updateOrderStatus(orderId, PENDING);
            }

            cout << "Orders created successfully! Please proceed to payment." << endl;
            shoppingCart.clear();
        }
    }
    else if (choice == 8)
    {
        cout << "Enter number of days: ";
        int k;
        cin >> k;

        double spending = globalStore.getBuyerSpendingLastKDays(buyer->getId(), k);
        cout << "Total spending in last " << k << " days: $" << spending << endl;
    }
}

// payment functionality
void paymentMenu(Buyer *buyer)
{
    cout << "\n=== Payment ===" << endl;

    auto pendingOrders = globalStore.getOrdersByStatus(buyer->getId(), PENDING, true);

    if (pendingOrders.empty())
    {
        cout << "No pending payments." << endl;
        return;
    }

    cout << "Pending Invoices:" << endl;
    for (const auto &order : pendingOrders)
    {
        cout << "\nInvoice ID: " << order.getOrderId() << endl;
        cout << "Item: " << order.getItemName() << endl;
        cout << "Amount: $" << order.getTotalPrice() << endl;
    }

    cout << "\nEnter Invoice ID to pay (0 to cancel): ";
    int invoiceId;
    cin >> invoiceId;

    if (invoiceId == 0)
        return;

    Order *order = globalStore.getOrderById(invoiceId);

    if (!order || order->getBuyerId() != buyer->getId() || order->getStatus() != PENDING)
    {
        cout << "Invalid invoice!" << endl;
        return;
    }

    cout << "\n=== Payment Confirmation ===" << endl;
    cout << "Amount: $" << order->getTotalPrice() << endl;
    cout << "Your Balance: $" << buyer->getAccount().getBalance() << endl;

    if (buyer->getAccount().getBalance() < order->getTotalPrice())
    {
        cout << "Insufficient balance!" << endl;
        return;
    }

    cout << "Enter invoice ID again to confirm: ";
    int confirmId;
    cin >> confirmId;

    if (confirmId == invoiceId)
    {
        
        buyer->getAccount().withdrawBalance(order->getTotalPrice());
        globalBank.recordTransaction(buyer->getAccount().getId(), DEBIT,
                                     order->getTotalPrice(), "Purchase: " + order->getItemName());

       
        Seller *seller = sellers[order->getSellerId()];
        seller->getAccount().addBalance(order->getTotalPrice());
        globalBank.recordTransaction(seller->getAccount().getId(), CREDIT,
                                     order->getTotalPrice(), "Sale: " + order->getItemName());

       
        Item *item = seller->getItemById(order->getItemId());
        if (item)
        {
            item->setQuantity(item->getQuantity() - order->getQuantity());
        }

        globalStore.updateOrderStatus(invoiceId, PAID);

        cout << "\nPayment successful!" << endl;
        cout << "New balance: $" << buyer->getAccount().getBalance() << endl;
    }
    else
    {
        cout << "Payment cancelled." << endl;
    }
}

// seller inventory 
void sellerInventoryMenu(Seller *seller)
{
    cout << "\n=== My Inventory ===" << endl;

    auto items = seller->getItems();

    if (items.empty())
    {
        cout << "No items in inventory." << endl;
    }
    else
    {
        for (const auto &item : items)
        {
            cout << "ID: " << item.getId() << " | " << item.getName()
                 << " | Price: $" << item.getPrice()
                 << " | Stock: " << item.getQuantity() << endl;
        }
    }
}

void addItemMenu(Seller *seller)
{
    cout << "\n=== Add New Item ===" << endl;

    string name;
    int quantity;
    double price;

    cout << "Item name: ";
    cin.ignore();
    getline(cin, name);
    cout << "Quantity: ";
    cin >> quantity;
    cout << "Price: $";
    cin >> price;

    if (quantity >= 0 && price >= 0)
    {
        seller->addNewItem(nextItemId, name, quantity, price);
        seller->makeItemVisibleToCustomer(nextItemId);
        cout << "Item added successfully! Item ID: " << nextItemId << endl;
        nextItemId++;
    }
    else
    {
        cout << "Invalid input!" << endl;
    }
}

void updateItemMenu(Seller *seller)
{
    cout << "\n=== Update Item ===" << endl;

    sellerInventoryMenu(seller);

    cout << "\nEnter Item ID to update: ";
    int itemId;
    cin >> itemId;

    Item *item = seller->getItemById(itemId);

    if (!item)
    {
        cout << "Item not found!" << endl;
        return;
    }

    cout << "1. Update Price" << endl;
    cout << "2. Replenish Stock" << endl;
    cout << "3. Discard Stock" << endl;
    cout << "4. Update All Details" << endl;
    cout << "Choice: ";

    int choice;
    cin >> choice;

    if (choice == 1)
    {
        double newPrice;
        cout << "New price: $";
        cin >> newPrice;

        if (seller->updatePrice(itemId, newPrice))
        {
            cout << "Price updated!" << endl;
        }
    }
    else if (choice == 2)
    {
        int qty;
        cout << "Quantity to add: ";
        cin >> qty;

        if (seller->replenishItem(itemId, qty))
        {
            cout << "Stock replenished!" << endl;
        }
    }
    else if (choice == 3)
    {
        int qty;
        cout << "Quantity to remove: ";
        cin >> qty;

        if (seller->discardItem(itemId, qty))
        {
            cout << "Stock discarded!" << endl;
        }
        else
        {
            cout << "Insufficient stock!" << endl;
        }
    }
    else if (choice == 4)
    {
        string name;
        int qty;
        double price;

        cout << "New name: ";
        cin.ignore();
        getline(cin, name);
        cout << "New quantity: ";
        cin >> qty;
        cout << "New price: $";
        cin >> price;

        seller->updateItem(itemId, name, qty, price);
        cout << "Item updated!" << endl;
    }
}

void viewSellerOrders(Seller *seller)
{
    cout << "\n=== Seller Orders ===" << endl;

    auto orders = globalStore.getSellerOrders(seller->getId());

    if (orders.empty())
    {
        cout << "No orders found." << endl;
        return;
    }

    for (const auto &order : orders)
    {
        if (order.getStatus() == PAID || order.getStatus() == COMPLETED)
        {
            cout << "\nOrder ID: " << order.getOrderId() << endl;
            cout << "Buyer ID: " << order.getBuyerId() << endl;
            cout << "Item: " << order.getItemName() << endl;
            cout << "Quantity: " << order.getQuantity() << endl;
            cout << "Total: $" << order.getTotalPrice() << endl;
            cout << "Status: ";
            printOrderStatus(order.getStatus());
            cout << endl;
            cout << "Date: " << getTimeString(order.getTimestamp()) << endl;
        }
    }

    cout << "\nMark order as completed? (Enter Order ID or 0): ";
    int orderId;
    cin >> orderId;

    if (orderId > 0)
    {
        Order *order = globalStore.getOrderById(orderId);
        if (order && order->getSellerId() == seller->getId() && order->getStatus() == PAID)
        {
            globalStore.updateOrderStatus(orderId, COMPLETED);
            cout << "Order marked as completed!" << endl;
        }
        else
        {
            cout << "Invalid order or order not paid yet!" << endl;
        }
    }
}

void sellerAnalytics(Seller *seller)
{
    cout << "\n=== Seller Analytics ===" << endl;
    cout << "1. Top K Most Popular Items This Month" << endl;
    cout << "2. Loyal Customers (Repeat Buyers This Month)" << endl;
    cout << "3. Back" << endl;
    cout << "Choice: ";

    int choice;
    cin >> choice;

    if (choice == 1)
    {
        cout << "Enter K: ";
        int k;
        cin >> k;

        auto topItems = globalStore.getTopMSoldItems(k);

        cout << "\n=== Top " << k << " Most Sold Items ===" << endl;
        for (size_t i = 0; i < topItems.size(); i++)
        {
            cout << i + 1 << ". " << topItems[i].first
                 << " - " << topItems[i].second << " units sold" << endl;
        }
    }
    else if (choice == 2)
    {
        auto loyalCustomers = globalStore.getLoyalCustomers(seller->getId());

        cout << "\n=== Loyal Customers ===" << endl;
        if (loyalCustomers.empty())
        {
            cout << "No loyal customers yet." << endl;
        }
        else
        {
            for (const auto &pair : loyalCustomers)
            {
                Buyer *buyer = buyers[pair.first];
                cout << "Buyer: " << buyer->getName()
                     << " (ID: " << pair.first << ")"
                     << " - " << pair.second << " purchases this month" << endl;
            }
        }
    }
}

// buyer menu
void buyerMenu(int userId)
{
    Buyer *buyer = buyers[userId];

    while (true)
    {
        cout << "\n========================================" << endl;
        cout << "Welcome, " << buyer->getName() << "!" << endl;
        cout << "Balance: $" << buyer->getAccount().getBalance() << endl;
        cout << "========================================" << endl;
        cout << "1. Check Account Status" << endl;
        cout << "2. Upgrade to Seller Account" << endl;
        cout << "3. Banking Functions" << endl;
        cout << "4. Browse Store" << endl;
        cout << "5. View Orders" << endl;
        cout << "6. Payment" << endl;
        cout << "7. Logout" << endl;
        cout << "Choice: ";

        int choice;
        cin >> choice;

        if (choice == 1)
        {
            cout << "\n=== Account Status ===" << endl;
            cout << "Name: " << buyer->getName() << endl;
            cout << "Buyer ID: " << buyer->getId() << endl;
            cout << "Bank Account ID: " << buyer->getAccount().getId() << endl;
            cout << "Balance: $" << buyer->getAccount().getBalance() << endl;
            cout << "Account Type: ";

            if (sellers.find(userId) != sellers.end())
            {
                cout << "Buyer & Seller" << endl;
            }
            else
            {
                cout << "Buyer" << endl;
            }
        }
        else if (choice == 2)
        {
            if (sellers.find(userId) != sellers.end())
            {
                cout << "You are already a seller!" << endl;
            }
            else
            {
                cout << "\n=== Upgrade to Seller ===" << endl;
                cout << "Confirm upgrade? (1=Yes, 0=No): ";
                int confirm;
                cin >> confirm;

                if (confirm == 1)
                {
                    Seller *newSeller = new Seller(userId, buyer->getName(), buyer->getAccount());
                    sellers[userId] = newSeller;
                    cout << "Account upgraded to Seller!" << endl;
                }
            }
        }
        else if (choice == 3)
        {
            bankingMenu(buyer);
        }
        else if (choice == 4)
        {
            browseStore(buyer);
        }
        else if (choice == 5)
        {
            viewOrders(buyer);
        }
        else if (choice == 6)
        {
            paymentMenu(buyer);
        }
        else if (choice == 7)
        {
            cout << "Logging out..." << endl;
            break;
        }
    }
}

// seller menu
void sellerMainMenu(int userId)
{
    Seller *seller = sellers[userId];

    while (true)
    {
        cout << "\n========================================" << endl;
        cout << "Seller Panel - " << seller->getName() << endl;
        cout << "Balance: $" << seller->getAccount().getBalance() << endl;
        cout << "========================================" << endl;
        cout << "1. Check Inventory" << endl;
        cout << "2. Add Item" << endl;
        cout << "3. Update Item" << endl;
        cout << "4. View Seller Orders" << endl;
        cout << "5. Analytics" << endl;
        cout << "6. Switch to Buyer Mode" << endl;
        cout << "7. Logout" << endl;
        cout << "Choice: ";

        int choice;
        cin >> choice;

        if (choice == 1)
        {
            sellerInventoryMenu(seller);
        }
        else if (choice == 2)
        {
            addItemMenu(seller);
        }
        else if (choice == 3)
        {
            updateItemMenu(seller);
        }
        else if (choice == 4)
        {
            viewSellerOrders(seller);
        }
        else if (choice == 5)
        {
            sellerAnalytics(seller);
        }
        else if (choice == 6)
        {
            buyerMenu(userId);
        }
        else if (choice == 7)
        {
            cout << "Logging out..." << endl;
            break;
        }
    }
}

// admin menu
void adminMenu()
{
    while (true)
    {
        cout << "\n=== Admin Panel ===" << endl;
        cout << "1. View All Buyers" << endl;
        cout << "2. View All Sellers" << endl;
        cout << "3. View All Bank Accounts" << endl;
        cout << "4. View Dormant Accounts" << endl;
        cout << "5. Top N Active Users Today" << endl;
        cout << "6. System Report" << endl;
        cout << "7. Store Analytics" << endl;
        cout << "8. Logout" << endl;
        cout << "Choice: ";

        int choice;
        cin >> choice;

        if (choice == 1)
        {
            cout << "\n=== All Buyers ===" << endl;
            for (const auto &pair : buyers)
            {
                cout << "ID: " << pair.first << " | Name: " << pair.second->getName()
                     << " | Balance: $" << pair.second->getAccount().getBalance() << endl;
            }
        }
        else if (choice == 2)
        {
            cout << "\n=== All Sellers ===" << endl;
            for (const auto &pair : sellers)
            {
                cout << "ID: " << pair.first << " | Name: " << pair.second->getName()
                     << " | Items: " << pair.second->getItems().size() << endl;
            }
        }
        else if (choice == 3)
        {
            cout << "\n=== All Bank Accounts ===" << endl;
            auto accounts = globalBank.getAllCustomers();
            for (const auto &acc : accounts)
            {
                acc->printInfo();
                cout << "---" << endl;
            }
        }
        else if (choice == 4)
        {
            cout << "\n=== Dormant Accounts (30+ days) ===" << endl;
            auto dormant = globalBank.getDormantAccounts();

            if (dormant.empty())
            {
                cout << "No dormant accounts." << endl;
            }
            else
            {
                for (const auto &acc : dormant)
                {
                    acc->printInfo();
                    cout << "---" << endl;
                }
            }
        }
        else if (choice == 5)
        {
            cout << "Enter N: ";
            int n;
            cin >> n;

            auto topUsers = globalBank.getTopNActiveUsersToday(n);

            cout << "\n=== Top " << n << " Active Users Today ===" << endl;
            for (size_t i = 0; i < topUsers.size(); i++)
            {
                BankCustomer *acc = globalBank.findAccount(topUsers[i].first);
                cout << i + 1 << ". " << acc->getName()
                     << " - " << topUsers[i].second << " transactions" << endl;
            }
        }
        else if (choice == 6)
        {
            cout << "\n=== System Report ===" << endl;
            cout << "Total Buyers: " << buyers.size() << endl;
            cout << "Total Sellers: " << sellers.size() << endl;
            cout << "Total Bank Accounts: " << globalBank.getCustomerCount() << endl;
            cout << "Total Orders: " << globalStore.getAllOrders().size() << endl;
        }
        else if (choice == 7)
        {
            cout << "\n=== Store Analytics ===" << endl;
            cout << "1. Transactions Last K Days" << endl;
            cout << "2. Paid but Not Completed Orders" << endl;
            cout << "3. Top M Most Sold Items" << endl;
            cout << "4. Most Active Buyers Today" << endl;
            cout << "5. Most Active Sellers Today" << endl;
            cout << "Choice: ";

            int subChoice;
            cin >> subChoice;

            if (subChoice == 1)
            {
                cout << "Enter K: ";
                int k;
                cin >> k;

                auto transactions = globalStore.getTransactionsLastKDays(k);
                cout << "\n=== Transactions (Last " << k << " Days) ===" << endl;
                cout << "Total: " << transactions.size() << " transactions" << endl;
            }
            else if (subChoice == 2)
            {
                auto orders = globalStore.getPaidNotCompletedOrders();
                cout << "\n=== Paid but Not Completed ===" << endl;

                if (orders.empty())
                {
                    cout << "No pending orders." << endl;
                }
                else
                {
                    for (const auto &order : orders)
                    {
                        cout << "Order ID: " << order.getOrderId()
                             << " | Item: " << order.getItemName()
                             << " | Amount: $" << order.getTotalPrice() << endl;
                    }
                }
            }
            else if (subChoice == 3)
            {
                cout << "Enter M: ";
                int m;
                cin >> m;

                auto topItems = globalStore.getTopMSoldItems(m);
                cout << "\n=== Top " << m << " Most Sold Items ===" << endl;

                for (size_t i = 0; i < topItems.size(); i++)
                {
                    cout << i + 1 << ". " << topItems[i].first
                         << " - " << topItems[i].second << " units" << endl;
                }
            }
            else if (subChoice == 4)
            {
                auto activeBuyers = globalStore.getMostActiveBuyersToday();
                cout << "\n=== Most Active Buyers Today ===" << endl;

                for (size_t i = 0; i < activeBuyers.size() && i < 10; i++)
                {
                    Buyer *buyer = buyers[activeBuyers[i].first];
                    cout << i + 1 << ". " << buyer->getName()
                         << " - " << activeBuyers[i].second << " orders" << endl;
                }
            }
            else if (subChoice == 5)
            {
                auto activeSellers = globalStore.getMostActiveSellersToday();
                cout << "\n=== Most Active Sellers Today ===" << endl;

                for (size_t i = 0; i < activeSellers.size() && i < 10; i++)
                {
                    Seller *seller = sellers[activeSellers[i].first];
                    cout << i + 1 << ". " << seller->getName()
                         << " - " << activeSellers[i].second << " sales" << endl;
                }
            }
        }
        else if (choice == 8)
        {
            cout << "Logging out from admin panel..." << endl;
            break;
        }
    }
}

// main function
int main()
{
    const string ADMIN_USERNAME = "root";
    const string ADMIN_PASSWORD = "toor";

    cout << "=== Online Store System ===" << endl;
    cout << "Loading data..." << endl;

   

    while (true)
    {
        cout << "\n========================================" << endl;
        cout << "        MAIN MENU" << endl;
        cout << "========================================" << endl;
        cout << "1. Login" << endl;
        cout << "2. Register" << endl;
        cout << "3. Exit" << endl;
        cout << "4. Admin Login" << endl;
        cout << "Choice: ";

        int choice;
        cin >> choice;

        if (choice == 1)
        {
            
            string username, password;
            cout << "\nUsername: ";
            cin >> username;
            cout << "Password: ";
            cin >> password;

            if (userCredentials.find(username) != userCredentials.end())
            {
                int userId = userCredentials[username].first;
                string role = userCredentials[username].second;

                cout << "Login successful!" << endl;

                if (role == "seller")
                {
                    sellerMainMenu(userId);
                }
                else
                {
                    buyerMenu(userId);
                }
            }
            else
            {
                cout << "Invalid credentials!" << endl;
            }
        }
        else if (choice == 2)
        {
        
            cout << "\n=== Register ===" << endl;
            cout << "1. Register as Buyer" << endl;
            cout << "2. Register as Seller" << endl;
            cout << "3. Back" << endl;
            cout << "Choice: ";

            int regChoice;
            cin >> regChoice;

            if (cin.fail())
            {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "Invalid input!" << endl;
                continue;
            }

            if (regChoice == 1)
            {
                registerBuyer();
            }
            else if (regChoice == 2)
            {
                registerSeller();
            }
            else if (regChoice == 3)
            {
                
                continue;
            }
            else
            {
                cout << "Invalid choice!" << endl;
            }
        }
        else if (choice == 3)
        {
           
            cout << "\nSaving data..." << endl;


            cout << "Thank you for using the system!" << endl;
            break;
        }
        else if (choice == 4)
        {
            
            string username, password;
            cout << "\nAdmin Username: ";
            cin >> username;
            cout << "Admin Password: ";
            cin >> password;

            if (username == ADMIN_USERNAME && password == ADMIN_PASSWORD)
            {
                cout << "Admin login successful!" << endl;
                adminMenu();
            }
            else
            {
                cout << "Invalid admin credentials!" << endl;
            }
        }
        else
        {
            cout << "Invalid choice!" << endl;
        }
    }


    for (auto &pair : buyers)
    {
        delete pair.second;
    }

    return 0;
}