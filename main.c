#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_SIZE 128
#define MAX_LOGIN_ATTEMPTS 3
#define ACCOUNT_NUMBER "10001234"
#define PIN "1234"
#define STARTING_BALANCE_PAISE 2500000LL

typedef struct
{
    const char *account_number;
    const char *status;
    long long balance_paise;
} Account;

/* Read one line without leaving excess characters for the next prompt. */
static int read_line(const char *prompt, char *buffer, size_t capacity)
{
    size_t length;
    int ch;

    fputs(prompt, stdout);
    fflush(stdout);
    if (fgets(buffer, (int)capacity, stdin) == NULL)
    {
        return 0;
    }

    length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n')
    {
        buffer[length - 1] = '\0';
        --length;
    }
    else if (!feof(stdin))
    {
        while ((ch = getchar()) != '\n' && ch != EOF)
        {
            /* Discard the remainder of an overlong line. */
        }
        puts("Input is too long. Please try again.");
        buffer[0] = '\0';
        length = 0;
    }
    if (length > 0 && buffer[length - 1] == '\r')
    {
        buffer[length - 1] = '\0';
    }

    return 1;
}

static int parse_menu_choice(const char *input, int *choice)
{
    char *end;
    long value;

    errno = 0;
    value = strtol(input, &end, 10);
    while (isspace((unsigned char)*end))
    {
        ++end;
    }
    if (input == end || *end != '\0' || errno == ERANGE || value < 1 || value > 6)
    {
        return 0;
    }

    *choice = (int)value;
    return 1;
}

/* Parse a positive decimal amount with at most two digits after the point. */
static int parse_amount_paise(const char *input, long long *amount_paise)
{
    const unsigned char *cursor = (const unsigned char *)input;
    unsigned long long whole = 0;
    unsigned int fraction = 0;
    unsigned int fraction_digits = 0;
    int whole_digits = 0;

    while (isspace(*cursor))
    {
        ++cursor;
    }

    while (isdigit(*cursor))
    {
        unsigned int digit = (unsigned int)(*cursor - '0');
        if (whole > ((unsigned long long)LLONG_MAX / 100ULL - digit) / 10ULL)
        {
            return 0;
        }
        whole = whole * 10ULL + digit;
        ++whole_digits;
        ++cursor;
    }

    if (*cursor == '.')
    {
        ++cursor;
        while (isdigit(*cursor) && fraction_digits < 2)
        {
            fraction = fraction * 10U + (unsigned int)(*cursor - '0');
            ++fraction_digits;
            ++cursor;
        }
        if (fraction_digits == 0 || isdigit(*cursor))
        {
            return 0;
        }
        if (fraction_digits == 1)
        {
            fraction *= 10U;
        }
    }

    while (isspace(*cursor))
    {
        ++cursor;
    }

    if (whole_digits == 0 || *cursor != '\0' ||
        whole > ((unsigned long long)LLONG_MAX - fraction) / 100ULL)
    {
        return 0;
    }

    *amount_paise = (long long)(whole * 100ULL + fraction);
    return *amount_paise > 0;
}

static void print_money(long long amount_paise)
{
    printf("%lld.%02lld INR", amount_paise / 100LL, amount_paise % 100LL);
}

static int read_positive_amount(const char *prompt, long long *amount_paise)
{
    char input[INPUT_SIZE];

    for (;;)
    {
        if (!read_line(prompt, input, sizeof(input)))
        {
            return 0;
        }
        if (parse_amount_paise(input, amount_paise))
        {
            return 1;
        }
        puts("Enter an amount greater than zero with up to two decimal places (for example, 125.50).");
    }
}

static int valid_receiver_account(const char *account_number)
{
    size_t index;

    if (strlen(account_number) != 8 || strcmp(account_number, ACCOUNT_NUMBER) == 0)
    {
        return 0;
    }
    for (index = 0; account_number[index] != '\0'; ++index)
    {
        if (!isdigit((unsigned char)account_number[index]))
        {
            return 0;
        }
    }
    return 1;
}

