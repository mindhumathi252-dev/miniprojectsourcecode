/*
================================================================================
  BANK ACCOUNT MANAGEMENT SYSTEM
  A random-access file management program for bank transactions
  
  Features:
  - Create formatted text file of all accounts
  - Update existing account balances
  - Add new account records
  - Delete account records
  - All data stored in binary random-access file
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Constants for data validation
#define MAX_ACCOUNTS 100
#define MAX_LASTNAME 15
#define MAX_FIRSTNAME 10
#define FILENAME "credit.dat"
#define OUTPUT_FILENAME "accounts.txt"

/*
 * Client Data Structure
 * Stores information for a single bank account
 */
struct clientData
{
    unsigned int acctNum;           // Account number (1-100)
    char lastName[MAX_LASTNAME];    // Last name
    char firstName[MAX_FIRSTNAME];  // First name
    double balance;                 // Account balance
};

// Function prototypes
unsigned int enterChoice(void);
int textFile(FILE *readPtr);
int updateRecord(FILE *fPtr);
int newRecord(FILE *fPtr);
int deleteRecord(FILE *fPtr);
void clearInputBuffer(void);
int isValidAccountNum(unsigned int acctNum);


/*
 * Main Program Entry Point
 * Displays menu and processes user choices for account management
 */
int main(int argc __attribute__((unused)), char *argv[] __attribute__((unused)))
{
    FILE *cfPtr = NULL;  // credit.dat file pointer
    unsigned int choice; // user's menu choice

    // Attempt to open database file
    cfPtr = fopen(FILENAME, "rb+");
    if (cfPtr == NULL)
    {
        fprintf(stderr, "ERROR: Cannot open file '%s'\n", FILENAME);
        fprintf(stderr, "The program creates this file on first run.\n");
        
        // Try to create a new file
        cfPtr = fopen(FILENAME, "wb+");
        if (cfPtr == NULL)
        {
            fprintf(stderr, "ERROR: Cannot create file '%s'\n", FILENAME);
            exit(EXIT_FAILURE);
        }
        printf("Created new database file: %s\n", FILENAME);
    }

    printf("\n=====================================\n");
    printf("  BANK ACCOUNT MANAGEMENT SYSTEM\n");
    printf("=====================================\n");

    // Main program loop - continue until user exits
    while ((choice = enterChoice()) != 5)
    {
        switch (choice)
        {
        // Option 1: Export accounts to text file
        case 1:
            if (textFile(cfPtr) == 0)
            {
                printf("SUCCESS: Accounts exported to '%s'\n", OUTPUT_FILENAME);
            }
            break;

        // Option 2: Update existing account
        case 2:
            updateRecord(cfPtr);
            break;

        // Option 3: Create new account
        case 3:
            newRecord(cfPtr);
            break;

        // Option 4: Delete account
        case 4:
            deleteRecord(cfPtr);
            break;

        // Handle invalid choice
        default:
            printf("ERROR: Invalid choice. Please enter 1-5.\n");
            break;
        }
    }

    // Close file and exit gracefully
    if (fclose(cfPtr) == EOF)
    {
        fprintf(stderr, "WARNING: Problem closing file.\n");
    }
    
    printf("\nThank you for using Bank Account Management System!\n");
    exit(EXIT_SUCCESS);
}


/*
 * Export Accounts to Text File
 * Reads all accounts from binary file and writes formatted text file for printing
 * 
 * Parameter: readPtr - pointer to opened binary database file
 * Returns: 0 on success, -1 on failure
 */
int textFile(FILE *readPtr)
{
    FILE *writePtr = NULL;           // accounts.txt file pointer
    struct clientData client = {0}; // Record structure, initialized to zero
    int recordsExported = 0;        // Counter for exported records

    // Validate input parameter
    if (readPtr == NULL)
    {
        fprintf(stderr, "ERROR: Invalid file pointer\n");
        return -1;
    }

    // Open output file for writing
    writePtr = fopen(OUTPUT_FILENAME, "w");
    if (writePtr == NULL)
    {
        fprintf(stderr, "ERROR: Cannot open '%s' for writing\n", OUTPUT_FILENAME);
        return -1;
    }

    // Reset file pointer to beginning
    rewind(readPtr);

    // Write header to output file
    fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    fprintf(writePtr, "------------------------------------------------------\n");

    // Read all records from binary file
    while (fread(&client, sizeof(struct clientData), 1, readPtr) == 1)
    {
        // Only write non-empty records (acctNum != 0 means valid record)
        if (client.acctNum != 0)
        {
            fprintf(writePtr, "%-6u%-16s%-11s%10.2f\n", 
                    client.acctNum, client.lastName, client.firstName, client.balance);
            recordsExported++;
        }
    }

    // Check for read errors (not EOF)
    if (ferror(readPtr))
    {
        fprintf(stderr, "ERROR: Problem reading from database file\n");
        fclose(writePtr);
        return -1;
    }

    fprintf(writePtr, "------------------------------------------------------\n");
    fprintf(writePtr, "Total accounts: %d\n", recordsExported);

    // Close output file
    if (fclose(writePtr) == EOF)
    {
        fprintf(stderr, "WARNING: Problem closing '%s'\n", OUTPUT_FILENAME);
        return -1;
    }

    printf("Exported %d account(s) to '%s'\n", recordsExported, OUTPUT_FILENAME);
    return 0;
}


