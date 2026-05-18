// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// clientData structure definition
struct clientData
{
    unsigned int acctNum; // account number
    char lastName[15];    // account last name
    char firstName[10];   // account first name
    double balance;       // account balance
};                        // end structure clientData

// prototypes
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void displayAccounts(FILE *readPtr);
void searchAccount(FILE *readPtr);
void readAccount(FILE *readPtr);
void transferFunds(FILE *fPtr);
void searchFirstName(FILE *readPtr);
void resetDatabase(FILE *fPtr);
void clearInputBuffer(void);
void logTransaction(const char *action, int acctNum, double amount, const char *details);
int compareByBalance(const void *a, const void *b);
void applyInterest(FILE *fPtr);
void generateReport(FILE *readPtr);

int main(int argc, char *argv[])
{
    FILE *cfPtr;         // credit.dat file pointer
    unsigned int choice; // user's choice

    // fopen opens the file; if it doesn't exist, create it
    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL)
    {
        if ((cfPtr = fopen("credit.dat", "wb+")) == NULL)
        {
            printf("%s: File could not be opened.\n", argv[0]);
            exit(-1);
        }
    }

    // enable user to specify action
    while ((choice = enterChoice()) != 13)
    {
        switch (choice)
        {
        case 1:
            textFile(cfPtr);
            break;
        case 2:
            updateRecord(cfPtr);
            break;
        case 3:
            newRecord(cfPtr);
            break;
        case 4:
            deleteRecord(cfPtr);
            break;
        case 5:
            displayAccounts(cfPtr);
            break;
        case 6:
            searchAccount(cfPtr);
            break;
        case 7:
            readAccount(cfPtr);
            break;
        case 8:
            transferFunds(cfPtr);
            break;
        case 9:
            searchFirstName(cfPtr);
            break;
        case 10:
            resetDatabase(cfPtr);
            break;
        case 11:
            applyInterest(cfPtr);
            break;
        case 12:
            generateReport(cfPtr);
            break;
        default:
            puts("Incorrect choice");
            break;
        } // end switch
    }     // end while

    fclose(cfPtr); // fclose closes the file
} // end main

// create formatted text file for printing
void textFile(FILE *readPtr)
{
    FILE *writePtr; // accounts.txt file pointer
    int result;     // used to test whether fread read any bytes
    // create clientData with default information
    struct clientData client = {0, "", "", 0.0};

    // fopen opens the file; exits if file cannot be opened
    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
    } // end if
    else
    {
        double totalBalance = 0.0;
        rewind(readPtr); // sets pointer to beginning of file
        fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
        fprintf(writePtr, "--------------------------------------------\n");

        // copy all records from random-access file into text file
        while (fread(&client, sizeof(struct clientData), 1, readPtr) == 1)
        {
            // write single record to text file
            if (client.acctNum != 0)
            {
                fprintf(writePtr, "%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName,
                        client.balance);
                totalBalance += client.balance;
            } // end if
        }     // end while

        fprintf(writePtr, "--------------------------------------------\n");
        fprintf(writePtr, "Total Bank Balance:                  %10.2f\n", totalBalance);

        fclose(writePtr); // fclose closes the file
    }                     // end else
} // end function textFile

// update balance in record
void updateRecord(FILE *fPtr)
{
    unsigned int account; // account number
    double transaction;   // transaction amount
    // create clientData with no information
    struct clientData client = {0, "", "", 0.0};

    // obtain number of account to update
    printf("%s", "Enter account to update ( 1 - 100 ): ");
    if (scanf("%d", &account) != 1)
    {
        clearInputBuffer();
        account = 0; // Trigger the invalid check below
    }

    if (account < 1 || account > 100)
    {
        puts("Invalid account number.");
        return;
    }

    // move file pointer to correct record in file
    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account does not exist
    if (client.acctNum == 0)
    {
        printf("Account #%d has no information.\n", account);
    }
    else
    { // update record
        printf("%-6d%-16s%-11s%10.2f\n\n", client.acctNum, client.lastName, client.firstName, client.balance);

        // request transaction amount from user
        printf("%s", "Enter charge ( + ) or payment ( - ): ");
        if (scanf("%lf", &transaction) != 1)
        {
            puts("Invalid input. Transaction cancelled.");
            clearInputBuffer();
            return;
        }

        if (transaction < 0 && (client.balance + transaction) < 0)
        {
            printf("Error: Insufficient funds. Cannot withdraw %.2f from balance %.2f\n", -transaction, client.balance);
            return;
        }

        client.balance += transaction; // update record balance

        printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);

        // move file pointer to correct record in file
        fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
        // write updated record over old record in file
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
        
        logTransaction(transaction > 0 ? "DEPOSIT" : "WITHDRAWAL", account, transaction, "Account update");
    } // end else
} // end function updateRecord