static int authenticate(void)
{
    char entered_pin[INPUT_SIZE];
    int attempt;

    puts("========================================");
    puts("          SIMPLE BANKING SYSTEM          ");
    puts("========================================");

    for (attempt = 1; attempt <= MAX_LOGIN_ATTEMPTS; ++attempt)
    {
        if (!read_line("Enter PIN: ", entered_pin, sizeof(entered_pin)))
        {
            return 0;
        }

        if (strcmp(entered_pin, PIN) == 0)
        {
            puts("\nLogin successful. Welcome to your account.\n");
            return 1;
        }

        if (attempt < MAX_LOGIN_ATTEMPTS)
        {
            printf("Incorrect PIN. %d attempt(s) remaining.\n\n",
                   MAX_LOGIN_ATTEMPTS - attempt);
        }
    }

    puts("\nAccess denied after three unsuccessful attempts.");
    return 0;
}

static void show_menu(void)
{
    puts("-------------- Banking Menu --------------");
    puts("1. Check balance");
    puts("2. Deposit money");
    puts("3. Withdraw money");
    puts("4. Transfer money");
    puts("5. Account information");
    puts("6. Exit");
    puts("-------------------------------------------");
}

static void check_balance(const Account *account)
{
    printf("Current balance: ");
    print_money(account->balance_paise);
    putchar('\n');
}

static int deposit_money(Account *account)
{
    long long amount_paise;

    if (!read_positive_amount("Deposit amount (INR): ", &amount_paise))
    {
        return 0;
    }
    if (account->balance_paise > LLONG_MAX - amount_paise)
    {
        puts("Deposit could not be completed because the balance limit would be exceeded.");
        return 1;
    }

    account->balance_paise += amount_paise;
    printf("Deposit successful. New balance: ");
    print_money(account->balance_paise);
    putchar('\n');
    return 1;
}

static int withdraw_money(Account *account)
{
    long long amount_paise;

    if (!read_positive_amount("Withdrawal amount (INR): ", &amount_paise))
    {
        return 0;
    }
    if (amount_paise > account->balance_paise)
    {
        puts("Withdrawal declined: insufficient funds.");
        return 1;
    }

    account->balance_paise -= amount_paise;
    printf("Withdrawal successful. New balance: ");
    print_money(account->balance_paise);
    putchar('\n');
    return 1;
}

static int transfer_money(Account *account)
{
    char receiver_account[INPUT_SIZE];
    long long amount_paise;

    for (;;)
    {
        if (!read_line("Receiver account number (8 digits): ", receiver_account,
                       sizeof(receiver_account)))
        {
            return 0;
        }
        if (valid_receiver_account(receiver_account))
        {
            break;
        }
        puts("Enter a different 8-digit account number.");
    }

    if (!read_positive_amount("Transfer amount (INR): ", &amount_paise))
    {
        return 0;
    }
    if (amount_paise > account->balance_paise)
    {
        puts("Transfer declined: insufficient funds.");
        return 1;
    }

    account->balance_paise -= amount_paise;
    printf("Transfer recorded for account %s. Amount: ", receiver_account);
    print_money(amount_paise);
    printf("\nNew balance: ");
    print_money(account->balance_paise);
    puts("\nNote: this demo does not maintain or credit recipient accounts.");
    return 1;
}

static void show_account_information(const Account *account)
{
    puts("-------------- Account Information --------------");
    printf("Account number : %s\n", account->account_number);
    printf("Account status : %s\n", account->status);
    printf("Current balance: ");
    print_money(account->balance_paise);
    puts("\n--------------------------------------------------");
}

int main(void)
{
    Account account = {ACCOUNT_NUMBER, "Active", STARTING_BALANCE_PAISE};
    char input[INPUT_SIZE];
    int choice;
    int keep_running = 1;

    if (!authenticate())
    {
        return 0;
    }

    while (keep_running)
    {
        show_menu();
        if (!read_line("Choose an option (1-6): ", input, sizeof(input)))
        {
            puts("\nSession ended.");
            break;
        }
        if (!parse_menu_choice(input, &choice))
        {
            puts("Please enter a whole number from 1 to 6.\n");
            continue;
        }

        switch (choice)
        {
        case 1:
            check_balance(&account);
            break;
        case 2:
            keep_running = deposit_money(&account);
            break;
        case 3:
            keep_running = withdraw_money(&account);
            break;
        case 4:
            keep_running = transfer_money(&account);
            break;
        case 5:
            show_account_information(&account);
            break;
        case 6:
            puts("Thank you for using the Simple Banking System.");
            keep_running = 0;
            break;
        default:
            /* parse_menu_choice restricts choices to 1 through 6. */
            break;
        }
        if (keep_running)
        {
            putchar('\n');
        }
    }

    return 0;
}
