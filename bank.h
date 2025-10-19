#ifndef BANK_H
#define BANK_H

#include "bank_customer.h"
#include "transaction.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <map>

using namespace std;

class Bank
{
private:
    string name;
    vector<BankCustomer *> Accounts;
    vector<Transaction> transactions;
    int customerCount;
    int nextTransactionId;

public:
    Bank(const string &name) : name(name), customerCount(0), nextTransactionId(1) {}

    ~Bank()
    {
        for (auto account : Accounts)
        {
            delete account;
        }
    }

    string getName() const { return name; }

    // Create new bank account
    BankCustomer *createAccount(const string &customerName, double initialBalance)
    {
        customerCount++;
        BankCustomer *newAccount = new BankCustomer(customerCount, customerName, initialBalance);
        Accounts.push_back(newAccount);

        // Record initial deposit transaction
        if (initialBalance > 0)
        {
            Transaction trans(nextTransactionId++, customerCount, CREDIT, initialBalance, "Initial deposit");
            transactions.push_back(trans);
        }

        return newAccount;
    }

    // Find account by ID
    BankCustomer *findAccount(int accountId)
    {
        for (auto account : Accounts)
        {
            if (account->getId() == accountId)
            {
                return account;
            }
        }
        return nullptr;
    }

    // Top-up balance
    bool topUp(int accountId, double amount)
    {
        BankCustomer *account = findAccount(accountId);
        if (account && amount > 0)
        {
            account->addBalance(amount);
            Transaction trans(nextTransactionId++, accountId, CREDIT, amount, "Top-up");
            transactions.push_back(trans);
            return true;
        }
        return false;
    }

    // Withdraw balance
    bool withdraw(int accountId, double amount)
    {
        BankCustomer *account = findAccount(accountId);
        if (account && account->withdrawBalance(amount))
        {
            Transaction trans(nextTransactionId++, accountId, DEBIT, amount, "Withdrawal");
            transactions.push_back(trans);
            return true;
        }
        return false;
    }

    // Record a transaction (for purchases)
    void recordTransaction(int accountId, TransactionType type, double amount, const string &description)
    {
        Transaction trans(nextTransactionId++, accountId, type, amount, description);
        transactions.push_back(trans);
    }

    // Get transactions for last 7 days
    vector<Transaction> getTransactionsLast7Days(int accountId) const
    {
        vector<Transaction> recentTrans;
        time_t now = time(nullptr);
        time_t sevenDaysAgo = now - (7 * 24 * 60 * 60);

        for (const auto &trans : transactions)
        {
            if (trans.getCustomerId() == accountId && trans.getTimestamp() >= sevenDaysAgo)
            {
                recentTrans.push_back(trans);
            }
        }
        return recentTrans;
    }

    // Get transactions for today
    vector<Transaction> getTransactionsToday(int accountId) const
    {
        vector<Transaction> todayTrans;
        time_t now = time(nullptr);
        time_t startOfDay = now - (now % 86400);

        for (const auto &trans : transactions)
        {
            if (trans.getCustomerId() == accountId && trans.getTimestamp() >= startOfDay)
            {
                todayTrans.push_back(trans);
            }
        }
        return todayTrans;
    }

    // Get transactions for this month
    vector<Transaction> getTransactionsThisMonth(int accountId) const
    {
        vector<Transaction> monthTrans;
        time_t now = time(nullptr);
        struct tm *timeInfo = localtime(&now);
        timeInfo->tm_mday = 1;
        timeInfo->tm_hour = 0;
        timeInfo->tm_min = 0;
        timeInfo->tm_sec = 0;
        time_t startOfMonth = mktime(timeInfo);

        for (const auto &trans : transactions)
        {
            if (trans.getCustomerId() == accountId && trans.getTimestamp() >= startOfMonth)
            {
                monthTrans.push_back(trans);
            }
        }
        return monthTrans;
    }

    // List all customers
    vector<BankCustomer *> getAllCustomers() const
    {
        return Accounts;
    }

    // List dormant accounts (no activity for 30+ days)
    vector<BankCustomer *> getDormantAccounts() const
    {
        vector<BankCustomer *> dormant;
        time_t now = time(nullptr);
        time_t thirtyDaysAgo = now - (30 * 24 * 60 * 60);

        for (auto account : Accounts)
        {
            bool hasRecentActivity = false;
            for (const auto &trans : transactions)
            {
                if (trans.getCustomerId() == account->getId() && trans.getTimestamp() >= thirtyDaysAgo)
                {
                    hasRecentActivity = true;
                    break;
                }
            }
            if (!hasRecentActivity)
            {
                dormant.push_back(account);
            }
        }
        return dormant;
    }

    // Get top N active users by transaction count today
    vector<pair<int, int>> getTopNActiveUsersToday(int n) const
    {
        time_t now = time(nullptr);
        time_t startOfDay = now - (now % 86400);

        std::map<int, int> transactionCounts;
        for (const auto &trans : transactions)
        {
            if (trans.getTimestamp() >= startOfDay)
            {
                transactionCounts[trans.getCustomerId()]++;
            }
        }

        vector<pair<int, int>> sortedUsers(transactionCounts.begin(), transactionCounts.end());
        sort(sortedUsers.begin(), sortedUsers.end(),
             [](const pair<int, int> &a, const pair<int, int> &b)
             {
                 return a.second > b.second;
             });

        if (sortedUsers.size() > (size_t)n)
        {
            sortedUsers.resize(n);
        }

        return sortedUsers;
    }

    // Calculate cash flow (credit - debit)
    pair<double, double> getCashFlow(int accountId, time_t startTime) const
    {
        double credit = 0.0;
        double debit = 0.0;

        for (const auto &trans : transactions)
        {
            if (trans.getCustomerId() == accountId && trans.getTimestamp() >= startTime)
            {
                if (trans.getType() == CREDIT)
                {
                    credit += trans.getAmount();
                }
                else
                {
                    debit += trans.getAmount();
                }
            }
        }

        return {credit, debit};
    }

    int getCustomerCount() const { return customerCount; }
};

#endif // BANK_H