// delete an existing record
void deleteRecord(FILE *fPtr)
{
    struct clientData client;                       // stores record read from file
    struct clientData blankClient = {0, "", "", 0}; // blank client
    unsigned int accountNum;                        // account number

    // obtain number of account to delete
    printf("%s", "Enter account number to delete ( 1 - 100 ): ");
    if (scanf("%d", &accountNum) != 1)
    {
        clearInputBuffer();
        accountNum = 0; // Trigger the invalid check below
    }

    if (accountNum < 1 || accountNum > 100)
    {
        puts("Invalid account number.");
        return;
    }

    // move file pointer to correct record in file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if record does not exist
    if (client.acctNum == 0)
    {
        printf("Account %d does not exist.\n", accountNum);
    } // end if
    else
    { // delete record
        // move file pointer to correct record in file
        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        // replace existing record with blank record
        fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
        printf("Account #%d successfully deleted.\n", accountNum);
        
        logTransaction("DELETE", accountNum, 0.0, "Account closed");
    } // end else
} // end function deleteRecord

// create and insert record
void newRecord(FILE *fPtr)
{
    // create clientData with default information
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum; // account number

    // obtain number of account to create
    printf("%s", "Enter new account number ( 1 - 100 ): ");
    if (scanf("%d", &accountNum) != 1)
    {
        clearInputBuffer();
        accountNum = 0; // Trigger the invalid check below
    }

    if (accountNum < 1 || accountNum > 100)
    {
        puts("Invalid account number.");
        return;
    }

    // move file pointer to correct record in file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account already exists
    if (client.acctNum != 0)
    {
        printf("Account #%d already contains information.\n", client.acctNum);
    } // end if
    else
    { // create record
        // user enters last name, first name and balance
        printf("%s", "Enter lastname, firstname, balance\n? ");
        if (scanf("%14s%9s%lf", client.lastName, client.firstName, &client.balance) != 3)
        {
            puts("Invalid input. Record creation cancelled.");
            clearInputBuffer();
            return;
        }

        client.acctNum = accountNum;
        // move file pointer to correct record in file
        fseek(fPtr, (client.acctNum - 1) * sizeof(struct clientData), SEEK_SET);
        // insert record in file
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
        printf("Account #%d successfully created.\n", accountNum);
        
        logTransaction("CREATE", accountNum, client.balance, "Account opened");
    } // end else
} // end function newRecord

// enable user to input menu choice
unsigned int enterChoice(void)
{
    unsigned int menuChoice; // variable to store user's choice
    // display available options
    printf("%s", "\nEnter your choice\n"
                 "1 - store a formatted text file of accounts called\n"
                 "    \"accounts.txt\" for printing\n"
                 "2 - update an account\n"
                 "3 - add a new account\n"
                 "4 - delete an account\n"
                 "5 - display all active accounts\n"
                 "6 - search account by last name\n"
                 "7 - read account by ID\n"
                 "8 - transfer funds\n"
                 "9 - search account by first name\n"
                 "10 - reset database\n"
                 "11 - apply interest\n"
                 "12 - generate summary report\n"
                 "13 - end program\n? ");

    if (scanf("%u", &menuChoice) != 1)
    {
        clearInputBuffer();
        menuChoice = 0; // return invalid choice
    }
    return menuChoice;
} // end function enterChoice

// helper to clear input buffer
void clearInputBuffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