/*
 * Update Account Balance
 * Adds charge or payment to an existing account
 * 
 * Parameter: fPtr - pointer to opened binary database file
 * Returns: 0 on success, -1 on failure
 */
int updateRecord(FILE *fPtr)
{
    unsigned int accountNum;       // Account number to update
    double transaction;            // Transaction amount (positive or negative)
    struct clientData client = {0}; // Record buffer

    // Validate input parameter
    if (fPtr == NULL)
    {
        fprintf(stderr, "ERROR: Invalid file pointer\n");
        return -1;
    }

    printf("\n--- UPDATE ACCOUNT ---\n");

    // Get account number from user
    printf("Enter account number (1-100): ");
    if (scanf("%u", &accountNum) != 1)
    {
        fprintf(stderr, "ERROR: Invalid input. Please enter a number.\n");
        clearInputBuffer();
        return -1;
    }
    clearInputBuffer();

    // Validate account number
    if (!isValidAccountNum(accountNum))
    {
        fprintf(stderr, "ERROR: Account number must be between 1 and %d\n", MAX_ACCOUNTS);
        return -1;
    }

    // Seek to correct record position
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);

    // Read the record
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        fprintf(stderr, "ERROR: Cannot read account record\n");
        return -1;
    }

    // Check if account exists
    if (client.acctNum == 0)
    {
        printf("Account #%u does not exist.\n", accountNum);
        return -1;
    }

    // Display current account information
    printf("\nCurrent Account Information:\n");
    printf("  Account #   : %u\n", client.acctNum);
    printf("  Name        : %s, %s\n", client.lastName, client.firstName);
    printf("  Balance     : $%.2f\n\n", client.balance);

    // Get transaction amount
    printf("Enter transaction amount:\n");
    printf("  Positive for charge, Negative for payment: ");
    if (scanf("%lf", &transaction) != 1)
    {
        fprintf(stderr, "ERROR: Invalid transaction amount\n");
        clearInputBuffer();
        return -1;
    }
    clearInputBuffer();

    // Update balance
    client.balance += transaction;

    // Display updated information
    printf("\nUpdated Account Information:\n");
    printf("  Account #   : %u\n", client.acctNum);
    printf("  Name        : %s, %s\n", client.lastName, client.firstName);
    printf("  New Balance : $%.2f\n", client.balance);

    // Write updated record back to file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    if (fwrite(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        fprintf(stderr, "ERROR: Cannot write updated record\n");
        return -1;
    }

    printf("Account updated successfully!\n");
    return 0;
}


/*
 * Delete Account Record
 * Marks an account as deleted by zeroing it out
 * 
 * Parameter: fPtr - pointer to opened binary database file
 * Returns: 0 on success, -1 on failure
 */
int deleteRecord(FILE *fPtr)
{
    unsigned int accountNum;                         // Account number to delete
    struct clientData client = {0};                 // Current record
    struct clientData blankClient = {0};            // Blank record for deletion

    // Validate input parameter
    if (fPtr == NULL)
    {
        fprintf(stderr, "ERROR: Invalid file pointer\n");
        return -1;
    }

    printf("\n--- DELETE ACCOUNT ---\n");

    // Get account number from user
    printf("Enter account number to delete (1-100): ");
    if (scanf("%u", &accountNum) != 1)
    {
        fprintf(stderr, "ERROR: Invalid input. Please enter a number.\n");
        clearInputBuffer();
        return -1;
    }
    clearInputBuffer();

    // Validate account number
    if (!isValidAccountNum(accountNum))
    {
        fprintf(stderr, "ERROR: Account number must be between 1 and %d\n", MAX_ACCOUNTS);
        return -1;
    }

    // Seek to record position
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);

    // Read the record
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        fprintf(stderr, "ERROR: Cannot read account record\n");
        return -1;
    }

    // Check if account exists
    if (client.acctNum == 0)
    {
        printf("Account #%u does not exist.\n", accountNum);
        return -1;
    }

    // Display account to be deleted
    printf("\nDeleting the following account:\n");
    printf("  Account #   : %u\n", client.acctNum);
    printf("  Name        : %s, %s\n", client.lastName, client.firstName);
    printf("  Balance     : $%.2f\n\n", client.balance);

    // Confirm deletion
    printf("Are you sure you want to delete this account? (yes/no): ");
    char response[10];
    if (fgets(response, sizeof(response), stdin) != NULL)
    {
        if (response[0] != 'y' && response[0] != 'Y')
        {
            printf("Deletion cancelled.\n");
            return 0;
        }
    }

    // Write blank record to delete
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    if (fwrite(&blankClient, sizeof(struct clientData), 1, fPtr) != 1)
    {
        fprintf(stderr, "ERROR: Cannot write to file\n");
        return -1;
    }

    printf("Account #%u has been deleted successfully!\n", accountNum);
    return 0;
}