// display all active accounts to the console, sorted by balance
void displayAccounts(FILE *readPtr)
{
    double totalBalance = 0.0;
    struct clientData client = {0, "", "", 0.0};
    struct clientData accounts[100];
    int count = 0;

    rewind(readPtr); // sets pointer to beginning of file

    // read records into array
    while (fread(&client, sizeof(struct clientData), 1, readPtr) == 1)
    {
        if (client.acctNum != 0)
        {
            accounts[count++] = client;
            totalBalance += client.balance;
        }
    }

    // sort array by balance descending
    qsort(accounts, count, sizeof(struct clientData), compareByBalance);

    printf("\n%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("--------------------------------------------\n");

    for (int i = 0; i < count; i++)
    {
        printf("%-6d%-16s%-11s%10.2f\n", accounts[i].acctNum, accounts[i].lastName, accounts[i].firstName, accounts[i].balance);
    }

    printf("--------------------------------------------\n");
    printf("Total Bank Balance:                  %10.2f\n\n", totalBalance);
} // end displayAccounts

// search for account by last name
void searchAccount(FILE *readPtr)
{
    struct clientData client = {0, "", "", 0.0};
    char searchName[15];
    int found = 0;

    printf("Enter last name to search: ");
    if (scanf("%14s", searchName) != 1)
    {
        puts("Invalid input.");
        clearInputBuffer();
        return;
    }

    rewind(readPtr); // sets pointer to beginning of file
    printf("\n%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

    // read records and check name
    while (fread(&client, sizeof(struct clientData), 1, readPtr) == 1)
    {
        if (client.acctNum != 0 && strcmp(client.lastName, searchName) == 0)
        {
            printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
            found = 1;
        }
    }

    if (!found)
    {
        puts("No accounts found with that last name.");
    }
} // end searchAccount

// read a specific account by ID
void readAccount(FILE *readPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum;

    printf("Enter account number to read ( 1 - 100 ): ");
    if (scanf("%d", &accountNum) != 1)
    {
        clearInputBuffer();
        accountNum = 0; // Trigger the invalid check below
    }

    if (accountNum < 1 || accountNum > 100)
    {
        puts("Invalid account number.");
        return;
    }

    fseek(readPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, readPtr);

    if (client.acctNum == 0)
    {
        printf("Account #%d has no information.\n", accountNum);
    }
    else
    {
        printf("\n%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
        printf("--------------------------------------------\n");
        printf("%-6d%-16s%-11s%10.2f\n\n", client.acctNum, client.lastName, client.firstName, client.balance);
    }
} // end readAccount

// transfer funds between two accounts
void transferFunds(FILE *fPtr)
{
    unsigned int sourceAcct, destAcct;
    double amount;
    struct clientData sourceClient = {0, "", "", 0.0};
    struct clientData destClient = {0, "", "", 0.0};

    printf("Enter source account ( 1 - 100 ): ");
    if (scanf("%d", &sourceAcct) != 1) {
        clearInputBuffer();
        sourceAcct = 0;
    }

    if (sourceAcct < 1 || sourceAcct > 100) {
        puts("Invalid source account number.");
        return;
    }

    printf("Enter destination account ( 1 - 100 ): ");
    if (scanf("%d", &destAcct) != 1) {
        clearInputBuffer();
        destAcct = 0;
    }

    if (destAcct < 1 || destAcct > 100) {
        puts("Invalid destination account number.");
        return;
    }

    if (sourceAcct == destAcct) {
        puts("Cannot transfer to the same account.");
        return;
    }

    // Read Source
    fseek(fPtr, (sourceAcct - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&sourceClient, sizeof(struct clientData), 1, fPtr);

    if (sourceClient.acctNum == 0) {
        printf("Source account #%d has no information.\n", sourceAcct);
        return;
    }

    // Read Dest
    fseek(fPtr, (destAcct - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&destClient, sizeof(struct clientData), 1, fPtr);

    if (destClient.acctNum == 0) {
        printf("Destination account #%d has no information.\n", destAcct);
        return;
    }

    printf("Enter amount to transfer: ");
    if (scanf("%lf", &amount) != 1) {
        clearInputBuffer();
        puts("Invalid input.");
        return;
    }

    if (amount <= 0) {
        puts("Transfer amount must be greater than zero.");
        return;
    }

    if (sourceClient.balance < amount) {
        printf("Error: Insufficient funds in source account. Current balance: %.2f\n", sourceClient.balance);
        return;
    }

    // Process
    sourceClient.balance -= amount;
    destClient.balance += amount;

    // Save Source
    fseek(fPtr, (sourceAcct - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&sourceClient, sizeof(struct clientData), 1, fPtr);

    // Save Dest
    fseek(fPtr, (destAcct - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&destClient, sizeof(struct clientData), 1, fPtr);

    printf("Successfully transferred %.2f from account %d to %d.\n", amount, sourceAcct, destAcct);
    
    char details[50];
    sprintf(details, "Transfer to %d", destAcct);
    logTransaction("TRANSFER_OUT", sourceAcct, amount, details);
    sprintf(details, "Transfer from %d", sourceAcct);
    logTransaction("TRANSFER_IN", destAcct, amount, details);
}

// search for account by first name
void searchFirstName(FILE *readPtr)
{
    struct clientData client = {0, "", "", 0.0};
    char searchName[15];
    int found = 0;

    printf("Enter first name to search: ");
    if (scanf("%9s", searchName) != 1)
    {
        puts("Invalid input.");
        clearInputBuffer();
        return;
    }

    rewind(readPtr);
    printf("\n%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("--------------------------------------------\n");

    while (fread(&client, sizeof(struct clientData), 1, readPtr) == 1)
    {
        if (client.acctNum != 0 && strcmp(client.firstName, searchName) == 0)
        {
            printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
            found = 1;
        }
    }

    if (!found)
    {
        puts("No accounts found with that first name.");
    }
}

// wipe all accounts
void resetDatabase(FILE *fPtr)
{
    struct clientData blankClient = {0, "", "", 0.0};
    char confirm;

    printf("WARNING: This will delete ALL accounts. Are you sure? (y/n): ");
    if (scanf(" %c", &confirm) != 1) {
        clearInputBuffer();
        return;
    }

    if (confirm == 'y' || confirm == 'Y') {
        rewind(fPtr);
        for (int i = 0; i < 100; i++) {
            fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
        }
        puts("Database has been reset.");
        logTransaction("RESET", 0, 0.0, "Database wiped");
    } else {
        puts("Database reset cancelled.");
    }
}

// Compare function for sorting by balance descending
int compareByBalance(const void *a, const void *b)
{
    struct clientData *clientA = (struct clientData *)a;
    struct clientData *clientB = (struct clientData *)b;
    if (clientA->balance < clientB->balance) return 1;
    if (clientA->balance > clientB->balance) return -1;
    return 0;
}

// Log transactions to a text file
void logTransaction(const char *action, int acctNum, double amount, const char *details)
{
    FILE *logPtr;
    if ((logPtr = fopen("transactions.log", "a")) != NULL)
    {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char timeStr[26];
        strftime(timeStr, 26, "%Y-%m-%d %H:%M:%S", tm_info);

        fprintf(logPtr, "[%s] %-12s Acct: %-4d Amount: %9.2f Details: %s\n", 
                timeStr, action, acctNum, amount, details);
        fclose(logPtr);
    }
}

// apply interest to all accounts
void applyInterest(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    double rate;

    printf("Enter interest rate percentage (e.g., 5 for 5%%): ");
    if (scanf("%lf", &rate) != 1) {
        clearInputBuffer();
        puts("Invalid input.");
        return;
    }

    if (rate <= 0) {
        puts("Interest rate must be positive.");
        return;
    }

    rewind(fPtr);
    int accountsUpdated = 0;
    while (fread(&client, sizeof(struct clientData), 1, fPtr) == 1) {
        if (client.acctNum != 0 && client.balance > 0) {
            double interest = client.balance * (rate / 100.0);
            client.balance += interest;

            // move pointer back to overwrite
            fseek(fPtr, -sizeof(struct clientData), SEEK_CUR);
            fwrite(&client, sizeof(struct clientData), 1, fPtr);
            // After fwrite, the pointer advances, so we fseek to current position to reset read/write state
            fseek(fPtr, 0, SEEK_CUR); 

            accountsUpdated++;

            char details[50];
            sprintf(details, "Interest applied: %.2f%%", rate);
            logTransaction("INTEREST", client.acctNum, interest, details);
        }
    }
    printf("Successfully applied %.2f%% interest to %d accounts.\n", rate, accountsUpdated);
}

// generate summary report of the bank
void generateReport(FILE *readPtr)
{
    struct clientData client = {0, "", "", 0.0};
    double totalBalance = 0.0;
    double highestBalance = -999999999.0;
    double lowestBalance = 999999999.0;
    int highestAcct = 0, lowestAcct = 0;
    int activeAccounts = 0;

    rewind(readPtr);
    while (fread(&client, sizeof(struct clientData), 1, readPtr) == 1) {
        if (client.acctNum != 0) {
            totalBalance += client.balance;
            activeAccounts++;
            if (client.balance > highestBalance) {
                highestBalance = client.balance;
                highestAcct = client.acctNum;
            }
            if (client.balance < lowestBalance) {
                lowestBalance = client.balance;
                lowestAcct = client.acctNum;
            }
        }
    }

    if (activeAccounts == 0) {
        puts("No active accounts to generate a report.");
        return;
    }

    printf("\n--- BANK SUMMARY REPORT ---\n");
    printf("Total Active Accounts: %d\n", activeAccounts);
    printf("Total Assets:          $%.2f\n", totalBalance);
    printf("Average Balance:       $%.2f\n", totalBalance / activeAccounts);
    printf("Highest Balance:       $%.2f (Account #%d)\n", highestBalance, highestAcct);
    printf("Lowest Balance:        $%.2f (Account #%d)\n", lowestBalance, lowestAcct);
    printf("---------------------------\n\n");
}