/*
 * Create New Account Record
 * Adds a new account to the database
 * 
 * Parameter: fPtr - pointer to opened binary database file
 * Returns: 0 on success, -1 on failure
 */
int newRecord(FILE *fPtr)
{
    unsigned int accountNum;        // New account number
    struct clientData client = {0}; // New client record

    // Validate input parameter
    if (fPtr == NULL)
    {
        fprintf(stderr, "ERROR: Invalid file pointer\n");
        return -1;
    }

    printf("\n--- CREATE NEW ACCOUNT ---\n");

    // Get account number from user
    printf("Enter new account number (1-100): ");
    if (scanf("%u", &accountNum) != 1)
    {
        fprintf(stderr, "ERROR: Invalid input. Please enter a number.\n");
        clearInputBuffer();
        return -1;
    }
    clearInputBuffer();

    // Validate account number
    if (!isValidAccountNum(accountNum))
    {
        fprintf(stderr, "ERROR: Account number must be between 1 and %d\n", MAX_ACCOUNTS);
        return -1;
    }

    // Seek to record position
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);

    // Check if account already exists
    if (fread(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        fprintf(stderr, "ERROR: Cannot read from file\n");
        return -1;
    }

    if (client.acctNum != 0)
    {
        printf("ERROR: Account #%u already exists.\n", accountNum);
        printf("  Name: %s, %s\n", client.lastName, client.firstName);
        return -1;
    }

    // Get account holder information
    printf("\nEnter account holder information:\n");

    printf("  Last Name (max %d chars): ", MAX_LASTNAME - 1);
    if (fgets(client.lastName, MAX_LASTNAME, stdin) == NULL)
    {
        fprintf(stderr, "ERROR: Cannot read last name\n");
        return -1;
    }
    // Remove newline if present
    size_t len = strlen(client.lastName);
    if (len > 0 && client.lastName[len - 1] == '\n')
    {
        client.lastName[len - 1] = '\0';
    }

    printf("  First Name (max %d chars): ", MAX_FIRSTNAME - 1);
    if (fgets(client.firstName, MAX_FIRSTNAME, stdin) == NULL)
    {
        fprintf(stderr, "ERROR: Cannot read first name\n");
        return -1;
    }
    // Remove newline if present
    len = strlen(client.firstName);
    if (len > 0 && client.firstName[len - 1] == '\n')
    {
        client.firstName[len - 1] = '\0';
    }

    // Validate names are not empty
    if (strlen(client.lastName) == 0 || strlen(client.firstName) == 0)
    {
        fprintf(stderr, "ERROR: Names cannot be empty\n");
        return -1;
    }

    printf("  Initial Balance: $");
    if (scanf("%lf", &client.balance) != 1)
    {
        fprintf(stderr, "ERROR: Invalid balance amount\n");
        clearInputBuffer();
        return -1;
    }
    clearInputBuffer();

    // Validate balance is not negative
    if (client.balance < 0)
    {
        fprintf(stderr, "ERROR: Balance cannot be negative\n");
        return -1;
    }

    // Set the account number
    client.acctNum = accountNum;

    // Write record to file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    if (fwrite(&client, sizeof(struct clientData), 1, fPtr) != 1)
    {
        fprintf(stderr, "ERROR: Cannot write to file\n");
        return -1;
    }

    // Display new account information
    printf("\n--- ACCOUNT CREATED ---\n");
    printf("  Account #   : %u\n", client.acctNum);
    printf("  Name        : %s, %s\n", client.lastName, client.firstName);
    printf("  Balance     : $%.2f\n", client.balance);

    return 0;
}


/*
 * Display Menu and Get User Choice
 * Shows available options and reads user's selection
 * 
 * Returns: Menu choice (1-5), returns INT_MAX on error
 */
unsigned int enterChoice(void)
{
    unsigned int menuChoice;

    printf("\n=====================================\n");
    printf("      ACCOUNT MANAGEMENT MENU\n");
    printf("=====================================\n");
    printf("1 - Export accounts to text file\n");
    printf("2 - Update account balance\n");
    printf("3 - Add new account\n");
    printf("4 - Delete account\n");
    printf("5 - EXIT\n");
    printf("=====================================\n");
    printf("Enter your choice (1-5): ");

    // Read user input with validation
    if (scanf("%u", &menuChoice) != 1)
    {
        fprintf(stderr, "ERROR: Invalid input. Please enter a number between 1 and 5.\n");
        clearInputBuffer();
        return INT_MAX;
    }
    clearInputBuffer();

    return menuChoice;
}

/*
 * Validate Account Number
 * Checks if account number is in valid range
 * 
 * Parameter: acctNum - account number to validate
 * Returns: 1 if valid, 0 if invalid
 */
int isValidAccountNum(unsigned int acctNum)
{
    return (acctNum >= 1 && acctNum <= MAX_ACCOUNTS);
}

/*
 * Clear Input Buffer
 * Removes leftover characters from input stream
 * Prevents issues with subsequent input operations
 */
void clearInputBuffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
        // Discard characters until newline
    }
}